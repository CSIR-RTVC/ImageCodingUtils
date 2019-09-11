//**********************************************************
// TITLE       :2-DIM DWT STILL IMAGE COMPRESSION CODEC 
//              CLASS DEFINITION FILE
// VERSION     :1.0
// FILE        :ImgCodec.cpp
// DESCRIPTION :A class for implementing a DWT-based still
//              image compression and decompression codec.
//              A YUV422 image is coded into a bit stream 
//              and the bit stream is decoded into a YUV422
//              image.
// DATE        :March 1999
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include "ImgCodec.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////
// Quantisation Tables.
////////////////////////////////////////////////////////////

// Psychovisual DWT table constructed for a 15" monitor at
// 800x600 viewed at 0.6m up to 6 DWT levels.

#define LxLy 0
#define LxHy 1
#define HxLy 2
#define HxHy 3

//		            Level					
//Colour	Orient	     1      2	     3	    4	     5	    6
//Y	      LxLy	   4.176  1.658	 0.800	0.469	 0.334	0.288
//	      LxHy	   6.389  2.264	 0.975	0.510	 0.324	0.250
//	      HxLy	   6.389  2.264	 0.975	0.510	 0.324	0.250
//	      HxHy	  14.668  4.359	 1.574	0.690	 0.367	0.238
//							
//U	      LxLy	   8.796	4.192	 2.315	1.482	 1.099	0.944
//	      LxHy	  11.976	5.223	 2.639	1.546	 1.049	0.824
//	      HxLy	  11.976	5.223	 2.639	1.546	 1.049	0.824
//	      HxHy	  23.549	8.871	 3.872	1.958	 1.148	0.779
//							  					                           
//V	      LxLy	   3.566	1.738	 1.053	0.793	 0.742	0.864
//	      LxHy	   6.236	2.499	 1.245	0.771	 0.593	0.567
//	      HxLy	   6.236	2.499	 1.245	0.771	 0.593	0.567
//	      HxHy	  13.731	4.472	 1.810	0.911	 0.569	0.442

static double YPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
  4.176, 1.658, 0.800, 0.469, 0.334,	0.288,
  6.389, 2.264, 0.975, 0.510, 0.324,	0.250,
  6.389, 2.264, 0.975, 0.510, 0.324,	0.250,
 14.668, 4.359, 1.574, 0.690, 0.367,	0.238
};

static double UPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
   8.796,	4.192, 2.315,	1.482, 1.099,	0.944,
  11.976,	5.223, 2.639,	1.546, 1.049,	0.824,
  11.976,	5.223, 2.639,	1.546, 1.049,	0.824,
  23.549,	8.871, 3.872,	1.958, 1.148,	0.779
};

static double VPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
{
   3.566,	1.738, 1.053,	0.793, 0.742,	0.864,
   6.236,	2.499, 1.245,	0.771, 0.593,	0.567,
   6.236,	2.499, 1.245,	0.771, 0.593,	0.567,
  13.731,	4.472, 1.810,	0.911, 0.569,	0.442
};

////////////////////////////////////////////////////////////
// Watson et al. table.
////////////////////////////////////////////////////////////
//static double YPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
//{
//  14.050,11.110,11.360,14.500, 0.334,	0.288,
//  23.030,14.680,12.710,14.160, 0.324,	0.250,
//  23.030,14.680,12.710,14.160, 0.324,	0.250,
//  58.760,28.410,19.540,17.860, 0.367,	0.238
//};
//
//static double UPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
//{
//  55.250,46.560,48.450,59.990, 1.099,	0.944,
//  86.790,60.480,54.570,60.480, 1.049,	0.824,
//  86.790,60.480,54.570,60.480, 1.049,	0.824,
// 215.840,117.45,86.740,81.230, 1.148,	0.779
//};
//
//static double VPsyQTable[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS] =
//{
//  25.040,19.280,19.670,25.600, 0.742,	0.864,
//  60.020,34.340,27.280,28.500, 0.593,	0.567,
//  60.020,34.340,27.280,28.500, 0.593,	0.567,
// 184.640,77.570,47.440,39.470, 0.569,	0.442
//};

////////////////////////////////////////////////////////////
// VLC variables using the H261 tables.

static int ESCAPEBits = 6;
static unsigned int ESCAPECode = 0x00000020;

static int EOBBits = 2;
static unsigned int EOBCode = 0x00000001;

/////////////// Run Length Table ///////////////////////////////
//#define TableSize  63
//static CodeWord VLCTable[TableSize] =
//{
//  { 0,  1, 2, 0x00000003},
//  { 0,  2, 4, 0x00000004},
//  { 0,  3, 5, 0x00000005},
//  { 0,  4, 7, 0x00000006},
//  { 0,  5, 8, 0x00000026},
//  { 0,  6, 8, 0x00000021},
//  { 0,  7,10, 0x0000000A},
//  { 0,  8,12, 0x0000001D},
//  { 0,  9,12, 0x00000018},
//  { 0, 10,12, 0x00000013},
//  { 0, 11,12, 0x00000010},
//  { 0, 12,13, 0x0000001A},
//  { 0, 13,13, 0x00000019},
//  { 0, 14,13, 0x00000018},
//  { 0, 15,13, 0x00000017},
//  { 1,  1, 3, 0x00000003},
//  { 1,  2, 6, 0x00000006},
//  { 1,  3, 8, 0x00000025},
//  { 1,  4,10, 0x0000000C},
//  { 1,  5,12, 0x0000001B},
//  { 1,  6,13, 0x00000016},
//  { 1,  7,13, 0x00000015},
//  { 2,  1, 4, 0x00000005},
//  { 2,  2, 7, 0x00000004},
//  { 2,  3,10, 0x0000000B},
//  { 2,  4,12, 0x00000014},
//  { 2,  5,13, 0x00000014},
//  { 3,  1, 5, 0x00000007},
//  { 3,  2, 8, 0x00000024},
//  { 3,  3,12, 0x0000001C},
//  { 3,  4,13, 0x00000013},
//  { 4,  1, 5, 0x00000006},
//  { 4,  2,10, 0x0000000F},
//  { 4,  3,12, 0x00000012},
//  { 5,  1, 6, 0x00000007},
//  { 5,  2,10, 0x00000009},
//  { 5,  3,13, 0x00000012},
//  { 6,  1, 6, 0x00000005},
//  { 6,  2,12, 0x0000001E},
//  { 7,  1, 6, 0x00000004},
//  { 7,  2,12, 0x00000015},
//  { 8,  1, 7, 0x00000007},
//  { 8,  2,12, 0x00000011},
//  { 9,  1, 7, 0x00000005},
//  { 9,  2,13, 0x00000011},
//  {10,  1, 8, 0x00000027},
//  {10,  2,13, 0x00000010},
//  {11,  1, 8, 0x00000023},
//  {12,  1, 8, 0x00000022},
//  {13,  1, 8, 0x00000020},
//  {14,  1,10, 0x0000000E},
//  {15,  1,10, 0x0000000D},
//  {16,  1,10, 0x00000008},
//  {17,  1,12, 0x0000001F},
//  {18,  1,12, 0x0000001A},
//  {19,  1,12, 0x00000019},
//  {20,  1,12, 0x00000017},
//  {21,  1,12, 0x00000016},
//  {22,  1,13, 0x0000001F},
//  {23,  1,13, 0x0000001E},
//  {24,  1,13, 0x0000001D},
//  {25,  1,13, 0x0000001C},
//  {26,  1,13, 0x0000001B}
//};

#define TableSize  65
static CodeWord VLCTable[TableSize] =
{
  { 0,  0, 2, 0x00000001}, //EOB sequence.
  { 0,  1, 2, 0x00000003},
  { 1,  1, 3, 0x00000006},
  { 0,  2, 4, 0x00000002},
  { 2,  1, 4, 0x0000000A},
  { 0,  3, 5, 0x00000014},
  { 3,  1, 5, 0x0000001C},
  { 4,  1, 5, 0x0000000C},
  { 0,  0, 6, 0x00000020}, //Escape sequence.
  { 1,  2, 6, 0x00000018},
  { 5,  1, 6, 0x00000038},
  { 6,  1, 6, 0x00000028},
  { 7,  1, 6, 0x00000008},
  { 0,  4, 7, 0x00000030},
  { 2,  2, 7, 0x00000010},
  { 8,  1, 7, 0x00000070},
  { 9,  1, 7, 0x00000050},
  { 0,  5, 8, 0x00000064},
  { 0,  6, 8, 0x00000084},
  { 1,  3, 8, 0x000000A4},
  { 3,  2, 8, 0x00000024},
  {10,  1, 8, 0x000000E4},
  {11,  1, 8, 0x000000C4},
  {12,  1, 8, 0x00000044},
  {13,  1, 8, 0x00000004},
  { 0,  7,10, 0x00000140},
  { 1,  4,10, 0x000000C0},
  { 2,  3,10, 0x00000340},
  { 4,  2,10, 0x000003C0},
  { 5,  2,10, 0x00000240},
  {14,  1,10, 0x000001C0},
  {15,  1,10, 0x000002C0},
  {16,  1,10, 0x00000040},
  { 0,  8,12, 0x00000B80},
  { 0,  9,12, 0x00000180},
  { 0, 10,12, 0x00000C80},
  { 0, 11,12, 0x00000080},
  { 1,  5,12, 0x00000D80},
  { 2,  4,12, 0x00000280},
  { 3,  3,12, 0x00000380},
  { 4,  3,12, 0x00000480},
  { 6,  2,12, 0x00000780},
  { 7,  2,12, 0x00000A80},
  { 8,  2,12, 0x00000880},
  {17,  1,12, 0x00000F80},
  {18,  1,12, 0x00000580},
  {19,  1,12, 0x00000980},
  {20,  1,12, 0x00000E80},
  {21,  1,12, 0x00000680},
  { 0, 12,13, 0x00000B00},
  { 0, 13,13, 0x00001300},
  { 0, 14,13, 0x00000300},
  { 0, 15,13, 0x00001D00},
  { 1,  6,13, 0x00000D00},
  { 1,  7,13, 0x00001500},
  { 2,  5,13, 0x00000500},
  { 3,  4,13, 0x00001900},
  { 5,  3,13, 0x00000900},
  { 9,  2,13, 0x00001100},
  {10,  2,13, 0x00000100},
  {22,  1,13, 0x00001F00},
  {23,  1,13, 0x00000F00},
  {24,  1,13, 0x00001700},
  {25,  1,13, 0x00000700},
  {26,  1,13, 0x00001B00}
};

