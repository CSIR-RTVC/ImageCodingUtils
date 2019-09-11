/************************************************************/
/* TITLE       :DWT STILL IMAGE COMPRESSION CODEC FILE FOR  */
/*              C IMPLEMENTATIONS WITH 16 BIT CPU           */
/*              ARCHITECTURES.                              */
/* VERSION     :2.0                                         */
/* FILE        :WaveletCompress.c                           */
/* DESCRIPTION :Functions for implementing a DWT-based      */
/*              still image compression and decompression   */
/*              codec. The input and output image data is   */
/*              in 24bit BGR colour format. The input image */
/*              is converted to YUV422 and coded into a bit */
/*              stream (compression) or the bit stream is   */
/*              decoded into a YUV422 and converted back    */
/*              into 24bit BGR.                             */
/* DATE        :January 2000                                */
/* AUTHOR      :K.L.Ferguson                                */
/************************************************************/
#include <stdlib.h>
#include "WaveletCompress.h"

#define CODEC_MAX_LEVELS 6
#define NUM_OF_SUBBANDS 4

/* Fixed point aritmetic adventure. */
#define FxPt 13
#define FxPt_int (int)(1 << FxPt)
#define FIXPOINT(x) ((x) << FxPt)
#define FIXPOINTADJUST(x) ((x) >> FxPt)
typedef short int fixedpoint;

/* Quantization tables operate with a different fixed point.*/
#define QFxPt 8
#define QFxPt_int (int)(1 << QFxPt)
#define Q_ONE (int)(1 << QFxPt)

/************************************************************/
/* Define the global codec parameters.                      */
/************************************************************/

/*Required operating parameters.*/
static int Q;
/*Description of the source image.*/
static int RGB_X;
static int RGB_Y;
/*Compressed data limit.*/
static unsigned long int MAX_BITSTREAM_BYTE_SIZE;
/*Status flags and reporting.*/
static int CodecIsOpen;
static char *ErrorStr;

/************************************************************/
/* VLC structures and tables.                               */
/************************************************************/

#define QFXROUND (int)(1 << (QFxPt-1))

#define MAX_RUN_BITS 9
#define MAX_RUN ((1<<MAX_RUN_BITS)-1)

/* Define the bit allocation table data format.*/
typedef struct
{
  int Run;
  int Value;
  int NumBits;
  unsigned int Code;
}CodeWord;

static int ESCAPEBits = 6;
static unsigned int ESCAPECode = 0x00000020;

static int EOBBits = 2;
static unsigned int EOBCode = 0x00000001;

#define TableSize  65
static CodeWord VLCTable[TableSize] =
{
  { 0,  0, 2, 0x0001}, /*EOB sequence.*/
  { 0,  1, 2, 0x0003},
  { 1,  1, 3, 0x0006},
  { 0,  2, 4, 0x0002},
  { 2,  1, 4, 0x000A},
  { 0,  3, 5, 0x0014},
  { 3,  1, 5, 0x001C},
  { 4,  1, 5, 0x000C},
  { 0,  0, 6, 0x0020}, /*Escape sequence.*/
  { 1,  2, 6, 0x0018},
  { 5,  1, 6, 0x0038},
  { 6,  1, 6, 0x0028},
  { 7,  1, 6, 0x0008},
  { 0,  4, 7, 0x0030},
  { 2,  2, 7, 0x0010},
  { 8,  1, 7, 0x0070},
  { 9,  1, 7, 0x0050},
  { 0,  5, 8, 0x0064},
  { 0,  6, 8, 0x0084},
  { 1,  3, 8, 0x00A4},
  { 3,  2, 8, 0x0024},
  {10,  1, 8, 0x00E4},
  {11,  1, 8, 0x00C4},
  {12,  1, 8, 0x0044},
  {13,  1, 8, 0x0004},
  { 0,  7,10, 0x0140},
  { 1,  4,10, 0x00C0},
  { 2,  3,10, 0x0340},
  { 4,  2,10, 0x03C0},
  { 5,  2,10, 0x0240},
  {14,  1,10, 0x01C0},
  {15,  1,10, 0x02C0},
  {16,  1,10, 0x0040},
  { 0,  8,12, 0x0B80},
  { 0,  9,12, 0x0180},
  { 0, 10,12, 0x0C80},
  { 0, 11,12, 0x0080},
  { 1,  5,12, 0x0D80},
  { 2,  4,12, 0x0280},
  { 3,  3,12, 0x0380},
  { 4,  3,12, 0x0480},
  { 6,  2,12, 0x0780},
  { 7,  2,12, 0x0A80},
  { 8,  2,12, 0x0880},
  {17,  1,12, 0x0F80},
  {18,  1,12, 0x0580},
  {19,  1,12, 0x0980},
  {20,  1,12, 0x0E80},
  {21,  1,12, 0x0680},
  { 0, 12,13, 0x0B00},
  { 0, 13,13, 0x1300},
  { 0, 14,13, 0x0300},
  { 0, 15,13, 0x1D00},
  { 1,  6,13, 0x0D00},
  { 1,  7,13, 0x1500},
  { 2,  5,13, 0x0500},
  { 3,  4,13, 0x1900},
  { 5,  3,13, 0x0900},
  { 9,  2,13, 0x1100},
  {10,  2,13, 0x0100},
  {22,  1,13, 0x1F00},
  {23,  1,13, 0x0F00},
  {24,  1,13, 0x1700},
  {25,  1,13, 0x0700},
  {26,  1,13, 0x1B00}
};

/************************************************************/
/* Define the local codec variables.                        */
/************************************************************/

/* Internal protected attributes. */
static fixedpoint *L_b; /*Line buffer.*/

static fixedpoint *Y; /*Lum to compress from and decompress to.*/
static int Y_X; /*Lum pix width.*/
static int Y_Y; /*Lum pix height.*/

static fixedpoint *U;
static fixedpoint *V;
static int UV_X;
static int UV_Y;

#define LxLy 0
#define LxHy 1
#define HxLy 2
#define HxHy 3
static int Y_l; /*Lum DWT levels.*/
static int UV_l;

/************************************************************/
/* Quantisation definitions and tables.                     */
/************************************************************/

static long int YPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
  (long)(1069),(long)( 424),(long)( 205),(long)( 120),(long)(  86),(long)(  74),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(3755),(long)(1116),(long)( 403),(long)( 177),(long)(  94),(long)(  61)
};

static long int UPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
  (long)(1069),(long)( 424),(long)( 205),(long)( 120),(long)(  86),(long)(  74),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(3755),(long)(1116),(long)( 403),(long)( 177),(long)(  94),(long)(  61)
};

static long int VPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
  (long)(1069),(long)( 424),(long)( 205),(long)( 120),(long)(  86),(long)(  74),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(1635),(long)( 580),(long)( 250),(long)( 131),(long)(  83),(long)(  64),
  (long)(3755),(long)(1116),(long)( 403),(long)( 177),(long)(  94),(long)(  61)
};

/*Quantisation tables.*/
static long int Y_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
static long int U_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
static long int V_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
static long int IY_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
static long int IU_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
static long int IV_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];

/************************************************************/
/* Fast lifting filter 9 - 7.                               */
/************************************************************/

