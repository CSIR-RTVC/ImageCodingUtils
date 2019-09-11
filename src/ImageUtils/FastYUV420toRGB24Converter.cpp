/** @file

MODULE						: FastYUV420toRGB24Converter

TAG								: FYUVRGB24C

FILE NAME					: FastYUV420toRGB24Converter.cpp

DESCRIPTION				: Fast YUV420 to RGB 24 bit colour convertion derived from
										the FastYUV420toRGBConverter base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

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

#include "FastYUV420toRGB24Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define FYUVRGB24C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void FastYUV420toRGB24Converter::NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_BYTE* 	optr	=	(VICS_BYTE *)pRgb;
	VICS_INT		lumX	= _width;
	VICS_INT		lumY	= _height;
	VICS_INT		uvX		= _width >> 1;

	VICS_INT x,y;
	VICS_INT lumposy;
	VICS_INT rgbposy;
	VICS_INT uvposy;
  VICS_INT r,b,g;

	VICS_INT tworows  = lumX << 1;
	VICS_INT rgb1row  = (_width * 3);
	VICS_INT rgb2rows = (_width * 3) << 1;

	for(y = 0,lumposy = 0,uvposy = 0,rgbposy = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposy += rgb2rows)
	{
		VICS_INT lumpos0 = lumposy;					// Reset to start of rows.
		VICS_INT lumpos1 = lumposy + lumX;
		VICS_INT uvpos	 = uvposy;

		VICS_INT rgbpos0 = rgbposy;
		VICS_INT rgbpos1 = rgbposy + rgb1row;

		for(x = 0; x < lumX; x += 2)
		{
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

			// Less accurate fast calculation with divide by 16 built in.
//			VICS_INT cc =  (u >> 1) - 256;
//			VICS_INT cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
//			VICS_INT ca =  (v >> 2) + (v >> 5) - 144;

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			VICS_INT cc =  (u << 3) + (u >> 3) - 260;
			VICS_INT cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			VICS_INT ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void FastYUV420toRGB24Converter::RotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_BYTE* 	optr	=	(VICS_BYTE *)pRgb;
	VICS_INT		lumX	= _width;
	VICS_INT		lumY	= _height;
	VICS_INT		uvX		= _width >> 1;

	VICS_INT x,y;
	VICS_INT lumposy;
	VICS_INT rgbposx;
	VICS_INT uvposy;
  VICS_INT r,b,g;

	VICS_INT tworows  = lumX << 1;
	//VICS_INT rgb1row  = (_width * 3);
	VICS_INT rgb1row  = (_height * 3);

	for(y = 0,lumposy = 0,uvposy = 0,rgbposx = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposx += 6)
	{
		VICS_INT lumpos0 = lumposy;					// Reset to start of rows.
		VICS_INT lumpos1 = lumposy + lumX;
		VICS_INT uvpos	 = uvposy;

		VICS_INT rgbpos0 = rgbposx;
		VICS_INT rgbpos1 = rgbposx + 3;

		for(x = 0; x < lumX; x += 2)
		{
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

			// Less accurate fast calculation with divide by 16 built in.
//			VICS_INT cc =  (u >> 1) - 256;
//			VICS_INT cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
//			VICS_INT ca =  (v >> 2) + (v >> 5) - 144;

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			VICS_INT cc =  (u << 3) + (u >> 3) - 260;
			VICS_INT cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			VICS_INT ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbpos0]		= (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) << 2;	// 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			optr[rgbpos0]		= (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			optr[rgbpos1]		= (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 bit to 8 bit conversion

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			optr[rgbpos1]		= (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (VICS_BYTE)(FYUVRGB24C_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end NonNonRotateConvert.