////////////////////////////////////////////////////////////
// Construction and destruction.
////////////////////////////////////////////////////////////

CIMGCODEC::CIMGCODEC()
{
  hL_b = NULL;
  L_b = NULL;
  hY = NULL;
  hU = NULL;
  hV = NULL;
  ErrorStr = "No Erorr";
  BitStreamSize = 0;
  CodecIsOpen = 0;

  PS.Q = 1;
  PS.Bpp = 8;
  PS.pRGB = NULL;
  PS.RGB_X = 0;
  PS.RGB_Y = 0;
  PS.BitStream = NULL;
  PS.MAX_BITSTREAM_SIZE = 0;
  Y = NULL; 
  Y_X = 0;
  Y_Y = 0;
  U = NULL;
  V = NULL;
  UV_X = 0;
  UV_Y = 0;
}//end constructor.

CIMGCODEC::~CIMGCODEC()
{
}//end destructor.

////////////////////////////////////////////////////////////
// Public Implementation.
////////////////////////////////////////////////////////////

int CIMGCODEC::Open(CIMGCODEC_STRUCT *Params)
{
  int i,j;

  //If already open then close first before continuing.
  if(CodecIsOpen)
    Close();

  //Update to the new parameters.
  PS = *Params;

  //Create a YUV422 memory space.
	hY = GlobalAlloc(GMEM_FIXED,(PS.RGB_X * PS.RGB_Y * sizeof(fixedpoint)));
	if(!hY)
  {
    ErrorStr = "Luminance memory unavailable!";
    Close();
	  return(0);
  }//end if !hY...
	Y = (fixedpoint *)GlobalLock(hY);
  Y_X = PS.RGB_X;
  Y_Y = PS.RGB_Y;
	hU = GlobalAlloc(GMEM_FIXED,((PS.RGB_X/2) * PS.RGB_Y * sizeof(fixedpoint)));
	if(!hU)
  {
    ErrorStr = "U Chrominance memory unavailable!";
    Close();
	  return(0);
  }//end if !hU...
	U = (fixedpoint *)GlobalLock(hU);
  UV_X = PS.RGB_X/2;
  UV_Y = PS.RGB_Y;
	hV = GlobalAlloc(GMEM_FIXED,((PS.RGB_X/2) * PS.RGB_Y * sizeof(fixedpoint)));
	if(!hV)
  {
    ErrorStr = "V Chrominance memory unavailable!";
    Close();
	  return(0);
  }//end if !hV...
	V = (fixedpoint *)GlobalLock(hV);

  //Establish the DWT levels.
  Y_l = GetMaxLevels(Y_X,Y_Y);
  UV_l = GetMaxLevels(UV_X,UV_Y);

  //Construct the quantization tables.
  for(i = 0; i < NUM_OF_SUBBANDS; i++)
    for(j = 0; j < CODEC_MAX_LEVELS; j++)
    {
      if(PS.Q > 0)
      {
        double q;
        q = (double)(PS.Q) * YPsyQTable[i][j];
        q = 1.0 + (q/4.0);
        if( q < 1.0 )
          q = 1.0;
        IY_Q[i][j] = (int)(q * QFxPt_float);
        Y_Q[i][j] = (int)((1.0/q) * QFxPt_float);

        q = (double)(PS.Q) * UPsyQTable[i][j];
        q = 1.0 + (q/4.0);
        if( q < 1.0 )
          q = 1.0;
        IU_Q[i][j] = (int)(q * QFxPt_float);
        U_Q[i][j] = (int)((1.0/q) * QFxPt_float);
  
        q = (double)(PS.Q) * VPsyQTable[i][j];
        q = 1.0 + (q/4.0);
        if( q < 1.0 )
          q = 1.0;
        IV_Q[i][j] = (int)(q * QFxPt_float);
        V_Q[i][j] = (int)((1.0/q) * QFxPt_float);
      }//end if Q...
      else
      {
        IY_Q[i][j] = (int)(1.0 * QFxPt_float);
        Y_Q[i][j] = (int)(1.0 * QFxPt_float);
        IU_Q[i][j] = (int)(1.0 * QFxPt_float);
        U_Q[i][j] = (int)(1.0 * QFxPt_float);
        IV_Q[i][j] = (int)(1.0 * QFxPt_float);
        V_Q[i][j] = (int)(1.0 * QFxPt_float);
      }//end else...
    }//end for i & j...

	//Allocate working memory as a 2 line image line buffer.
	hL_b = GlobalAlloc(GMEM_FIXED,(2 * Y_X * sizeof(fixedpoint)));
	if(!hL_b)
  {
    ErrorStr = "Working memory unavailable!";
    Close();
	  return(0);
  }//end if !hL_b...
	L_b = (fixedpoint *)GlobalLock(hL_b);

  ErrorStr = "No Erorr";
  CodecIsOpen = 1;
  return(1);
}//end Open.

int CIMGCODEC::Code()
{
  if(!RGB24toYUV422())
    return(0);

  Dwt2D(Y,U,V);

  if(!RunLengthCode())
    return(0);

  return(1);
}//end Code.

int CIMGCODEC::Decode()
{

  if(!RunLengthDecode())
    return(0);

  IDwt2D(Y,U,V);

  if(!YUV422toRGB24())
    return(0);

  return(1);
}//end Decode.

void CIMGCODEC::Close(void)
{
  //Free the working memory.
	if(hL_b != NULL)
	{
		GlobalUnlock(hL_b);
 		GlobalFree(hL_b);
 		hL_b = NULL;
	}//end if hL_b...
	L_b = NULL;
	if(hY != NULL)
	{
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
	}//end if hY...
	Y = NULL;
	if(hU != NULL)
	{
		GlobalUnlock(hU);
 		GlobalFree(hU);
 		hU = NULL;
	}//end if hU...
	U = NULL;
	if(hV != NULL)
	{
		GlobalUnlock(hV);
 		GlobalFree(hV);
 		hV = NULL;
	}//end if hV...
	V = NULL;

  CodecIsOpen = 0;
}//end Close.

////////////////////////////////////////////////////////////
// Private Implementation.
////////////////////////////////////////////////////////////

// Get the number of possible wavelet levels for these
// image dimensions. i.e. the level at which the texture 
// image will have an odd dimension or less than 16.
int CIMGCODEC::GetMaxLevels(int x,int y)
{
  int level = 0;
  while( ((y % 2)==0)&&((x % 2)==0)&&(y > 16)&&(x > 16) )
  {
    x = x/2;
    y = y/2;
    level++;
  }//end while...
	return(level);
}//end GetMaxLevels.


///////////////////////////////////////////////////////////////
// Fast lifting filter 9 - 7.
///////////////////////////////////////////////////////////////

#define P7_0 (fixedpoint)(   25) /*(0.003004 * FxPt_float)  */
#define P7_1 (fixedpoint)( -401) /*(-0.048980 * FxPt_float) */
#define P7_2 (fixedpoint)( 2425) /*(0.295988 * FxPt_float)  */
#define P7_3 (fixedpoint)( 2425) /*(0.295988 * FxPt_float)  */
#define P7_4 (fixedpoint)( -401) /*(-0.048980 * FxPt_float) */
#define P7_5 (fixedpoint)(   25) /*(0.003004 * FxPt_float)  */

#define U9_0 (fixedpoint)(  -93) /*(-0.011330 * FxPt_float) */
#define U9_1 (fixedpoint)(  605) /*(0.073860 * FxPt_float)  */
#define U9_2 (fixedpoint)(-4859) /*(-0.593200 * FxPt_float) */
#define U9_3 (fixedpoint)(-4859) /*(-0.593200 * FxPt_float) */
#define U9_4 (fixedpoint)(  605) /*(0.073860 * FxPt_float)  */
#define U9_5 (fixedpoint)(  -93) /*(-0.011330 * FxPt_float) */

#define MINUS_HALF (fixedpoint)(-4096) /*(-0.5 * FxPt_float)*/
#define HALF (fixedpoint)(4096) /*(0.5 * FxPt_float)       */

//#define P7_0 (fixedpoint)(0.003004 * FxPt_float)
//#define P7_1 (fixedpoint)(-0.048980 * FxPt_float)
//#define P7_2 (fixedpoint)(0.295988 * FxPt_float)
//#define P7_3 (fixedpoint)(0.295988 * FxPt_float)
//#define P7_4 (fixedpoint)(-0.048980 * FxPt_float)
//#define P7_5 (fixedpoint)(0.003004 * FxPt_float)

//#define U9_0 (fixedpoint)(-0.011330 * FxPt_float)
//#define U9_1 (fixedpoint)(0.073860 * FxPt_float)
//#define U9_2 (fixedpoint)(-0.593200 * FxPt_float)
//#define U9_3 (fixedpoint)(-0.593200 * FxPt_float)
//#define U9_4 (fixedpoint)(0.073860 * FxPt_float)
//#define U9_5 (fixedpoint)(-0.011330 * FxPt_float)

//#define MINUS_HALF (fixedpoint)(-0.5 * FxPt_float)
//#define HALF (fixedpoint)(0.5 * FxPt_float)