#define P7_0 (fixedpoint)(   25)
#define P7_1 (fixedpoint)( -401)
#define P7_2 (fixedpoint)( 2425)
#define P7_3 (fixedpoint)( 2425)
#define P7_4 (fixedpoint)( -401)
#define P7_5 (fixedpoint)(   25)

#define U9_0 (fixedpoint)(  -93)
#define U9_1 (fixedpoint)(  605)
#define U9_2 (fixedpoint)(-4859)
#define U9_3 (fixedpoint)(-4859)
#define U9_4 (fixedpoint)(  605)
#define U9_5 (fixedpoint)(  -93)

#define MINUS_HALF (fixedpoint)(-4096)
#define HALF (fixedpoint)(4096)

/************************************************************/
/* Colour Space Conversions.                                */
/************************************************************/
#define Y_MAX    (fixedpoint)( 8191)
#define Y_MIN    (fixedpoint)(0)
#define UV_MAX   (fixedpoint)( 4096)
#define UV_MIN   (fixedpoint)(-4096)

#define R_Y_0   (fixedpoint)( 2449) 
#define R_Y_1   (fixedpoint)( 4809) 
#define R_Y_2   (fixedpoint)(  934) 
#define R_Y_3   (fixedpoint)( 3572) 
#define R_Y_4   (fixedpoint)(-1204) 
#define R_Y_5   (fixedpoint)(-2367) 
#define R_Y_6   (fixedpoint)( 5038) 
#define R_Y_7   (fixedpoint)(-4219) 
#define R_Y_8   (fixedpoint)( -819) 

#define Y_R_0   (fixedpoint)( 9339) 
#define Y_R_1   (fixedpoint)(-3228) 
#define Y_R_2   (fixedpoint)(-4756) 
#define Y_R_3   (fixedpoint)(16646) 
#define FXROUND (fixedpoint)( 4096) 

/************************************************************/
/* Public codec function definitions.                       */
/************************************************************/
void InitializeWaveletCompression(void);
int OpenWaveletCompression(int Quality,int RGB_Width,int RGB_Height,
         unsigned long int BitStreamByteLimit);
void CloseWaveletCompression(void);

int WaveletCompressionCode(void *pRGB,void *BitStream,unsigned long int *BitStreamByteSize);
int WaveletCompressionDecode(void *BitStream,unsigned long int BitStreamByteSize,void *pRGB);

int WaveletCompressionCodecOpen(void) { return(CodecIsOpen); }
char *GetWaveletCompressionCodecError(void) { return(ErrorStr); }

/************************************************************/
/* Protected codec function definitions.                    */
/************************************************************/

int RGB24toYUV422(void *pRGB);
int YUV422toRGB24(void *pRGB);

int GetMaxLevels(int x,int y);
void Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
void IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
int RunLengthCode(void *BitStream,unsigned long int *BitStreamByteSize);
int RunLengthDecode(void *BitStream,unsigned long int BitStreamByteSize);
int FindBits(int Run,int Value,int *NumCodeBits,int *CdeWord);
unsigned short int *PutBits(unsigned short int *CurrPtr,int *NextBitPos,int BitsToAdd,
                      int CodeBits);
unsigned short int *GetBits(unsigned short int *CurrPtr,int *NextBitPos,int BitsToGet,
                      int *CodeBits);
unsigned short int *GetNextBit(unsigned short int *CurrPtr,int *NextBitPos,int *CodeBit);
unsigned short int *GetRunLevel(unsigned short int *CurrPtr,int *NextBitPos,
                            int *Run, int *Level,int *BitsExtracted);

/************************************************************/
/* Codec function implementations.                          */
/************************************************************/

/* This function is only ever called once in a program.*/
void InitializeWaveletCompression(void)
{
  ErrorStr = "No Erorr";
  CodecIsOpen = 0;
  L_b = NULL;
  Q = 1;
  RGB_X = 0;
  RGB_Y = 0;
  MAX_BITSTREAM_BYTE_SIZE = 0;
  Y = NULL; 
  Y_X = 0;
  Y_Y = 0;
  U = NULL;
  V = NULL;
  UV_X = 0;
  UV_Y = 0;
}/*end InitializeWaveletCompression.*/

int OpenWaveletCompression(int Quality,int RGB_Width,int RGB_Height,
                           unsigned long int BitStreamByteLimit)
{
  int i,j;
  long int q;

  /*If already open then close first before continuing.*/
  if(CodecIsOpen)
    CloseWaveletCompression();

  /*Update to the new parameters.*/
  Q = Quality;
  RGB_X = RGB_Width;
  RGB_Y = RGB_Height;
  MAX_BITSTREAM_BYTE_SIZE = BitStreamByteLimit;

  /*Create a YUV422 memory space.*/
	Y = (fixedpoint *)malloc(RGB_X * RGB_Y * sizeof(fixedpoint));
	if(Y == NULL)
  {
    ErrorStr = "Luminance memory unavailable!";
    CloseWaveletCompression();
	  return(0);
  }/*end if Y...*/
  Y_X = RGB_X;
  Y_Y = RGB_Y;
	U = (fixedpoint *)malloc((RGB_X/2) * RGB_Y * sizeof(fixedpoint));
	if(U == NULL)
  {
    ErrorStr = "U Chrominance memory unavailable!";
    CloseWaveletCompression();
	  return(0);
  }/*end if U...*/
  UV_X = RGB_X/2;
  UV_Y = RGB_Y;
	V = (fixedpoint *)malloc((RGB_X/2) * RGB_Y * sizeof(fixedpoint));
	if(V == NULL)
  {
    ErrorStr = "V Chrominance memory unavailable!";
    CloseWaveletCompression();
	  return(0);
  }/*end if V...*/

  /*Establish the DWT levels.*/
  Y_l = GetMaxLevels(Y_X,Y_Y);
  UV_l = GetMaxLevels(UV_X,UV_Y);

  /*Construct the quantization tables.*/
  for(i = 0; i < NUM_OF_SUBBANDS; i++)
    for(j = 0; j < CODEC_MAX_LEVELS; j++)
    {
      if(Q > 0)
      {
        q = ((Q << QFxPt) * YPsyQTable[i][j]) >> QFxPt;
        q = Q_ONE + (q >> 2);
        if(q < Q_ONE)
          q = Q_ONE;
        IY_Q[i][j] = q;
        Y_Q[i][j] = (Q_ONE << QFxPt)/q;

        q = ((Q << QFxPt) * UPsyQTable[i][j]) >> QFxPt;
        q = Q_ONE + (q >> 2);
        if(q < Q_ONE)
          q = Q_ONE;
        IU_Q[i][j] = q;
        U_Q[i][j] = (Q_ONE << QFxPt)/q;

        q = ((Q << QFxPt) * VPsyQTable[i][j]) >> QFxPt;
        q = Q_ONE + (q >> 2);
        if(q < Q_ONE)
          q = Q_ONE;
        IV_Q[i][j] = q;
        V_Q[i][j] = (Q_ONE << QFxPt)/q;

      }/*end if Q...*/
      else
      {
        IY_Q[i][j] = Q_ONE;
        Y_Q[i][j] = Q_ONE;
        IU_Q[i][j] = Q_ONE;
        U_Q[i][j] = Q_ONE;
        IV_Q[i][j] = Q_ONE;
        V_Q[i][j] = Q_ONE;
      }/*end else...*/
    }/*end for i & j...*/

	/*Allocate working memory as a 2 line image line buffer.*/
	L_b = (fixedpoint *)malloc(2 * Y_X * sizeof(fixedpoint));
	if(L_b == NULL)
  {
    ErrorStr = "Working memory unavailable!";
    CloseWaveletCompression();
	  return(0);
  }/*end if L_b...*/

  ErrorStr = "No Erorr";
  CodecIsOpen = 1;
  return(1);
}/*end OpenWaveletCompression.*/

