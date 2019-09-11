//**********************************************************
// TITLE       :DWT QUANTIZER CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :DwtQnt.h
// DESCRIPTION :A class for implementing psychovisual 
//              quantization, linear quantization and 
//              thresholding on a wavelet transformed image.
// DATE        :February 1999
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _DWTQNT_H
#define _DWTQNT_H

#include <math.h>

#define MAX_QNT_LEVELS 6
#define MAX_ORIENT 4

#define Qnt_LxLy 0
#define Qnt_LxHy 1
#define Qnt_HxLy 2
#define Qnt_HxHy 3

#define Qnt_Lum 0
#define Qnt_UChr 1
#define Qnt_VChr 2

// Define parameter structure.
typedef struct tagDWT_QNT_STRUCTURE
  {
    int Cols;
    int Rows;
    int WaveLevels;
    int Colour;
    int PsychoQuality;
    int Threshold;
    int MonitorDiagDim;
    int ViewDist;
    int ScreenXPels;
    int ScreenYPels;
    int ChrToLumXRatio; //YUV411 = 2, YUV422 = 2, YUV444 = 1. 
    int ChrToLumYRatio; //YUV411 = 2, YUV422 = 1, YUV444 = 1.
  } DWT_QNT_STRUCTURE;

class CDWTQNTCODEC
{
private :

protected :
  //members.
	int error;

  DWT_QNT_STRUCTURE CurrParams;

  //methods.
  double Sensitivity(double freq,int level,int Orient,int Scale);

public :
  double Qnt[MAX_ORIENT][MAX_QNT_LEVELS];
  double IQnt[MAX_ORIENT][MAX_QNT_LEVELS];
  int Thresh[MAX_ORIENT][MAX_QNT_LEVELS];

public :
	CDWTQNTCODEC(void);
	~CDWTQNTCODEC(void);

	int Open(void *Params);
	int Code(void *Img);
	int Decode(void *Img);
	void Close(void);
	int Error(void) {int reterr = error;error=0;return(reterr);}
	const char *GetErrorStr(int ErrorNum, char *ErrStr);

};//end CDWTQNTCODEC class.

#endif