/////////////////////////////////////////////////////////////////
// In-place 2-dim DWT of the local YUV image pointed to by the 
// input parameters. The dim of the YUV image is assumed to be
// that of the parameter list.
/////////////////////////////////////////////////////////////////
void CIMGCODEC::Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
{
  int col,lev,X,Y,levels;
  fixedpoint *Src,*High;
  High = (fixedpoint *)L_b;

  for(col = 0; col < 3; col++)
  {
    if(col==0) //Lum.
    {
      levels = Y_l;
      X = Y_X;
      Y = Y_Y;
      Src = locY;
    }//end if col...
    else //Chr.
    {
      levels = UV_l;
      X = UV_X;
      Y = UV_Y;
      if(col==1) //U Chr.
        Src = locU;
      else //V Chr.
        Src = locV;
    }//end else...

    for(lev = 0; lev < levels; lev++)
    {
      int LevY = Y >> lev;
      int LevX = X >> lev;
  
      int i,j;
      int SubLen, limit;
      register long r,s;
      register fixedpoint a,b,c,d,e,f,g,h,k;
      register int m,n;
      register fixedpoint *x,*p;

      //In x direction.
      SubLen = LevX >> 1;
      for(i = 0; i < LevY; i++)
      {
        //Do the 1-D DWT along the rows.
        x = (fixedpoint *)(Src + i*X);
        p = High;
        ////////////////// Interpolation /////////////////////////////
        a = x[0]; b = x[1]; c = x[2]; g = x[3]; d = x[4]; e = x[6]; f = x[8];
        //For position x[1];
        r = (long)((a + c)*P7_2) + (long)(b*MINUS_HALF) +
            (long)((c + d)*P7_1) + (long)((d + e)*P7_0);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        //For position x[3];
        s = (long)((c + d)*P7_2) + (long)(g*MINUS_HALF) +
            (long)((c + f)*P7_0) + (long)((a + e)*P7_1); 
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 4);
        for(n = 2; n < limit; n+=2) //Two at a time for pipeline efficiency.
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
        }//end for n...
        a = x[LevX-10]; b = x[LevX-8]; k = x[LevX-7]; c = x[LevX-6]; d = x[LevX-5];
        e = x[LevX-4]; f = x[LevX-3]; g = x[LevX-2]; h = x[LevX-1];
        if(n == (SubLen-4))
        {
          //For position x[LevX-7];
          s = (long)((x[LevX-12] + g)*P7_0) + (long)((a + e)*P7_1) + 
              (long)((b + c)*P7_2) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }//end if n...
        //For position x[LevX-5];
        r = (long)((a + g)*P7_0) + (long)((b + g)*P7_1) + 
            (long)((c + e)*P7_2) + (long)(d*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        //For position x[LevX-3];
        s = (long)((b + e)*P7_0) + (long)((c + g)*P7_1) + 
            (long)((e + g)*P7_2) + (long)(f*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        //For position x[LevX-1];
        r = (long)((c + c)*P7_0) + (long)((e + e)*P7_1) + 
            (long)((g + g)*P7_2) + (long)(h*MINUS_HALF);
        *p = (fixedpoint)(FIXPOINTADJUST(r));
        ////////////////// Update //////////////////////////////////////
        p = x;
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        //For Low[0].
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + 
            (long)((a + b)*U9_2);
        *p++ = x[0] + (fixedpoint)(FIXPOINTADJUST(r));
        //For Low[1].
        *p++ = x[2] + (fixedpoint)(FIXPOINTADJUST(r));
        //For Low[2].
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
        }//end for n...
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        //For Low[SubLen-3].
        if(n == (SubLen-3))
        {
          s = (long)((a + e)*U9_0) + (long)((b + e)*U9_1) + (long)((c + d)*U9_2);
          *p++ = x[LevX-6] + (fixedpoint)(FIXPOINTADJUST(s));
        }//end if n...
        //For Low[SubLen-2].
        r = (long)((b + f)*U9_0) + (long)((c + f)*U9_1) + (long)((d + e)*U9_2);
        *p++ = x[LevX-4] + (fixedpoint)(FIXPOINTADJUST(r));
        //For Low[SubLen-1].
        s = (long)((c + e)*U9_0) + (long)((d + f)*U9_1) + (long)((e + f)*U9_2);
        *p++ = x[LevX-2] + (fixedpoint)(FIXPOINTADJUST(s));
  
        //Copy remaining wavelet coefficients to in-place.
        x = High;
        for(j = 0; j < SubLen; j++)
          *p++ = *x++;
      }//end for i...
    
      //In y direction.
      SubLen = LevY >> 1;
      for(j = 0; j < LevX; j++)
      {
        // 1-D DWT down the columns.
        x = (fixedpoint *)(Src + j);
        p = High;
        ////////////////// Interpolation /////////////////////////////
        a = x[0]; b = x[X]; m = X<<1; c = x[m]; m += X; g = x[m];
        m += X; d = x[m]; m += X; e = x[m]; m += X; f = x[m];
        //For position x[1];
        r = (long)((a + c)*P7_2) + (long)(b*MINUS_HALF) +
            (long)((c + d)*P7_1) + (long)((d + e)*P7_0);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        //For position x[3];
        s = (long)((c + d)*P7_2) + (long)(g*MINUS_HALF) +
            (long)((c + f)*P7_0) + (long)((a + e)*P7_1); 
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        limit = (SubLen - 4);
        for(n = 2; n < limit; n+=2) //Two at a time for pipeline efficiency.
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
        }//end for n...
        m = (LevY-10)*X; a = x[m]; 
        m += (X<<1); b = x[m]; m += X; k = x[m]; 
        m += X; c = x[m]; m += X; d = x[m]; m += X; e = x[m]; 
        m += X; f = x[m]; m += X; g = x[m]; m += X; h = x[m];
        if(n == (SubLen-4))
        {
          //For position x[LevY-7];
          s = (long)((x[(LevY-12)*X] + g)*P7_0) + (long)((a + e)*P7_1) + 
              (long)((b + c)*P7_2) + (long)(k*MINUS_HALF);
          *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        }//end if n...
        //For position x[LevY-5];
        r = (long)((a + g)*P7_0) + (long)((b + g)*P7_1) + 
            (long)((c + e)*P7_2) + (long)(d*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(r));
        //For position x[LevY-3];
        s = (long)((b + e)*P7_0) + (long)((c + g)*P7_1) + 
            (long)((e + g)*P7_2) + (long)(f*MINUS_HALF);
        *p++ = (fixedpoint)(FIXPOINTADJUST(s));
        //For position x[LevY-1];
        r = (long)((c + c)*P7_0) + (long)((e + e)*P7_1) + 
            (long)((g + g)*P7_2) + (long)(h*MINUS_HALF);
        *p = (fixedpoint)(FIXPOINTADJUST(r));
        ////////////////// Update //////////////////////////////////////
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        //For Low[0].
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + 
            (long)((a + b)*U9_2);
        x[0] = x[0] + (fixedpoint)(FIXPOINTADJUST(r));
        //For Low[1].
        m = X<<1;
        x[X] = x[m] + (fixedpoint)(FIXPOINTADJUST(r));
        //For Low[2].
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
        }//end for n...
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        m = n*X;
        //For Low[SubLen-3].
        if(n == (SubLen-3))
        {
          s = (long)((a + e)*U9_0) + (long)((b + e)*U9_1) + (long)((c + d)*U9_2);
          x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
          m += X;
        }//end if n...
        //For Low[SubLen-2].
        r = (long)((b + f)*U9_0) + (long)((c + f)*U9_1) + (long)((d + e)*U9_2);
        x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(r));
        m += X;
        //For Low[SubLen-1].
        s = (long)((c + e)*U9_0) + (long)((d + f)*U9_1) + (long)((e + f)*U9_2);
        x[m] = x[m<<1] + (fixedpoint)(FIXPOINTADJUST(s));
        m += X;
  
        //Copy remaining wavelet coefficients to in-place.
        p = High;
        for(i = 0; i < SubLen; i++,m += X)
          x[m] = *p++;
      }//end for j...
    }//end for lev...
  }//end for col...

}//end Dwt2D.