void CloseWaveletCompression(void)
{
  /*Free the working memory.*/
	if(L_b != NULL)
		free(L_b);
	L_b = NULL;

	if(Y != NULL)
    free(Y);
	Y = NULL;

	if(U != NULL)
    free(U);
	U = NULL;

	if(V != NULL)
    free(V);
	V = NULL;

  CodecIsOpen = 0;
}/*end CloseWaveletCompression.*/

int WaveletCompressionCode(void *pRGB,
                           void *BitStream,unsigned long int *BitStreamByteSize)
{
  if(!RGB24toYUV422(pRGB))
    return(0);

  Dwt2D(Y,U,V);

  if(!RunLengthCode(BitStream,BitStreamByteSize))
    return(0);

  return(1);
}/*end WaveletCompressionCode.*/

int WaveletCompressionDecode(void *BitStream,unsigned long int BitStreamByteSize,
                             void *pRGB)
{
  if(!RunLengthDecode(BitStream,BitStreamByteSize))
    return(0);

  IDwt2D(Y,U,V);

  if(!YUV422toRGB24(pRGB))
    return(0);

  return(1);
}/*end WaveletCompressionDecode.*/

int RGB24toYUV422(void *pRGB)
{
	unsigned char *S;
	register int xb,yb,XB,YB;
  int xpix;
	fixedpoint y,u,v;
  fixedpoint accu,accv;
	fixedpoint r,g,b;
  unsigned char BppMask = 0xFF;

  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* Y1 & Y2 have range 0..0.999,*/
	/* U & V have range -0.5..0.5.*/
	xpix = RGB_X;
	S = (unsigned char *)(pRGB);
	/*2 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		/*Left pix.  255->0.999.*/
		b = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3) & BppMask )) >> 8;
		g = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3 + 1) & BppMask )) >> 8;
		r = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3 + 2) & BppMask )) >> 8;
		y = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));
    if(y < Y_MIN) 
      y = Y_MIN;
    if(y > Y_MAX)
     y = Y_MAX;
		Y[yb*xpix + (xb<<1)] = y;

		accu = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
		accv = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));

		/*Right pix.*/
		b = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3 + 3) & BppMask )) >> 8;
		g = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3 + 4) & BppMask )) >> 8;
		r = FIXPOINT((long int)( *(S + (yb*xpix + xb*2)*3 + 5) & BppMask )) >> 8;
		y = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));
    if(y < Y_MIN) 
      y = Y_MIN;
    if(y > Y_MAX)
     y = Y_MAX;
		Y[yb*xpix + (xb<<1) + 1] = y;
		u = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
    u = (accu + u);
    if(u == -1)
      u = 0;
    else
      u = (u >> 1);
    if(u < UV_MIN)
      u = UV_MIN;
    if(u > UV_MAX)
      u = UV_MAX;
		U[yb*(xpix>>1) + xb] = u;
		v = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));
    v = (accv + v);
    if(v == -1)
      v = 0;
    else
      v = (v >> 1);
    if(v < UV_MIN)
      v = UV_MIN;
    if(v > UV_MAX)
      v = UV_MAX;
		V[yb*(xpix>>1) + xb] = v;
 	}/*end for xb & yb...*/

  return(1);
}/*end RGB24toYUV422.*/

int YUV422toRGB24(void *pRGB)
{
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
  int r,b,g;
	fixedpoint y,u,v;

  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* R, G & B have range 0..255,*/
	xpix = RGB_X;
	D = (unsigned char *)(pRGB);
	/*2 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = U[yb*(xpix>>1) + xb];
		v = V[yb*(xpix>>1) + xb];

		/*Left pix.*/
		y = Y[yb*xpix + (xb<<1)];
		r = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( r > 255 )
		  r = 255;
		else if( r < 0 )
		  r = 0;
		if( g > 255 )
		  g = 255;
		else if( g < 0 )
		  g = 0;
		if( b > 255 )
		  b = 255;
		else if( b < 0 )
		  b = 0;
		*(D + (yb*xpix + (xb<<1))*3) = (unsigned char)b;
		*(D + (yb*xpix + (xb<<1))*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + (xb<<1))*3 + 2) = (unsigned char)r;

		/*Right pix.*/
		y = Y[yb*xpix + (xb<<1) + 1];
		r = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( r > 255 )
		  r = 255;
		else if( r < 0 )
		  r = 0;
		if( g > 255 )
		  g = 255;
		else if( g < 0 )
		  g = 0;
		if( b > 255 )
		  b = 255;
		else if( b < 0 )
		  b = 0;
		*(D + (yb*xpix + (xb<<1))*3 + 3) = (unsigned char)b;
		*(D + (yb*xpix + (xb<<1))*3 + 4) = (unsigned char)g;
		*(D + (yb*xpix + (xb<<1))*3 + 5) = (unsigned char)r;
	}/*end for xb & yb...*/

  return(1);
}/*end YUV422toRGB24.*/

int GetMaxLevels(int x,int y)
{
  int level = 0;
  while( ((y % 2)==0)&&((x % 2)==0)&&(y > 16)&&(x > 16) )
  {
    x = x/2;
    y = y/2;
    level++;
  }/*end while...*/
	return(level);
}/*end GetMaxLevels.*/

void Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
{
  int col,lev,X,Y,levels;
  fixedpoint *Src,*High;
  int i,j;
  int SubLen, limit;
  register long r,s;
  register fixedpoint a,b,c,d,e,f,g,h,k;
  register int m,n;
  register fixedpoint *x,*p;
  int LevY;
  int LevX;

  High = (fixedpoint *)L_b;
  for(col = 0; col < 3; col++)
  {
    if(col==0) /*Lum.*/
    {
      levels = Y_l;
      X = Y_X;
      Y = Y_Y;
      Src = locY;
    }/*end if col...*/
    else /*Chr.*/
    {
      levels = UV_l;
      X = UV_X;
      Y = UV_Y;
      if(col==1) /*U Chr.*/
        Src = locU;
      else /*V Chr.*/
        Src = locV;
    }/*end else...*/

    for(lev = 0; lev < levels; lev++)
    {
      LevY = Y >> lev;
      LevX = X >> lev;
  
      /*In x direction.*/
      SubLen = LevX >> 1;
      for(i = 0; i < LevY; i++)
      {
        /*Do the 1-D DWT along the rows.*/
        x = (fixedpoint *)(Src + i*X);
        p = High;
        /*//////////////// Interpolation ///////////////////////////*/
        a = x[0]; b = x[1]; c = x[2]; g = x[3]; d = x[4]; e = x[6]; f = x[8];
        /*For position x[1];*/
        r = (long)((a + c)*P7_2) + (long)(b*MINUS_HALF) +
            (long)((c + d)*P7_1) + (long)((d + e)*P7_0);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        /*For position x[3];*/
        s = (long)((c + d)*P7_2) + (long)(g*MINUS_HALF) +
            (long)((c + f)*P7_0) + (long)((a + e)*P7_1); 
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 4);
        for(n = 2; n < limit; n+=2) /*Two at a time for pipeline efficiency.*/
        {
          m = n<<1;
          a = x[m-4]; b = x[m-2]; c = x[m]; k = x[m+1]; d = x[m+2];
          h = x[m+3]; e = x[m+4]; f = x[m+6]; g = x[m+8];
  
          r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + 
              (long)((a + f)*P7_0) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        
          s = (long)(h*MINUS_HALF) + (long)((d + e)*P7_2) +
              (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        a = x[LevX-10]; b = x[LevX-8]; k = x[LevX-7]; c = x[LevX-6]; d = x[LevX-5];
        e = x[LevX-4]; f = x[LevX-3]; g = x[LevX-2]; h = x[LevX-1];
        if(n == (SubLen-4))
        {
          /*For position x[LevX-7];*/
          s = (long)((x[LevX-12] + g)*P7_0) + (long)((a + e)*P7_1) + 
              (long)((b + c)*P7_2) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }/*end if n...*/
        /*For position x[LevX-5];*/
        r = (long)((a + g)*P7_0) + (long)((b + g)*P7_1) + 
            (long)((c + e)*P7_2) + (long)(d*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        /*For position x[LevX-3];*/
        s = (long)((b + e)*P7_0) + (long)((c + g)*P7_1) + 
            (long)((e + g)*P7_2) + (long)(f*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        /*For position x[LevX-1];*/
        r = (long)((c + c)*P7_0) + (long)((e + e)*P7_1) + 
            (long)((g + g)*P7_2) + (long)(h*MINUS_HALF);
        *p = (fixedpoint)(FIXPOINTADJUST(r));
        /*//////////////// Update ////////////////////////////////////*/
        p = x;
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        /*For Low[0].*/
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + 
            (long)((a + b)*U9_2);
        *p++ = x[0] + (fixedpoint)(FIXPOINTADJUST(r));
        /*For Low[1].*/
        *p++ = x[2] + (fixedpoint)(FIXPOINTADJUST(r));
        /*For Low[2].*/
        s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + 
            (long)((b + c)*U9_2);
        *p++ = x[4] + (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 3);
        for(n = 3; n < limit; n+=2)
        {
          m = n << 1;
          a = High[n-3]; b = High[n-2]; c = High[n-1];
          d = High[n]; e = High[n+1]; f = High[n+2];
          r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          g = High[n+4];
          *p++ = x[m] + (fixedpoint)(FIXPOINTADJUST(r));
          s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
          *p++ = x[m+2] + (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        /*For Low[SubLen-3].*/
        if(n == (SubLen-3))
        {
          s = (long)((a + e)*U9_0) + (long)((b + e)*U9_1) + (long)((c + d)*U9_2);
          *p++ = x[LevX-6] + (fixedpoint)(FIXPOINTADJUST(s));
        }/*end if n...*/
        /*For Low[SubLen-2].*/
        r = (long)((b + f)*U9_0) + (long)((c + f)*U9_1) + (long)((d + e)*U9_2);
        *p++ = x[LevX-4] + (fixedpoint)(FIXPOINTADJUST(r));
        /*For Low[SubLen-1].*/
        s = (long)((c + e)*U9_0) + (long)((d + f)*U9_1) + (long)((e + f)*U9_2);
        *p++ = x[LevX-2] + (fixedpoint)(FIXPOINTADJUST(s));
  
        /*Copy remaining wavelet coefficients to in-place.*/
        x = High;
        for(j = 0; j < SubLen; j++)
          *p++ = *x++;
      }/*end for i...*/
    
      /*In y direction.*/
      SubLen = LevY >> 1;
      for(j = 0; j < LevX; j++)
      {
        /* 1-D DWT down the columns.*/
        x = (fixedpoint *)(Src + j);
        p = High;
        /*//////////////// Interpolation ///////////////////////////*/
        a = x[0]; b = x[X]; m = X<<1; c = x[m]; m += X; g = x[m];
        m += X; d = x[m]; m += X; e = x[m]; m += X; f = x[m];
        /*For position x[1];*/
        r = (long)((a + c)*P7_2) + (long)(b*MINUS_HALF) +
            (long)((c + d)*P7_1) + (long)((d + e)*P7_0);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        /*For position x[3];*/
        s = (long)((c + d)*P7_2) + (long)(g*MINUS_HALF) +
            (long)((c + f)*P7_0) + (long)((a + e)*P7_1); 
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 4);
        for(n = 2; n < limit; n+=2) /*Two at a time for pipeline efficiency.*/
        {
          m = ((n<<1)-4)*X; a = x[m]; m += (X<<1); b = x[m]; 
          m += (X<<1); c = x[m]; m += X; k = x[m]; 
          m += X; d = x[m]; m += X; h = x[m];
          m += X; e = x[m]; m += (X<<1); f = x[m]; 
          m += (X<<1); g = x[m];
  
          r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + 
              (long)((a + f)*P7_0) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        
          s = (long)(h*MINUS_HALF) + (long)((d + e)*P7_2) +
              (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        m = (LevY-10)*X; a = x[m]; 
        m += (X<<1); b = x[m]; m += X; k = x[m]; 
        m += X; c = x[m]; m += X; d = x[m]; m += X; e = x[m]; 
        m += X; f = x[m]; m += X; g = x[m]; m += X; h = x[m];
        if(n == (SubLen-4))
        {
          /*For position x[LevY-7];*/
          s = (long)((x[(LevY-12)*X] + g)*P7_0) + (long)((a + e)*P7_1) + 
              (long)((b + c)*P7_2) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }/*end if n...*/
        /*For position x[LevY-5];*/
        r = (long)((a + g)*P7_0) + (long)((b + g)*P7_1) + 
            (long)((c + e)*P7_2) + (long)(d*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        /*For position x[LevY-3];*/
        s = (long)((b + e)*P7_0) + (long)((c + g)*P7_1) + 
            (long)((e + g)*P7_2) + (long)(f*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        /*For position x[LevY-1];*/
        r = (long)((c + c)*P7_0) + (long)((e + e)*P7_1) + 
            (long)((g + g)*P7_2) + (long)(h*MINUS_HALF);
        *p = (fixedpoint)(FIXPOINTADJUST(r));
        /*//////////////// Update ////////////////////////////////////*/
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        /*For Low[0].*/
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + 
            (long)((a + b)*U9_2);
        x[0] = x[0] + (fixedpoint)(FIXPOINTADJUST(r));
        /*For Low[1].*/
        m = X<<1;
        x[X] = x[m] + (fixedpoint)(FIXPOINTADJUST(r));
        /*For Low[2].*/
        s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + 
            (long)((b + c)*U9_2);
        x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 3);
        for(n = 3; n < limit; n+=2)
        {
          m = n*X;
          a = High[n-3]; b = High[n-2]; c = High[n-1];
          d = High[n]; e = High[n+1]; f = High[n+2];
          r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          g = High[n+4];
          x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(r));
          m += X;
          s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
          x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        m = n*X;
        /*For Low[SubLen-3].*/
        if(n == (SubLen-3))
        {
          s = (long)((a + e)*U9_0) + (long)((b + e)*U9_1) + (long)((c + d)*U9_2);
          x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
          m += X;
        }/*end if n...*/
        /*For Low[SubLen-2].*/
        r = (long)((b + f)*U9_0) + (long)((c + f)*U9_1) + (long)((d + e)*U9_2);
        x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(r));
        m += X;
        /*For Low[SubLen-1].*/
        s = (long)((c + e)*U9_0) + (long)((d + f)*U9_1) + (long)((e + f)*U9_2);
        x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
        m += X;
  
        /*Copy remaining wavelet coefficients to in-place.*/
        p = High;
        for(i = 0; i < SubLen; i++,m += X)
          x[m] = *p++;
      }/*end for j...*/
    }/*end for lev...*/
  }/*end for col...*/
}/*end Dwt2D.*/

void IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
{
  int col,lev,X,Y,levels;
  fixedpoint *Low,*High,*Src;
  int i,j;
  int SubLen, limit;
  register long r,s;
  register fixedpoint a,b,c,d,e,f,g;
  register int m,n;
  register fixedpoint *x,*p;
  int LevY;
  int LevX;
  
  for(col = 0; col < 3; col++)
  {
    if(col==0) /*Lum.*/
    {
      levels = Y_l;
      X = Y_X;
      Y = Y_Y;
      Src = locY;
    }/*end if col...*/
    else /*Chr.*/
    {
      levels = UV_l;
      X = UV_X;
      Y = UV_Y;
      if(col==1) /*U Chr.*/
        Src = locU;
      else /*V Chr.*/
        Src = locV;
    }/*end else...*/

    for(lev = (levels - 1); lev >= 0 ; lev--)
    {
      LevY = Y >> lev;
      LevX = X >> lev;
  
      /*In y direction.*/
      SubLen = LevY >> 1;
      for(j = 0; j < LevX; j++)
      {
        /*Do 1-D Inverse transform.*/
        Low = (fixedpoint *)(Src + j);
        High = (fixedpoint *)(Low + (LevY>>1)*X);
        x = Low;
        p = (fixedpoint *)L_b;
  
        /* Do 1-D DWT down cols.*/
        /*/////////// Create even coeff with piece of wavelet //////////*/
        a = High[0]; b = High[X]; m = X<<1; c = High[m]; 
        m += X; d = High[m]; m += X; e = High[m];
        /*For Recon[0].*/
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + (long)((a + b)*U9_2);
        p[0] = Low[0] - (fixedpoint)(FIXPOINTADJUST(r));
        /*For Recon[2].*/
        p[2] = Low[X] - (fixedpoint)(FIXPOINTADJUST(r));
        /*For Recon[4].*/
        s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + (long)((b + c)*U9_2);
        p[4] = Low[X<<1] - (fixedpoint)(FIXPOINTADJUST(s));
        limit = SubLen - 3;
        for(n = 3; n < limit; n+=2)
        {
          i = (n-3)*X;
          a = High[i]; i += X; b = High[i]; i += X; c = High[i]; i += X; d = High[i];
          i += X; e = High[i]; i += X; f = High[i]; i += X; g = High[i];
          r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          m = n<<1;
          i = n*X;
          p[m] = Low[i] - (fixedpoint)(FIXPOINTADJUST(r));
          s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
          p[m+2] = Low[i+X] - (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        i = (SubLen-6)*X; a = High[i]; i += X; b = High[i]; 
        i += X; c = High[i]; i += X; d = High[i]; 
        i += X; e = High[i]; f = High[i];
        m = n*X;
        /*For Recon[LevY-6].*/
        if(n == limit)
        {
          s = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          p[LevY-6] = Low[m] - (fixedpoint)(FIXPOINTADJUST(s));
          m += X;
        }/*end if n...*/
        /*For Recon[LevY-4].*/
        r = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + f)*U9_0);
        p[LevY-4] = Low[m] - (fixedpoint)(FIXPOINTADJUST(r));
        m += X;
        /*For Recon[LevY-2].*/
        s = (long)((e + f)*U9_2) + (long)((d + f)*U9_1) + (long)((c + e)*U9_0);
        p[LevY-2] = Low[m] - (fixedpoint)(FIXPOINTADJUST(s));
      
        /*/////// Interpolate the odd coeff with w and the new even coeff.*/
        a = p[0]; b = p[2]; c = p[4]; d = p[6]; e = p[8];
        /*For Recon[1].*/
        r = (long)((a + b)*P7_2) + (long)((b + c)*P7_1) + (long)((c + d)*P7_0); 
        x[X] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[0]) << 1;
        /*For Recon[3].*/
        s = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((b + e)*P7_0); 
        x[3*X] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[X]) << 1;
        limit = LevY - 8;
        for(n = 4; n < limit; n+=4)
        {
          m = (n>>1)*X;
          a = p[n-4]; b = p[n-2]; c = p[n]; d = p[n+2];
          e = p[n+4]; f = p[n+6]; g = p[n+8];
          i = (n-4)*X; x[i] = a;
          i += (X<<1); x[i] = b;
          r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + f)*P7_0);
          i += ((X<<1)+X);
          x[i] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[m]) << 1;
          s = (long)((d + e)*P7_2) + (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
          i += (X<<1);
          x[i] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[m+X]) << 1;
        }/*end for n...*/
        f = p[LevY-12]; a = p[LevY-10]; b = p[LevY-8];
        c = p[LevY-6]; d = p[LevY-4]; e = p[LevY-2];
        /*For Recon[LevY-7].*/
        m = (LevY-12)*X;
        i = (LevY-7)*X;
        if(n == limit)
        {
          r = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((f + e)*P7_0);
          x[m] = f;
          x[i] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[(SubLen-4)*X]) << 1;
        }/*end if n...*/
        m += (X<<1);
        i += (X<<1);
        /*For Recon[LevY-5].*/
        s = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + e)*P7_0);
        x[m] = a; m += (X<<1);
        n = (SubLen-3)*X;
        x[i] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[n]) << 1;
        i += (X<<1);
        n += X;
        /*For Recon[LevY-3].*/
        r = (long)((b + d)*P7_0) + (long)((c + e)*P7_1) + (long)((d + e)*P7_2);
        x[m] = b; m += (X<<1);
        x[i] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[n]) << 1;
        i += (X<<1);
        n += X;
        /*For Recon[LevY-1].*/
        s = (long)((c + c)*P7_0) + (long)((d + d)*P7_1) + (long)((e + e)*P7_2);
        x[m] = c; m += (X<<1);
        x[i] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[n]) << 1;
  
        /*Copy remaining even coefficients.*/
        x[m] = d; m += (X<<1);
        x[m] = e;
      }//end for j...
    
      /*In x direction.*/
      SubLen = LevX >> 1;
      for(i = 0; i < LevY; i++)
      {
        /*Do 1-D Inverse transform.*/
        Low = (fixedpoint *)(Src + i*X);
        High = (fixedpoint *)(Low + (LevX>>1));
        x = Low;
        p = (fixedpoint *)L_b;
  
        /*/////////// Create even coeff with piece of wavelet //////////*/
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        /*For Recon[0].*/
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + (long)((a + b)*U9_2);
        p[0] = Low[0] - (fixedpoint)(FIXPOINTADJUST(r));
        /*For Recon[2].*/
        p[2] = Low[1] - (fixedpoint)(FIXPOINTADJUST(r));
        /*For Recon[4].*/
        s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + (long)((b + c)*U9_2);
        p[4] = Low[2] - (fixedpoint)(FIXPOINTADJUST(s));
        limit = SubLen - 3;
        for(n = 3; n < limit; n+=2)
        {
          m = n<<1;
          a = High[n-3]; b = High[n-2]; c = High[n-1]; d = High[n];
          e = High[n+1]; f = High[n+2]; g = High[n+3];
          r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          p[m] = Low[n] - (fixedpoint)(FIXPOINTADJUST(r));
          s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
          p[m+2] = Low[n+1] - (fixedpoint)(FIXPOINTADJUST(s));
        }/*end for n...*/
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        /*For Recon[LevX-6].*/
        if(n == (SubLen-3))
        {
          s = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          p[LevX-6] = Low[n++] - (fixedpoint)(FIXPOINTADJUST(s));
        }/*end if n...*/
        /*For Recon[LevX-4].*/
        r = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + f)*U9_0);
        p[LevX-4] = Low[n++] - (fixedpoint)(FIXPOINTADJUST(r));
        /*For Recon[LevX-2].*/
        s = (long)((e + f)*U9_2) + (long)((d + f)*U9_1) + (long)((c + e)*U9_0);
        p[LevX-2] = Low[n] - (fixedpoint)(FIXPOINTADJUST(s));
      
        /*/////// Interpolate the odd coeff with w and the new even coeff.*/
        a = p[0]; b = p[2]; c = p[4]; d = p[6]; e = p[8];
        /*For Recon[1].*/
        r = (long)((a + b)*P7_2) + (long)((b + c)*P7_1) + (long)((c + d)*P7_0); 
        x[1] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[0]) << 1;
        /*For Recon[3].*/
        s = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((b + e)*P7_0); 
        x[3] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[1]) << 1;
        limit = LevX - 8;
        for(n = 4; n < limit; n+=4)
        {
          m = n>>1;
          a = p[n-4]; b = p[n-2]; c = p[n]; d = p[n+2];
          e = p[n+4]; f = p[n+6]; g = p[n+8];
          x[n-4] = a;
          x[n-2] = b;
          r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + f)*P7_0);
          x[n + 1] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[m]) << 1;
          s = (long)((d + e)*P7_2) + (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
          x[n + 3] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[m+1]) << 1;
        }/*end for n...*/
        f = p[LevX-12]; a = p[LevX-10]; b = p[LevX-8];
        c = p[LevX-6]; d = p[LevX-4]; e = p[LevX-2];
        /*For Recon[LevX-7].*/
        if(n == limit)
        {
          r = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((f + e)*P7_0);
          x[LevX-12] = f;
          x[LevX-7] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[SubLen-4]) << 1;
        }/*end if n...*/
        /*For Recon[LevX-5].*/
        s = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + e)*P7_0);
        x[LevX-10] = a;
        x[LevX-5] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[SubLen-3]) << 1;
        /*For Recon[LevX-3].*/
        r = (long)((b + d)*P7_0) + (long)((c + e)*P7_1) + (long)((d + e)*P7_2);
        x[LevX-8] = b;
        x[LevX-3] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[SubLen-2]) << 1;
        /*For Recon[LevX-1].*/
        s = (long)((c + c)*P7_0) + (long)((d + d)*P7_1) + (long)((e + e)*P7_2);
        x[LevX-6] = c;
        x[LevX-1] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[SubLen-1]) << 1;
  
        /*Copy remaining even coefficients.*/
        x[LevX-4] = d;
        x[LevX-2] = e;
  
      }/*end for i...*/
    }/*end for lev...*/
  }/*end for col...*/
}/*end IDwt2D*/

