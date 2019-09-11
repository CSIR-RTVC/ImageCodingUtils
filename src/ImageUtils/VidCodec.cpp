/*******************************************************************/
/* TITLE       :DWT VIDEO COMPRESSION CODEC CLASS DEFINITION       */
/*              FILE                                               */
/* VERSION     :1.0                                                */
/* FILE        :VidCodec.cpp                                       */
/* DESCRIPTION :A class for implementing a DWT-domain video        */
/*              compression and decompression codec.               */
/*              A RGB24/YUV411 video is coded into a bit stream    */
/*              and the bit stream is decoded into a RGB24/YUV411  */
/*              image.                                             */
/* DATE        :May 1999                                           */
/* AUTHOR      :K.L.Ferguson                                       */
/*******************************************************************/
#include "stdafx.h"

#include "VidCodec.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************/
/* Quantisation Tables.                                    */
/***********************************************************/

#define QFXROUND 64 /*(int)(0.5 * QFxPt_float)*/

/* Psychovisual DWT table constructed for a 15" monitor at */
/* 800x600 viewed at 0.6m up to 6 DWT levels.              */

/*		            Level					                             */
/*Colour	Orient	1	     2	    3	     4	    5	     6       */
/*Y	      LxLy	   4.176  1.658	 0.800	0.469	 0.334	0.288  */
/*	      LxHy	   6.389  2.264	 0.975	0.510	 0.324	0.250  */
/*	      HxLy	   6.389  2.264	 0.975	0.510	 0.324	0.250  */
/*	      HxHy	  14.668  4.359	 1.574	0.690	 0.367	0.238  */
/*							                                             */
/*U	      LxLy	  17.194  7.071	 3.371	1.862	 1.192	0.884  */
/*	      LxHy	  25.589  9.632	 4.201	2.123	 1.243	0.843  */
/*	      HxLy	  25.589  9.632	 4.201	2.123	 1.243	0.843  */
/*	      HxHy	  58.255 18.939	 7.134	3.114	 1.575	0.923  */
/*							                                             */
/*V	      LxLy	   7.310  2.867	 1.398	0.847	 0.638	0.597  */
/*	      LxHy	  15.552  5.015	 2.010	1.001	 0.620	0.477  */
/*	      HxLy	  15.552  5.015	 2.010	1.001	 0.620	0.477  */
/*	      HxHy	  42.143 11.043	 3.596	1.456	 0.732	0.458  */

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

/****************************************************************/
/* VLC definitions and tables for coding the run-length         */
/* run value.                                                   */
/****************************************************************/
#define CVIDCODEC_RUN_VLC_TABLE_SIZE 32
#define EOSB_INDEX 32
#define EOI_INDEX 33
#define CVIDCODEC_EOSBBits 11
#define CVIDCODEC_EOSBCode 0x00000340
#define CVIDCODEC_EOIBits 11
#define CVIDCODEC_EOICode 0x00000740

#define CVIDCODEC_RunEOIBits 11
#define CVIDCODEC_RunEOICode 0x00000740 /* <- LSB unique.  0x00000017 MSB unique ->. */
#define CVIDCODEC_RunEOBBits 11
#define CVIDCODEC_RunEOBCode 0x00000340 /* <- LSB unique.  0x00000016 MSB unique ->. */

#define CVIDCODEC_RunEscBits 5
#define CVIDCODEC_RunEscCode 0x00000018 /* <- LSB unique.  0x00000003 MSB unique ->. */
#define CVIDCODEC_RunEscValBits 9
#define CVIDCODEC_RunEscValMask 0x000001FF

static VLC_TYPE CVIDCODEC_RunVlcTable[CVIDCODEC_RUN_VLC_TABLE_SIZE + 2] =
{ /* <- LSB unique.           MSB unique ->. */
  { 1, 0x00000001},           /*  0x00000001},  //0  */
  { 3, 0x00000002},           /*  0x00000002},  //1  */
  { 3, 0x00000006},           /*  0x00000003},  //2  */
  { 4, 0x00000004},           /*  0x00000002},  //3  */
  { 4, 0x0000000C},           /*  0x00000003},  //4  */
  { 5, 0x00000008},           /*  0x00000002},  //5  */
  { 7, 0x00000030},           /*  0x00000006},  //6  */
  { 7, 0x00000070},           /*  0x00000007},  //7  */
  { 8, 0x00000050},           /*  0x0000000A},  //8  */
  { 8, 0x000000D0},           /*  0x0000000B},  //9  */
  { 8, 0x00000010},           /*  0x00000008},  //10 */
  { 8, 0x00000090},           /*  0x00000009},  //11 */
  { 8, 0x00000060},           /*  0x00000006},  //12 */
  { 8, 0x000000E0},           /*  0x00000007},  //13 */
  {10, 0x000001A0},           /*  0x00000016},  //14 */
  {10, 0x000003A0},           /*  0x00000017},  //15 */
  {10, 0x000000A0},           /*  0x00000014},  //16 */
  {10, 0x000002A0},           /*  0x00000015},  //17 */
  {10, 0x00000120},           /*  0x00000012},  //18 */
  {10, 0x00000320},           /*  0x00000013},  //19 */
  {11, 0x00000220},           /*  0x00000022},  //20 */
  {11, 0x00000620},           /*  0x00000023},  //21 */
  {11, 0x00000020},           /*  0x00000020},  //22 */
  {11, 0x00000420},           /*  0x00000021},  //23 */
  {11, 0x000003C0},           /*  0x0000001E},  //24 */
  {11, 0x000007C0},           /*  0x0000001F},  //25 */
  {11, 0x000001C0},           /*  0x0000001C},  //26 */
  {11, 0x000005C0},           /*  0x0000001D},  //27 */
  {11, 0x000002C0},           /*  0x0000001A},  //28 */
  {11, 0x000006C0},           /*  0x0000001B},  //29 */
  {11, 0x000000C0},           /*  0x00000018},  //30 */
  {11, 0x000004C0},           /* 0x00000019},   //31 */
  {11, 0x00000340},           /* 0x00000016},   //32 */
  {11, 0x00000740}            /* 0x00000017}   //33  */
};

/****************************************************************/
/* VLC definitions and tables for coding the motion vector value.*/
/****************************************************************/
#define CVIDCODEC_MOTIONVEC_VLC_TABLE_SIZE 18
/* The table excludes the sign bit. */
static VLC_TYPE CVIDCODEC_MotionVlcTable[CVIDCODEC_MOTIONVEC_VLC_TABLE_SIZE] =
{ /* <- LSB unique.           MSB unique ->. */
  { 1, 0x00000001},                   /* 1}, //0.      0.           */
  { 2, 0x00000002},                   /* 1}, //+1 -1.  +0.5 -0.5    */
  { 3, 0x00000004},                   /* 1}, //+2 -2.  +1   -1      */
  { 4, 0x00000008},                   /* 1}, //+3 -3.  +1.5 -1.5    */
  { 6, 0x00000030},                   /* 3}, //+4 -4.  +2   -2      */
  { 7, 0x00000050},                   /* 5}, //+5 -5.  +2.5 -2.5    */
  { 7, 0x00000010},                   /* 4}, //+6 -6.  +3   -3      */
  { 7, 0x00000060},                   /* 3}, //+7 -7.  +3.5 -3.5    */
  { 9, 0x000001A0},                   /*11}, //+8 -8.  +4   -4      */
  { 9, 0x000000A0},                   /*10}, //+9 -9.  +4.5 -4.5    */
  { 9, 0x00000120},                   /* 9}, //+10-10. +5   -5      */
  {10, 0x00000220},                   /*17}, //+11-11. +5.5 -5.5    */
  {10, 0x00000020},                   /*16}, //+12-12. +6   -6      */
  {10, 0x000003C0},                   /*15}, //+13-13. +6.5 -6.5    */
  {10, 0x000001C0},                   /*14}, //+14-14. +7   -7      */
  {10, 0x000002C0},                   /*13}, //+15-15. +7.5 -7.5    */
  {10, 0x000000C0},                   /*12}, //+16-16. +8   -8      */
  {10, 0x00000340}                    /*11}  //+17-17. +8.5 -8.5     */
};

#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L1LxHy4x4x1_9.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L1HxHy4x4x1_3.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L1HxLy4x4x1_9.h"

#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L2LxHy2x2x1_0.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L2HxHy2x2x1_4.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L2HxLy2x2x1_1.h"

#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L3LxHy2x2x1_25.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L3HxHy2x2x1_5.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L3HxLy2x2x1_30.h"

#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM32L4LxLy1x3x1_2.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L4LxHy1x3x1_18.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM8L4HxHy1x3x1_4.h"
#include "..\VWbaseCodec\CodecHeaders\D_Y411HCSOM16L4HxLy1x3x1_26.h"

#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L1LxHy4x4x1_0.h"
/*#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L1HxHy4x4x1_0.h"*/
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L1HxLy4x4x1_0.h"

#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L2LxHy2x2x1_5.h"
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L2HxHy2x2x1_1.h"
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L2HxLy2x2x1_8.h"

#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM16L3LxLy1x3x1_6.h"
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L3LxHy1x3x1_7.h"
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L3HxHy1x3x1_0.h"
#include "..\VWbaseCodec\CodecHeaders\D_U411HCSOM8L3HxLy1x3x1_7.h"

#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L1LxHy4x4x1_0.h"
/*#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L1HxHy4x4x1_0.h"*/
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L1HxLy4x4x1_0.h"

#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L2LxHy2x2x1_10.h"
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L2HxHy2x2x1_1.h"
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L2HxLy2x2x1_7.h"

#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM16L3LxLy1x3x1_10.h"
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L3LxHy1x3x1_8.h"
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L3HxHy1x3x1_0.h"
#include "..\VWbaseCodec\CodecHeaders\D_V411HCSOM8L3HxLy1x3x1_3.h"

/*/////////////////////////////////////////////////////////////*/
/*/ Fast lifting filter 9 - 7.                                 */
/*/////////////////////////////////////////////////////////////*/

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

#define MINUS_HALF (fixedpoint)(-4096) /*(-0.5 * FxPt_float) */
#define HALF       (fixedpoint)( 4096) /*(0.5 * FxPt_float)  */     

/*//////////////////////////////////////////////////////////*/
/*/ Construction and destruction.                           */
/*//////////////////////////////////////////////////////////*/

CVIDCODEC::CVIDCODEC()
{
  hL_b = NULL;
  L_b = NULL;
  hY = NULL;
  hU = NULL;
  hV = NULL;
  ErrorStr = "No Erorr";
  BitStream = NULL;
  BitStreamSize = 0;
  CodecIsOpen = 0;

  PS.Q = 1;
  PS.Thresh = 0;
  PS.Bpp = 8;
  PS.pRGB = NULL;
  PS.RGB_X = 0;
  PS.RGB_Y = 0;
  PS.MAX_BITSTREAM_SIZE = 0;
  PS.Motion = 0;
  PS.MotionLevel = 0;
  PS.ColourMotion = 0;

  /*Work image.*/
  Y = NULL; 
  Y_X = 0;
  Y_Y = 0;
  U = NULL;
  V = NULL;
  UV_X = 0;
  UV_Y = 0;
  /*Reference image.*/
  hrY = NULL;
  hrU = NULL;
  hrV = NULL;
  rY = NULL;
  rU = NULL;
  rV = NULL;
  /*Compensated reference image.*/
  hcrY = NULL;
  hcrU = NULL;
  hcrV = NULL;
  crY = NULL;
  crU = NULL;
  crV = NULL;

  pGroup = NULL;
  GroupLen = 0;

}/*end constructor.*/

CVIDCODEC::~CVIDCODEC()
{
  /*If open then close first before exiting.*/
  if(CodecIsOpen)
    Close();
}/*end destructor.*/

/*//////////////////////////////////////////////////////////*/
/*/ Public Implementation.                                  */
/*//////////////////////////////////////////////////////////*/