/////////////////////////////////////////////////////////////////
// In-place 2-dim IDWT of the local YUV image pointed to by the 
// input parameters. The dim of the YUV image is assumed to be
// that of the parameter list.
/////////////////////////////////////////////////////////////////
void CIMGCODEC::IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
{
  int col,lev,X,Y,levels;
  fixedpoint *Low,*High,*Src;

  for(col = 0; col < 3; col++)
  {
    if(col==0) //Lum.
    {
      levels = Y_l;
      X = Y_X;
      Y = Y_Y;
      Src = locY;
    }//end if col...
    else //Chr.
    {
      levels = UV_l;
      X = UV_X;
      Y = UV_Y;
      if(col==1) //U Chr.
        Src = locU;
      else //V Chr.
        Src = locV;
    }//end else...

    for(lev = (levels - 1); lev >= 0 ; lev--)
    {
      int LevY = Y >> lev;
      int LevX = X >> lev;
  
      int i,j;
      int SubLen, limit;
      register long r,s;
      register fixedpoint a,b,c,d,e,f,g;
      register int m,n;
      register fixedpoint *x,*p;
  
      //In y direction.
      SubLen = LevY >> 1;
      for(j = 0; j < LevX; j++)
      {
        //Do 1-D Inverse transform.
        Low = (fixedpoint *)(Src + j);
        High = (fixedpoint *)(Low + (LevY>>1)*X);
        x = Low;
        p = (fixedpoint *)L_b;
  
        // Do 1-D DWT down cols.
        ///////////// Create even coeff with piece of wavelet ////////////
        a = High[0]; b = High[X]; m = X<<1; c = High[m]; 
        m += X; d = High[m]; m += X; e = High[m];
        //For Recon[0].
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + (long)((a + b)*U9_2);
        p[0] = Low[0] - (fixedpoint)(FIXPOINTADJUST(r));
        //For Recon[2].
        p[2] = Low[X] - (fixedpoint)(FIXPOINTADJUST(r));
        //For Recon[4].
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
        }//end for n...
        i = (SubLen-6)*X; a = High[i]; i += X; b = High[i]; 
        i += X; c = High[i]; i += X; d = High[i]; 
        i += X; e = High[i]; f = High[i];
        m = n*X;
        //For Recon[LevY-6].
        if(n == limit)
        {
          s = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          p[LevY-6] = Low[m] - (fixedpoint)(FIXPOINTADJUST(s));
          m += X;
        }//end if n...
        //For Recon[LevY-4].
        r = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + f)*U9_0);
        p[LevY-4] = Low[m] - (fixedpoint)(FIXPOINTADJUST(r));
        m += X;
        //For Recon[LevY-2].
        s = (long)((e + f)*U9_2) + (long)((d + f)*U9_1) + (long)((c + e)*U9_0);
        p[LevY-2] = Low[m] - (fixedpoint)(FIXPOINTADJUST(s));
      
        ///////// Interpolate the odd coeff with w and the new even coeff.
        a = p[0]; b = p[2]; c = p[4]; d = p[6]; e = p[8];
        //For Recon[1].
        r = (long)((a + b)*P7_2) + (long)((b + c)*P7_1) + (long)((c + d)*P7_0); 
        x[X] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[0]) << 1;
        //For Recon[3].
        s = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((b + e)*P7_0); 
        x[3*X] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[X]) << 1;
        limit = LevY - 8;
        for(n = 4; n < limit; n+=4)
        {
          m = (n>>1)*X;
          //Create odd coeff with sum of wavelet coeff and interpolated values.
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
        }//end for n...
        f = p[LevY-12]; a = p[LevY-10]; b = p[LevY-8];
        c = p[LevY-6]; d = p[LevY-4]; e = p[LevY-2];
        //For Recon[LevY-7].
        m = (LevY-12)*X;
        i = (LevY-7)*X;
        if(n == limit)
        {
          r = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((f + e)*P7_0);
          x[m] = f;
          x[i] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[(SubLen-4)*X]) << 1;
        }//end if n...
        m += (X<<1);
        i += (X<<1);
        //For Recon[LevY-5].
        s = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + e)*P7_0);
        x[m] = a; m += (X<<1);
        n = (SubLen-3)*X;
        x[i] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[n]) << 1;
        i += (X<<1);
        n += X;
        //For Recon[LevY-3].
        r = (long)((b + d)*P7_0) + (long)((c + e)*P7_1) + (long)((d + e)*P7_2);
        x[m] = b; m += (X<<1);
        x[i] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[n]) << 1;
        i += (X<<1);
        n += X;
        //For Recon[LevY-1].
        s = (long)((c + c)*P7_0) + (long)((d + d)*P7_1) + (long)((e + e)*P7_2);
        x[m] = c; m += (X<<1);
        x[i] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[n]) << 1;
  
        //Copy remaining even coefficients.
        x[m] = d; m += (X<<1);
        x[m] = e;
      }//end for j...
    
      //In x direction.
      SubLen = LevX >> 1;
      for(i = 0; i < LevY; i++)
      {
        //Do 1-D Inverse transform.
        Low = (fixedpoint *)(Src + i*X);
        High = (fixedpoint *)(Low + (LevX>>1));
        x = Low;
        p = (fixedpoint *)L_b;
  
        // Do 1-D DWT along rows.
        ///////////// Create even coeff with piece of wavelet ////////////
        a = High[0]; b = High[1]; c = High[2]; d = High[3]; e = High[4];
        //For Recon[0].
        r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + (long)((a + b)*U9_2);
        p[0] = Low[0] - (fixedpoint)(FIXPOINTADJUST(r));
        //For Recon[2].
        p[2] = Low[1] - (fixedpoint)(FIXPOINTADJUST(r));
        //For Recon[4].
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
        }//end for n...
        a = High[SubLen-6]; b = High[SubLen-5]; c = High[SubLen-4];
        d = High[SubLen-3]; e = High[SubLen-2]; f = High[SubLen-1];
        //For Recon[LevX-6].
        if(n == (SubLen-3))
        {
          s = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
          p[LevX-6] = Low[n++] - (fixedpoint)(FIXPOINTADJUST(s));
        }//end if n...
        //For Recon[LevX-4].
        r = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + f)*U9_0);
        p[LevX-4] = Low[n++] - (fixedpoint)(FIXPOINTADJUST(r));
        //For Recon[LevX-2].
        s = (long)((e + f)*U9_2) + (long)((d + f)*U9_1) + (long)((c + e)*U9_0);
        p[LevX-2] = Low[n] - (fixedpoint)(FIXPOINTADJUST(s));
      
        ///////// Interpolate the odd coeff with w and the new even coeff.
        a = p[0]; b = p[2]; c = p[4]; d = p[6]; e = p[8];
        //For Recon[1].
        r = (long)((a + b)*P7_2) + (long)((b + c)*P7_1) + (long)((c + d)*P7_0); 
        x[1] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[0]) << 1;
        //For Recon[3].
        s = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((b + e)*P7_0); 
        x[3] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[1]) << 1;
        limit = LevX - 8;
        for(n = 4; n < limit; n+=4)
        {
          m = n>>1;
          //Create odd coeff with sum of wavelet coeff and interpolated values.
          a = p[n-4]; b = p[n-2]; c = p[n]; d = p[n+2];
          e = p[n+4]; f = p[n+6]; g = p[n+8];
          x[n-4] = a;
          x[n-2] = b;
          r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + f)*P7_0);
          x[n + 1] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[m]) << 1;
          s = (long)((d + e)*P7_2) + (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
          x[n + 3] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[m+1]) << 1;
        }//end for n...
        f = p[LevX-12]; a = p[LevX-10]; b = p[LevX-8];
        c = p[LevX-6]; d = p[LevX-4]; e = p[LevX-2];
        //For Recon[LevX-7].
        if(n == limit)
        {
          r = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((f + e)*P7_0);
          x[LevX-12] = f;
          x[LevX-7] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[SubLen-4]) << 1;
        }//end if n...
        //For Recon[LevX-5].
        s = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + e)*P7_0);
        x[LevX-10] = a;
        x[LevX-5] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[SubLen-3]) << 1;
        //For Recon[LevX-3].
        r = (long)((b + d)*P7_0) + (long)((c + e)*P7_1) + (long)((d + e)*P7_2);
        x[LevX-8] = b;
        x[LevX-3] = ((fixedpoint)(FIXPOINTADJUST(r)) - High[SubLen-2]) << 1;
        //For Recon[LevX-1].
        s = (long)((c + c)*P7_0) + (long)((d + d)*P7_1) + (long)((e + e)*P7_2);
        x[LevX-6] = c;
        x[LevX-1] = ((fixedpoint)(FIXPOINTADJUST(s)) - High[SubLen-1]) << 1;
  
        //Copy remaining even coefficients.
        x[LevX-4] = d;
        x[LevX-2] = e;
  
      }//end for i...
    }//end for lev...
  }//end for col...
  
}//end IDwt2D.

