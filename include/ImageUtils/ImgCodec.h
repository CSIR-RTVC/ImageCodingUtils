//**********************************************************
// TITLE       :2-DIM DWT STILL IMAGE COMPRESSION CODEC 
//              CLASS HEADER FILE
// VERSION     :1.0
// FILE        :ImgCodec.h
// DESCRIPTION :A class for implementing a DWT-based still
//              image compression and decompression codec.
//              A YUV422 image is coded into a bit stream 
//              and the bit stream is decoded into a YUV422
//              image.
// DATE        :March 1999
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _IMGCODEC_H
#define _IMGCODEC_H

#include <math.h>

#define CODEC_MAX_LEVELS 6
#define NUM_OF_SUBBANDS 4

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

// Define a structure to hold the codec operational parameters.
// Define parameter structure.
typedef struct tagCIMGCODEC_STRUCT
{
  //Required operating parameters.
  int Q;
  int Bpp; //YUV representation.
  //Description and location of the source image.
  void *pRGB;
  int RGB_X;
  int RGB_Y;
  //Compressed data location.
  unsigned int *BitStream;
  unsigned int MAX_BITSTREAM_SIZE;

} CIMGCODEC_STRUCT;

///////////////////////////////////////////////////////////////////
// VLC structures and tables.

// Define the bit allocation table data format.
typedef struct
{
  int Run;
  int Value;
  int NumBits;
  unsigned int Code;
}CodeWord;

class CIMGCODEC
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

  int Y_l; //Lum DWT levels.
  int UV_l;

  CIMGCODEC_STRUCT PS;
  //Quantisation tables.
  int Y_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int U_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int V_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IY_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IU_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];
  int IV_Q[NUM_OF_SUBBANDS][CODEC_MAX_LEVELS];

  int CodecIsOpen;

public:
	char *ErrorStr;
  unsigned int BitStreamSize; //In bytes.

// Implementation
public:
	CIMGCODEC();
	~CIMGCODEC();

	int Open(CIMGCODEC_STRUCT *Params);
  int CodecOpen(void) { return(CodecIsOpen); }

	int Code();
	int Decode();

	void Close(void);

  int RGB24toYUV422(void);
  int YUV422toRGB24(void);

protected:
  int GetMaxLevels(int x,int y);
  void Dwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
  void IDwt2D(fixedpoint *locY,fixedpoint *locU,fixedpoint *locV);
  void Dwt1D(fixedpoint *x,int Len,fixedpoint *y_Low,fixedpoint *y_High);
  void IDwt1D(fixedpoint *x_Low,fixedpoint *x_High,int Len,fixedpoint *Recon);
  int RunLengthCode(void);
  int RunLengthDecode(void);
  int FindBits(int Run,int Value,int *NumCodeBits,int *CdeWord);
  unsigned int *PutBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToAdd,
                        int CodeBits);
  unsigned int *GetBits(unsigned int *CurrPtr,int *NextBitPos,int BitsToGet,
                        int *CodeBits);
  unsigned int *GetNextBit(unsigned int *CurrPtr,int *NextBitPos,int *CodeBit);
  unsigned int *GetRunLevel(unsigned int *CurrPtr,int *NextBitPos,
                            int *Run, int *Level,int *BitsExtracted);
  unsigned int *FastGetRunLevel(unsigned int *CurrPtr,int *NextBitPos,
                            int *Run, int *Level,int *BitsExtracted);

};//CIMGCODEC class.

#endif
