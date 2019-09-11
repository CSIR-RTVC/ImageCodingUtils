/** @file

MODULE						: ImageHandler

TAG								: IH

FILE NAME					: ImageHandler.h

DESCRIPTION				: A Microsoft Managed class  that owns and operates on 
										DIB .bmp formatted images. Also provides functions for 
										colour space conversion and the like.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#pragma once

#include "IExistance.h"
#include <windows.h>

using namespace System;

public __gc class ImageHandler : public IExistance
{
// Implementation
public:
	ImageHandler();
	virtual ~ImageHandler();

	// IExistance interface.
	int			Exists(void)			{ return(ImageMemIsAlloc()); }
	String* GetErrorStr(void)	{ return(_errorStr); }

	int		CreateImage(BITMAPINFOHEADER* pbmpih);
	void	DestroyImage(void);
  int		ImageMemIsAlloc(void);
	int		ConvertToYUV(int YUVFormat);
  void	UpdateBmp(void);

	int		GetUVHeight(void);
	int		GetUVWidth(void);
	int		GetYHeight(void);
	int		GetYWidth(void);

  virtual void	SaveBmp(String* filename);
  virtual void	SaveBmp(void);
  virtual int		LoadBmp(String* filename);
  virtual int		LoadBmp(void);
  int						SaveRaw(String* filename);

// Operations
public:
	//ImageHandler		&operator=(ImageHandler &I);
	//ImageHandler		&operator-=(ImageHandler &I);
	//ImageHandler		&operator+=(ImageHandler &I);

	// Operator overloads are rubbish in managed C++.
	int Assign(ImageHandler& I);
	int Subtract(ImageHandler &I);
	int Add(ImageHandler &I);

  int			Copy(ImageHandler& FromImg);
  int			EqualMemFormat(ImageHandler& Img);
  void		absolute(void);
  double	PSNR(ImageHandler &NoiseImg);
	int			IncreaseLum(int By);
	int			DecreaseLum(int By);
  void		MirrorXLeft(int PixPoint);
  void		MirrorXRight(int PixPoint);
  void		MirrorYUp(int PixPoint);
  void		MirrorYDown(int PixPoint);
  void		KillColor(void);

protected:
	int CreateYUVMem(int YUVType);
  void DestroyYUVMem(void);
  void ConvertRGB24BITtoYUV411(void);
  void ConvertYUV411toRGB24BIT(void);
  void ConvertRGB24BITtoYUV422(void);
  void ConvertYUV422toRGB24BIT(void);
  void ConvertRGB24BITtoYUV444(void);
  void ConvertYUV444toRGB24BIT(void);

	static void RGBtoYUV(double R,double G,double B,
											 double &Y,double &U,double &V);
	static void RGBtoYUV(int R,int G,int B,
											 int &Y,int &U,int &V);
	static void YUVtoRGB(int Y,int U,int V,
											 int &R,int &G,int &B);
// Extra colour type constants.
public:
	static const int BMP_INFO_SPECIFIES	= 0;
	static const int YUV411							= 1;
	static const int YUV422							= 2;
	static const int YUV444							= 3;

// Attributes
public:
	int								_paletteSize;
	void*							_pbmp;	// Mem that holds the whole struct and data contiguously.
	BITMAPINFOHEADER* _bmih;
	RGBQUAD*					_bmic;
	void*							_bmptr;

	String*						_errorStr;
	int								_extraColorType;
	int*							_Y;
	int*							_U;
	int*							_V;

protected:
  String*						_bmpFilename;

};//ImageHandler class.