int CVIDCODEC::Open(CVIDCODEC_STRUCT *Params)
{
  int i,j;

  /*If already open then close first before continuing.*/
  if(CodecIsOpen)
    Close();

  /*Update to the new parameters.*/
  PS = *Params;

  /*Create a YUV411 memory space.*/
	hY = GlobalAlloc(GMEM_FIXED,(PS.RGB_X * PS.RGB_Y * sizeof(fixedpoint)));
	if(!hY)
  {
    ErrorStr = "Luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hY...*/
	Y = (fixedpoint *)GlobalLock(hY);
  Y_X = PS.RGB_X;
  Y_Y = PS.RGB_Y;
  UV_X = PS.RGB_X/2;
  UV_Y = PS.RGB_Y/2;
	hU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hU)
  {
    ErrorStr = "U Chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hU...*/
	U = (fixedpoint *)GlobalLock(hU);
	hV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hV)
  {
    ErrorStr = "V Chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hV...*/
	V = (fixedpoint *)GlobalLock(hV);

  /*Create a reference YUV411 memory space.*/
	hrY = GlobalAlloc(GMEM_FIXED,(Y_X * Y_Y * sizeof(fixedpoint)));
	if(!hrY)
  {
    ErrorStr = "Reference luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrY...*/
	rY = (fixedpoint *)GlobalLock(hrY);
	hrU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hrU)
  {
    ErrorStr = "Reference U chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrU...*/
	rU = (fixedpoint *)GlobalLock(hrU);
	hrV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hrV)
  {
    ErrorStr = "Reference V chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrV...*/
	rV = (fixedpoint *)GlobalLock(hrV);

  /*Create a compensated reference YUV411 memory space.*/
	hcrY = GlobalAlloc(GMEM_FIXED,(Y_X * Y_Y * sizeof(fixedpoint)));
	if(!hcrY)
  {
    ErrorStr = "Compensated reference luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrY...*/
	crY = (fixedpoint *)GlobalLock(hcrY);
	hcrU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hcrU)
  {
    ErrorStr = "Compensated reference U chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrU...*/
	crU = (fixedpoint *)GlobalLock(hcrU);
	hcrV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(fixedpoint)));
	if(!hcrV)
  {
    ErrorStr = "Compensated reference V chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrV...*/
	crV = (fixedpoint *)GlobalLock(hcrV);

  /*Zero the reference and the compensated reference image.*/
  int size = Y_X * Y_Y;
  for(i = 0; i < size; i++)
  {
    rY[i] = 0;
    crY[i] = 0;
  }/*end for size...*/
  size = UV_X * UV_Y;
  for(i = 0; i < size; i++)
  {
    rU[i] = 0;
    rV[i] = 0;
    crU[i] = 0;
    crV[i] = 0;
  }/*end for i...*/

  /*Establish the DWT levels.*/
  Y_l = GetMaxLevels(Y_X,Y_Y);
  UV_l = GetMaxLevels(UV_X,UV_Y);

  /*Construct the quantization tables.*/
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
      }/*end if Q...*/
      else
      {
        IY_Q[i][j] = (int)(1.0 * QFxPt_float);
        Y_Q[i][j] = (int)(1.0 * QFxPt_float);
        IU_Q[i][j] = (int)(1.0 * QFxPt_float);
        U_Q[i][j] = (int)(1.0 * QFxPt_float);
        IV_Q[i][j] = (int)(1.0 * QFxPt_float);
        V_Q[i][j] = (int)(1.0 * QFxPt_float);
      }/*end else...*/
    }/*end for i & j...*/

	/*Allocate DWT working memory as a 2 line image line buffer.*/
	hL_b = GlobalAlloc(GMEM_FIXED,(2 * Y_X * sizeof(fixedpoint)));
	if(!hL_b)
  {
    ErrorStr = "Working memory unavailable!";
    Close();
	  return(0);
  }/*end if !hL_b...*/
	L_b = (fixedpoint *)GlobalLock(hL_b);

  /*Construct the operational sub-band parameter structure.*/
  if(!ConstructSubBndInfo())
  {
    /*ErrorStr is set within the function.*/
    Close();
	  return(0);
  }/*end if !ConstructSubBndInfo...*/

  /*Clear the partition bit positions.*/
  for(i = 0; i < LEV4YUV411_ORDER_LENGTH; i++)
    PartitionBitPos[i] = 0;

  ErrorStr = "No Erorr";
  CodecIsOpen = 1;
  return(1);
}/*end Open.*/

int CVIDCODEC::Code(void *pI,void *pCmp,unsigned int FrameBitLimit)
{
  int grp,subgrp;
  int RunOutOfBits,finished;
  int CodeBits,RunCodeBits,MXCodeBits,MYCodeBits,BitPos;
  int VqBits,RunBits,MXBits,MYBits;
  unsigned int Bits,BitLimit;
  SUBBND_INFO_TYPE *pInfo;
  int DoMotion,DoVectors,SubGrpDone;
  float SubGrpRatio;
  int i,cnt;

  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/
  if(FrameBitLimit > PS.MAX_BITSTREAM_SIZE)
  {
    ErrorStr = "Frame bits exceed memory capacity!";
    return(0);
  }/*end if FrameBitLimit...*/

  /*Image to compress must be 24 bit RGB.*/
  PS.pRGB = pI;
  BitStream = (unsigned int *)pCmp;

  RGB24toYUV411(PS.pRGB, PS.RGB_X, PS.RGB_Y, Y, U, V);

  Dwt2D(Y,U,V);

  /* Process each sub-group within each group in the correct order.*/
  RunOutOfBits = 0;
  BitStreamSize = 0;
  BitPos = 0;
  BitLimit = FrameBitLimit-16; /*Allow some slack*/
  for(grp = 0; (grp < GroupLen) && !RunOutOfBits; grp++)
  {
    /*Process all the vectors within each wavelet sub-band and */
    /*collect motion vectors and run length lists before bit coding. */
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      CodeSubBnd(&(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]));
    }/*end for subgrp...*/

    /*Now bit code the motion vectors and runlength lists until */
    /*run out of bits. */
    /*Mark the vector pos of no more bits and put zeros in the  */
    /*ref image up to this mark in all remaining sub-bands. */
    /*Pick out runlength collections of motion vectors and */
    /*quantised diff vectors. */
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
      pInfo->VqRunLenPos = 0;
      pInfo->MotionRunPos = 0;
      pInfo->MotionDone = 0;
      pInfo->VqDone = 0;
    }/*end for subgrp...*/
    finished = 0;
    while(!finished)
    {
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
  
        /*EOSB marker code for motion run.*/
        if(!RunOutOfBits && pInfo->MotionEstimationRequired && !pInfo->MotionDone && 
           (pInfo->MotionRunPos >= pInfo->MotionRunLength))
        {
          pInfo->MotionDone = 1;
          /*Add EOSB bits to bit stream.*/
          BitStream = PutBits(BitStream,&BitPos,CVIDCODEC_EOSBBits,CVIDCODEC_EOSBCode);
          BitStreamSize += CVIDCODEC_EOSBBits;
        }/*end if !RunOutOfBits ...*/

        DoMotion = !RunOutOfBits && pInfo->MotionEstimationRequired && 
                   !pInfo->MotionDone;
        if(DoMotion)
        {
          RunBits = GetRunBits(pInfo->MotionRun[pInfo->MotionRunPos],&RunCodeBits);
          MXBits = GetMotionVecBits(pInfo->MotionVec[pInfo->MotionRunPos].x,&MXCodeBits);
          MYBits = GetMotionVecBits(pInfo->MotionVec[pInfo->MotionRunPos].y,&MYCodeBits);
          Bits = RunBits + MXBits + MYBits;
          if(!RunBits || !MXBits || !MYBits)
            return(0);
          if((BitStreamSize + Bits) < BitLimit)
          {
            /*Add the coded bits to the bit stream.*/
            BitStream = PutBits(BitStream,&BitPos,RunBits,RunCodeBits);
            BitStream = PutBits(BitStream,&BitPos,MXBits,MXCodeBits);
            BitStream = PutBits(BitStream,&BitPos,MYBits,MYCodeBits);
            BitStreamSize += Bits;
            pInfo->MotionRunPos++;
          }/*end if BitStreamSize...*/
          else
          {
            RunOutOfBits = 1;
            /*Add EOI bits to bit stream.*/
            BitStream = PutBits(BitStream,&BitPos,CVIDCODEC_EOIBits,CVIDCODEC_EOICode);
            BitStreamSize += CVIDCODEC_EOIBits;
            cnt = 0;
            for(i = 0; i < pInfo->VqRunLenLength; i++)
              cnt += (pInfo->VqRunLen[i].run + 1);
            SubGrpRatio = ((float)cnt)/((float)(pInfo->SeqLength));
          }/*end else...*/
        }/*end if DoMotion...*/

        /*EOSB marker code for vq run.*/
        if(!RunOutOfBits && !pInfo->VqDone && 
           (pInfo->VqRunLenPos >= pInfo->VqRunLenLength))
        {
          pInfo->VqDone = 1;
          /*Add EOSB bits to bit stream.*/
          BitStream = PutBits(BitStream,&BitPos,CVIDCODEC_EOSBBits,CVIDCODEC_EOSBCode);
          BitStreamSize += CVIDCODEC_EOSBBits;
        }/*end if !RunOutOfBits ...*/

        DoVectors = !RunOutOfBits && !pInfo->VqDone;
        if(DoVectors)
        {
          RunBits = GetRunBits(pInfo->VqRunLen[pInfo->VqRunLenPos].run,&RunCodeBits);
          VqBits = GetVecBits(&(pInfo->Vq),pInfo->VqRunLen[pInfo->VqRunLenPos].val,&CodeBits);
          Bits = RunBits + VqBits;
          if(!RunBits || !VqBits)
            return(0);
          if((BitStreamSize + Bits) < BitLimit)
          {
            /*Add the coded bits to the bit stream.*/
            BitStream = PutBits(BitStream,&BitPos,RunBits,RunCodeBits);
            BitStream = PutBits(BitStream,&BitPos,VqBits,CodeBits);
            BitStreamSize += Bits;
            pInfo->VqRunLenPos++;
          }/*end if BitStreamSize...*/
          else
          {
            RunOutOfBits = 1;
            /*Add EOI bits to bit stream.*/
            BitStream = PutBits(BitStream,&BitPos,CVIDCODEC_EOIBits,CVIDCODEC_EOICode);
            BitStreamSize += CVIDCODEC_EOIBits;
            cnt = 0;
            for(i = 0; i < pInfo->VqRunLenPos; i++)
              cnt += (pInfo->VqRunLen[i].run + 1);
            SubGrpRatio = ((float)cnt)/((float)(pInfo->SeqLength));
          }/*end else...*/
        }/*end if DoVectors...*/
      }/*end for subgrp...*/
      /*Check if all subgrps are finished.*/
      SubGrpDone = 1;
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
        if(pInfo->MotionEstimationRequired)
          SubGrpDone = SubGrpDone && pInfo->MotionDone;
        SubGrpDone = SubGrpDone && pInfo->VqDone;
      }/*end for subgrp...*/
      finished = RunOutOfBits || SubGrpDone;

    }/*end while !finished...*/

    /*Decode to the reference image.*/
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
      /*Truncate the motion vector list and the runlength */
      /*vector list for this subgrp.*/
      pInfo->MotionRunLength = pInfo->MotionRunPos;
      pInfo->VqRunLenLength = pInfo->VqRunLenPos;
      DecodeSubBndRef(pInfo);
    }/*end for subgrp...*/

    /*Mark the partition bit position.*/
    PartitionBitPos[grp] = BitStreamSize;

  }/*end for grp...*/

  /*Zero the reference for all remaining sub-groups up to the */
  /*last valid runlength position.*/
  if(RunOutOfBits)
  {
    for(; grp < GroupLen; grp++)
    {
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
        ZeroRefSubBnd(pInfo,(int)(SubGrpRatio * (float)(pInfo->SeqLength)));
      }/*end for subgrp...*/

      /*Mark the partition bit position.*/
      PartitionBitPos[grp] = BitStreamSize;

    }/*end for grp...*/
  }/*end if RunOutOfBits...*/
  else
  {
    /*Put EOI marker code.*/
    BitStream = PutBits(BitStream,&BitPos,CVIDCODEC_EOIBits,CVIDCODEC_EOICode);
    BitStreamSize += CVIDCODEC_EOIBits;
  }/*end else...*/

  return(1);
}/*end Code.*/

