//**********************************************************
// TITLE       :DCT 2-D ODD CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :Dct2dodd.h
// DESCRIPTION :A class for implementing an odd 2 dimensional
//              discrete cosine transform on images. This is 
//              a derived class from the abstract class of 
//              type CODEC.
// DATE        :February 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _DCT2DODD_H
#define _DCT2DODD_H

#include <math.h>

//Base abstract class
#include "codec.h"

// Define parameter structure.
typedef struct tagDCT_PARAMTYPE
  {
    int Cols;
    int Rows;
  } DCT_PARAMTYPE;

class CDCTODDCODEC : public CODEC
{
private :

protected :
  //members.
  HGLOBAL hCoeff;
  double *pRFCoeff;
  double *pRICoeff;
  double *pCFCoeff;
  double *pCICoeff;
  int *pWk;

  DCT_PARAMTYPE CurrParams;

  //methods.
  void MatMult(int x,int y,double *Mat,double *Vin,double *Vout);

public :
	CDCTODDCODEC(void);
	~CDCTODDCODEC(void);

	int OpenCODEC(void *Params);
	int CODE(void *Img);
	int DECODE(void *Coeff);
	void CloseCODEC(void);
	const char *GetErrorStr(int ErrorNum, char *ErrStr);

};//end CDCTODDCODEC class

#endif