int RunLengthCode(void *BitStream,unsigned long int *BitStreamByteSize)
{
  int i,j,s,lev;
  int run;
  char level;
  int DummyBitCnt, DummyBits;
  long int pix;
  unsigned short int *Bits;
  unsigned long int MaxBits;
  unsigned long int BitCnt;
  int BitPos;
  fixedpoint *P;
  fixedpoint *PU;
  fixedpoint *PV;
  int X,LevY,LevX;

  Bits = (unsigned short int *)(BitStream);
  *BitStreamByteSize = 0; /*In bytes.*/
  MaxBits = MAX_BITSTREAM_BYTE_SIZE * 8; /*8bits/byte.*/
  BitCnt = 0; /*In Bits.*/
  BitPos = 0; /*Bit position within the current 16bit word.*/

  /* Code the Lum texture 1st with 8 bpp.*/
  P = Y;
  X = Y_X;
  LevY = Y_Y >> Y_l;
  LevX = X >> Y_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      /* Quantize first.*/
      pix = (((long int)P[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * Y_Q[LxLy][Y_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      /* pix is now in 8bpp form in range 0..255.*/
      if(pix > 255)
        pix = 255;
      if(pix < 0)
        pix = 0;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }/*end if BitCnt...*/
      Bits = PutBits(Bits,&BitPos,8,(int)pix);
    }/*end if i & j...*/

  /* Code the Lum Sub-Bands.*/
  for(lev = Y_l; lev > 0 ; lev--)
  {
    LevY = Y_Y >> lev;
    LevX = X >> lev;

    for(s = 1; s < NUM_OF_SUBBANDS; s++)
    {
      if(s == LxHy)
        P = (fixedpoint *)(Y + LevY*X);
      else if(s == HxLy)
        P = (fixedpoint *)(Y + LevX);
      else /*if(s == HxHy)*/
        P = (fixedpoint *)(Y + LevY*X + LevX);

      /*Never code HxHy level 1 Sub-band.*/
      if( (lev==1)&&(s==HxHy) )
      {
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Kill.*/
            P[i*X + j] = 0;
          }/*end for i & j...*/
      }/*end if lev...*/
      else
      {
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Quantize first.*/
            pix = (((long int)P[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * Y_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            /* Runlength encode.*/
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }/*end if level...*/
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Esc sequence.*/
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }/*end if DummyBits...*/
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                /*Add sign bit.*/
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }/*end else...*/

              run = 0; /*Reset the run.*/
            }/*end else...*/
          }/*end if i & j...*/
        BitCnt += EOBBits; /*EndOfSubBand bits.*/
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }/*end if BitCnt...*/
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
      }/*end else...*/
    }//end for s...

  }//end for lev...

  /* Code the Chr texture 1st with 8 bpp.*/
  PU = U;
  PV = V;
  X = UV_X;
  LevY = UV_Y >> UV_l;
  LevX = X >> UV_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      /* Quantize first.*/
      pix = (((long int)PU[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * U_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }/*end if BitCnt...*/
      Bits = PutBits(Bits,&BitPos,8,(int)pix);

      pix = (((long int)PV[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * V_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }/*end if BitCnt...*/
      Bits = PutBits(Bits,&BitPos,8,(int)pix);
    }/*end if i & j...*/

  /* Code the Chr Sub-Bands.*/
  for(lev = UV_l; lev > 0 ; lev--)
  {
    LevY = UV_Y >> lev;
    LevX = X >> lev;

    for(s = 1; s < NUM_OF_SUBBANDS; s++)
    {
      if(s == LxHy)
      {
        PU = (fixedpoint *)(U + LevY*X);
        PV = (fixedpoint *)(V + LevY*X);
      }/*end if s...*/
      else if(s == HxLy)
      {
        PU = (fixedpoint *)(U + LevX);
        PV = (fixedpoint *)(V + LevX);
      }/*end if s...*/
      else /*if(s == HxHy)*/
      {
        PU = (fixedpoint *)(U + LevY*X + LevX);
        PV = (fixedpoint *)(V + LevY*X + LevX);
      }/*end else...*/

      /*Never code HxHy level 1 Sub-band.*/
      if( (lev==1)&&(s==HxHy) )
      {
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Kill.*/
            PU[i*X + j] = 0;
            PV[i*X + j] = 0;
          }/*end for i & j...*/
      }/*end if lev...*/
      else
      {
        /* U Chr.*/
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Quantize first.*/
            pix = (((long int)PU[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * U_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            /* Runlength encode.*/
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }/*end if level...*/
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Esc sequence.*/
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }/*end if DummyBits...*/
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                /*Add sign bit.*/
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }/*end else...*/

              run = 0; /*Reset the run.*/
            }/*end else...*/
          }/*end if i & j...*/
        BitCnt += EOBBits; /*EndOfSubBand bits.*/
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }/*end if BitCnt...*/
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
  
        /* V Chr.*/
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Quantize first.*/
            pix = (((long int)PV[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * V_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            /* Runlength encode.*/
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }/*end if level...*/
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Esc sequence.*/
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }/*end if DummyBits...*/
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                /*Add sign bit.*/
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }/*end else...*/

              run = 0; /*Reset the run.*/
            }/*end else...*/
          }/*end if i & j...*/
        BitCnt += EOBBits; /*EndOfSubBand bits.*/
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }/*end if BitCnt...*/
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
      }/*end else...*/
    }/*end for s...*/
  }/*end for lev...*/

  *BitStreamByteSize = (BitCnt>>3) + 1;
  if(*BitStreamByteSize >= MAX_BITSTREAM_BYTE_SIZE)
  {
    ErrorStr = "Coded bits too long!";
    return(0);
  }//end if BitStreamByteSize...

  return(1);
}/*end RunLengthCode.*/