int CVIDCODEC::Decode(void *pCmp,unsigned int FrameBitSize,void *pI)
{
  int grp,subgrp;
  int RunOutOfBits,finished,MarkerFound;
  int VqIndex,Run,MX,MY,BitPos;
  int VqBits,RunBits,MXBits,MYBits;
  int extractbits;
  SUBBND_INFO_TYPE *pInfo;
  int DoMotion,DoVectors,SubGrpDone;
  float SubGrpRatio;
  int i,j,cnt;

  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

  BitStream = (unsigned int *)pCmp;
  BitStreamSize = FrameBitSize;

  /*Decompresed image must be 24 bit RGB.*/
  PS.pRGB = pI;

  /*Extract bits from the input stream and construct the valid */
  /*data structures for motion compensation and vq decoding.*/
  /* Process each sub-group within each group in the correct order.*/
  RunOutOfBits = 0;
  BitPos = 0;
  for(grp = 0; (grp < GroupLen) && !RunOutOfBits; grp++)
  {
    /*Now decode the motion vectors and runlength lists from the */
    /*input bit stream until run out of bits. */
    /*Pick out runlength collections of motion vectors and */
    /*quantised diff vectors. */
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
      pInfo->MotionRunLength = 0;
      pInfo->VqRunLenLength = 0;
      pInfo->MotionDone = 0;
      pInfo->VqDone = 0;
    }/*end for subgrp...*/
    finished = 0;
    while(!finished)
    {
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);

        DoMotion = !RunOutOfBits && pInfo->MotionEstimationRequired && 
                   !pInfo->MotionDone;
        if(DoMotion)
        {
          /*If expecting a motion run, then extract it from the bit stream */
          /* and check that it is not an EOSB or EOI marker.*/
          BitStream = ExtractRunBits(BitStream,&BitPos,&Run,&RunBits,&MarkerFound,&extractbits);
          FrameBitSize -= RunBits;
          if(RunBits == 0)
          {
            /*Loss of bit stream sync, catastrophic failure.*/
            return(0);
          }/*end if RunBits...*/
          if(MarkerFound)
          {
            pInfo->MotionDone = 1; /*Default is Run == EOSBIndex.*/
            if(Run == EOI_INDEX)
            {
              RunOutOfBits = 1;
              cnt = 0;
              for(i = 0; i < pInfo->VqRunLenLength; i++)
                cnt += (pInfo->VqRunLen[i].run + 1);
              SubGrpRatio = ((float)cnt)/((float)(pInfo->SeqLength));
            }/*end if Run...*/
          }/*end if MarkerFound...*/
          else
          {
            pInfo->MotionRun[pInfo->MotionRunLength] = Run;
            /*Extract the motion vector and update the motion run length struct.*/
            BitStream = ExtractMotionVecBits(BitStream,&BitPos,&MX,&MXBits,&extractbits);
            BitStream = ExtractMotionVecBits(BitStream,&BitPos,&MY,&MYBits,&extractbits);
            FrameBitSize -= (MXBits + MYBits);
            if( (MXBits == 0)||(MYBits == 0) )
            {
              /*Loss of bit stream sync, catastrophic failure.*/
              return(0);
            }/*end if MXBits...*/

            pInfo->MotionVec[pInfo->MotionRunLength].x = MX;
            pInfo->MotionVec[pInfo->MotionRunLength].y = MY;
            pInfo->MotionRunLength++;
          }/*end else...*/
        }/*end if DoMotion...*/

        DoVectors = !RunOutOfBits && !pInfo->VqDone;
        if(DoVectors)
        {
          /*If expecting a vq run, then extract it from the bit stream */
          /* and check that it is not an EOSB or EOI marker.*/
          BitStream = ExtractRunBits(BitStream,&BitPos,&Run,&RunBits,&MarkerFound,&extractbits);
          FrameBitSize -= RunBits;
          if(RunBits == 0)
          {
            /*Loss of bit stream sync, catastrophic failure.*/
            return(0);
          }/*end if RunBits...*/
          if(MarkerFound)
          {
            pInfo->VqDone = 1; /*Default is Run == EOSBIndex.*/
            if(Run == EOI_INDEX)
            {
              RunOutOfBits = 1;
              cnt = 0;
              for(i = 0; i < pInfo->VqRunLenLength; i++)
                cnt += (pInfo->VqRunLen[i].run + 1);
              SubGrpRatio = ((float)cnt)/((float)(pInfo->SeqLength));
            }/*end if Run...*/
          }/*end if MarkerFound...*/
          else
          {
            pInfo->VqRunLen[pInfo->VqRunLenLength].run = Run;
            /*Extract the vq index and update the vq run length struct.*/
            BitStream = ExtractVqIndexBits(BitStream,&BitPos,&(pInfo->Vq),
                                           &VqIndex,&VqBits,&extractbits);
            FrameBitSize -= VqBits;
            if(VqBits == 0)
            {
              /*Loss of bit stream sync, catastrophic failure.*/
              return(0);
            }/*end if VqBits...*/
            pInfo->VqRunLen[pInfo->VqRunLenLength].val = VqIndex;
            pInfo->VqRunLenLength++;
          }/*end else...*/
        }/*end if DoVectors...*/
      }/*end for subgrp...*/
      /*Check if all subgrps are finished.*/
      SubGrpDone = 1;
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
        if(pInfo->MotionEstimationRequired)
          SubGrpDone = SubGrpDone && pInfo->MotionDone;
        SubGrpDone = SubGrpDone && pInfo->VqDone;
      }/*end for subgrp...*/
      finished = RunOutOfBits || SubGrpDone;

    }/*end while !finished...*/

    /*Decode to the reference image.*/
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      /*Decode each sub-band by motion compensating to the CRef and*/
      /*then adding to the decoded vq and writing to the Ref.*/
      DecodeSubBnd(&(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]));
    }/*end for subgrp...*/

  }/*end for grp...*/

  /*Zero the reference for all remaining sub-groups up to the */
  /*last valid runlength position.*/
  if(RunOutOfBits)
  {
    for(; grp < GroupLen; grp++)
    {
      for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
      {
        pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
        ZeroRefSubBnd(pInfo,(int)(SubGrpRatio * (float)(pInfo->SeqLength)));
      }/*end for subgrp...*/
    }/*end for grp...*/
  }/*end if RunOutOfBits...*/

  /*Inverse psycho quantise through to the output YUV image.*/
  int seq,ilim,jlim;
  fixedpoint *Dst;
  fixedpoint *Ref;
  int X,IQ,Off;

  for(grp = 0; grp < GroupLen; grp++)
  {
    for(subgrp = 0; subgrp < pGroup[grp].NumOfGroupSubBnds; subgrp++)
    {
      pInfo = &(SubBndInfo[pGroup[grp].SubBndGroup[subgrp]]);
      if(pInfo->Colour == Col_Y)
      {
        X = Y_X;
        IQ = IY_Q[pInfo->Orient][pInfo->Level - 1];
      }/*end if Colour...*/
      else
      {
        X = UV_X;
        if(pInfo->Colour == Col_U)
          IQ = IU_Q[pInfo->Orient][pInfo->Level - 1];
        else
          IQ = IV_Q[pInfo->Orient][pInfo->Level - 1];
      }/*end else...*/

      ilim = pInfo->Vq.vy; /*Vector rows.*/
      jlim = pInfo->Vq.vx; /*Vector cols.*/
      for(seq = 0; seq < pInfo->SeqLength; seq++)
      {
        Off = (pInfo->SubBndSeq[seq].Y * X) + pInfo->SubBndSeq[seq].X;
        Ref = (fixedpoint *)(pInfo->pRPel + Off); /*Reference vector.*/
        Dst = (fixedpoint *)(pInfo->pPel + Off); /*Destination vector.*/
        for(i = 0; i < ilim; i++)
        {
          int rowaddr = i*X;
          for(j = 0; j < jlim; j++)
          {
            /*Write through the ref image and inverse psycho quantise */
            /*to the YUV storage space.*/
            Dst[rowaddr + j] = (fixedpoint)
                               (((((((int)Ref[rowaddr + j]) << 8) * IQ) >> 8) + 
                               QFXROUND) >> 3);
          }/*end for j...*/
        }/*end for i...*/
      }/*end for seq...*/
    }/*end for subgrp...*/
  }/*end for grp...*/

  IDwt2D(Y,U,V);

  YUV411toRGB24(Y, U, V, PS.pRGB, PS.RGB_X, PS.RGB_Y);

  return(1);
}/*end Decode.*/

void CVIDCODEC::Close(void)
{
  /*Free the working memory.*/
	if(hL_b != NULL)
	{
		GlobalUnlock(hL_b);
 		GlobalFree(hL_b);
 		hL_b = NULL;
	}/*end if hL_b...*/
	L_b = NULL;
	if(hY != NULL)
	{
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
	}/*end if hY...*/
	Y = NULL;
	if(hU != NULL)
	{
		GlobalUnlock(hU);
 		GlobalFree(hU);
 		hU = NULL;
	}/*end if hU...*/
	U = NULL;
	if(hV != NULL)
	{
		GlobalUnlock(hV);
 		GlobalFree(hV);
 		hV = NULL;
	}/*end if hV...*/
	V = NULL;

	if(hrY != NULL)
	{
		GlobalUnlock(hrY);
 		GlobalFree(hrY);
 		hrY = NULL;
	}/*end if hrY...*/
	rY = NULL;
	if(hrU != NULL)
	{
		GlobalUnlock(hrU);
 		GlobalFree(hrU);
 		hrU = NULL;
	}/*end if hrU...*/
	rU = NULL;
	if(hrV != NULL)
	{
		GlobalUnlock(hrV);
 		GlobalFree(hrV);
 		hrV = NULL;
	}/*end if hrV...*/
	rV = NULL;

	if(hcrY != NULL)
	{
		GlobalUnlock(hcrY);
 		GlobalFree(hcrY);
 		hcrY = NULL;
	}/*end if hcrY...*/
	crY = NULL;
	if(hcrU != NULL)
	{
		GlobalUnlock(hcrU);
 		GlobalFree(hcrU);
 		hcrU = NULL;
	}/*end if hcrU...*/
	crU = NULL;
	if(hcrV != NULL)
	{
		GlobalUnlock(hcrV);
 		GlobalFree(hcrV);
 		hcrV = NULL;
	}/*end if hcrV...*/
	crV = NULL;

  CodecIsOpen = 0;
}/*end Close.*/

void CVIDCODEC::ZeroRefPartition(int partition)
{
  if(partition >= GroupLen)
    return;

  SUBBND_INFO_TYPE *pInfo;
  for(int subgrp = 0; subgrp < pGroup[partition].NumOfGroupSubBnds; subgrp++)
  {
    pInfo = &(SubBndInfo[pGroup[partition].SubBndGroup[subgrp]]);
    ZeroRefSubBnd(pInfo,pInfo->SeqLength);
  }/*end for subgrp...*/
}//end ZeroRefPartition.

/*//////////////////////////////////////////////////////////*/
/* Private Implementation.                                  */
/*//////////////////////////////////////////////////////////*/

/* Get the number of possible wavelet levels for these   */
/* image dimensions. i.e. the level at which the texture */
/* image will have an odd dimension or less than 16.     */
int CVIDCODEC::GetMaxLevels(int x,int y)
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

