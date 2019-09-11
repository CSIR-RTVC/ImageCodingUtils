//**********************************************************
// TITLE       :H263 VIDEO COMPRESSION CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :H263Codec.h
// DESCRIPTION :A class for implementing an H263 sub-set video
//              compression and decompression codec.
//              A RGB24/YUV411 video is coded into a bit stream 
//              and the bit stream is decoded into a RGB24/YUV411
//              image.
// DATE        :February 2000
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _H263CODEC_H
#define _H263CODEC_H

#include <math.h>

#define NUM_OF_COLOUR_COMP 3
#define BLOCK_SIZE   64

#define Col_Y 0
#define Col_U 1
#define Col_V 2

#define PARTITIONS  (BLOCK_SIZE*NUM_OF_COLOUR_COMP) //8x8 block for each colour.

//Types.
typedef short int pel;
typedef short int colfixedpoint;

// Fixed point aritmetic adventure.
#define FxPt 13
#define FxPt_int (VICS_INT32)(1 << FxPt)
#define FxPt_float (double)(FxPt_int)
#define FIXPOINT(x) ((x) << FxPt)
#define FIXPOINTADJUST(x) ((x) >> FxPt)

// Colour conversions that include 255 -> 0.999, 8 pts.
#define CFIXPOINT(x) ((x) << 5) // 5 = 13 - 8.
#define CFIXPOINTADJUST(x) ((x) >> 18) //18 = 13 + 5.

// DCT operate with a different fixed point.
#define DctFxPt 9
#define DctFxPt_int 512 
#define DctFxPt2x 18 
#define DctFxPt_double (double)(512.0) 

#define Conv(x) (long int)((x) * DctFxPt_double)
#define DCTFIXPOINT(x) (long int)((x) << DctFxPt)
#define DCTFIXPOINTADJUST(x) (long int)((x) >> DctFxPt)

///////////////////////////////////////////////////////////////////
// Define a structure to hold the codec operational parameters.
typedef struct tagCH263CODEC_STRUCT
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
  int Motion; //Motion? 0 = None, 1 = 1 pix, 2 = 1/2 pix.
} CH263CODEC_STRUCT;

///////////////////////////////////////////////////////////////////
// Define a structure to hold the difference motion vectors.
typedef struct tagMOTIONVECTOR_STRUCT
{
  short int x;
  short int y;
} MOTIONVECTOR_STRUCT;

///////////////////////////////////////////////////////////////////
// Define a structure to hold the variable length codes.
typedef struct tagH236_VLC_TYPE
{
  int last;
  int run;
  int level;
  int numbits;
  int bits;
} H263_VLC_TYPE;

///////////////////////////////////////////////////////////////////
// Define a structure for each partition.
typedef struct tagH263_PARTITION_TYPE
{
  int Colour;
  COORD Base_coeff_pos; //Defines the 8x8 block pos for this partition.
  int CODE_STRUCT_SIZE; //Dependent on num of 8x8 blocks for colour.
  H263_VLC_TYPE *pCodes;
  HGLOBAL hCodes;
  int structs_coded;
  int structs_pos;
  int coeffs_coded;
  int COD;
  int done;
} H263_PARTITION_TYPE;

class CH263CODEC
{
// Attributes
protected:
	HGLOBAL hY;
	HGLOBAL hU;
	HGLOBAL hV;

  //Intermediate variables.
  pel *Y; //Lum to compress from and decompress to.
  int Y_X; //Lum pix width.
  int Y_Y; //Lum pix height.
  pel *U;
  pel *V;
  int UV_X;
  int UV_Y;
  //Reference variables.
	HGLOBAL hrY;
	HGLOBAL hrU;
	HGLOBAL hrV;
  pel *rY;
  pel *rU;
  pel *rV;
  //Compensated reference variables.
	HGLOBAL hcrY;
	HGLOBAL hcrU;
	HGLOBAL hcrV;
  pel *crY;
  pel *crU;
  pel *crV;

  //Motion vector variables.
	HGLOBAL hMVD;
  MOTIONVECTOR_STRUCT *pMVD;
  //Partition variables.
  H263_PARTITION_TYPE Partition[PARTITIONS];

  /*8x8/16x16 pix + extended boundary.*/
  pel SrchWin[32*32];

  /* Operational parameters.*/
  CH263CODEC_STRUCT PS;

  //Quantisation values.
  long int QTable[BLOCK_SIZE];
  long int IQTable[BLOCK_SIZE];
  long int ITable[BLOCK_SIZE];
  double FQTable[BLOCK_SIZE];
  double FIQTable[BLOCK_SIZE];

  int CodecIsOpen;
  unsigned long int FrameCounter;

  //Operational parameters.
	char *ErrorStr;
  //Compressed data location.
  unsigned int *BitStream;
  unsigned int BitStreamSize; //In bits.

  // 8x8 Block variables.
  int Y_XBlk;
  int Y_YBlk;
  int UV_XBlk;
  int UV_YBlk;

public:
  //Partition bit positions.
  long int PartitionBitPos[PARTITIONS+1]; //Add 1 for motion vectors.

// Implementation
public:
	CH263CODEC();
	~CH263CODEC();
  CH263CODEC_STRUCT GetH263CODECParams(void) { return(PS); }

	int Open(CH263CODEC_STRUCT *Params);
  int CodecOpen(void) { return(CodecIsOpen); }
	int Code(void *pI,void *pCmp,unsigned int FrameBitLimit);
	int Decode(void *pCmp,unsigned int FrameBitSize,void *pI);
	void Close(void);
  char *GetErrorStr(void) { return(ErrorStr); }
  unsigned int GetCompressedBitLength(void) { return(BitStreamSize); }
  void ZeroRefPartition(int partition);

protected:
  void Dct2D(pel *locY,pel *locU,pel *locV);
  void FDct2D(pel *locY,pel *locU,pel *locV);
  void FDct2DnoQ(pel *locY,pel *locU,pel *locV);
  void IDct2D(pel *locY,pel *locU,pel *locV);
  void IDct2DnoQ(pel *locY,pel *locU,pel *locV);

  int RGB24toYUV411(void *pRGB,int RGB_X,int RGB_Y,
                    pel *pY,pel *pU,pel *pV);
  int YUV411toRGB24(pel *pY,pel *pU,pel *pV,
                    void *pRGB,int RGB_X,int RGB_Y);
  int MotionEstimationAndCompensation(pel *locY,pel *locU,pel *locV);
  int MotionCompensation(void);
  int CodePartitions(pel *locY, pel *locU, pel *locV);
  int DecodePartitions(pel *locY, pel *locU, pel *locV);
  unsigned int *PutBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToAdd,
                        int CodeBits);
  unsigned int *GetBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToGet,
                        int *CodeBits);
  unsigned int *GetNextBit(unsigned int *CurrPtr,int *NextBitPos,int *CodeBit);
  int GetMotionVecBits(int VecCoord,int *CdeWord);
  unsigned int *ExtractMotionVecBits(unsigned int *CurrPtr,
												             int *NextBitPos,int *Vec,int *BitsExtracted,
                                     int *Codeword);
  int GetRunLevelBits(int Last,int Run,int Level,int *CdeWord);
  unsigned int *ExtractRunLevelBits(unsigned int *CurrPtr,int *NextBitPos,
                                    int *Last,int *Run,int *Level,int *Marker,
                                    int *BitsExtracted,int *Codeword);
  int CreatePartitions(void);
  void DeletePartitions(void);
};//CH263CODEC class.

#endif