/////////////////////////////////////////////////////////////////
// Fast discrete wavelet transform in 1-dimension with
// down sampling by 2.
// Input: Signal x of length Len.
// Output: Low and High pass signals of length Len/2.
/////////////////////////////////////////////////////////////////
void CIMGCODEC::Dwt1D(fixedpoint *x,int Len,fixedpoint *y_Low,fixedpoint *y_High)
{
  int i,j;
  int SubLen = Len>>1;

  long r;

  //The boundary wrapping is done in line.

  ////////////////// Interpolation /////////////////////////////
  //For position x[1];
  r = (long)((x[4] + x[6])*P7_0) + (long)((x[2] + x[4])*P7_1) + 
      (long)((x[0] + x[2])*P7_2) + (long)(x[1]*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[0] = (fixedpoint)(FIXPOINTADJUST(r));

  //For position x[3];
  r = (long)((x[2] + x[8])*P7_0) + (long)((x[0] + x[6])*P7_1) + 
      (long)((x[2] + x[4])*P7_2) + (long)(x[3]*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[1] = (fixedpoint)(FIXPOINTADJUST(r));

  int limit = (SubLen - 3);
  for(j = 2; j < limit; j++)
  {
    //Interpolate the odd coeff with learned w coeff 
    //from the even coefficients.
    i = j<<1;
    r = (long)((x[i-4] + x[i+6])*P7_0) + (long)((x[i-2] + x[i+4])*P7_1) + 
        (long)((x[i] + x[i+2])*P7_2) + (long)(x[i+1]*MINUS_HALF);

    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[j] = (fixedpoint)(FIXPOINTADJUST(r));

  }//end for j...

  //For position x[Len-5];
  r = (long)((x[Len-10] + x[Len-2])*P7_0) + (long)((x[Len-8] + x[Len-2])*P7_1) + 
      (long)((x[Len-6] + x[Len-4])*P7_2) + (long)(x[Len-5]*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-3] = (fixedpoint)(FIXPOINTADJUST(r));

  //For position x[Len-3];
  r = (long)((x[Len-8] + x[Len-4])*P7_0) + (long)((x[Len-6] + x[Len-2])*P7_1) + 
      (long)((x[Len-4] + x[Len-2])*P7_2) + (long)(x[Len-3]*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-2] = (fixedpoint)(FIXPOINTADJUST(r));

  //For position x[Len-1];
  r = (long)((x[Len-6] + x[Len-6])*P7_0) + (long)((x[Len-4] + x[Len-4])*P7_1) + 
      (long)((x[Len-2] + x[Len-2])*P7_2) + (long)(x[Len-1]*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-1] = (fixedpoint)(FIXPOINTADJUST(r));

  ////////////////// Update //////////////////////////////////////
  //Update even coeff with piece of wavelet.

  //For y_Low[0].
  r = (long)((y_High[2] + y_High[3])*U9_0) + (long)((y_High[1] + y_High[2])*U9_1) + 
      (long)((y_High[0] + y_High[1])*U9_2);
  y_Low[0] = x[0] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[1].
  r = (long)((y_High[2] + y_High[3])*U9_0) + (long)((y_High[1] + y_High[2])*U9_1) + 
      (long)((y_High[0] + y_High[1])*U9_2);
  y_Low[1] = x[2] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[2].
  r = (long)((y_High[1] + y_High[4])*U9_0) + (long)((y_High[0] + y_High[3])*U9_1) + 
      (long)((y_High[1] + y_High[2])*U9_2);
  y_Low[2] = x[4] + (fixedpoint)(FIXPOINTADJUST(r));

  limit = (SubLen - 2);
  for(j = 3; j < limit; j++)
  {
    r = (long)((y_High[j-3] + y_High[j+2])*U9_0) + (long)((y_High[j-2] + y_High[j+1])*U9_1) + 
        (long)((y_High[j-1] + y_High[j])*U9_2);
    y_Low[j] = x[j<<1] + (fixedpoint)(FIXPOINTADJUST(r));
  }//end for j...

  //For y_Low[SubLen-2].
  r = (long)((y_High[SubLen-5] + y_High[SubLen-1])*U9_0) + 
      (long)((y_High[SubLen-4] + y_High[SubLen-1])*U9_1) + 
      (long)((y_High[SubLen-3] + y_High[SubLen-2])*U9_2);
  y_Low[SubLen-2] = x[Len-4] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[SubLen-1].
  r = (long)((y_High[SubLen-4] + y_High[SubLen-2])*U9_0) + 
      (long)((y_High[SubLen-3] + y_High[SubLen-1])*U9_1) + 
      (long)((y_High[SubLen-2] + y_High[SubLen-1])*U9_2);
  y_Low[SubLen-1] = x[Len-2] + (fixedpoint)(FIXPOINTADJUST(r));

}//end Dwt1D.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CIMGCODEC::IDwt1D(fixedpoint *x_Low,fixedpoint *x_High,int Len,fixedpoint *Recon)
{
  int j;
  int SubLen = Len>>1;

  long r;

  // The boundaries are performed as inline.

  ///////////// Create even coeff with piece of wavelet ////////////

  //For Recon[0].
  r = (long)((x_High[2] + x_High[3])*U9_0) + (long)((x_High[1] + x_High[2])*U9_1) + 
      (long)((x_High[0] + x_High[1])*U9_2);
  Recon[0] = x_Low[0] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[2].
  r = (long)((x_High[2] + x_High[3])*U9_0) + (long)((x_High[1] + x_High[2])*U9_1) + 
      (long)((x_High[0] + x_High[1])*U9_2);
  Recon[2] = x_Low[1] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[4].
  r = (long)((x_High[1] + x_High[4])*U9_0) + (long)((x_High[0] + x_High[3])*U9_1) + 
      (long)((x_High[1] + x_High[2])*U9_2);
  Recon[4] = x_Low[2] - (fixedpoint)(FIXPOINTADJUST(r));

  int limit = SubLen - 2;
  for(j = 3; j < limit; j++)
  {
    r = (long)((x_High[j-3] + x_High[j+2])*U9_0) + (long)((x_High[j-2] + x_High[j+1])*U9_1) + 
        (long)((x_High[j-1] + x_High[j])*U9_2);
    Recon[2*j] = x_Low[j] - (fixedpoint)(FIXPOINTADJUST(r));
  }//end for j...

  //For Recon[Len-4].
  r = (long)((x_High[SubLen-5] + x_High[SubLen-1])*U9_0) + 
      (long)((x_High[SubLen-4] + x_High[SubLen-1])*U9_1) + 
      (long)((x_High[SubLen-3] + x_High[SubLen-2])*U9_2);
  Recon[Len-4] = x_Low[SubLen-2] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[Len-2].
  r = (long)((x_High[SubLen-4] + x_High[SubLen-2])*U9_0) + 
      (long)((x_High[SubLen-3] + x_High[SubLen-1])*U9_1) + 
      (long)((x_High[SubLen-2] + x_High[SubLen-1])*U9_2);
  Recon[Len-2] = x_Low[SubLen-1] - (fixedpoint)(FIXPOINTADJUST(r));

  ///////// Interpolate the odd coeff with w and the new even coeff.

  //For Recon[1].
  r = (long)((Recon[4] + Recon[6])*P7_0) + (long)((Recon[2] + Recon[4])*P7_1) + 
      (long)((Recon[0] + Recon[2])*P7_2);
  Recon[1] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[0]) << 1;

  //For Recon[3].
  r = (long)((Recon[2] + Recon[8])*P7_0) + (long)((Recon[0] + Recon[6])*P7_1) + 
      (long)((Recon[2] + Recon[4])*P7_2);
  Recon[3] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[1]) << 1;

  limit = Len - 6;
  for(j = 4; j < Len; j+=2)
  {
    //Create odd coeff with sum of wavelet coeff and interpolated values.
    r = (long)((Recon[j-4] + Recon[j+6])*P7_0) + 
        (long)((Recon[j-2] + Recon[j+4])*P7_1) + 
        (long)((Recon[j] + Recon[j+2])*P7_2);
    Recon[j + 1] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[j>>1]) << 1;

  }//end for j...

  //For Recon[Len-5].
  r = (long)((Recon[Len-10] + Recon[Len-2])*P7_0) + 
      (long)((Recon[Len-8] + Recon[Len-2])*P7_1) + 
      (long)((Recon[Len-6] + Recon[Len-4])*P7_2);
  Recon[Len-5] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[SubLen-3]) << 1;

  //For Recon[Len-3].
  r = (long)((Recon[Len-8] + Recon[Len-4])*P7_0) + 
      (long)((Recon[Len-6] + Recon[Len-2])*P7_1) + 
      (long)((Recon[Len-4] + Recon[Len-2])*P7_2);
  Recon[Len-3] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[SubLen-2]) << 1;

  //For Recon[Len-1].
  r = (long)((Recon[Len-6] + Recon[Len-6])*P7_0) + 
      (long)((Recon[Len-4] + Recon[Len-4])*P7_1) + 
      (long)((Recon[Len-2] + Recon[Len-2])*P7_2);
  Recon[Len-1] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[SubLen-1]) << 1;

}//end IDwt1D.

//************* Quantization and Bit coding ******************

#define QFXROUND (int)(0.5 * QFxPt_float)

#define MAX_RUN_BITS 9
#define MAX_RUN ((1<<MAX_RUN_BITS)-1)