int RunLengthDecode(void *BitStream,unsigned long int BitStreamByteSize)
{
  int i,j,s,lev;
  long int pix;
  int pix16;
  int run;
  char level;
  int DummyBits;
  int EndOfBlock;
  unsigned short int *Bits;
  unsigned long int BitStreamBits;
  unsigned long int BitCnt;
  int BitPos;
  fixedpoint *P;
  fixedpoint *PU;
  fixedpoint *PV;
  int LevX,LevY,X;

  Bits = (unsigned short int *)(BitStream);
  BitStreamBits = BitStreamByteSize * 8; /*8bits/byte.*/
  BitCnt = 0; /*In Bits.*/
  BitPos = 0; /*Bit position within the current 16bit word.*/

  /* Decode the Lum texture 1st with 8 bpp.*/
  P = Y;
  LevX = Y_X >> Y_l;
  LevY = Y_Y >> Y_l;
  X = Y_X;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      Bits = GetBits(Bits,&BitPos,8,&pix16);
      pix = (long int)((unsigned char)(pix16 & 0x00FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }/*end if BitCnt...*/
      /* Inverse Quantize.*/
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IY_Q[LxLy][Y_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
    }/*end if i & j...*/

  /* Decode the Lum Sub-Bands.*/
  for(lev = Y_l; lev > 0 ; lev--)
  {
    LevY = Y_Y >> lev;
    LevX = X >> lev;

    for(s = 1; s < NUM_OF_SUBBANDS; s++)
    {
      if(s == LxHy)
        P = (fixedpoint *)(Y + LevY*X);
      else if(s == HxLy)
        P = (fixedpoint *)(Y + LevX);
      else /*if(s == HxHy)*/
        P = (fixedpoint *)(Y + LevY*X + LevX);

      /*HxHy level 1 Sub-band is not coded.*/
      if( (lev==1)&&(s==HxHy) )
      {
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Fill with zeros.*/
            P[i*X + j] = 0;
          }/*end for i & j...*/
      }/*end if lev...*/
      else
      {
        run = -1;
        level = 0;
        pix16 = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) /*Ready for next run-level pair.*/
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Error has occurred.*/
                return(0);
              if((run==0)&&(pix16==0)&&(DummyBits==2)) /*EOB.*/
              {
                pix16 = 0;
                EndOfBlock = 1;
              }/*end if run...*/
            }/*end if run...*/

            if( run || EndOfBlock )
              P[i*X + j] = 0;
            else
            {
              /* Inverse Quantize first.*/
              pix = (FIXPOINT((long int)pix16)) >> (FxPt - QFxPt);
              pix = (((pix * IY_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }/*end else...*/
            run--;

          }/*end if i & j...*/
          /*Make sure the EOB marker is received.*/
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
            if((run!=0)||(pix16!=0)||(DummyBits!=2)) /*Check EOB.*/
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }/*end if run...*/
          }/*end if !EndOfBlock...*/
      }/*end else...*/

    }/*end for s...*/
  }/*end for lev...*/

  /* Decode the Chr texture 1st with 8 bpp.*/
  PU = U;
  PV = V;
  X = UV_X;
  LevY = UV_Y >> UV_l;
  LevX = X >> UV_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      Bits = GetBits(Bits,&BitPos,8,&pix16);
      pix = (long int)((char)(pix16 & 0x00FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }/*end if BitCnt...*/
      /* Inverse Quantize.*/
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IU_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);

      Bits = GetBits(Bits,&BitPos,8,&pix16);
      pix = (long int)((char)(pix16 & 0x00FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }/*end if BitCnt...*/
      /* Inverse Quantize.*/
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IV_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);

    }/*end if i & j...*/

  /* Decode the Chr Sub-Bands.*/
  for(lev = UV_l; lev > 0 ; lev--)
  {
    LevY = UV_Y >> lev;
    LevX = X >> lev;

    for(s = 1; s < NUM_OF_SUBBANDS; s++)
    {
      if(s == LxHy)
      {
        PU = (fixedpoint *)(U + LevY*X);
        PV = (fixedpoint *)(V + LevY*X);
      }/*end if s...*/
      else if(s == HxLy)
      {
        PU = (fixedpoint *)(U + LevX);
        PV = (fixedpoint *)(V + LevX);
      }/*end if s...*/
      else /*if(s == HxHy)*/
      {
        PU = (fixedpoint *)(U + LevY*X + LevX);
        PV = (fixedpoint *)(V + LevY*X + LevX);
      }/*end else...*/

      /*HxHy level 1 Sub-band is not coded.*/
      if( (lev==1)&&(s==HxHy) )
      {
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            /* Fill with zeros.*/
            PU[i*X + j] = 0;
            PV[i*X + j] = 0;
          }/*end for i & j...*/
      }/*end if lev...*/
      else
      {
        /* Chr U.*/
        run = -1;
        level = 0;
        pix16 = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) /*Ready for next run-level pair.*/
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Error has occurred.*/
                return(0);
              if((run==0)&&(pix16==0)&&(DummyBits==2)) /*EOB.*/
              {
                pix16 = 0;
                EndOfBlock = 1;
              }/*end if run...*/
            }/*end if run...*/

            if( run || EndOfBlock )
              PU[i*X + j] = 0;
            else
            {
              /* Inverse Quantize first.*/
              pix = (FIXPOINT((long int)pix16)) >> (FxPt - QFxPt);
              pix = (((pix * IU_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }/*end else...*/
            run--;

          }/*end if i & j...*/
          /*Make sure the EOB marker is received.*/
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
            if((run!=0)||(pix16!=0)||(DummyBits!=2)) /*Check EOB.*/
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }/*end if run...*/
          }/*end if !EndOfBlock...*/

        /* Chr V.*/
        run = -1;
        level = 0;
        pix16 = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) /*Ready for next run-level pair.*/
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }/*end if BitCnt...*/
              if(DummyBits == 0) /*Error has occurred.*/
                return(0);
              if((run==0)&&(pix16==0)&&(DummyBits==2)) /*EOB.*/
              {
                pix16 = 0;
                EndOfBlock = 1;
              }/*end if run...*/
            }/*end if run...*/

            if( run || EndOfBlock )
              PV[i*X + j] = 0;
            else
            {
              /* Inverse Quantize first.*/
              pix = (FIXPOINT((long int)pix16)) >> (FxPt - QFxPt);
              pix = (((pix * IV_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }/*end else...*/
            run--;

          }/*end if i & j...*/
          /*Make sure the EOB marker is received.*/
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix16,&DummyBits);
            if((run!=0)||(pix16!=0)||(DummyBits!=2)) /*Check EOB.*/
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }/*end if run...*/
          }/*end if !EndOfBlock...*/

      }/*end else...*/

    }/*end for s...*/
  }/*end for lev...*/

  return(1);
}/*end RunLengthDecode.*/

