//**********************************************************
// TITLE       :DWT VIDEO COMPRESSION CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :VidCodec.h
// DESCRIPTION :A class for implementing a DWT-domain video
//              compression and decompression codec.
//              A RGB24/YUV411 video is coded into a bit stream 
//              and the bit stream is decoded into a RGB24/YUV411
//              image.
// DATE        :May 1999
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _VIDCODEC_H
#define _VIDCODEC_H

#include <math.h>

#define CODEC_MAX_LEVELS 6
#define NUM_OF_SUBBANDS 4
#define NUM_OF_COLOUR_COMP 3

#define Col_Y 0
#define Col_U 1
#define Col_V 2
#define LxLy 0
#define LxHy 1
#define HxLy 2
#define HxHy 3

// Fixed point aritmetic adventure.
#define FxPt 13
#define FxPt_int (int)(1 << FxPt)
#define FxPt_float (double)(FxPt_int)
#define FIXPOINT(x) ((x) << FxPt)
#define FIXPOINTADJUST(x) ((x) >> FxPt)
typedef short int fixedpoint;

// Quantization tables operate with a different fixed point.
#define QFxPt 8
#define QFxPt_int (int)(1 << QFxPt)
#define QFxPt_float (double)(QFxPt_int)

#include "..\VWbaseCodec\codeseq.h"

///////////////////////////////////////////////////////////////////
// Define a structure to hold the codec operational parameters.
typedef struct tagCVIDCODEC_STRUCT
{
  //Required operating parameters.
  int Q;
  int Thresh;
  int Bpp; //For YUV representation.
  //Description and location of the source/destination image.
  void *pRGB;
  int RGB_X;
  int RGB_Y;

  //Compressed data absolute limit in bits.
  unsigned int MAX_BITSTREAM_SIZE;

  //Motion Estimation and compensation parameters.
  int Motion; //Motion?
  int MotionLevel; //Wavelet level to start motion.
  int ColourMotion; //Estimate with colour components?
} CVIDCODEC_STRUCT;

///////////////////////////////////////////////////////////////////
// Vector quantiser definition.

typedef struct tagVLC_TYPE
{
  int numbits;
  int bits;
} VLC_TYPE;

typedef struct tagVEC_QUANT_TYPE
{
  int vx;
  int vy;
  int vdim;
  int CodebookLength;
  fixedpoint *Codebook;
  VLC_TYPE *CodebookVlc;
} VEC_QUANT_TYPE;

//**********************************************************
// Define the information for each wavelet sub-band.
typedef struct tagRUNLENGTH_TYPE
{
  int run;
  int val;
} RUNLENGTH_TYPE;

typedef struct tagMOTION_VEC_TYPE
{
  int x;
  int y;
} MOTION_VEC_TYPE;

typedef struct tagSUBBND_INFO_TYPE
{
  int Level;
  int Colour;
  int Orient;
  // The vector.
  VEC_QUANT_TYPE Vq;
  // The sub-band sequence.
  VICS_COORD *SubBndSeq;
  int SeqLength;
  // The sub-band.
  fixedpoint *pPel; //Top left corner of sub-band.
  int sX; //Size of sub-band.
  int sY;

  RUNLENGTH_TYPE VqRunLen[MAX_POSSIBLE_VECTORS]; //Run-length collector.
  int VqRunLenLength; //No. of valid structures in RunLen[].
  int VqRunLenPos;
  int VqDone;

  // The reference sub-band.
  fixedpoint *pRPel;

  // The motion compensated reference sub-band.
  int MotionEstimationRequired; // Short cut to dicision.
  fixedpoint *pCRPel;
  MOTION_VEC_TYPE MotionVec[MAX_POSSIBLE_VECTORS]; // Vector collector. (rhymes!)
  int MotionRun[MAX_POSSIBLE_VECTORS]; //Motion Run-length collector.
  int MotionRunLength; //No. of valid structures in MotionRun[].
  int MotionRunPos;
  int MotionDone;

} SUBBND_INFO_TYPE;

class CVIDCODEC
{
// Attributes
protected:
	HGLOBAL hL_b;
  fixedpoint *L_b; //Line buffer;
	HGLOBAL hY;
	HGLOBAL hU;
	HGLOBAL hV;

  //Intermediate variables.
  fixedpoint *Y; //Lum to compress from and decompress to.
  int Y_X; //Lum pix width.
  int Y_Y; //Lum pix height.
  fixedpoint *U;
  fixedpoint *V;
  int UV_X;
  int UV_Y;
  //Reference variables.
	HGLOBAL hrY;
	HGLOBAL hrU;
	HGLOBAL hrV;
  fixedpoint *rY;
  fixedpoint *rU;
  fixedpoint *rV;
  //Compensated reference variables.
	HGLOBAL hcrY;
	HGLOBAL hcrU;
	HGLOBAL hcrV;
  fixedpoint *crY;
  fixedpoint *crU;
  fixedpoint *crV;
  /*8-half pix x 8-half pix + extended boundary.*/
  fixedpoint SrchWin[(2*4*3)*(2*4*3)];