int CIMGCODEC::RunLengthCode(void)
{
  int i,j,s,lev;
  int run;
  char level;
  int DummyBitCnt, DummyBits;
  int pix;

  unsigned int *Bits = (unsigned int *)(PS.BitStream);
  BitStreamSize = 0; //In bytes.
  unsigned int MaxBits = PS.MAX_BITSTREAM_SIZE * 8; //8bits/byte.
  unsigned int BitCnt = 0; //In Bits.
  int BitPos = 0; //Bit position within the current 32bit word.

  // Code the Lum texture 1st with 8 bpp.
  fixedpoint *P = Y;
  int X = Y_X;
  int LevY = Y_Y >> Y_l;
  int LevX = X >> Y_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      // Quantize first.
      pix = ((int)(P[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * Y_Q[LxLy][Y_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      // pix is now in 8bpp form in range 0..255.
      if(pix > 255)
        pix = 255;
      if(pix < 0)
        pix = 0;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }//end if BitCnt...
      Bits = PutBits(Bits,&BitPos,8,pix);

      //Temp decode.
      // Test code.
//      P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//      P[i*X + j] = 0;
    }//end if i & j...

  // Code the Lum Sub-Bands.
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
      else //if(s == HxHy)
        P = (fixedpoint *)(Y + LevY*X + LevX);

//      //Never code HxHy level 1 Sub-band.
//      if( (lev==1)&&(s==HxHy) )
//      {
//        for(i = 0; i < LevY; i++)
//          for(j = 0; j < LevX; j++)
//          {
//            // Kill.
//            P[i*X + j] = 0;
//          }//end for i & j...
//      }//end if lev...
//      else
//      {
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            // Quantize first.
            pix = ((int)(P[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * Y_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            //Temp decode.
            // Test code.
//            P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//            P[i*X + j] = 0;
            // Runlength encode.
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }//end if level...
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Esc sequence.
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }//end if DummyBits...
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                //Add sign bit.
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }//end else...

              run = 0; //Reset the run.
            }//end else...
          }//end if i & j...
        BitCnt += EOBBits; //EndOfSubBand bits.
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }//end if BitCnt...
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
//      }//end else...
    }//end for s...

  }//end for lev...

  // Code the Chr texture 1st with 8 bpp.
  fixedpoint *PU = U;
  fixedpoint *PV = V;
  X = UV_X;
  LevY = UV_Y >> UV_l;
  LevX = X >> UV_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      // Quantize first.
      pix = ((int)(PU[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * U_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }//end if BitCnt...
      Bits = PutBits(Bits,&BitPos,8,pix);
      // Test code.
//      PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//      PU[i*X + j] = 0;

      pix = ((int)(PV[i*X + j]) << 8) >> (FxPt - QFxPt);
      pix = (((pix * V_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      BitCnt += 8;
      if( BitCnt >= MaxBits )
      {
        ErrorStr = "Coded bits too long!";
        return(0);
      }//end if BitCnt...
      Bits = PutBits(Bits,&BitPos,8,pix);
      // Test code.
//      PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//      PV[i*X + j] = 0;

    }//end if i & j...

  // Code the Chr Sub-Bands.
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
      }//end if s...
      else if(s == HxLy)
      {
        PU = (fixedpoint *)(U + LevX);
        PV = (fixedpoint *)(V + LevX);
      }//end if s...
      else //if(s == HxHy)
      {
        PU = (fixedpoint *)(U + LevY*X + LevX);
        PV = (fixedpoint *)(V + LevY*X + LevX);
      }//end else...

      //Never code HxHy level 1 Sub-band.
//      if( (lev==1)&&(s==HxHy) )
//      {
//        for(i = 0; i < LevY; i++)
//          for(j = 0; j < LevX; j++)
//          {
//            // Kill.
//            PU[i*X + j] = 0;
//            PV[i*X + j] = 0;
//          }//end for i & j...
//      }//end if lev...
//      else
//      {
        // U Chr.
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            // Quantize first.
            pix = ((int)(PU[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * U_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            // Test code.
//            PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//            PU[i*X + j] = 0;
            // Runlength encode.
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }//end if level...
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Esc sequence.
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }//end if DummyBits...
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                //Add sign bit.
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }//end else...

              run = 0; //Reset the run.
            }//end else...
          }//end if i & j...
        BitCnt += EOBBits; //EndOfSubBand bits.
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }//end if BitCnt...
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
  
        // V Chr.
        run = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            // Quantize first.
            pix = ((int)(PV[i*X + j]) << 8) >> (FxPt - QFxPt);
            pix = (((pix * V_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
            // Test code.
//            PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
//            PV[i*X + j] = 0;
            // Runlength encode.
            if(pix > 127)
              level = 127;
            else if(pix < -128)
              level = -128;
            else
              level = (char)pix;
            if((level == 0)&&(run < MAX_RUN))
            {
              run++;
            }//end if level...
            else
            {
              BitCnt += FindBits(run,level,&DummyBitCnt,&DummyBits);
              if( BitCnt >= MaxBits )
              {
                ErrorStr = "Coded bits too long!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Esc sequence.
              {
                Bits = PutBits(Bits,&BitPos,ESCAPEBits,ESCAPECode);
                Bits = PutBits(Bits,&BitPos,MAX_RUN_BITS,run);
                Bits = PutBits(Bits,&BitPos,8,level);
              }//end if DummyBits...
              else
              {
                Bits = PutBits(Bits,&BitPos,DummyBitCnt,DummyBits);
                //Add sign bit.
                BitCnt++;
                if(level < 0)
                  Bits = PutBits(Bits,&BitPos,1,1);
                else
                  Bits = PutBits(Bits,&BitPos,1,0);
              }//end else...

              run = 0; //Reset the run.
            }//end else...
          }//end if i & j...
        BitCnt += EOBBits; //EndOfSubBand bits.
        if( BitCnt >= MaxBits )
        {
          ErrorStr = "Coded bits too long!";
          return(0);
        }//end if BitCnt...
        Bits = PutBits(Bits,&BitPos,EOBBits,EOBCode);
//      }//end else...
    }//end for s...
  }//end for lev...

  BitStreamSize = (BitCnt>>3) + 1;
  if(BitStreamSize >= PS.MAX_BITSTREAM_SIZE)
  {
    ErrorStr = "Coded bits too long!";
    return(0);
  }//end if BitStreamSize...

  return(1);
}//end RunLengthCode.

int CIMGCODEC::RunLengthDecode(void)
{
  int i,j,s,lev;
  int pix;
  int run;
  char level;
  int DummyBits;
  int EndOfBlock;

  unsigned int *Bits = (unsigned int *)(PS.BitStream);
  unsigned int BitStreamBits = BitStreamSize * 8; //8bits/byte.
  unsigned int BitCnt = 0; //In Bits.
  int BitPos = 0; //Bit position within the current 32bit word.

  // Decode the Lum texture 1st with 8 bpp.
  fixedpoint *P = Y;
  int LevX = Y_X >> Y_l;
  int LevY = Y_Y >> Y_l;
  int X = Y_X;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      Bits = GetBits(Bits,&BitPos,8,&pix);
      pix = (int)((unsigned char)(pix & 0x000000FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }//end if BitCnt...
      // Inverse Quantize.
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IY_Q[LxLy][Y_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
    }//end if i & j...

  // Decode the Lum Sub-Bands.
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
      else //if(s == HxHy)
        P = (fixedpoint *)(Y + LevY*X + LevX);

      //HxHy level 1 Sub-band is not coded.
//      if( (lev==1)&&(s==HxHy) )
//      {
//        for(i = 0; i < LevY; i++)
//          for(j = 0; j < LevX; j++)
//          {
//            // Fill with zeros.
//            P[i*X + j] = 0;
//          }//end for i & j...
//      }//end if lev...
//      else
//      {
        run = -1;
        level = 0;
        pix = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) //Ready for next run-level pair.
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Error has occurred.
                return(0);
              if((run==0)&&(pix==0)&&(DummyBits==2)) //EOB.
              {
                pix = 0;
                EndOfBlock = 1;
              }//end if run...
            }//end if run...

            if( run || EndOfBlock )
              P[i*X + j] = 0;
            else
            {
              // Inverse Quantize first.
              pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
              pix = (((pix * IY_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              P[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }//end else...
            run--;

          }//end if i & j...
          //Make sure the EOB marker is received.
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
            if((run!=0)||(pix!=0)||(DummyBits!=2)) //Check EOB.
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }//end if run...
          }//end if !EndOfBlock...
//      }//end else...

    }//end for s...
  }//end for lev...

  // Decode the Chr texture 1st with 8 bpp.
  fixedpoint *PU = U;
  fixedpoint *PV = V;
  X = UV_X;
  LevY = UV_Y >> UV_l;
  LevX = X >> UV_l;
  for(i = 0; i < LevY; i++)
    for(j = 0; j < LevX; j++)
    {
      Bits = GetBits(Bits,&BitPos,8,&pix);
      pix = (int)((char)(pix & 0x000000FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }//end if BitCnt...
      // Inverse Quantize.
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IU_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);

      Bits = GetBits(Bits,&BitPos,8,&pix);
      pix = (int)((char)(pix & 0x000000FF));
      BitCnt += 8;
      if(BitCnt > BitStreamBits)
      {
        ErrorStr = "Decoder bit overrun!";
        return(0);
      }//end if BitCnt...
      // Inverse Quantize.
      pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
      pix = (((pix * IV_Q[LxLy][UV_l-1]) >> QFxPt) + QFXROUND) >> QFxPt;
      PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);

    }//end if i & j...

  // Decode the Chr Sub-Bands.
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
      }//end if s...
      else if(s == HxLy)
      {
        PU = (fixedpoint *)(U + LevX);
        PV = (fixedpoint *)(V + LevX);
      }//end if s...
      else //if(s == HxHy)
      {
        PU = (fixedpoint *)(U + LevY*X + LevX);
        PV = (fixedpoint *)(V + LevY*X + LevX);
      }//end else...

      //HxHy level 1 Sub-band is not coded.
//      if( (lev==1)&&(s==HxHy) )
//      {
//        for(i = 0; i < LevY; i++)
//          for(j = 0; j < LevX; j++)
//          {
//            // Fill with zeros.
//            PU[i*X + j] = 0;
//            PV[i*X + j] = 0;
//          }//end for i & j...
//      }//end if lev...
//      else
//      {
        // Chr U.
        run = -1;
        level = 0;
        pix = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) //Ready for next run-level pair.
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Error has occurred.
                return(0);
              if((run==0)&&(pix==0)&&(DummyBits==2)) //EOB.
              {
                pix = 0;
                EndOfBlock = 1;
              }//end if run...
            }//end if run...

            if( run || EndOfBlock )
              PU[i*X + j] = 0;
            else
            {
              // Inverse Quantize first.
              pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
              pix = (((pix * IU_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              PU[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }//end else...
            run--;

          }//end if i & j...
          //Make sure the EOB marker is received.
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
            if((run!=0)||(pix!=0)||(DummyBits!=2)) //Check EOB.
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }//end if run...
          }//end if !EndOfBlock...

        // Chr V.
        run = -1;
        level = 0;
        pix = 0;
        EndOfBlock = 0;
        for(i = 0; i < LevY; i++)
          for(j = 0; j < LevX; j++)
          {
            if((run < 0)&&(!EndOfBlock)) //Ready for next run-level pair.
            {
              Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
              BitCnt += DummyBits;
              if(BitCnt > BitStreamBits)
              {
                ErrorStr = "Decoder bit overrun!";
                return(0);
              }//end if BitCnt...
              if(DummyBits == 0) //Error has occurred.
                return(0);
              if((run==0)&&(pix==0)&&(DummyBits==2)) //EOB.
              {
                pix = 0;
                EndOfBlock = 1;
              }//end if run...
            }//end if run...

            if( run || EndOfBlock )
              PV[i*X + j] = 0;
            else
            {
              // Inverse Quantize first.
              pix = (FIXPOINT(pix)) >> (FxPt - QFxPt);
              pix = (((pix * IV_Q[s][lev-1]) >> QFxPt) + QFXROUND) >> QFxPt;
              PV[i*X + j] = (fixedpoint)((FIXPOINT(pix)) >> 8);
            }//end else...
            run--;

          }//end if i & j...
          //Make sure the EOB marker is received.
          if(!EndOfBlock)
          {
            Bits = GetRunLevel(Bits,&BitPos,&run,&pix,&DummyBits);
            if((run!=0)||(pix!=0)||(DummyBits!=2)) //Check EOB.
            {
              ErrorStr = "Missing EOB marker during decoding!";
              return(0);
            }//end if run...
          }//end if !EndOfBlock...

//      }//end else...

    }//end for s...
  }//end for lev...

  return(1);
}//end RunLengthDecode.