/////////////////////////////////////////////////////////////////
// In-place 2-dim DWT of the local YUV image pointed to by the 
// input parameters. The dim of the YUV image is assumed to be
// that of the parameter list.
/////////////////////////////////////////////////////////////////
void CVIDCODEC::Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
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
void CVIDCODEC::IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV)
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
void CVIDCODEC::Dwt1D(fixedpoint *x,int Len,fixedpoint *y_Low,fixedpoint *y_High)
{
  int m,n;
  int SubLen = Len>>1;

  register long r,s;
  register fixedpoint a,b,c,d,e,f,g,h,k;

  //The boundary wrapping is done in line.

  ////////////////// Interpolation /////////////////////////////
  a = x[0];
  b = x[1];
  c = x[2];
  g = x[3];
  d = x[4];
  e = x[6];
  f = x[8];
  //For position x[1];
  //Wavelet coeff is diff. between odd and interpolated values.
  r = (long)((a + c)*P7_2) + (long)(b*MINUS_HALF) +
      (long)((c + d)*P7_1) + (long)((d + e)*P7_0);
  y_High[0] = (fixedpoint)(FIXPOINTADJUST(r));

  //For position x[3];
  s = (long)((c + d)*P7_2) + (long)(g*MINUS_HALF) +
      (long)((c + f)*P7_0) + (long)((a + e)*P7_1); 
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[1] = (fixedpoint)(FIXPOINTADJUST(s));

  int limit = (SubLen - 4);
  for(n = 2; n < limit; n+=2) //Two at a time for pipeline efficiency.
  {
    //Interpolate the odd coeff with learned w coeff 
    //from the even coefficients.
    m = n<<1;
    a = x[m-4];
    b = x[m-2];
    c = x[m];
    k = x[m+1];
    d = x[m+2];
    h = x[m+3];
    e = x[m+4];
    f = x[m+6];
    g = x[m+8];

    //Wavelet coeff is diff. between odd and interpolated values.
    r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + 
        (long)((a + f)*P7_0) + (long)(k*MINUS_HALF);
    y_High[n] = (fixedpoint)(FIXPOINTADJUST(r));

    s = (long)(h*MINUS_HALF) + (long)((d + e)*P7_2) +
        (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
    y_High[n+1] = (fixedpoint)(FIXPOINTADJUST(s));
  }//end for n...

  a = x[Len-10];
  b = x[Len-8];
  c = x[Len-6];
  d = x[Len-5];
  e = x[Len-4];
  f = x[Len-3];
  g = x[Len-2];
  h = x[Len-1];

  //For position x[Len-7];
  if(n == (SubLen-4))
  {
    s = (long)((x[Len-12] + g)*P7_0) + (long)((a + e)*P7_1) + 
        (long)((b + c)*P7_2) + (long)(x[Len-7]*MINUS_HALF);
    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[SubLen-4] = (fixedpoint)(FIXPOINTADJUST(s));
  }//end if n...

  //For position x[Len-5];
  r = (long)((a + g)*P7_0) + (long)((b + g)*P7_1) + 
      (long)((c + e)*P7_2) + (long)(d*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-3] = (fixedpoint)(FIXPOINTADJUST(r));

  //For position x[Len-3];
  s = (long)((b + e)*P7_0) + (long)((c + g)*P7_1) + 
      (long)((e + g)*P7_2) + (long)(f*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-2] = (fixedpoint)(FIXPOINTADJUST(s));

  //For position x[Len-1];
  r = (long)((c + c)*P7_0) + (long)((e + e)*P7_1) + 
      (long)((g + g)*P7_2) + (long)(h*MINUS_HALF);
  //Wavelet coeff is diff. between odd and interpolated values.
  y_High[SubLen-1] = (fixedpoint)(FIXPOINTADJUST(r));

  ////////////////// Update //////////////////////////////////////
  //Update even coeff with piece of wavelet.

  a = y_High[0];
  b = y_High[1];
  c = y_High[2];
  d = y_High[3];
  e = y_High[4];

  //For y_Low[0].
  r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + 
      (long)((a + b)*U9_2);
  y_Low[0] = x[0] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[1].
//  r = (long)((y_High[2] + y_High[3])*U9_0) + (long)((y_High[1] + y_High[2])*U9_1) + 
//      (long)((y_High[0] + y_High[1])*U9_2);
  y_Low[1] = x[2] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[2].
  s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + 
      (long)((b + c)*U9_2);
  y_Low[2] = x[4] + (fixedpoint)(FIXPOINTADJUST(s));

  limit = (SubLen - 3);
  for(n = 3; n < limit; n+=2)
  {
    m = n<<1;
    a = y_High[n-3];
    b = y_High[n-2];
    c = y_High[n-1];
    d = y_High[n];
    e = y_High[n+1];
    f = y_High[n+2];
    r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
    g = y_High[n+4];
    y_Low[n] = x[m] + (fixedpoint)(FIXPOINTADJUST(r));

    s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
    y_Low[n+1] = x[m+2] + (fixedpoint)(FIXPOINTADJUST(s));
  }//end for n...

  a = y_High[SubLen-6];
  b = y_High[SubLen-5];
  c = y_High[SubLen-4];
  d = y_High[SubLen-3];
  e = y_High[SubLen-2];
  f = y_High[SubLen-1];

  //For y_Low[SubLen-3].
  if( n == (SubLen-3))
  {
    s = (long)((a + e)*U9_0) + (long)((b + e)*U9_1) + (long)((c + d)*U9_2);
    y_Low[n++] = x[Len-6] + (fixedpoint)(FIXPOINTADJUST(s));
  }//end if n...

  //For y_Low[SubLen-2].
  r = (long)((b + f)*U9_0) + (long)((c + f)*U9_1) + (long)((d + e)*U9_2);
  y_Low[n++] = x[Len-4] + (fixedpoint)(FIXPOINTADJUST(r));

  //For y_Low[SubLen-1].
  s = (long)((c + e)*U9_0) + (long)((d + f)*U9_1) + (long)((e + f)*U9_2);
  y_Low[n] = x[Len-2] + (fixedpoint)(FIXPOINTADJUST(s));

}//end Dwt1D.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CVIDCODEC::IDwt1D(fixedpoint *x_Low,fixedpoint *x_High,int Len,fixedpoint *Recon)
{
  int m,n;
  int SubLen = Len>>1;

  register long r,s;
  register fixedpoint a,b,c,d,e,f,g;

  // The boundaries are performed as inline.

  ///////////// Create even coeff with piece of wavelet ////////////
  a = x_High[0];
  b = x_High[1];
  c = x_High[2];
  d = x_High[3];
  e = x_High[4];

  //For Recon[0].
  r = (long)((c + d)*U9_0) + (long)((b + c)*U9_1) + (long)((a + b)*U9_2);
  Recon[0] = x_Low[0] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[2].
//  r = (long)((x_High[2] + x_High[3])*U9_0) + (long)((x_High[1] + x_High[2])*U9_1) + 
//      (long)((x_High[0] + x_High[1])*U9_2);
  Recon[2] = x_Low[1] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[4].
  s = (long)((b + e)*U9_0) + (long)((a + d)*U9_1) + (long)((b + c)*U9_2);
  Recon[4] = x_Low[2] - (fixedpoint)(FIXPOINTADJUST(s));

  int limit = SubLen - 3;
  for(n = 3; n < limit; n+=2)
  {
    m = n<<1;
    a = x_High[n-3];
    b = x_High[n-2];
    c = x_High[n-1];
    d = x_High[n];
    e = x_High[n+1];
    f = x_High[n+2];
    g = x_High[n+3];
    r = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
    Recon[m] = x_Low[n] - (fixedpoint)(FIXPOINTADJUST(r));

    s = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + g)*U9_0);
    Recon[m+2] = x_Low[n+1] - (fixedpoint)(FIXPOINTADJUST(s));
  }//end for j...

  a = x_High[SubLen-6];
  b = x_High[SubLen-5];
  c = x_High[SubLen-4];
  d = x_High[SubLen-3];
  e = x_High[SubLen-2];
  f = x_High[SubLen-1];

  //For Recon[Len-6].
  if(n == (SubLen-3))
  {
    s = (long)((c + d)*U9_2) + (long)((b + e)*U9_1) + (long)((a + f)*U9_0);
    Recon[Len-6] = x_Low[n++] - (fixedpoint)(FIXPOINTADJUST(s));
  }//end if j...

  //For Recon[Len-4].
  r = (long)((d + e)*U9_2) + (long)((c + f)*U9_1) + (long)((b + f)*U9_0);
  Recon[Len-4] = x_Low[n++] - (fixedpoint)(FIXPOINTADJUST(r));

  //For Recon[Len-2].
  s = (long)((e + f)*U9_2) + (long)((d + f)*U9_1) + (long)((c + e)*U9_0);
  Recon[Len-2] = x_Low[n] - (fixedpoint)(FIXPOINTADJUST(s));

  ///////// Interpolate the odd coeff with w and the new even coeff.
  a = Recon[0];
  b = Recon[2];
  c = Recon[4];
  d = Recon[6];
  e = Recon[8];

  //For Recon[1].
  r = (long)((a + b)*P7_2) + (long)((b + c)*P7_1) + (long)((c + d)*P7_0); 
  Recon[1] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[0]) << 1;

  //For Recon[3].
  s = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((b + e)*P7_0); 
  Recon[3] = ((fixedpoint)(FIXPOINTADJUST(s)) - x_High[1]) << 1;

  limit = Len - 8;
  for(n = 4; n < limit; n+=4)
  {
    m = n>>1;
    //Create odd coeff with sum of wavelet coeff and interpolated values.
    a = Recon[n-4];
    b = Recon[n-2];
    c = Recon[n];
    d = Recon[n+2];
    e = Recon[n+4];
    f = Recon[n+6];
    g = Recon[n+8];
    r = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + f)*P7_0);
    Recon[n + 1] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[m]) << 1;

    s = (long)((d + e)*P7_2) + (long)((c + f)*P7_1) + (long)((b + g)*P7_0);
    Recon[n + 3] = ((fixedpoint)(FIXPOINTADJUST(s)) - x_High[m+1]) << 1;
  }//end for n...

  f = Recon[Len-12];
  a = Recon[Len-10];
  b = Recon[Len-8];
  c = Recon[Len-6];
  d = Recon[Len-4];
  e = Recon[Len-2];

  //For Recon[Len-7].
  if(n == limit)
  {
    r = (long)((b + c)*P7_2) + (long)((a + d)*P7_1) + (long)((f + e)*P7_0);
    Recon[Len-7] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[SubLen-4]) << 1;
  }//end if n...

  //For Recon[Len-5].
  s = (long)((c + d)*P7_2) + (long)((b + e)*P7_1) + (long)((a + e)*P7_0);
  Recon[Len-5] = ((fixedpoint)(FIXPOINTADJUST(s)) - x_High[SubLen-3]) << 1;

  //For Recon[Len-3].
  r = (long)((b + d)*P7_0) + (long)((c + e)*P7_1) + (long)((d + e)*P7_2);
  Recon[Len-3] = ((fixedpoint)(FIXPOINTADJUST(r)) - x_High[SubLen-2]) << 1;

  //For Recon[Len-1].
  s = (long)((c + c)*P7_0) + (long)((d + d)*P7_1) + (long)((e + e)*P7_2);
  Recon[Len-1] = ((fixedpoint)(FIXPOINTADJUST(s)) - x_High[SubLen-1]) << 1;

}//end IDwt1D.

////////// Colour Space conversions ///////////////////

#define Y_MAX    (fixedpoint)( 8191) /*(0.99999999 * FxPt_float) */
#define UV_MAX   (fixedpoint)( 4096) /*(0.5 * FxPt_float)        */
#define UV_MIN   (fixedpoint)(-4096) /*(-0.5 * FxPt_float)       */

#define R_Y_0   (fixedpoint)( 2449) /*(0.299 * FxPt_float)  */
#define R_Y_1   (fixedpoint)( 4809) /*(0.587 * FxPt_float)  */
#define R_Y_2   (fixedpoint)(  934) /*(0.114 * FxPt_float)  */
#define R_Y_3   (fixedpoint)( 3572) /*(0.436 * FxPt_float)  */
#define R_Y_4   (fixedpoint)(-1204) /*(-0.147 * FxPt_float) */
#define R_Y_5   (fixedpoint)(-2367) /*(-0.289 * FxPt_float) */
#define R_Y_6   (fixedpoint)( 5038) /*(0.615 * FxPt_float)  */
#define R_Y_7   (fixedpoint)(-4219) /*(-0.515 * FxPt_float) */
#define R_Y_8   (fixedpoint)( -819) /*(-0.100 * FxPt_float) */

int CVIDCODEC::RGB24toYUV411(void *pRGB,int RGB_X,int RGB_Y,
                             fixedpoint *pY,fixedpoint *pU,fixedpoint *pV)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* Y1 & Y2 have range 0..0.999,*/
	/* U & V have range -0.5..0.5.*/
	unsigned char *S;
	register int xb,yb,XB,YB;
  int xpix;
	fixedpoint u,v;
	fixedpoint r,g,b;

	xpix = RGB_X;
	S = (unsigned char *)(pRGB);
	/*4 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y >> 1;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
    int C_Off = yb*XB + xb;
    int Y_Off = (yb*xpix + xb) << 1;
    unsigned char *T = S + Y_Off*3;

		/*Top left pix.  255->0.999.*/
		b = (fixedpoint)( *(T) ) << 5;
		g = (fixedpoint)( *(T + 1) ) << 5;
		r = (fixedpoint)( *(T + 2) ) << 5;
    /*    RGBtoYUV(r,g,b,y,accu,accv);  */
		pY[Y_Off] = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));

		u = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
		v = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));

		/*Top right pix.*/
		b = (fixedpoint)( *(T + 3) ) << 5;
		g = (fixedpoint)( *(T + 4) ) << 5;
		r = (fixedpoint)( *(T + 5) ) << 5;
    /*    RGBtoYUV(r,g,b,y,u,v);  */
		pY[Y_Off+1] = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));

		u += (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
		v += (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));

    Y_Off += xpix;
    T += xpix*3;
		/*Bottom left pix.  255->0.999.*/
		b = (fixedpoint)( *(T) ) << 5;
		g = (fixedpoint)( *(T + 1) ) << 5;
		r = (fixedpoint)( *(T + 2) ) << 5;
    /*    RGBtoYUV(r,g,b,y,accu,accv);  */
		pY[Y_Off] = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));

		u += (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)));
		v += (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)));

		/*Bottom right pix. */
		b = (fixedpoint)( *(T + 3) ) << 5;
		g = (fixedpoint)( *(T + 4) ) << 5;
		r = (fixedpoint)( *(T + 5) ) << 5;
    /*    RGBtoYUV(r,g,b,y,u,v); */
		pY[Y_Off+1] = (fixedpoint)(FIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b)));

		pU[C_Off] = (u + (fixedpoint)(FIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g)))) >> 2;
		pV[C_Off] = (v + (fixedpoint)(FIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b)))) >> 2;
 	}/*end for xb & yb...*/

  return(1);
}/*end RGB24toYUV411.*/

