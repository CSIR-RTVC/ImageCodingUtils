/** @file

MODULE						: Real6BitYUV420toRGB24Converter

TAG								: R6BYUVRGB24C

FILE NAME					: Real6BitYUV420toRGB24Converter.cpp

DESCRIPTION				: Floating point implementation of YUV420 (6 bpp) to RGB 24 bit 
										colour convertion derived from	the YUV420toRGBConverter base 
										class.

REVISION HISTORY	:
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
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

#include "Real6BitYUV420toRGB24Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define R6BYUVRGB24C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

#define R6BYUVRGB24C_U0		 2.032
#define R6BYUVRGB24C_U1		-0.394
#define R6BYUVRGB24C_V1		-0.581
#define R6BYUVRGB24C_V0		 1.140

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void Real6BitYUV420toRGB24Converter::NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb)
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
			double lum00 = (double)((int)(py[lumpos0++]) << 2);	// 6 bit to 8 bit conversion.
			double u		 = (double)(((int)(pu[uvpos]) << 2) - 128);
			double v		 = (double)(((int)(pv[uvpos++]) << 2) - 128);

			// Lum00, u and v.

  		// Fast calculation intermediate variables. 
			double cc = (R6BYUVRGB24C_U0 * u) + 0.5;
			double cb = (R6BYUVRGB24C_U1 * u) + (R6BYUVRGB24C_V1 * v) + 0.5;
			double ca = (R6BYUVRGB24C_V0 * v) + 0.5;

			b = (int)(lum00 + cc);
			g = (int)(lum00 + cb);
			r = (int)(lum00 + ca);
  
			// R, G & B have range 0..255.
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum01.
			double lum01 = (double)((int)(py[lumpos0++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum01 + cc);
			g = (int)(lum01 + cb);
			r = (int)(lum01 + ca);
  
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum10.
			double lum10 = (double)((int)(py[lumpos1++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum10 + cc);
			g = (int)(lum10 + cb);
			r = (int)(lum10 + ca);
  
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum11.
			double lum11 = (double)((int)(py[lumpos1++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum11 + cc);
			g = (int)(lum11 + cb);
			r = (int)(lum11 + ca);
  
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void Real6BitYUV420toRGB24Converter::RotateConvert(void* pY, void* pU, void* pV, void* pRgb)
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
			double lum00 = (double)((int)(py[lumpos0++]) << 2);	// 6 bit to 8 bit conversion.
			double u		 = (double)(((int)(pu[uvpos]) << 2) - 128);
			double v		 = (double)(((int)(pv[uvpos++]) << 2) - 128);

			// Lum00, u and v.

  		// Fast calculation intermediate variables. 
			double cc = (R6BYUVRGB24C_U0 * u) + 0.5;
			double cb = (R6BYUVRGB24C_U1 * u) + (R6BYUVRGB24C_V1 * v) + 0.5;
			double ca = (R6BYUVRGB24C_V0 * v) + 0.5;

			b = (int)(lum00 + cc);
			g = (int)(lum00 + cb);
			r = (int)(lum00 + ca);
  
			// R, G & B have range 0..255.
			optr[rgbpos0]		= (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum01.
			double lum01 = (double)((int)(py[lumpos0++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum01 + cc);
			g = (int)(lum01 + cb);
			r = (int)(lum01 + ca);
  
			optr[rgbpos0]		= (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum10.
			double lum10 = (double)((int)(py[lumpos1++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum10 + cc);
			g = (int)(lum10 + cb);
			r = (int)(lum10 + ca);
  
			optr[rgbpos1]		= (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

			// Lum11.
			double lum11 = (double)((int)(py[lumpos1++]) << 2);	// 6 bit to 8 bit conversion.

			b = (int)(lum11 + cc);
			g = (int)(lum11 + cb);
			r = (int)(lum11 + ca);
  
			optr[rgbpos1]		= (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(R6BYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end NonNonRotateConvert.




