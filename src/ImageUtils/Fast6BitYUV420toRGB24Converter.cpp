/** @file

MODULE						: Fast6BitYUV420toRGB24Converter

TAG								: F6BYUVRGB24C

FILE NAME					: Fast6BitYUV420toRGB24Converter.cpp

DESCRIPTION				: Fast YUV420 (6 bpp) to RGB 24 bit colour convertion derived from
										the YUV420toRGBConverter base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2006  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "Fast6BitYUV420toRGB24Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define F6BYUVRGB24C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void Fast6BitYUV420toRGB24Converter::NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb)
{
	unsigned char* 	optr	=	(unsigned char *)pRgb;
	yuvType*				py		= (yuvType *)pY;
	yuvType*				pu		= (yuvType *)pU;
	yuvType*				pv		= (yuvType *)pV;
	int		lumX	= _width;
	int		lumY	= _height;
	int		uvX		= _width >> 1;

	int x,y;
	int lumposy;
	int rgbposy;
	int uvposy;
  int r,b,g;

	int tworows  = lumX << 1;
	int rgb1row  = (_width * 3);
	int rgb2rows = (_width * 3) << 1;

	for(y = 0,lumposy = 0,uvposy = 0,rgbposy = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposy += rgb2rows)
	{
		int lumpos0 = lumposy;					// Reset to start of rows.
		int lumpos1 = lumposy + lumX;
		int uvpos	 = uvposy;

		int rgbpos0 = rgbposy;
		int rgbpos1 = rgbposy + rgb1row;

		for(x = 0; x < lumX; x += 2)
		{
			int lum00 = (int)(py[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			int u		 = (int)(pu[uvpos]);
			int v		 = (int)(pv[uvpos++]);

			// Lum00, u and v.

			// Less accurate fast calculation with divide by 16 built in.
//			int cc =  (u >> 1) - 256;
//			int cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
//			int ca =  (v >> 2) + (v >> 5) - 144;

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			int cc =  (u << 3) + (u >> 3) - 260;
			int cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			int ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum01.
			int lum01	= (int)(py[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum10.
			int lum10	= (int)(py[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum11.
			int lum11	= (int)(py[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void Fast6BitYUV420toRGB24Converter::RotateConvert(void* pY, void* pU, void* pV, void* pRgb)
{
	unsigned char* 	optr	=	(unsigned char *)pRgb;
	yuvType*				py		= (yuvType *)pY;
	yuvType*				pu		= (yuvType *)pU;
	yuvType*				pv		= (yuvType *)pV;
	int		lumX	= _width;
	int		lumY	= _height;
	int		uvX		= _width >> 1;

	int x,y;
	int lumposy;
	int rgbposx;
	int uvposy;
  int r,b,g;

	int tworows  = lumX << 1;
	//int rgb1row  = (_width * 3);
	int rgb1row  = (_height * 3);

	for(y = 0,lumposy = 0,uvposy = 0,rgbposx = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposx += 6)
	{
		int lumpos0 = lumposy;					// Reset to start of rows.
		int lumpos1 = lumposy + lumX;
		int uvpos	 = uvposy;

		int rgbpos0 = rgbposx;
		int rgbpos1 = rgbposx + 3;

		for(x = 0; x < lumX; x += 2)
		{
			int lum00 = (int)(py[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			int u		 = (int)(pu[uvpos]);
			int v		 = (int)(pv[uvpos++]);

			// Lum00, u and v.

			// Less accurate fast calculation with divide by 16 built in.
//			int cc =  (u >> 1) - 256;
//			int cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
//			int ca =  (v >> 2) + (v >> 5) - 144;

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			int cc =  (u << 3) + (u >> 3) - 260;
			int cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			int ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbpos0]		= (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum01.
			int lum01	= (int)(py[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			optr[rgbpos0]		= (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum10.
			int lum10	= (int)(py[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			optr[rgbpos1]		= (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

			// Lum11.
			int lum11	= (int)(py[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			optr[rgbpos1]		= (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(F6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end NonNonRotateConvert.