#define Y_R_0   (fixedpoint)( 9339) /*(1.140 * FxPt_float)   */
#define Y_R_1   (fixedpoint)(-3228) /*(-0.394 * FxPt_float)  */
#define Y_R_2   (fixedpoint)(-4756) /*(-0.581 * FxPt_float)  */
#define Y_R_3   (fixedpoint)(16646) /*(2.032 * FxPt_float)   */
#define FXROUND (fixedpoint)( 4096) /*(0.5 * FxPt_float)     */

int CVIDCODEC::YUV411toRGB24(fixedpoint *pY,fixedpoint *pU,fixedpoint *pV,
                             void *pRGB,int RGB_X,int RGB_Y)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* R, G & B have range 0..255,*/
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
  int r,b,g;
	fixedpoint y,u,v;

	xpix = RGB_X;
	D = (unsigned char *)(pRGB);
	/*4 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y >> 1;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
    int Y_Off = (yb*xpix + xb)<<1;
    int C_Off = yb*(xpix>>1) + xb;
    unsigned char *T = D + (yb*xpix + xb)*6;
		u = pU[C_Off];
		v = pV[C_Off];

		/*Top left pix.*/
		y = pY[Y_Off];
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 1) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 2) = (unsigned char)r;

		/*Top right pix. */
		y = pY[Y_Off + 1];
    /*    YUVtoRGB(y,u,v,r,g,b); */
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T + 3) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 4) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 5) = (unsigned char)r;

    Y_Off += xpix;
    T += xpix*3;
		/*Bottom left pix.*/
		y = pY[Y_Off];
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 1) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 2) = (unsigned char)r;

		/*Bottom right pix.*/
		y = pY[Y_Off + 1];
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_0*v)) << 8) + FXROUND);
		g = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((int)(y + FIXPOINTADJUST(Y_R_3*u)) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T + 3) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 4) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 5) = (unsigned char)r;

	}/*end for xb & yb...*/

  return(1);
}/*end YUV411toRGB24.*/

/* Function to calculate the sub-band parameters and load them*/
/* into the SubBndInfo structure.*/
int CVIDCODEC::ConstructSubBndInfo(void)
{
  int col,lev,orient;
  int ColLev;
  int InfoPos;
  int ColX,ColY,LevX,LevY;
  fixedpoint *pPel;
  fixedpoint *pRPel;
  fixedpoint *pCRPel;

  /*Select the appropriate group order list for YUV411 depending*/
  /*on the luminance level.*/
  switch(Y_l)
  {
  case 4:
    pGroup = (SUBBND_GROUP_TYPE *)Lev4YUV411SubBndCodingOrder;
    GroupLen = LEV4YUV411_ORDER_LENGTH;
    break;
  default :
    //Catastrophe.
    ErrorStr = "Group order does not exist!";
    return(0);
    break;
  }//end switch Y_l...

  /*Step through each level and orientation for the luminace*/
  /*first and then the chrominance colour components.*/
  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      ColLev = Y_l;
      ColX = Y_X;
      ColY = Y_Y;
      pPel = Y;
      pRPel = rY;
      pCRPel = crY;
    }/*end if col...*/
    else
    {
      ColLev = UV_l;
      ColX = UV_X;
      ColY = UV_Y;
      if(col == Col_U)
      {
        pPel = U;
        pRPel = rU;
        pCRPel = crU;
      }/*end if col...*/
      else /*col == Col_V*/
      {
        pPel = V;
        pRPel = rV;
        pCRPel = crV;
      }/*end else...*/
    }/*end else...*/

    for(lev = ColLev; lev > 0; lev--)
    {
      LevX = ColX >> lev;
      LevY = ColY >> lev;
      for(orient = 0; orient < NUM_OF_SUBBANDS; orient++)
      {
        /*Only texture sub-band at max level.*/
        if((orient > LxLy) || (lev == ColLev))
        {
          InfoPos = SubBndInfoPos(lev,col,orient);
          SubBndInfo[InfoPos].Level = lev;
          SubBndInfo[InfoPos].Colour = col;
          SubBndInfo[InfoPos].Orient = orient;
          /* Sub-band pointers, dimensions and image coord. limits.*/
          SubBndInfo[InfoPos].sX = LevX;
          SubBndInfo[InfoPos].sY = LevY;
          SubBndInfo[InfoPos].VqRunLenLength = 0;
          switch(orient)
          {
          case LxLy :
            SubBndInfo[InfoPos].pPel = pPel;
            SubBndInfo[InfoPos].pRPel = pRPel;
            SubBndInfo[InfoPos].pCRPel = pCRPel;
            break;
          case LxHy :
            SubBndInfo[InfoPos].pPel = (fixedpoint *)(pPel + LevY*ColX);
            SubBndInfo[InfoPos].pRPel = (fixedpoint *)(pRPel + LevY*ColX);
            SubBndInfo[InfoPos].pCRPel = (fixedpoint *)(pCRPel + LevY*ColX);
            break;
          case HxLy :
            SubBndInfo[InfoPos].pPel = (fixedpoint *)(pPel + LevX);
            SubBndInfo[InfoPos].pRPel = (fixedpoint *)(pRPel + LevX);
            SubBndInfo[InfoPos].pCRPel = (fixedpoint *)(pCRPel + LevX);
            break;
          case HxHy :
            SubBndInfo[InfoPos].pPel = (fixedpoint *)(pPel + LevY*ColX + LevX);
            SubBndInfo[InfoPos].pRPel = (fixedpoint *)(pRPel + LevY*ColX + LevX);
            SubBndInfo[InfoPos].pCRPel = (fixedpoint *)(pCRPel + LevY*ColX + LevX);
            break;
          }/*end switch orient...*/
          /*Sequence and dim dependent on vector quantiser dim.*/
          LoadVectorQuant(&(SubBndInfo[InfoPos].Vq),lev,col,orient);
          if(!LoadSubBndSequence(&(SubBndInfo[InfoPos])))
          {
            ErrorStr = "Sequence structure does not exist!";
            return(0);
          }/*end if !LoadSubBndSequence...*/

          /* Set the motion indicator.*/
          if(PS.Motion && (orient != HxHy) && 
               ( ((col == Col_Y) && (lev <= PS.MotionLevel)) ||
                 ((col != Col_Y) && PS.ColourMotion && (lev < PS.MotionLevel)) ))
          {
            SubBndInfo[InfoPos].MotionEstimationRequired = 1;
          }/*end if Motion...*/
          else
            SubBndInfo[InfoPos].MotionEstimationRequired = 0;
        }/*end if orient...*/

      }/*end for orient...*/
    }/*end for lev...*/
  }/*end for col...*/

  return(1);
}/*end ConstructSubBndInfo.*/

/*************************************************************/
/* A very manual process of setting each vector quantiser to */
/* the right constant values with nested switch statements. */
/* These constants are defined in the vq header files. */
void CVIDCODEC::LoadVectorQuant(VEC_QUANT_TYPE *VQ,int level,int colour,int orient)
{
  switch(level)
  {
  case 1:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = YL1LxHy_VEC_X_DIM;
        VQ->vy = YL1LxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL1LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL1LxHy_Codebook;
        VQ->CodebookVlc = YL1LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = YL1HxLy_VEC_X_DIM;
        VQ->vy = YL1HxLy_VEC_Y_DIM;
        VQ->CodebookLength = YL1HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL1HxLy_Codebook;
        VQ->CodebookVlc = YL1HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = YL1HxHy_VEC_X_DIM;
        VQ->vy = YL1HxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL1HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL1HxHy_Codebook;
        VQ->CodebookVlc = YL1HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = UL1LxHy_VEC_X_DIM;
        VQ->vy = UL1LxHy_VEC_Y_DIM;
        VQ->CodebookLength = UL1LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL1LxHy_Codebook;
        VQ->CodebookVlc = UL1LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = UL1HxLy_VEC_X_DIM;
        VQ->vy = UL1HxLy_VEC_Y_DIM;
        VQ->CodebookLength = UL1HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL1HxLy_Codebook;
        VQ->CodebookVlc = UL1HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = UL1HxLy_VEC_X_DIM;
        VQ->vy = UL1HxLy_VEC_Y_DIM;
        VQ->CodebookLength = UL1HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL1HxLy_Codebook;
        VQ->CodebookVlc = UL1HxLy_CodebookVlc;
//        VQ->vx = UL1HxHy_VEC_X_DIM;
//        VQ->vy = UL1HxHy_VEC_Y_DIM;
//        VQ->CodebookLength = UL1HxHy_CODEBOOK_SIZE;
//        VQ->Codebook = (fixedpoint *)UL1HxHy_Codebook;
//        VQ->CodebookVlc = UL1HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = VL1LxHy_VEC_X_DIM;
        VQ->vy = VL1LxHy_VEC_Y_DIM;
        VQ->CodebookLength = VL1LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL1LxHy_Codebook;
        VQ->CodebookVlc = VL1LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = VL1HxLy_VEC_X_DIM;
        VQ->vy = VL1HxLy_VEC_Y_DIM;
        VQ->CodebookLength = VL1HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL1HxLy_Codebook;
        VQ->CodebookVlc = VL1HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = VL1HxLy_VEC_X_DIM;
        VQ->vy = VL1HxLy_VEC_Y_DIM;
        VQ->CodebookLength = VL1HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL1HxLy_Codebook;
        VQ->CodebookVlc = VL1HxLy_CodebookVlc;
//        VQ->vx = VL1HxHy_VEC_X_DIM;
//        VQ->vy = VL1HxHy_VEC_Y_DIM;
//        VQ->CodebookLength = VL1HxHy_CODEBOOK_SIZE;
//        VQ->Codebook = (fixedpoint *)VL1HxHy_Codebook;
//        VQ->CodebookVlc = VL1HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  case 2:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = YL2LxHy_VEC_X_DIM;
        VQ->vy = YL2LxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL2LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL2LxHy_Codebook;
        VQ->CodebookVlc = YL2LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = YL2HxLy_VEC_X_DIM;
        VQ->vy = YL2HxLy_VEC_Y_DIM;
        VQ->CodebookLength = YL2HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL2HxLy_Codebook;
        VQ->CodebookVlc = YL2HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = YL2HxHy_VEC_X_DIM;
        VQ->vy = YL2HxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL2HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL2HxHy_Codebook;
        VQ->CodebookVlc = YL2HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = UL2LxHy_VEC_X_DIM;
        VQ->vy = UL2LxHy_VEC_Y_DIM;
        VQ->CodebookLength = UL2LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL2LxHy_Codebook;
        VQ->CodebookVlc = UL2LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = UL2HxLy_VEC_X_DIM;
        VQ->vy = UL2HxLy_VEC_Y_DIM;
        VQ->CodebookLength = UL2HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL2HxLy_Codebook;
        VQ->CodebookVlc = UL2HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = UL2HxHy_VEC_X_DIM;
        VQ->vy = UL2HxHy_VEC_Y_DIM;
        VQ->CodebookLength = UL2HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL2HxHy_Codebook;
        VQ->CodebookVlc = UL2HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = VL2LxHy_VEC_X_DIM;
        VQ->vy = VL2LxHy_VEC_Y_DIM;
        VQ->CodebookLength = VL2LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL2LxHy_Codebook;
        VQ->CodebookVlc = VL2LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = VL2HxLy_VEC_X_DIM;
        VQ->vy = VL2HxLy_VEC_Y_DIM;
        VQ->CodebookLength = VL2HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL2HxLy_Codebook;
        VQ->CodebookVlc = VL2HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = VL2HxHy_VEC_X_DIM;
        VQ->vy = VL2HxHy_VEC_Y_DIM;
        VQ->CodebookLength = VL2HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL2HxHy_Codebook;
        VQ->CodebookVlc = VL2HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  case 3:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = YL3LxHy_VEC_X_DIM;
        VQ->vy = YL3LxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL3LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL3LxHy_Codebook;
        VQ->CodebookVlc = YL3LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = YL3HxLy_VEC_X_DIM;
        VQ->vy = YL3HxLy_VEC_Y_DIM;
        VQ->CodebookLength = YL3HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL3HxLy_Codebook;
        VQ->CodebookVlc = YL3HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = YL3HxHy_VEC_X_DIM;
        VQ->vy = YL3HxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL3HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL3HxHy_Codebook;
        VQ->CodebookVlc = YL3HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = UL3LxLy_VEC_X_DIM;
        VQ->vy = UL3LxLy_VEC_Y_DIM;
        VQ->CodebookLength = UL3LxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL3LxLy_Codebook;
        VQ->CodebookVlc = UL3LxLy_CodebookVlc;
        break;
      case LxHy:
        VQ->vx = UL3LxHy_VEC_X_DIM;
        VQ->vy = UL3LxHy_VEC_Y_DIM;
        VQ->CodebookLength = UL3LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL3LxHy_Codebook;
        VQ->CodebookVlc = UL3LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = UL3HxLy_VEC_X_DIM;
        VQ->vy = UL3HxLy_VEC_Y_DIM;
        VQ->CodebookLength = UL3HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL3HxLy_Codebook;
        VQ->CodebookVlc = UL3HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = UL3HxHy_VEC_X_DIM;
        VQ->vy = UL3HxHy_VEC_Y_DIM;
        VQ->CodebookLength = UL3HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)UL3HxHy_Codebook;
        VQ->CodebookVlc = UL3HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = VL3LxLy_VEC_X_DIM;
        VQ->vy = VL3LxLy_VEC_Y_DIM;
        VQ->CodebookLength = VL3LxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL3LxLy_Codebook;
        VQ->CodebookVlc = VL3LxLy_CodebookVlc;
        break;
      case LxHy:
        VQ->vx = VL3LxHy_VEC_X_DIM;
        VQ->vy = VL3LxHy_VEC_Y_DIM;
        VQ->CodebookLength = VL3LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL3LxHy_Codebook;
        VQ->CodebookVlc = VL3LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = VL3HxLy_VEC_X_DIM;
        VQ->vy = VL3HxLy_VEC_Y_DIM;
        VQ->CodebookLength = VL3HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL3HxLy_Codebook;
        VQ->CodebookVlc = VL3HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = VL3HxHy_VEC_X_DIM;
        VQ->vy = VL3HxHy_VEC_Y_DIM;
        VQ->CodebookLength = VL3HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)VL3HxHy_Codebook;
        VQ->CodebookVlc = VL3HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  case 4:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = YL4LxLy_VEC_X_DIM;
        VQ->vy = YL4LxLy_VEC_Y_DIM;
        VQ->CodebookLength = YL4LxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL4LxLy_Codebook;
        VQ->CodebookVlc = YL4LxLy_CodebookVlc;
        break;
      case LxHy:
        VQ->vx = YL4LxHy_VEC_X_DIM;
        VQ->vy = YL4LxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL4LxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL4LxHy_Codebook;
        VQ->CodebookVlc = YL4LxHy_CodebookVlc;
        break;
      case HxLy:
        VQ->vx = YL4HxLy_VEC_X_DIM;
        VQ->vy = YL4HxLy_VEC_Y_DIM;
        VQ->CodebookLength = YL4HxLy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL4HxLy_Codebook;
        VQ->CodebookVlc = YL4HxLy_CodebookVlc;
        break;
      case HxHy:
        VQ->vx = YL4HxHy_VEC_X_DIM;
        VQ->vy = YL4HxHy_VEC_Y_DIM;
        VQ->CodebookLength = YL4HxHy_CODEBOOK_SIZE;
        VQ->Codebook = (fixedpoint *)YL4HxHy_Codebook;
        VQ->CodebookVlc = YL4HxHy_CodebookVlc;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  case 5:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  case 6:
    switch(colour)
    {
    case Col_Y:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    case Col_U:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    case Col_V:
      switch(orient)
      {
      case LxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case LxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxLy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      case HxHy:
        VQ->vx = 0;
        VQ->vy = 0;
        VQ->CodebookLength = 0;
        VQ->Codebook = NULL;
        VQ->CodebookVlc = NULL;
        break;
      }/*end switch orient...*/
      break;
    }/*end switch colour...*/
    break;
  }/*end switch level...*/

  VQ->vdim = VQ->vx * VQ->vy;
}/*end LoadVectorQuant.*/

