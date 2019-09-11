// Definition of a CWImgPce class that is used by CWImage
// objects. It provides functionality for holding pieces
// of YUV wavelet transformed images at specific levels and
// positions.
#ifndef _WIMGPCE_H
#define _WIMGPCE_H

// Definition of Wavelet Positions.
#define LOW_X_LOW_Y      0
#define LOW_X_HIGH_Y     1
#define HIGH_X_LOW_Y     2
#define HIGH_X_HIGH_Y    3

// Definition of Colours.
#define COLOUR_LUM   0
#define COLOUR_U_CHR 1
#define COLOUR_V_CHR 2

typedef struct tagYUVINFO_TYPE
{
  int *pY;
  int *pU;
  int *pV;
  int Lum_X;
  int Lum_Y;
  int Chr_X;
  int Chr_Y;
} YUVINFO_TYPE;

class CWImgPce
{
// Attributes
protected:
  YUVINFO_TYPE MainI; //Image piece is attached to this total image.
  int LumLevel;
  int ChrLevel;
  int WvltPos;

  HGLOBAL hImg;

public:
  int DirtyFlag;
  CString ErrorMsg;
  YUVINFO_TYPE PceI;

// Operations
public:
	int GetLumPieceEnergy(void);
	int GetChrPieceEnergy(void);
	int MultLumPiece(double By);
	int MultChrPiece(double By);
	void GetLumPieceHisto(int *H,int Rng);
	void GetChrPieceHisto(int *H,int Rng);

// Implementation
public:
	CWImgPce();
	~CWImgPce();
  int CreateImagePiece(YUVINFO_TYPE *WI,int LumLevel,int ChrLevel,int Position);
  void DestroyImagePiece(void);
  void UpdateImagePiece(void);
  void UpdateYUVImage(void);
  void UpdateImagePiece(int Colour);
  void UpdateYUVImage(int Colour);
  int GetWaveletPosition(void) {return(WvltPos);}

protected:

};//CWImgPce class.

#endif