int CIMGCODEC::FindBits(int Run,int Value,
                        int *NumCodeBits,int *CdeWord)
{
  //Parse table with Run-length and value.
  int v = abs(Value);
  int i;
  i = 0;
  BOOL Found;
  Found = FALSE;
  while( (i < TableSize)&&(!Found) )
  {
    //Does run-length match?
    if( Run == VLCTable[i].Run )
    {
      //Does value match?
      if( v == VLCTable[i].Value )
      {
        *NumCodeBits = VLCTable[i].NumBits;
        *CdeWord = VLCTable[i].Code;
        Found = TRUE;
      }//end if Value...
    }//end if Run...
    i++;
  }//end while i...
  //Run-length and value combination was not in table.
  if( !Found )
  {
    *NumCodeBits = ESCAPEBits + MAX_RUN_BITS + 8;
    *CdeWord = 0;
  }//end if !Found...
  return(*NumCodeBits);
}//end FindBits.

/*-------------------------------------------------------------------*/
/* Add BitsToAdd number of bits from the LSB of CodeBits, to the 32  */
/* bit word pointed to by CurrPtr from bit position NextBitPos up    */
/* towards the MSB.                                                  */       
/*-------------------------------------------------------------------*/
unsigned int *CIMGCODEC::PutBits(unsigned int *CurrPtr,
												 int *NextBitPos,int BitsToAdd,
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
    if(pos == 31)
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
}/*end PutBits.*/

/*-------------------------------------------------------------------*/
/* Get the next BitsToGet number of bits and return it in CodeBits.  */
/* This function is for multiple bit accesses.                       */
/*-------------------------------------------------------------------*/
unsigned int *CIMGCODEC::GetBits(unsigned int *CurrPtr,
													int *NextBitPos,int BitsToGet,
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
    if(pos == 31)
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

/*----------------------------------------------------------------------*/
/* Get the next bit and inc the bit counter.                            */
/*----------------------------------------------------------------------*/
unsigned int *CIMGCODEC::GetNextBit(unsigned int *CurrPtr,
												 int *NextBitPos,int *CodeBit)
{
  int pos;

  pos = *NextBitPos;

  /* Strip out the bit. */
  *CodeBit = (int)(*CurrPtr >> pos) & 1;

  /* Point to next available bit.*/
  if(pos == 31)
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

//------------------------------------------------------------
// Run-Level pair extraction.
//------------------------------------------------------------
unsigned int *CIMGCODEC::GetRunLevel(unsigned int *CurrPtr,
												 int *NextBitPos,int *Run, int *Level,
                         int *BitsExtracted)
{
  int bit;
  unsigned int bits;
  int tbl_pos;
  int Found;
  int bits_so_far,bits_needed;

  /* Decode the binary tree to determine the run/level.*/
  /* The table must be in ascending bit length order. Bits */
  /* are extracted from the bit stream depending on the next */
  /* no. of bits in the table. Keep going through the table */
  /* until a match is found or the table ends. */
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
    *Level = (int)((char)(bit | 0xFFFFFF00));
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
}//end GetRunLevel.

unsigned int *CIMGCODEC::FastGetRunLevel(unsigned int *CurrPtr,
												 int *NextBitPos,int *Run, int *Level,
                         int *BitsExtracted)
{
  int bit,i;

  /* Decode the binary tree to determine the run/level.*/
  /* The table is large and inline coding could take forever */
  /* so, the number of leading 0s are counted first. This */
  /* provides a jump start into the binary tree. */

  i = 0;
  do
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    i++;
  }while( !bit && (i < 10) ); /* The max zero code length is 10.*/

  /*If all 0s then there is an error.*/
  if( !bit && (i==10) )
    {
      *Run = 0;
      *Level = 0;
      *BitsExtracted = 0; /* Implies an error. */
			ErrorStr = "Non-existant Huffman code!";
      return(CurrPtr);
    }/*end if bit...*/

  /* At this point there are i bits = 00....01, and (i-1) */
  /* leading 0s. */
  if(i==1)   /* 1 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit) /* 10 */
    {
      *Run = 0;
      *Level = 0;
      *BitsExtracted = 2; /* EOB marker. */
    }/*end if...*/
    else     /* 11 */
    {
      *Run = 0;
      /* Get the sign bit. */
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(bit)
        *Level = -1;
      else
        *Level = 1;
      *BitsExtracted = 3;
    }/*end else...*/
  }/*end if i...*/
  else if(i==2)  /* 01 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(bit)      /* 011 */
    {
      *Run = 1;
      /* Get the sign bit. */
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(bit)
        *Level = -1;
      else
        *Level = 1;
      *BitsExtracted = 4;
    }/*end bit...*/
    else         /* 010 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)      /* 0100 */
      {
        *Run = 0;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -2;
        else
          *Level = 2;
        *BitsExtracted = 5;
      }/*end bit...*/
      else         /* 0101 */
      {
        *Run = 2;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 5;
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==3)  /* 001 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0010 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0010 0 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0010 00 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0010 000 */
          {
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(!bit) /* 0010 0000 */
            {
              *Run = 13;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -1;
              else
                *Level = 1;
              *BitsExtracted = 9;
            }/*end if !bit...*/
            else     /* 0010 0001 */
            {
              *Run = 0;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -6;
              else
                *Level = 6;
              *BitsExtracted = 9;
            }/*end else...*/
          }/*end if !bit...*/
          else     /* 0010 001 */
          {
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(!bit) /* 0010 0010 */
            {
              *Run = 12;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -1;
              else
                *Level = 1;
              *BitsExtracted = 9;
            }/*end if !bit...*/
            else     /* 0010 0011 */
            {
              *Run = 11;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -1;
              else
                *Level = 1;
              *BitsExtracted = 9;
            }/*end else...*/
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0010 01 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0010 010 */
          {
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(!bit) /* 0010 0100 */
            {
              *Run = 3;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -2;
              else
                *Level = 2;
              *BitsExtracted = 9;
            }/*end if !bit...*/
            else     /* 0010 0101 */
            {
              *Run = 1;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -3;
              else
                *Level = 3;
              *BitsExtracted = 9;
            }/*end else...*/
          }/*end if !bit...*/
          else     /* 0010 011 */
          {
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(!bit) /* 0010 0110 */
            {
              *Run = 0;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -5;
              else
                *Level = 5;
              *BitsExtracted = 9;
            }/*end if !bit...*/
            else     /* 0010 0111 */
            {
              *Run = 10;
              /* Get the sign bit. */
              CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
              if(bit)
                *Level = -1;
              else
                *Level = 1;
              *BitsExtracted = 9;
            }/*end else...*/
          }/*end else...*/
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0010 1 */
      {
        *Run = 0;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -3;
        else
          *Level = 3;
        *BitsExtracted = 6;
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0011 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0011 0 */
      {
        *Run = 4;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 6;
      }/*end if !bit...*/
      else       /* 0011 1 */
      {
        *Run = 3;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 6;
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==4)  /* 0001 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0001 0 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0001 00 */
      {
        *Run = 7;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 7;
      }/*end if !bit...*/
      else       /* 0001 01 */
      {
        *Run = 6;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 7;
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0001 1 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0001 10 */
      {
        *Run = 1;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -2;
        else
          *Level = 2;
        *BitsExtracted = 7;
      }/*end if !bit...*/
      else       /* 0001 11 */
      {
        *Run = 5;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 7;
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==5)  /* 0000 1 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0000 10 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 100 */
      {
        *Run = 2;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -2;
        else
          *Level = 2;
        *BitsExtracted = 8;
      }/*end if !bit...*/
      else       /* 0000 101 */
      {
        *Run = 9;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 8;
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0000 11 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 110 */
      {
        *Run = 0;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -4;
        else
          *Level = 4;
        *BitsExtracted = 8;
      }/*end if !bit...*/
      else       /* 0000 111 */
      {
        *Run = 8;
        /* Get the sign bit. */
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(bit)
          *Level = -1;
        else
          *Level = 1;
        *BitsExtracted = 8;
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==6)  /* 0000 01 Escape sequence. */
  {
    CurrPtr = GetBits(CurrPtr,NextBitPos,MAX_RUN_BITS,&bit);
    *Run = bit;
    CurrPtr = GetBits(CurrPtr,NextBitPos,8,&bit);
    *Level = (int)((char)(bit | 0xFFFFFF00));
    *BitsExtracted = 6 + MAX_RUN_BITS + 8;
  }/*end else if i...*/
  else if(i==7)  /* 0000 001 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0000 0010 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0010 0 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0010 00 */
        {
          *Run = 16;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -1;
          else
            *Level = 1;
          *BitsExtracted = 11;
        }/*end if !bit...*/
        else     /* 0000 0010 01 */
        {
          *Run = 5;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -2;
          else
            *Level = 2;
          *BitsExtracted = 11;
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0010 1 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0010 10 */
        {
          *Run = 0;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -7;
          else
            *Level = 7;
          *BitsExtracted = 11;
        }/*end if !bit...*/
        else     /* 0000 0010 11 */
        {
          *Run = 2;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -3;
          else
            *Level = 3;
          *BitsExtracted = 11;
        }/*end else...*/
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0000 0011 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0011 0 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0011 00 */
        {
          *Run = 1;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -4;
          else
            *Level = 4;
          *BitsExtracted = 11;
        }/*end if !bit...*/
        else     /* 0000 0011 01 */
        {
          *Run = 15;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -1;
          else
            *Level = 1;
          *BitsExtracted = 11;
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0011 1 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0011 10 */
        {
          *Run = 14;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -1;
          else
            *Level = 1;
          *BitsExtracted = 11;
        }/*end if !bit...*/
        else     /* 0000 0011 11 */
        {
          *Run = 4;
          /* Get the sign bit. */
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(bit)
            *Level = -2;
          else
            *Level = 2;
          *BitsExtracted = 11;
        }/*end else...*/
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==8)  /* 0000 0001 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0000 0001 0 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0001 00 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0001 000 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 0000 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -11;
            else
              *Level = 11;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 0001 */
          {
            *Run = 8;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -2;
            else
              *Level = 2;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0001 001 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 0010 */
          {
            *Run = 4;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -3;
            else
              *Level = 3;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 0011 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -10;
            else
              *Level = 10;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0001 01 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0001 010 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 0100 */
          {
            *Run = 2;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -4;
            else
              *Level = 4;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 0101 */
          {
            *Run = 7;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -2;
            else
              *Level = 2;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0001 011 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 0110 */
          {
            *Run = 21;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 0111 */
          {
            *Run = 20;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end else...*/
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0000 0001 1 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0001 10 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0001 100 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 1000 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -9;
            else
              *Level = 9;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 1001 */
          {
            *Run = 19;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0001 101 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 1010 */
          {
            *Run = 18;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 1011 */
          {
            *Run = 1;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -5;
            else
              *Level = 5;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0001 11 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0001 110 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 1100 */
          {
            *Run = 3;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -3;
            else
              *Level = 3;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 1101 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -8;
            else
              *Level = 8;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0001 111 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0001 1110 */
          {
            *Run = 6;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -2;
            else
              *Level = 2;
            *BitsExtracted = 13;
          }/*end if !bit...*/
          else     /* 0000 0001 1111 */
          {
            *Run = 17;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 13;
          }/*end else...*/
        }/*end else...*/
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else if(i==9)  /* 0000 0000 1 */
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    if(!bit)     /* 0000 0000 10 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0000 100 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0000 1000 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1000 0 */
          {
            *Run = 10;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -2;
            else
              *Level = 2;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1000 1 */
          {
            *Run = 9;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -2;
            else
              *Level = 2;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0000 1001 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1001 0 */
          {
            *Run = 5;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -3;
            else
              *Level = 3;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1001 1 */
          {
            *Run = 3;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -4;
            else
              *Level = 4;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0000 101 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0000 1010 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1010 0 */
          {
            *Run = 2;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -5;
            else
              *Level = 5;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1010 1 */
          {
            *Run = 1;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -7;
            else
              *Level = 7;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0000 1011 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1011 0 */
          {
            *Run = 1;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -6;
            else
              *Level = 6;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1011 1 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -15;
            else
              *Level = 15;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end else...*/
      }/*end else...*/
    }/*end if !bit...*/
    else         /* 0000 0000 11 */
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      if(!bit)   /* 0000 0000 110 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0000 1100 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1100 0 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -14;
            else
              *Level = 14;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1100 1 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -13;
            else
              *Level = 13;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0000 1101 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1101 0 */
          {
            *Run = 0;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -12;
            else
              *Level = 12;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1101 1 */
          {
            *Run = 26;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end else...*/
      }/*end if !bit...*/
      else       /* 0000 0000 111 */
      {
        CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
        if(!bit) /* 0000 0000 1110 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1110 0 */
          {
            *Run = 25;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1110 1 */
          {
            *Run = 24;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end if !bit...*/
        else     /* 0000 0000 1111 */
        {
          CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
          if(!bit) /* 0000 0000 1111 0 */
          {
            *Run = 23;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 14;
          }/*end if !bit...*/
          else     /* 0000 0000 1111 1 */
          {
            *Run = 22;
            /* Get the sign bit. */
            CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
            if(bit)
              *Level = -1;
            else
              *Level = 1;
            *BitsExtracted = 14;
          }/*end else...*/
        }/*end else...*/
      }/*end else...*/
    }/*end else...*/
  }/*end else if i...*/
  else
  {
    *Run = 0;
    *Level = 0;
    *BitsExtracted = 0; /* Implies an error. */
		ErrorStr = "Non-existant Huffman code!";
  }/*end else...*/

  return(CurrPtr); /* Success.*/
}/*end GetRunLevel.*/

////////// Colour Space conversions ///////////////////

//#define Y_MAX    (fixedpoint)(0.99999999 * FxPt_float)
//#define Y_MIN    (fixedpoint)(0)
////#define Y_MAX    (fixedpoint)(0.9215686 * FxPt_float)
////#define Y_MIN    (fixedpoint)(0.0627451 * FxPt_float)
//#define UV_MAX   (fixedpoint)(0.5 * FxPt_float)
//#define UV_MIN   (fixedpoint)(-0.5 * FxPt_float)
////#define UV_MAX   (fixedpoint)(0.4392 * FxPt_float)
////#define UV_MIN   (fixedpoint)(-0.4392 * FxPt_float)
//
//#define R_Y_0   (fixedpoint)(0.299 * FxPt_float)
//#define R_Y_1   (fixedpoint)(0.587 * FxPt_float)
//#define R_Y_2   (fixedpoint)(0.114 * FxPt_float)
//#define R_Y_3   (fixedpoint)(0.436 * FxPt_float)
//#define R_Y_4   (fixedpoint)(-0.147 * FxPt_float)
//#define R_Y_5   (fixedpoint)(-0.289 * FxPt_float)
//#define R_Y_6   (fixedpoint)(0.615 * FxPt_float)
//#define R_Y_7   (fixedpoint)(-0.515 * FxPt_float)
//#define R_Y_8   (fixedpoint)(-0.100 * FxPt_float)

#define Y_MAX    (fixedpoint)( 8191) /*(0.99999999 * FxPt_float)*/
#define Y_MIN    (fixedpoint)(0)
#define UV_MAX   (fixedpoint)( 4096) /*(0.5 * FxPt_float)       */
#define UV_MIN   (fixedpoint)(-4096) /*(-0.5 * FxPt_float)      */

#define R_Y_0   (fixedpoint)( 2449) /*(0.299 * FxPt_float) */ 
#define R_Y_1   (fixedpoint)( 4809) /*(0.587 * FxPt_float) */ 
#define R_Y_2   (fixedpoint)(  934) /*(0.114 * FxPt_float) */ 
#define R_Y_3   (fixedpoint)( 3572) /*(0.436 * FxPt_float) */ 
#define R_Y_4   (fixedpoint)(-1204) /*(-0.147 * FxPt_float)*/ 
#define R_Y_5   (fixedpoint)(-2367) /*(-0.289 * FxPt_float)*/ 
#define R_Y_6   (fixedpoint)( 5038) /*(0.615 * FxPt_float) */ 
#define R_Y_7   (fixedpoint)(-4219) /*(-0.515 * FxPt_float)*/ 
#define R_Y_8   (fixedpoint)( -819) /*(-0.100 * FxPt_float)*/ 

int CIMGCODEC::RGB24toYUV422(void)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }//end if !CodecIsOpen...

	// Y1 & Y2 have range 0..0.999,
	// U & V have range -0.5..0.5.
	unsigned char *S;
	register int xb,yb,XB,YB;
  int xpix;
	fixedpoint y,u,v;
  fixedpoint accu,accv;
	fixedpoint r,g,b;
  unsigned char BppMask = 0xFF << (8 - PS.Bpp);

	xpix = PS.RGB_X;
	S = (unsigned char *)(PS.pRGB);
	//2 pix per Block.
	XB = xpix >> 1;
	YB = PS.RGB_Y;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		//Left pix.  255->0.999.
		b = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3) & BppMask )) >> 8;
		g = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3 + 1) & BppMask )) >> 8;
		r = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3 + 2) & BppMask )) >> 8;
//    RGBtoYUV(r,g,b,y,accu,accv);
		y = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));
    if(y < Y_MIN) 
      y = Y_MIN;
    if(y > Y_MAX)
     y = Y_MAX;
		Y[yb*xpix + (xb<<1)] = y;

		accu = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
		accv = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));

		//Right pix.
		b = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3 + 3) & BppMask )) >> 8;
		g = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3 + 4) & BppMask )) >> 8;
		r = FIXPOINT((int)( *(S + (yb*xpix + xb*2)*3 + 5) & BppMask )) >> 8;