int CVIDCODEC::LoadSubBndSequence(SUBBND_INFO_TYPE *Info)
{
  if((Info->sX == 88)&&(Info->sY == 72)&&(Info->Vq.vx == 4)&&(Info->Vq.vy == 4))
  {
    Info->SubBndSeq = (VICS_COORD *)SubBnd_88x72_Vec_4x4;
    Info->SeqLength = SubBndXBlks_88_4 * SubBndYBlks_72_4;
  }/*end if LevX...*/
  else if((Info->sX == 44)&&(Info->sY == 36)&&(Info->Vq.vx == 4)&&(Info->Vq.vy == 4))
  {
    Info->SubBndSeq = (VICS_COORD *)SubBnd_44x36_Vec_4x4;
    Info->SeqLength = SubBndXBlks_44_4 * SubBndYBlks_36_4;
  }/*end else if LevX...*/
  else if((Info->sX == 44)&&(Info->sY == 36)&&(Info->Vq.vx == 2)&&(Info->Vq.vy == 2))
  {
    Info->SubBndSeq = (VICS_COORD *)SubBnd_44x36_Vec_2x2;
    Info->SeqLength = SubBndXBlks_44_2 * SubBndYBlks_36_2;
  }/*end else if LevX...*/
  else if((Info->sX == 22)&&(Info->sY == 18)&&(Info->Vq.vx == 2)&&(Info->Vq.vy == 2))
  {
    Info->SubBndSeq = (VICS_COORD *)SubBnd_22x18_Vec_2x2;
    Info->SeqLength = SubBndXBlks_22_2 * SubBndYBlks_18_2;
  }/*end else if LevX...*/
  else if((Info->sX == 11)&&(Info->sY == 9)&&(Info->Vq.vx == 1)&&(Info->Vq.vy == 3))
  {
    Info->SubBndSeq = (VICS_COORD *)SubBnd_11x9_Vec_1x3;
    Info->SeqLength = SubBndXBlks_11_1 * SubBndYBlks_9_3;
  }/*end else if LevX...*/
  else
  {
    return(0);
  }/*end else...*/
  return(1);
}/*end LoadSubBndSequence.*/

void CVIDCODEC::CodeSubBnd(SUBBND_INFO_TYPE *Info)
{
  int seq,i,j,ilim,jlim;
  int pix,diff;
  fixedpoint *Src;
  fixedpoint *Ref;
  fixedpoint *CRef;
  int X,Q,Off;
  long r_acc, cr_acc, r_dist, cr_dist;
  int r_index, cr_index;
  int seqrun,motionseqrun;
  int SrchX,SrchY,x,y,xpos,ypos,oddrows,oddcols;
  int dist,motion_x,motion_y;

  if(Info->Colour == Col_Y)
  {
    X = Y_X;
    Q = Y_Q[Info->Orient][Info->Level - 1];
  }/*end if Colour...*/
  else
  {
    X = UV_X;
    if(Info->Colour == Col_U)
      Q = U_Q[Info->Orient][Info->Level - 1];
    else
      Q = V_Q[Info->Orient][Info->Level - 1];
  }/*end else...*/

  seqrun = 0; /*Initialise for runlength.*/
  motionseqrun = 0;
  Info->VqRunLenLength = 0; /*Runlength struct counter.*/
  Info->MotionRunLength = 0;
  for(seq = 0; seq < Info->SeqLength; seq++)
  {
    Off = (Info->SubBndSeq[seq].Y * X) + Info->SubBndSeq[seq].X;
    ilim = Info->Vq.vy; /*Vector rows.*/
    jlim = Info->Vq.vx; /*Vector cols.*/
    Src = (fixedpoint *)(Info->pPel + Off); /*Input vector.*/
    Ref = (fixedpoint *)(Info->pRPel + Off); /*Reference vector.*/
    if(Info->MotionEstimationRequired)
    {
      CRef = (fixedpoint *)(Info->pCRPel + Off); /*Comp ref vector.*/
    }/*end if MotionEstimationRequired...*/
    r_acc = 0;

    /*Get input, ref and cref vectors and psychovisually quantise */
    /*on the way in. Calc difference vectors and threshold. */
    for(i = 0; i < ilim; i++)
    {
      int vecrow = i*jlim;
      int imgrow = i*X;
      for(j = 0; j < jlim; j++)
      {
        pix = (((((int)(Src[imgrow + j]) << 3) * Q) >> 8) + QFXROUND) >> 8;
//        pix = ((int)(Src[i*X + j]) << 8) >> (FxPt - QFxPt);
//        pix = (((pix * Q) >> QFxPt) + QFXROUND) >> QFxPt;
        /* pix is now in 8bpp integer form.*/
        /* Bound check.*/
        if((Info->Orient != LxLy)||(Info->Colour != Col_Y))
        {
          if(pix > 127)
            pix = 127;
          else if(pix < -128)
            pix = -128;
        }/*end if Orient...*/
        else /*Texture sub-band.*/
        {
          if(pix > 255)
            pix = 255;
          else if(pix < 0)
            pix = 0;
        }/*end else...*/
        InVec[vecrow + j] = (fixedpoint)pix;
        diff = pix - Ref[imgrow + j];
        /*Threshold the diff vector value.*/
        int th = PS.Thresh;
        int negtest = (diff < -(th));
        if((!negtest)&&(diff <= th))
          diff = 0;
        else if(negtest)
          diff += th;
        else
          diff -= th;
        DRVec[vecrow + j] = diff;
        /*Accum the distortion with zero vec i.e. vec energy. */
        r_acc += ((long)diff*(long)diff);
      }/*end for j...*/
    }/*end for i...*/

    /*Do motion estimation and compensation for */
    /*this sub-band vector and put the compensated */
    /*sub-band in CRef. */
    if(Info->MotionEstimationRequired)
    {
      /*Load the search area.*/
      SrchX = (jlim * 6)-1;
      int xsrchend = jlim << 1;
      SrchY = (ilim * 6)-1;
      int ysrchend = ilim << 1;
      /*The pixel grid.*/
      for(y = -(ilim); y < ysrchend; y++)
      {
        ypos = Info->SubBndSeq[seq].Y + y;
        int srchrow = ((y << 1) + ysrchend)*SrchX;
        for(x = -(jlim); x < xsrchend; x++)
        {
          xpos = Info->SubBndSeq[seq].X + x;
          int a_x = (x << 1) + xsrchend;
          if( (ypos < 0)||(ypos >= Info->sY)||(xpos < 0)||(xpos >= Info->sX) )
            /*On a half-pix grid.*/
            SrchWin[srchrow + a_x] = 0;
          else
            SrchWin[srchrow + a_x] = Ref[y*X + x];
        }/*end for x...*/
      }/*end for y...*/
      /*The half-pixel grid.*/
      for(y = 0; y < SrchY; y++)
      {
        oddrows = y % 2;
        int srchrow = y*SrchX;
        for(x = 0; x < SrchX; x++)
        {
          oddcols = x % 2;
          if(oddrows && oddcols)
          {
            int srchpos = srchrow + x;
            SrchWin[srchpos] = (SrchWin[srchpos-SrchX-1] +
                                SrchWin[srchpos-SrchX+1] +
                                SrchWin[srchpos+SrchX-1] +
                                SrchWin[srchpos+SrchX+1] + 2)/4;
          }/*end if oddrows...*/
          else if(oddrows && !oddcols)
          {
            int srchpos = srchrow + x;
            SrchWin[srchpos] = (SrchWin[srchpos-SrchX] +
                                SrchWin[srchpos+SrchX] + 1)/2;
          }/*end else if oddrows...*/
          else if(!oddrows && oddcols)
          {
            int srchpos = srchrow + x;
            SrchWin[srchpos] = (SrchWin[srchpos-1] +
                                SrchWin[srchpos+1] + 1)/2;
          }/*end else if !oddrows...*/
        }/*end for y...*/
      }/*end for y...*/

      /*Find best half-pixel motion vector within the search window.*/
      ypos = (ilim << 2) + 1; //(((ilim<<1)+1)<<1)-1;
      xpos = (jlim << 2) + 1; //(((jlim<<1)+1)<<1)-1;
      dist = 10000000;
      for(y = 0; y < ypos; y++)
      {
        for(x = 0; x < xpos; x++)
        {
          cr_acc = 0;
          for(i = 0; i < ilim; i++)
          {
            int vecrow = i*jlim;
            int srchrow = (y+(i<<1))*SrchX;
            for(j = 0; j < jlim; j++)
            {
              diff = InVec[vecrow + j] - SrchWin[srchrow + (x+(j<<1))];
              cr_acc += ((long)diff*(long)diff);
            }/*end for i...*/
          }/*end for j...*/
          if(cr_acc < dist)
          {
            dist = cr_acc;
            motion_x = x;
            motion_y = y;
          }/*end if cr_acc...*/
        }/*end for x...*/
      }/*end for y...*/
      motion_y = motion_y - (ilim<<1);
      motion_x = motion_x - (jlim<<1);
      Info->MotionVec[Info->MotionRunLength].x = motion_x;
      Info->MotionVec[Info->MotionRunLength].y = motion_y;

      /*After motion estimation load up the best vector and */
      /*determine the distortion with the input vector.*/
      cr_acc = 0;
      for(i = 0; i < ilim; i++)
      {
        int imgrow = i*X;
        int srchrow = (motion_y+((ilim+i)<<1))*SrchX;
        int vecrow = i*jlim;
        for(j = 0; j < jlim; j++)
        {
          CRef[imgrow + j] = SrchWin[srchrow + (motion_x+((jlim+j)<<1))];
          diff = InVec[vecrow + j] - CRef[imgrow + j];
          /*Threshold the diff vector value.*/
          int th = PS.Thresh;
          int negtest = (diff < -(th));
          if((!negtest)&&(diff <= th))
            diff = 0;
          else if(negtest)
            diff += th;
          else
            diff -= th;
          DCRVec[vecrow + j] = diff;
          cr_acc += ((long)diff*(long)diff);
        }/*end for j...*/
      }/*end for i...*/
    }/*end if MotionEstimationRequired...*/

    /*Vector quant the diff vectors with the corresponding sub-band */
    /*vq and collect the motion vectors and runlength of vectors. */
    if(r_acc == 0) /*1st short cut.*/
    { /*The diff vector is zero.*/
      seqrun++;
      if(Info->MotionEstimationRequired)
      {
        Info->MotionVec[Info->MotionRunLength].x = 0;
        Info->MotionVec[Info->MotionRunLength].y = 0;
      }/*end if MotionEstimationRequired...*/
    }/*end if r_acc...*/
    else if(Info->MotionEstimationRequired && (cr_acc == 0)) /*2nd short cut.*/
    { /*The compensated diff vector is zero.*/
      seqrun++;
      /*Compensated motion vector is valid.*/
    }/*end else if MotionEstimationRequired...*/
    else /*No short cut.*/
    {
      /*Require winning index and distortion for ref and cref.*/
      r_index = VectorQuantize(&(Info->Vq),DRVec,&r_dist);
      /*Choose the best including the dist with the zero vectors.*/
      if(Info->MotionEstimationRequired)
      {
        cr_index = VectorQuantize(&(Info->Vq),DCRVec,&cr_dist);
        if((cr_dist < r_dist)&&(cr_dist < r_acc))
        {
          if(cr_dist < cr_acc)
          {
            Info->VqRunLen[Info->VqRunLenLength].run = seqrun;
            Info->VqRunLen[Info->VqRunLenLength].val = cr_index;
            Info->VqRunLenLength++;
            seqrun = 0;
          }/*end if cr_dist...*/
          else
          {
            seqrun++;
          }/*end else...*/
        }/*end if cr_dist...*/
        else
        {
          /*The zero motion vector produces better dist.*/
          Info->MotionVec[Info->MotionRunLength].x = 0;
          Info->MotionVec[Info->MotionRunLength].y = 0;
          if(r_dist < r_acc)
          {
            Info->VqRunLen[Info->VqRunLenLength].run = seqrun;
            Info->VqRunLen[Info->VqRunLenLength].val = r_index;
            Info->VqRunLenLength++;
            seqrun = 0;
          }/*end if r_dist...*/
          else
          {
            seqrun++;
          }/*end else...*/
        }/*end else...*/
      }/*end if MotionEstimationRequired...*/
      else
      {
        if(r_dist < r_acc)
        {
          Info->VqRunLen[Info->VqRunLenLength].run = seqrun;
          Info->VqRunLen[Info->VqRunLenLength].val = r_index;
          Info->VqRunLenLength++;
          seqrun = 0;
        }/*end if r_dist...*/
        else
        {
          seqrun++;
        }/*end else...*/
      }/*end else...*/
    }/*end else...*/

    if(Info->MotionEstimationRequired)
    {
      if((Info->MotionVec[Info->MotionRunLength].x == 0)&&
         (Info->MotionVec[Info->MotionRunLength].y == 0))
      {
        motionseqrun++;
      }/*end if MotionVec...*/
      else
      {
        Info->MotionRun[Info->MotionRunLength] = motionseqrun;
        Info->MotionRunLength++;
        motionseqrun = 0;
      }/*end else...*/
    }/*end if MotionEstimationRequired...*/
  }/*end for seq...*/

}/*end CodeSubBnd.*/

