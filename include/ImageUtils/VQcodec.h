//**********************************************************
// TITLE       :VECTOR QUANTISER CODEC CLASS HEADER FILE
// VERSION     :1.0
// FILE        :VQcodec.h
// DESCRIPTION :A class for implementing a vector quantiser.
// DATE        :August 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _VQCODEC_H
#define _VQCODEC_H

#include <math.h>
#include "GeneralUtils\Som2d.h"

// Define vlc structure.
typedef struct tagVLC_BITS
  {
    int numbits;
    int codebits;
  } VLC_BITS;

class CVQCODEC : public CSOM2D
{
private :

protected :
  int *map_w_int;

  HGLOBAL h_vlc;
  VLC_BITS *vlc;

  int IntegerBitPrecision;

  int VLCTableExists;

  int CreateIntMemOnCurrParams(void);
  void DestroyIntMem(void);
  void DestroyVLCMem(void);

public :
	CVQCODEC(void);
	~CVQCODEC(void);

	int OpenCODEC(void);
	int OpenCODEC(const char *VQFilename);
  int AssocciateVLC(const char *VLCFilename);
	int CODE(double *vec,int *q_index);
	int CODE(int *vec,int *q_index);
  unsigned long int NumOfCodedBits(int q_index);
	int DECODE(int q_index,double *vec);
	int DECODE(int q_index,int *vec);
	void CloseCODEC(void);
	const char *GetErrorStr(int ErrorNum, char *ErrStr);
  void SetIntegerBitPrecision(int Bpp);

  void UpdateIntMap(void);
  void UpdateIntMap2(void);
  void UpdateRealMap(void);
  void UpdateRealMap2(void);
  double *GetRealMapPtr(void) { return(map); }
  int *GetIntMapPtr(void) { return(map_w_int); }
  int SaveRealMap(const char *VQFilename);
  int SaveIntMap(const char *VQFilename);
  void ImproveIntMapWithAvg(int passes);

};//end CVQCODEC class

#endif