int FindBits(int Run,int Value,int *NumCodeBits,int *CdeWord)
{
  int i;
  int Found;
  int v;
  /*Parse table with Run-length and value.*/
  v = abs(Value);
  i = 0;
  Found = 0;
  while( (i < TableSize)&&(!Found) )
  {
    /*Does run-length match?*/
    if( Run == VLCTable[i].Run )
    {
      /*Does value match?*/
      if( v == VLCTable[i].Value )
      {
        *NumCodeBits = VLCTable[i].NumBits;
        *CdeWord = VLCTable[i].Code;
        Found = 1;
      }/*end if Value...*/
    }/*end if Run...*/
    i++;
  }/*end while i...*/
  //Run-length and value combination was not in table.
  if( !Found )
  {
    *NumCodeBits = ESCAPEBits + MAX_RUN_BITS + 8;
    *CdeWord = 0;
  }/*end if !Found...*/
  return(*NumCodeBits);
}/*end FindBits.*/

unsigned short int *PutBits(unsigned short int *CurrPtr,int *NextBitPos,int BitsToAdd,
                      int CodeBits)
{
  int pos;
	int b,i;

  pos = *NextBitPos;
  b = CodeBits;

  for(i = BitsToAdd; i > 0; i--)
  {
    /* Strip out the LSB bit and write it to bit position pos.*/
    if(b & 1)
      *CurrPtr = *CurrPtr | (1 << pos);
    else
      *CurrPtr = *CurrPtr & ~(1 << pos);

    /* Point to next available bit.*/
    if(pos == 15)
    {
      pos = 0;
      CurrPtr++;
    }/*end if pos...*/
    else
      pos++;

    /* Update the bits still to add.*/
    b = b >> 1;
  }/*end for i...*/

  /* Update the global next bit position.*/
  *NextBitPos = pos;

  return(CurrPtr); /* Success.*/
}/*end PutBits*/