void CVIDCODEC::ZeroRefSubBnd(SUBBND_INFO_TYPE *Info,int UpTo)
{
  int seq,i,j,ilim,jlim;
  fixedpoint *Ref;
  int X,Off;

  if(Info->Colour == Col_Y)
  {
    X = Y_X;
  }/*end if Colour...*/
  else
  {
    X = UV_X;
  }/*end else...*/

  for(seq = 0; seq < UpTo; seq++)
  {
    Off = (Info->SubBndSeq[seq].Y * X) + Info->SubBndSeq[seq].X;
    ilim = Info->Vq.vy; /*Vector rows.*/
    jlim = Info->Vq.vx; /*Vector cols.*/
    Ref = (fixedpoint *)(Info->pRPel + Off); /*Reference vector.*/

    for(i = 0; i < ilim; i++)
    {
      int imgrow = i*X;
      for(j = 0; j < jlim; j++)
      {
        Ref[imgrow + j] = 0;
      }/*end for j...*/
    }/*end for i...*/
  }/*end for seq...*/
}/*end ZeroRefSubBnd.*/

void CVIDCODEC::ZeroRefSubBndToEnd(SUBBND_INFO_TYPE *Info,int From)
{
  int seq,i,j,ilim,jlim;
  fixedpoint *Ref;
  int X,Off;

  if(Info->Colour == Col_Y)
  {
    X = Y_X;
  }/*end if Colour...*/
  else
  {
    X = UV_X;
  }/*end else...*/

  for(seq = From; seq < Info->SeqLength; seq++)
  {
    Off = (Info->SubBndSeq[seq].Y * X) + Info->SubBndSeq[seq].X;
    ilim = Info->Vq.vy; /*Vector rows.*/
    jlim = Info->Vq.vx; /*Vector cols.*/
    Ref = (fixedpoint *)(Info->pRPel + Off); /*Reference vector.*/

    for(i = 0; i < ilim; i++)
    {
      int imgrow = i*X;
      for(j = 0; j < jlim; j++)
      {
        Ref[imgrow + j] = 0;
      }/*end for j...*/
    }/*end for i...*/
  }/*end for seq...*/
}/*end ZeroRefSubBndToEnd.*/

/* Decode the sub-band only as far as into the ref image and */
/* not into the input image space. This function is used in  */
/* the coding process and not the decoding process. The ref  */
/* and the cref images are valid and correct. */
void CVIDCODEC::DecodeSubBndRef(SUBBND_INFO_TYPE *Info)
{
  int seq,i,j,ilim,jlim;
  fixedpoint *Src;
  fixedpoint *Ref;
  fixedpoint *pC;
  int X,Off;
  int RunLenStructCnt, MotionRunStructCnt;
  int seqrun,motionseqrun;

  if(Info->Colour == Col_Y)
  {
    X = Y_X;
  }/*end if Colour...*/
  else
  {
    X = UV_X;
  }/*end else...*/

  seqrun = Info->VqRunLen[0].run; /*Initialise for runlength.*/
  motionseqrun = Info->MotionRun[0];
  RunLenStructCnt = 0; /*Runlength struct counter.*/
  MotionRunStructCnt = 0;
  for(seq = 0; seq < Info->SeqLength; seq++)
  {
    if((RunLenStructCnt >= Info->VqRunLenLength)&&
       (MotionRunStructCnt >= Info->MotionRunLength))
      break;

    if((seqrun > 0) &&  /*Short cut test.*/
       (!Info->MotionEstimationRequired || (motionseqrun > 0)))
    {
      seqrun--;
      if(Info->MotionEstimationRequired)
        motionseqrun--;
    }/*end if seqrun...*/
    else /*Decoding is required.*/
    {
      Off = (Info->SubBndSeq[seq].Y * X) + Info->SubBndSeq[seq].X;
      ilim = Info->Vq.vy; /*Vector rows.*/
      jlim = Info->Vq.vx; /*Vector cols.*/
      Ref = (fixedpoint *)(Info->pRPel + Off); /*Reference vector.*/
      Src = Ref; /*Default source vector.*/
      if(Info->MotionEstimationRequired)
      {                        /* No longer interested case.*/
        if((motionseqrun > 0)||(MotionRunStructCnt >= Info->MotionRunLength))
          motionseqrun--;
        else
        {
          /*A non-zero motion vector implies changing the source to the */
          /*compensated image space. */
          Src = (fixedpoint *)(Info->pCRPel + Off); /*Comp ref vector.*/
          MotionRunStructCnt++;
          motionseqrun = Info->MotionRun[MotionRunStructCnt];
        }/*end else...*/
      }/*end if MotionEstimationRequired...*/
      if((seqrun > 0)||(RunLenStructCnt >= Info->VqRunLenLength))
      {
        seqrun--;
        /*Straight copy*/
        if(Info->MotionEstimationRequired && (MotionRunStructCnt <= Info->MotionRunLength))
        {
          for(i = 0; i < ilim; i++)
          {
            int rowaddr = i*X;
            for(j = 0; j < jlim; j++)
            {
              fixedpoint pix = Src[rowaddr + j];
              if((Info->Orient != LxLy)||(Info->Colour != Col_Y))
              {
                if(pix > 127)
                  pix = 127;
                else if(pix < -128)
                  pix = -128;
              }/*end if Orient...*/
              else /*Texture sub-band.*/
              {
                if(pix > 255)
                  pix = 255;
                else if(pix < 0)
                  pix = 0;
              }/*end else...*/
              Ref[rowaddr + j] = pix;
            }/*end for j...*/
          }/*end for i...*/
        }/*end if MotionEstimationRequired...*/
      }/*end if seqrun...*/
      else
      {
        /*Codebook codevector pointer.*/
        pC = (fixedpoint *)(Info->Vq.Codebook + 
             (Info->VqRunLen[RunLenStructCnt].val * Info->Vq.vdim));
        for(i = 0; i < ilim; i++)
        {
          int imgrow = i*X;
          int vecrow = i*jlim;
          for(j = 0; j < jlim; j++)
          {
            int th = PS.Thresh;
            fixedpoint pix = pC[vecrow + j];
            if(pix)
            {
              if(pix < 0)
                pix -= th;
              else
                pix += th;
            }/*end if pix...*/
            pix += Src[imgrow + j];
            if((Info->Orient != LxLy)||(Info->Colour != Col_Y))
            {
              if(pix > 127)
                pix = 127;
              else if(pix < -128)
                pix = -128;
            }/*end if Orient...*/
            else /*Texture sub-band.*/
            {
              if(pix > 255)
                pix = 255;
              else if(pix < 0)
                pix = 0;
            }/*end else...*/
            Ref[imgrow + j] = pix;
          }/*end for j...*/
        }/*end for i...*/
        RunLenStructCnt++;
        seqrun = Info->VqRunLen[RunLenStructCnt].run;
      }/*end else...*/
    }/*end else...*/
  }/*end for seq...*/

}/*end DecodeSubBndRef.*/

void CVIDCODEC::DecodeSubBnd(SUBBND_INFO_TYPE *Info)
{
  /*Compensated image only has valid blocks at */
  /*locations where actual comp is required, otherwise the block  */
  /*is invalid and is not used for generating the new ref image.  */
  if(Info->MotionEstimationRequired)
    MotionCompSubBnd(Info);

  /*Sum in the decoded vq vectors from either the comp ref or the */
  /*old ref, into the new ref image. */
  DecodeSubBndRef(Info);

}/*end DecodeSubBnd.*/

