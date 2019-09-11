//**********************************************************
// TITLE       :DWT CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :Dwtcodec.h
// DESCRIPTION :A class for implementing a discrete wavelet
//              transform on images. This is a derived class 
//              from the abstract class of type CODEC.
// DATE        :December 1997
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _DWTCODEC_H
#define _DWTCODEC_H

#include <math.h>

//Base abstract class
#include "codec.h"
#include "cimage.h"

// Define Wavelet transform Types.
#define STD_FILTER_6_10       0
#define STD_FILTER_18_18      1
#define STD_FILTER_24_20      2
#define STD_FILTER_5_3        3
#define STD_FILTER_9_7        4
#define STD_FILTER_13_11      5
#define FAST_LINEAR           6
#define FAST_CUBIC_4          7
#define FAST_FILTER_5_3       8
#define FAST_FILTER_9_7       9
#define FAST_FILTER_13_11    10
#define NN_6HID_FILTER_9_7   11
#define NN_8HID_FILTER_13_11 12
#define NN_16HID_FILTER_9_7  13

// Define Convolulion Types.
#define BOUNDARY_REFLECTION   0
#define CIRCULAR              1
#define BOUNDARY_REPEAT       2

// Define biorthogonal filters.
typedef struct tagFILTERTYPE
	{
		double *ph;int h_off;int Num_h;
		double *pg;int g_off;int Num_g;
		double *pinv_h;int inv_h_off;int Num_inv_h;
		double *pinv_g;int inv_g_off;int Num_inv_g;
		double *pinv_h_even;int inv_h_even_off;int Num_inv_h_even;
		double *pinv_h_odd;int inv_h_odd_off;int Num_inv_h_odd;
		double *pinv_g_even;int inv_g_even_off;int Num_inv_g_even;
		double *pinv_g_odd;int inv_g_odd_off;int Num_inv_g_odd;
	} FILTERTYPE;
typedef struct tagFASTFILTERTYPE
	{
		double *pPred;int PredLen;
		double *pUpdate;int UpdateLen;
	} FASTFILTERTYPE;
typedef struct tagNNFILTERTYPE
	{
		double *pPred;int HidLay;int PredLen;
    double *pLin;int LinLen;
		double *pUpdate;int UpdateLen;
	} NNFILTERTYPE;

// Define parameter structure.
typedef struct tagDWT_TYPE
  {
    int WaveletType;
    int ConvType;
    int LumLevels;
    int ChrLevels;
  } DWT_TYPE;

class CDWTCODEC : public CODEC
{
private :

protected :
  HGLOBAL hWk;
  int *Wk;
  FILTERTYPE F;
  FASTFILTERTYPE Ff;
  NNFILTERTYPE Fn;
  DWT_TYPE CurrDwtParam;

  void SetFilter(int WaveType);
  void Dwt(int WaveType,int *x,int Len,int *y_Low,int *y_High);
  void IDwt(int WaveType,int *x_Low,int *x_High,int Len,int *Recon);

  void DWT_1D(int *x,int Len,int *y_Low,int *y_High);
  void IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon);
  void IDWT_1D(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon);

  void FAST_DWT_1D_CUBIC4(int *x,int Len,int *y_Low,int *y_High);
  void FAST_IDWT_1D_CUBIC4(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon);
  void FAST_IDWT_1D_CUBIC4(int *x_Low,int *x_High,int Len,int *Recon);
  int CubicSpline4(int *x_4,double *left,double *right);

  void FAST_DWT_1D_LINEAR(int *x,int Len,int *y_Low,int *y_High);
  void FAST_IDWT_1D_LINEAR(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon);
  void FAST_IDWT_1D_LINEAR(int *x_Low,int *x_High,int Len,int *Recon);

  void FAST_DWT_1D(int *x,int Len,int *y_Low,int *y_High);
  void FAST_IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon);

  void NN_DWT_1D(int *x,int Len,int *y_Low,int *y_High);
  void NN_IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon);

public :
	CDWTCODEC(void);
	~CDWTCODEC(void);

	int OpenCODEC(void *Img);
	int CODE(void *Img);
	int DECODE(void *Img);
	void CloseCODEC(void);
	const char *GetErrorStr(int ErrorNum, char *ErrStr);
	void SetParameters(void *Params);

};//end CDWTCODEC class

#endif
