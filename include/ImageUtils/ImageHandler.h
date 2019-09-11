/** @file

MODULE						: ImageHandler

TAG								: IH

FILE NAME					: ImageHandler.h

DESCRIPTION				: Definition of a CImage class that holds images in
										DIB .bmp format and provides functions for colour 
										space conversion and the like.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _IMAGEHANDLER_H
#define _IMAGEHANDLER_H

#pragma once

class ImageHandler
{
// Implementation
public:
	ImageHandler();
	virtual ~ImageHandler();

	int						CreateImage(BITMAPINFOHEADER *pbmpih);
	void					DestroyImage(void);
  int						ImageMemIsAlloc(void);
	int						ConvertToYUV(int YUVFormat);
  void					UpdateBmp(void);
	int						GetUVHeight(void);
	int						GetUVWidth(void);
	int						GetYHeight(void);
	int						GetYWidth(void);
  virtual void	SaveBmp(CString& filename);
  virtual void	SaveBmp(void);
  virtual int		LoadBmp(CString& filename);
  virtual int		LoadBmp(void);
  int						SaveRaw(CString& filename);
	CString&			GetErrorStr(void) { return(_errorStr); }

// Operations
public:
	ImageHandler &operator=(ImageHandler &I);
	ImageHandler &operator-=(ImageHandler &I);
	ImageHandler &operator+=(ImageHandler &I);

  int			Copy(ImageHandler& fromImg);
  int			EqualMemFormat(ImageHandler& img);
  void		Absolute(void);
  double	PSNR(ImageHandler& noiseImg);
	int			IncreaseLum(int By);
	int			DecreaseLum(int By);
  int			ChangeLumBlk(int blkWidth, int blkHeight, int posx, int posy, int By);
  void		MirrorXLeft(int PixPoint);
  void		MirrorXRight(int PixPoint);
  void		MirrorYUp(int PixPoint);
  void		MirrorYDown(int PixPoint);
  void		KillColor(void);

protected:
	int						CreateYUVMem(int YUVType);
  void					DestroyYUVMem(void);
  void					ConvertRGB24BITtoYUV411(void);
  void					ConvertYUV411toRGB24BIT(void);
  void					ConvertRGB24BITtoYUV422(void);
  void					ConvertYUV422toRGB24BIT(void);
  void					ConvertRGB24BITtoYUV444(void);
  void					ConvertYUV444toRGB24BIT(void);

	void					RGBtoYUV(double R,double G,double B,
												 double &Y,double &U,double &V);
	void					RGBtoYUV(int R,int G,int B,
												 int &Y,int &U,int &V);
	void					YUVtoRGB(int Y,int U,int V,
												 int &R,int &G,int &B);

// Extra colour types.
public:
	static const int BMP_INFO_SPECIFIES = 0;
	static const int YUV411             = 1;
	static const int YUV422             = 2;
	static const int YUV444             = 3;
	static const int YUV420             = 4;

// Attributes
public:
	unsigned int			_paletteSize;
	void*							_pBmp;
	BITMAPINFOHEADER*	_bmih;
	RGBQUAD*					_bmic;
	void*							_bmptr;

	CString						_errorStr;
	int								_extraColorType;
	int*							_Y;
	int*							_U;
	int*							_V;
	int								_lumWidth;
	int								_lumHeight;
	int								_chrWidth;
	int								_chrHeight;

protected:
  CString						_bmpFilename;

};//ImageHandler class.


#endif	//_IMAGEHANDLER_H