void CVIDCODEC::MotionCompSubBnd(SUBBND_INFO_TYPE *Info)
{
  int seq,i,j,ilim,jlim,x,y;
  fixedpoint *Ref;
  fixedpoint *CRef;
  int X,Off;
  int MotionRunStructCnt;
  int motionseqrun;

  if(Info->Colour == Col_Y)
  {
    X = Y_X;
  }/*end if Colour...*/
  else
  {
    X = UV_X;
  }/*end else...*/

  /*Use the decoded motion vectors to generate the comp Ref image */
  /*from the current ref image. Comp img only has valid blocks at */
  /*locations where actual comp is required, otherwise the block  */
  /*is invalid and is not used for generating the new ref image.  */

  if(Info->MotionEstimationRequired)
  {
    motionseqrun = Info->MotionRun[0]; /*Initialise for runlength.*/
    MotionRunStructCnt = 0; /*Runlength struct counter.*/
    for(seq = 0; seq < Info->SeqLength; seq++)
    {
      if(MotionRunStructCnt >= Info->MotionRunLength)
        break;
      if(motionseqrun > 0)  /*Compensation not required on this vector pos.*/
      {
        motionseqrun--;
      }/*end if motionseqrun...*/
      else /*Compensation is required.*/
      {
        x = Info->SubBndSeq[seq].X;
        y = Info->SubBndSeq[seq].Y;
        Off = (y * X) + x;
        ilim = Info->Vq.vy; /*Vector rows.*/
        jlim = Info->Vq.vx; /*Vector cols.*/
        CRef = (fixedpoint *)(Info->pCRPel + Off); /*Comp ref img vector.*/
        Ref = (fixedpoint *)(Info->pRPel + Off); /*Source is the ref img vector.*/
        /*Get the motion vectors.*/
        int dx = Info->MotionVec[MotionRunStructCnt].x;
        int dy = Info->MotionVec[MotionRunStructCnt].y;
        int halfx = dx % 2; /*Half pix pos.*/
        int halfy = dy % 2;
        /*Ref full pix grid position.*/
        int rx = (dx/2);
        int ry = (dy/2);
        for(i = 0; i < ilim; i++)
        {
          int ypos = ry + i; /*in full pix.*/
          int imgrow = i*X;
          int fullrow = ypos*X;
          int halfrow = (ypos+halfy)*X;
          for(j = 0; j < jlim; j++)
          {
            int xpos = rx + j; /*in full pix.*/
            /*Check overflow.*/
            if( ((y+ypos+halfy) < 0)||((y+ypos+halfy) >= Info->sY)||
                ((x+xpos+halfx) < 0)||((x+xpos+halfx) >= Info->sX) )
                CRef[imgrow + j] = 0;
            else
            {
              /*Get the ref src pix from possible half pix positions.*/
              if(!halfy && !halfx)
                CRef[imgrow + j] = Ref[fullrow + xpos];
              else if(halfy && !halfx)
                CRef[imgrow + j] = (Ref[fullrow + xpos] + 
                                    Ref[halfrow + xpos] + 1)/2;
              else if(!halfy && halfx)
                CRef[imgrow + j] = (Ref[fullrow + xpos] + 
                                    Ref[fullrow + (xpos+halfx)] + 1)/2;
              else
                CRef[imgrow + j] = (Ref[fullrow + xpos] + 
                                    Ref[fullrow + (xpos+halfx)] +
                                    Ref[halfrow + xpos] +
                                    Ref[halfrow + (xpos+halfx)] + 2)/4;
            }/*end else...*/
          }/*end for j...*/
        }/*end for i...*/
        /*Next motion run value.*/
        MotionRunStructCnt++;
        /*Note: This will read past the array limit on the last one.*/
        motionseqrun = Info->MotionRun[MotionRunStructCnt]; 
      }/*end else...*/
    }/*end for seq...*/
  }/*end if MotionEstimationRequired...*/

}/*end MotionCompSubBnd.*/

int CVIDCODEC::Threshold(int dpix, int th)
{
  if(dpix < 0)
  {
    /*-ve dpix values.*/
    if(dpix < -(th))
      return(dpix + th);
    else
      return(0);
  }/*end if dpix...*/
  else
  {
    /*+ve dpix values.*/
    if(dpix > th)
      return(dpix - th);
    else
      return(0);
  }/*end else...*/
}/*end Threshold.*/

int CVIDCODEC::InvThreshold(int dpix, int th)
{
  if(dpix == 0)
    return(0);

  if(dpix < 0)
  {
    /*-ve dpix values.*/
    return(dpix - th);
  }/*end if dpix...*/
  else
  {
    /*+ve dpix values.*/
    return(dpix + th);
  }/*end else...*/
}/*end InvThreshold.*/

int CVIDCODEC::VectorQuantize(VEC_QUANT_TYPE *VQ,fixedpoint *Vec,long *Dist)
{
  int book,booklim,dim,dimlim;
  fixedpoint *pC;
  long dist,best_dist;
  int best_index;
  fixedpoint x;

  dimlim = VQ->vdim;
  booklim = VQ->CodebookLength;
  best_dist = 10000000;
  best_index = 0;

  for(book = 0; book < booklim; book++)
  {
    dist = 0;
    pC = (fixedpoint *)(VQ->Codebook + book*dimlim);
    for(dim = 0; dim < dimlim; dim++)
    {
      x = Vec[dim] - pC[dim];
      dist += ((long)x*(long)x);
    }/*end for dim...*/
    /*Exact match short cut.*/
    if(dist == 0)
    {
      *Dist = 0;
      return(book);
    }/*end if dist...*/
    if(dist < best_dist)
    {
      best_dist = dist;
      best_index = book;
    }/*end if dist...*/
  }/*end for book...*/

  *Dist = best_dist;
  return(best_index);
}/*end VectorQuantize.*/

int CVIDCODEC::GetRunBits(int Run,int *CdeWord)
{
  if(Run < 0)
  {
    *CdeWord = 0;
    return(0);
  }/*end if Run...*/
  if(Run >= CVIDCODEC_RUN_VLC_TABLE_SIZE)
  {
    *CdeWord = (((Run & CVIDCODEC_RunEscValMask) << CVIDCODEC_RunEscBits) | CVIDCODEC_RunEscCode);
    return(CVIDCODEC_RunEscBits + CVIDCODEC_RunEscValBits);
  }/*end if Run...*/
  else
  {
    *CdeWord = CVIDCODEC_RunVlcTable[Run].bits;
    return(CVIDCODEC_RunVlcTable[Run].numbits);
  }/*end else...*/
}/*end GetRunBits.*/

/*------------------------------------------------------------*/
/* Run bit extraction from bit stream.*/
/*------------------------------------------------------------*/
unsigned int *CVIDCODEC::ExtractRunBits(unsigned int *CurrPtr,
												 int *NextBitPos,int *Run,
                         int *BitsExtracted,int *MarkerFlag,
                         int *Codeword)
{
  int bit;
  int bits;
  int tbl_pos,tbl_size;
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
  tbl_size = CVIDCODEC_RUN_VLC_TABLE_SIZE + 2; /*Include markers.*/
  *MarkerFlag = 0; /*Default setting.*/

  while( (tbl_pos < tbl_size) && !Found )
  {
    bits_needed = CVIDCODEC_RunVlcTable[tbl_pos].numbits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (CVIDCODEC_RunVlcTable[tbl_pos].numbits == bits_so_far) && 
           (tbl_pos < tbl_size) && !Found )
    {
      if(CVIDCODEC_RunVlcTable[tbl_pos].bits == bits)
        Found = 1;
      else
        tbl_pos++;

    }/*end while numbits...*/

    /*Check for escape sequence at the appropriate numbits pos.*/
    if(!Found && (bits_so_far == CVIDCODEC_RunEscBits))
    {
      if(bits == CVIDCODEC_RunEscCode)
      {
        CurrPtr = GetBits(CurrPtr,NextBitPos,
                          CVIDCODEC_RunEscValBits,Run);
        *BitsExtracted = CVIDCODEC_RunEscBits + CVIDCODEC_RunEscValBits;
        *Codeword = bits;
        return(CurrPtr);
      }/*end if bits...*/
    }/*end if !Found...*/
  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *Run = 0;
    *BitsExtracted = 0; /* Implies an error. */
    *Codeword = bits;
		ErrorStr = "Non-existant run-length Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  if((tbl_pos == EOSB_INDEX)||(tbl_pos == EOI_INDEX))
    *MarkerFlag = 1;
  *Run = tbl_pos;
  *BitsExtracted = bits_so_far;
  *Codeword = bits;
  return(CurrPtr);
}/*end ExtractRunBits.*/

int CVIDCODEC::GetMotionVecBits(int VecCoord,int *CdeWord)
{
  int abs_motion,bits;
  int code;

  if(VecCoord < 0)
    abs_motion = -(VecCoord);
  else
    abs_motion = VecCoord;

  if( abs_motion >= CVIDCODEC_MOTIONVEC_VLC_TABLE_SIZE )
    return(0);

  bits = CVIDCODEC_MotionVlcTable[abs_motion].numbits;
  code = CVIDCODEC_MotionVlcTable[abs_motion].bits;
  /*Add the sign bit to MSB if not zero.*/
  if(VecCoord < 0)
  {
    code = (1 << bits) | code; /*-ve.*/
    bits++;
  }/*end if VecCoord...*/
  else if(VecCoord > 0)
  {
    bits++;  /*+ve. zero sign bit is implicit.*/
  }/*end else if VecCoord...*/

  *CdeWord = code;
  return(bits);
}/*end GetMotionVecBits.*/

/*------------------------------------------------------------*/
/* Motion vector coordinate bit extraction from bit stream.*/
/*------------------------------------------------------------*/
unsigned int *CVIDCODEC::ExtractMotionVecBits(unsigned int *CurrPtr,
												 int *NextBitPos,int *Vec,int *BitsExtracted,
                         int *Codeword)
{
  int bit;
  int bits;
  int tbl_pos,tbl_size;
  int Found;
  int bits_so_far,bits_needed;

  /* Decode the binary tree to determine the motion vector coord.*/
  /* The table must be in ascending bit length order. Bits */
  /* are extracted from the bit stream depending on the next */
  /* no. of bits in the table. Keep going through the table */
  /* until a match is found or the table ends. */
  bits = 0;
  tbl_pos = 0;
  bits_so_far = 0;
  bits_needed = 0;
  Found = 0;
  tbl_size = CVIDCODEC_MOTIONVEC_VLC_TABLE_SIZE;

  while( (tbl_pos < tbl_size) && !Found )
  {
    bits_needed = CVIDCODEC_MotionVlcTable[tbl_pos].numbits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (CVIDCODEC_MotionVlcTable[tbl_pos].numbits == bits_so_far) && 
           (tbl_pos < tbl_size) && !Found )
    {
      if(CVIDCODEC_MotionVlcTable[tbl_pos].bits == bits)
        Found = 1;
      else
        tbl_pos++;
    }/*end while numbits...*/
  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *Vec = 0;
    *BitsExtracted = 0; /* Implies an error. */
    *Codeword = bits;
		ErrorStr = "Non-existant motion vector Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  /*Extract sign bit.*/
  if(tbl_pos != 0)
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far++;
    if(bit) /*1 => -ve.*/
    {
      tbl_pos = -(tbl_pos);
    }/*end if bit...*/
  }/*end if tbl_pos...*/

  *Vec = tbl_pos;
  *BitsExtracted = bits_so_far;
  *Codeword = bits;
  return(CurrPtr);
}/*end ExtractMotionVecBits.*/

int CVIDCODEC::GetVecBits(VEC_QUANT_TYPE *VQ,int VecIndex,int *CdeWord)
{
  if(VecIndex < VQ->CodebookLength)
  {
    *CdeWord = VQ->CodebookVlc[VecIndex].bits;
    return(VQ->CodebookVlc[VecIndex].numbits);
  }/*end if VecIndex...*/
  else
  {
    *CdeWord = 1;
    return(0);
  }/*end else...*/
}/*end GetVecBits.*/

/*------------------------------------------------------------*/
/* Motion vector coordinate bit extraction from bit stream.*/
/*------------------------------------------------------------*/
unsigned int *CVIDCODEC::ExtractVqIndexBits(unsigned int *CurrPtr,
												 int *NextBitPos,VEC_QUANT_TYPE *Vq,
                         int *VqIndex,int *BitsExtracted,int *Codeword)
{
  int bit;
  int bits;
  int tbl_pos,tbl_size;
  int Found;
  int bits_so_far,bits_needed;

  /* Decode the binary tree to determine the motion vector coord.*/
  /* The table must be in ascending bit length order. Bits */
  /* are extracted from the bit stream depending on the next */
  /* no. of bits in the table. Keep going through the table */
  /* until a match is found or the table ends. */
  bits = 0;
  tbl_pos = 0;
  bits_so_far = 0;
  bits_needed = 0;
  Found = 0;
  tbl_size = Vq->CodebookLength;

  while( (tbl_pos < tbl_size) && !Found )
  {
    bits_needed = Vq->CodebookVlc[tbl_pos].numbits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (Vq->CodebookVlc[tbl_pos].numbits == bits_so_far) && 
           (tbl_pos < tbl_size) && !Found )
    {
      if(Vq->CodebookVlc[tbl_pos].bits == bits)
        Found = 1;
      else
        tbl_pos++;
    }/*end while numbits...*/
  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *VqIndex = 0;
    *BitsExtracted = 0; /* Implies an error. */
    *Codeword = bits;
		ErrorStr = "Non-existant vector quantiser index Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  *VqIndex = tbl_pos;
  *BitsExtracted = bits_so_far;
  *Codeword = bits;
  return(CurrPtr);
}/*end ExtractVqIndexBits.*/

/*-------------------------------------------------------------------*/
/* Add BitsToAdd number of bits from the LSB of CodeBits, to the 32  */
/* bit word pointed to by CurrPtr from bit position NextBitPos up    */
/* towards the MSB.                                                  */       
/*-------------------------------------------------------------------*/
unsigned int *CVIDCODEC::PutBits(unsigned int *CurrPtr,
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
unsigned int *CVIDCODEC::GetBits(unsigned int *CurrPtr,
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
unsigned int *CVIDCODEC::GetNextBit(unsigned int *CurrPtr,
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

