// Definition of a CImage class that holds images in
// DIB .bmp format and provides functions for colour 
// space conversion and the like.
#ifndef _CIMAGE_H
#define _CIMAGE_H

#include <math.h>

// Extra colour types.
#define BMP_INFO_SPECIFIES    0
#define YUV411                1
#define YUV422                2
#define YUV444                3

class CImage
{
// Attributes
protected:
	HGLOBAL hbmp;
	HGLOBAL hY;
	HGLOBAL hU;
	HGLOBAL hV;
  CString BmpFilename;

public:
	UINT m_PaletteSize;
	BITMAPINFOHEADER *bmih;
	RGBQUAD *bmic;
	LPVOID bmptr;

	CString ErrorStr;
	int m_ExtraColorType;
	int *m_Y;
	int *m_U;
	int *m_V;

// Operations
public:
	CImage &operator=(CImage &I);
	CImage &operator-=(CImage &I);
	CImage &operator+=(CImage &I);
  int Copy(CImage &FromImg);
  int EqualMemFormat(CImage &Img);
  void absolute(void);
  double PSNR(CImage &NoiseImg);
	int IncreaseLum(int By);
	int DecreaseLum(int By);
  void MirrorXLeft(int PixPoint);
  void MirrorXRight(int PixPoint);
  void MirrorYUp(int PixPoint);
  void MirrorYDown(int PixPoint);
  void KillColor(void);

// Implementation
public:
	CImage();
	~CImage();

	int CreateImage(BITMAPINFOHEADER *pbmpih);
	void DestroyImage(void);
  int ImageMemIsAlloc(void);
	int ConvertToYUV(int YUVFormat);
  void UpdateBmp(void);
	int GetUVHeight(void);
	int GetUVWidth(void);
	int GetYHeight(void);
	int GetYWidth(void);
  virtual void SaveBmp(CString & Filename);
  virtual void SaveBmp(void);
  virtual int LoadBmp(CString & Filename);
  virtual int LoadBmp(void);
  int SaveRaw(CString & Filename);

protected:
	int CreateYUVMem(int YUVType);
  void DestroyYUVMem(void);
  void ConvertRGB24BITtoYUV411(void);
  void ConvertYUV411toRGB24BIT(void);
  void ConvertRGB24BITtoYUV422(void);
  void ConvertYUV422toRGB24BIT(void);
  void ConvertRGB24BITtoYUV444(void);
  void ConvertYUV444toRGB24BIT(void);
};//CImage class.

inline void RGBtoYUV(double R,double G,double B,
                     double &Y,double &U,double &V);
inline void RGBtoYUV(int R,int G,int B,
                     int &Y,int &U,int &V);
inline void YUVtoRGB(int Y,int U,int V,
                     int &R,int &G,int &B);

#endif