unsigned short int *GetBits(unsigned short int *CurrPtr,int *NextBitPos,int BitsToGet,
                      int *CodeBits)
{
  int pos;
	int b,i;

  pos = *NextBitPos;
  b = 0;

  for(i = 0; i < BitsToGet; i++)
  {
    /* Strip out the next bit and update in the bit position i.*/
    if((*CurrPtr >> pos) & 1)
      b = (int)(b | (1 << i)); /*bit=1*/
    else
      b = (int)(b & ~(1 << i)); /*bit=0*/

    /* Point to next available bit.*/
    if(pos == 15)
    {
      pos = 0;
      CurrPtr++;
    }/*end if pos...*/
    else
      pos++;
  }/*end for i...*/

  /* Update the global next bit position.*/
  *NextBitPos = pos;
  /* Output the result.*/
  *CodeBits = b;

  return(CurrPtr); /* Success.*/
}/*end GetBits.*/

unsigned short int *GetNextBit(unsigned short int *CurrPtr,int *NextBitPos,int *CodeBit)
{
  int pos;

  pos = *NextBitPos;

  /* Strip out the bit. */
  *CodeBit = (int)(*CurrPtr >> pos) & 1;

  /* Point to next available bit.*/
  if(pos == 15)
  {
    pos = 0;
    CurrPtr++;
  }/*end if pos...*/
  else
    pos++;

  /* Update the global next bit position.*/
  *NextBitPos = pos;

  return(CurrPtr); /* Success.*/
}/*end GetNextBit.*/

unsigned short int *GetRunLevel(unsigned short int *CurrPtr,int *NextBitPos,
                          int *Run, int *Level,int *BitsExtracted)
{
  int bit;
  unsigned int bits;
  int tbl_pos;
  int Found;
  int bits_so_far,bits_needed;

  bits = 0;
  tbl_pos = 0;
  bits_so_far = 0;
  bits_needed = 0;
  Found = 0;

  while( (tbl_pos < TableSize) && !Found )
  {
    bits_needed = VLCTable[tbl_pos].NumBits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (VLCTable[tbl_pos].NumBits == bits_so_far) && 
           (tbl_pos < TableSize) && !Found )
    {
      if(VLCTable[tbl_pos].Code == bits)
        Found = 1;
      else
        tbl_pos++;
    }/*end while NumBits...*/

  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *Run = 0;
    *Level = 0;
    *BitsExtracted = 0; /* Implies an error. */
		ErrorStr = "Non-existant Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  /* Check for Escape sequence. */
  if( (VLCTable[tbl_pos].NumBits == ESCAPEBits)&&
      (VLCTable[tbl_pos].Code == ESCAPECode)) 
  {
    CurrPtr = GetBits(CurrPtr,NextBitPos,MAX_RUN_BITS,&bit);
    *Run = bit;
    CurrPtr = GetBits(CurrPtr,NextBitPos,8,&bit);
    *Level = (int)((char)(bit | 0xFF00));
    bits_so_far += (MAX_RUN_BITS + 8);
  }/*end if...*/
  /* Check for EOB sequence. */
  else if( (VLCTable[tbl_pos].NumBits == EOBBits)&&
      (VLCTable[tbl_pos].Code == EOBCode))
  {
    *Run = VLCTable[tbl_pos].Run;
    *Level = VLCTable[tbl_pos].Value;
  }/*end else if...*/
  else /* Get sign bit. */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    *Run = VLCTable[tbl_pos].Run;
    if(bit) /* -ve. */
      *Level = -(VLCTable[tbl_pos].Value);
    else /* +ve. */
      *Level = VLCTable[tbl_pos].Value;
    bits_so_far++;
  }/*end else...*/

  *BitsExtracted = bits_so_far;
  return(CurrPtr);
}/*end GetRunLevel.*/

