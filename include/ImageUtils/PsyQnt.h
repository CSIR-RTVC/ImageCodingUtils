//**********************************************************
// TITLE       :PSYCHOVISUAL DWT QUANTIZER CODEC CLASS 
//              HEADER FILE
// VERSION     :1.0
// FILE        :PsyQnt.h
// DESCRIPTION :A class for implementing psychovisual 
//              quantization on a wavelet transformed image.
//              This is a derived class from the abstract 
//              class of type CODEC.
// DATE        :February 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _PSYQNT_H
#define _PSYQNT_H

#include <math.h>
#include "WImage.h"

//Base abstract class
#include "codec.h"

// Define parameter structure.
typedef struct tagQNT_PARAMTYPE
  {
    int Cols;
    int Rows;
    int WaveLevels;
    int Quality;
    int MonitorDiagDim;
    int ViewDist;
    int ScreenXPels;
    int ScreenYPels;
  } QNT_PARAMTYPE;

class CPSYWQNTCODEC : public CODEC
{
private :

protected :
  //members.
  HGLOBAL hQnt;

  QNT_PARAMTYPE CurrParams;

  //methods.
  double Sensitivity(double SpatialFreq);

public :
  double *pQnt;
  double *pIQnt;

public :
	CPSYWQNTCODEC(void);
	~CPSYWQNTCODEC(void);

	int OpenCODEC(void *Params);
	int CODE(void *Img);
	int DECODE(void *Img);
	void CloseCODEC(void);
	const char *GetErrorStr(int ErrorNum, char *ErrStr);

};//end CPSYWQNTCODEC class

#endif
