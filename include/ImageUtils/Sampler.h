//**********************************************************
// TITLE       :RANDOM SAMPLER CLASS HEADER FILE
// VERSION     :1.0
// FILE        :Sampler.h
// DESCRIPTION :A class for implementing a random vector 
//              sampler for use in training SOMs.
// DATE        :October 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _SAMPLER_H
#define _SAMPLER_H

// Define a sampler info structure.
typedef struct tagSAMPLER_INFO
  {
    int vec_x_dim;
    int vec_y_dim;
    int space_x_dim;
    int space_y_dim;
  } SAMPLER_INFO;

class CSAMPLER
{
private :

protected :
  HGLOBAL h_space;
  double *space;
  SAMPLER_INFO CurrParam;

  int XBlks;
  int YBlks;
  int LastX;
  int LastY;

  int Empty;
  int error;

  int CreateMemOnCurrParams(void);
  void DestroyMem(void);

public :
	CSAMPLER(void);
	~CSAMPLER(void);

	CSAMPLER &operator=(CSAMPLER &S);
	int Open(SAMPLER_INFO *Params);
  int RemoveZerosFromSpace(void);
  double *GetSpacePtr(void) {return(space);}
  int TakeUnbndRandomSample(double *vec);
  int TakeBndRandomSample(double *vec);
  int TakeNonZeroRandomSample(double *vec);
  int TakeSample(double *vec,int x_loc,int y_loc);
  int OverwriteSample(double *vec,int x_loc,int y_loc);
  int GetTotalSamplerVectors(void) {return(YBlks*XBlks);}
  int GetSamplerXVectors(void) {return(XBlks);}
  int GetSamplerYVectors(void) {return(YBlks);}
  void GetLastSampleCoord(int *X,int *Y) {*X = LastX; *Y = LastY;}
  void GetLastSampleBlkCoord(int *XB,int *YB) {*XB = LastX/XBlks; *YB = LastY/YBlks;}
  double *GetVectorLocation(int XBlkNum,int YBlkNum,int *x_loc,int *y_loc);
	void Close(void);
	int Error(void) {int reterr = error;error=0;return(reterr);}
	const char *GetErrorStr(int ErrorNum, char *ErrStr);
	void SetParameters(SAMPLER_INFO *Params);
  SAMPLER_INFO *GetParameters(void);

};//end CSAMPLER class

#endif