//    RGBtoYUV(r,g,b,y,u,v);
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
 	}//end for xb & yb...

  return(1);
}//end RGB24toYUV422.

#define Y_R_0   (fixedpoint)( 9339) /*(1.140 * FxPt_float) */ 
#define Y_R_1   (fixedpoint)(-3228) /*(-0.394 * FxPt_float)*/ 
#define Y_R_2   (fixedpoint)(-4756) /*(-0.581 * FxPt_float)*/ 
#define Y_R_3   (fixedpoint)(16646) /*(2.032 * FxPt_float) */ 
#define FXROUND (fixedpoint)( 4096) /*(0.5 * FxPt_float)   */ 

//#define Y_R_0   (fixedpoint)(1.140 * FxPt_float)
//#define Y_R_1   (fixedpoint)(-0.394 * FxPt_float)
//#define Y_R_2   (fixedpoint)(-0.581 * FxPt_float)
//#define Y_R_3   (fixedpoint)(2.032 * FxPt_float)
//#define FXROUND   (fixedpoint)(0.5 * FxPt_float)

int CIMGCODEC::YUV422toRGB24(void)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }//end if !CodecIsOpen...

	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
  int r,b,g;
	fixedpoint y,u,v;

	xpix = PS.RGB_X;
	D = (unsigned char *)(PS.pRGB);
	//2 pix per Block.
	XB = xpix >> 1;
	YB = PS.RGB_Y;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = U[yb*(xpix>>1) + xb];
		v = V[yb*(xpix>>1) + xb];

		//Left pix.
		y = Y[yb*xpix + (xb<<1)];
//    YUVtoRGB(y,u,v,r,g,b);
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
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

		//Right pix.
		y = Y[yb*xpix + (xb<<1) + 1];
//    YUVtoRGB(y,u,v,r,g,b);
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
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
	}//end for xb & yb...

  return(1);
}//end YUV422toRGB24.


