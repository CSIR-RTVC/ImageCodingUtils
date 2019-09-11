/** @file

MODULE						: FastYUV420toInvRGB32Converter

TAG								: FYUVIRGB32C

FILE NAME					: FastYUV420toInvRGB18Converter.cpp

DESCRIPTION				: Fast YUV420 to inverted RGB 32 bit colour convertion 
										derived from the FastYUV420toRGBConverter base class.
										The 24 colour bits are packed in a 32 bit word.

REVISION HISTORY	:

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

#include "FastYUV420toInvRGB32Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define FYUVIRGB32C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void FastYUV420toInvRGB32Converter::NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_INT32* optr	=	(VICS_INT32 *)pRgb;
	VICS_INT		lumX	= _width;
	VICS_INT		lumY	= _height;
	VICS_INT		uvX		= _width >> 1;

	VICS_INT x,y;
	VICS_INT lumposy;
	VICS_INT rgbposy;
	VICS_INT uvposy;
  VICS_INT r,b,g;

	VICS_INT tworows  = lumX << 1;
	VICS_INT rgb1row  = _width;
	VICS_INT rgb2rows = _width << 1;

	for(y = 0,lumposy = 0,uvposy = 0,rgbposy = ((_height-1)*rgb1row);	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposy -= rgb2rows)
	{
		VICS_INT lumpos0 = lumposy;					// Reset to start of rows.
		VICS_INT lumpos1 = lumposy + lumX;
		VICS_INT uvpos	 = uvposy;

		VICS_INT rgbpos0 = rgbposy;
		VICS_INT rgbpos1 = rgbposy - rgb1row;

		for(x = 0; x < lumX; x += 2)
		{
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) << 2;	// 6 to 8 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			VICS_INT cc =  (u << 3) + (u >> 3) - 260;
			VICS_INT cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			VICS_INT ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos0++]	= b | (g << 8) | (r << 16);

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) << 2;	// 6 to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos0++]	= b | (g << 8) | (r << 16);

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 to 8 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos1++]	= b | (g << 8) | (r << 16);

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 to 8 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos1++]	= b | (g << 8) | (r << 16);

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void FastYUV420toInvRGB32Converter::RotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_INT32* optr	=	(VICS_INT32 *)pRgb;
	VICS_INT		lumX	= _width;
	VICS_INT		lumY	= _height;
	VICS_INT		uvX		= _width >> 1;

	VICS_INT x,y;
	VICS_INT lumposy;
	VICS_INT rgbposx;
	VICS_INT uvposy;
  VICS_INT r,b,g;

	VICS_INT tworows  = lumX << 1;
	VICS_INT rgb1row  = _height;	// Width is now the height.

	for(y = 0,lumposy = 0,uvposy = 0,rgbposx = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposx += 2)
	{
		VICS_INT lumpos0 = lumposy;					// Reset to start of rows.
		VICS_INT lumpos1 = lumposy + lumX;
		VICS_INT uvpos	 = uvposy;

		VICS_INT rgbpos0 = rgbposx;
		VICS_INT rgbpos1 = rgbposx + 1;

		for(x = 0; x < lumX; x += 2)
		{
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) << 2;	// 6 to 8 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

  		// Fast calculation with 6 bit to 8 bit conversion built in. 
			VICS_INT cc =  (u << 3) + (u >> 3) - 260;
			VICS_INT cb = -u - (u >> 1) - (u >> 4) - (v << 1) - (v >> 2) - (v >> 4) + 124;
			VICS_INT ca =  (v << 2) + (v >> 1) + (v >> 4) - 146;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos0]	= b | (g << 8) | (r << 16);
			rgbpos0 += rgb1row;

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) << 2;	// 6 to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos0]	= b | (g << 8) | (r << 16);
			rgbpos0 += rgb1row;

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 to 8 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos1]	= b | (g << 8) | (r << 16);
			rgbpos1 += rgb1row;

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) << 2;	// 6 to 8 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			// R, G & B have ranges 0..255.
			b = FYUVIRGB32C_RANGECHECK_0TO255(b);
			g = FYUVIRGB32C_RANGECHECK_0TO255(g);
			r = FYUVIRGB32C_RANGECHECK_0TO255(r);

			optr[rgbpos1]	= b | (g << 8) | (r << 16);
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end RotateConvert.