  int Y_l; //Lum DWT levels.
  int UV_l;

  CVIDCODEC_STRUCT PS;
  //Quantisation tables.
  int Y_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int U_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int V_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IY_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IU_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IV_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];

  int CodecIsOpen;

  //Operational parameters.
  SUBBND_INFO_TYPE SubBndInfo[CODEC_MAX_LEVELS*NUM_OF_COLOUR_COMP*NUM_OF_SUBBANDS];
  SUBBND_GROUP_TYPE *pGroup;
  int GroupLen;
  //Vector work spaces.
  fixedpoint InVec[MAX_VECTOR_SIZE]; //Input vector;
  fixedpoint DRVec[MAX_VECTOR_SIZE]; //Difference vectors.
  fixedpoint DCRVec[MAX_VECTOR_SIZE];

	char *ErrorStr;
  //Compressed data location.
  unsigned int *BitStream;
  unsigned int BitStreamSize; //In bits.

public:
  //Partition bit positions.
  long int PartitionBitPos[LEV4YUV411_ORDER_LENGTH];

// Implementation
public:
	CVIDCODEC();
	~CVIDCODEC();
  CVIDCODEC_STRUCT GetVIDCODECParams(void) { return(PS); }

	int Open(CVIDCODEC_STRUCT *Params);
  int CodecOpen(void) { return(CodecIsOpen); }
	int Code(void *pI,void *pCmp,unsigned int FrameBitLimit);
	int Decode(void *pCmp,unsigned int FrameBitSize,void *pI);
	void Close(void);
  char *GetErrorStr(void) { return(ErrorStr); }
  unsigned int GetCompressedBitLength(void) { return(BitStreamSize); }
  void ZeroRefPartition(int partition);

protected:
  int ConstructMarkerImage(SUBBND_INFO_TYPE *Info);
  int UpdateMarkerImage(SUBBND_INFO_TYPE *Info);
  int GetMaxLevels(int x,int y);
  void Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
  void IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
  void Dwt1D(fixedpoint *x,int Len,fixedpoint *y_Low,fixedpoint *y_High);
  void IDwt1D(fixedpoint *x_Low,fixedpoint *x_High,int Len,fixedpoint *Recon);

  int RGB24toYUV411(void *pRGB,int RGB_X,int RGB_Y,
                    fixedpoint *pY,fixedpoint *pU,fixedpoint *pV);
  int YUV411toRGB24(fixedpoint *pY,fixedpoint *pU,fixedpoint *pV,
                    void *pRGB,int RGB_X,int RGB_Y);

  unsigned int *PutBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToAdd,
                        int CodeBits);
  unsigned int *GetBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToGet,
                        int *CodeBits);
  unsigned int *GetNextBit(unsigned int *CurrPtr,int *NextBitPos,int *CodeBit);
  int ConstructSubBndInfo(void);
  void LoadVectorQuant(VEC_QUANT_TYPE *VQ,int level,int colour,int orient);
  int LoadSubBndSequence(SUBBND_INFO_TYPE *Info);
  void CodeSubBnd(SUBBND_INFO_TYPE *Info);
  void DecodeSubBndRef(SUBBND_INFO_TYPE *Info);
  void DecodeSubBnd(SUBBND_INFO_TYPE *Info);
  void MotionCompSubBnd(SUBBND_INFO_TYPE *Info);

  int Threshold(int dpix, int th);
  int InvThreshold(int dpix, int th);
  int VectorQuantize(VEC_QUANT_TYPE *VQ,fixedpoint *Vec, long *Dist);
  int GetRunBits(int Run,int *CdeWord);
  unsigned int *ExtractRunBits(unsigned int *CurrPtr,int *NextBitPos,
                               int *Run,int *BitsExtracted,int *MarkerFlag,int *Codeword);
  int GetMotionVecBits(int VecCoord,int *CdeWord);
  unsigned int *ExtractMotionVecBits(unsigned int *CurrPtr,int *NextBitPos,
                                     int *Vec,int *BitsExtracted,int *Codeword);
  int GetVecBits(VEC_QUANT_TYPE *VQ,int VecIndex,int *CdeWord);
  unsigned int *ExtractVqIndexBits(unsigned int *CurrPtr,int *NextBitPos,VEC_QUANT_TYPE *Vq,
                                   int *VqIndex,int *BitsExtracted,int *Codeword);
  void ZeroRefSubBnd(SUBBND_INFO_TYPE *Info,int UpTo);
  void ZeroRefSubBndToEnd(SUBBND_INFO_TYPE *Info,int From);

};//CVIDCODEC class.

#endif
