/** @file

MODULE						: FastYUV420toInvRGB12Converter

TAG								: FYUVIRGB12C

FILE NAME					: FastYUV420toInvRGB12Converter.cpp

DESCRIPTION				: Fast YUV420 to inverted RGB 12 bit colour convertion 
										derived from the FastYUV420toRGBConverter base class.

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

#include "FastYUV420toInvRGB12Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define FYUVIRGB12C_RANGECHECK_0TO15(x) ( (((x) <= 15)&&((x) >= 0))?((x)):( ((x) > 15)?(15):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void FastYUV420toInvRGB12Converter::NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_INT16* optr	=	(VICS_INT16 *)pRgb;
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
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) >> 2;	// 6 bit to 4 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

  		// Fast calculation with bit resolution conversion built in. 
			// Method 1. All in one but larger accumulated rounding errors.
			//VICS_INT cc =  (u >> 1) - 15;
			//VICS_INT cb = -(u >> 4) - (u >> 5) - (v >> 3) + 8;
			//VICS_INT ca =  (v >> 2) + (v >> 5) - 9;
			// Method 2. Calc in 6 bit with extra accuracy terms and post shift to 4 bit.
			//           Rounding is included in the const. as an offset noting that the
			//           final result is to be positive.
			VICS_INT cc =  ((u << 1) + (u >> 5) - 63) >> 2;
			VICS_INT cb =  (-(u >> 2) - (u >> 3) - (v >> 1) - (v >> 4) + 33) >> 2;
			VICS_INT ca =  (v + (v >> 3) - 35) >> 2;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos0++]	= b | (g << 4) | (r << 8);

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos0++]	= b | (g << 4) | (r << 8);

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos1++]	= b | (g << 4) | (r << 8);

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos1++]	= b | (g << 4) | (r << 8);

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void FastYUV420toInvRGB12Converter::RotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_INT16* optr	=	(VICS_INT16 *)pRgb;
	VICS_INT		lumX	= _width;
	VICS_INT		lumY	= _height;
	VICS_INT		uvX		= _width >> 1;

	VICS_INT x,y;
	VICS_INT lumposy;
	VICS_INT rgbposx;
	VICS_INT uvposy;
  VICS_INT r,b,g;

	VICS_INT tworows  = lumX << 1;
	//VICS_INT rgb1row  = _width;
	VICS_INT rgb1row  = _height;

	for(y = 0,lumposy = 0,uvposy = 0,rgbposx = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposx += 2)
	{
		VICS_INT lumpos0 = lumposy;					// Reset to start of rows.
		VICS_INT lumpos1 = lumposy + lumX;
		VICS_INT uvpos	 = uvposy;

		VICS_INT rgbpos0 = rgbposx;
		VICS_INT rgbpos1 = rgbposx + 1;

		for(x = 0; x < lumX; x += 2)
		{
			VICS_INT lum00 = (VICS_INT)(pY[lumpos0++]) >> 2;	// 6 bit to 4 bit conversion.

			VICS_INT u		 = (VICS_INT)(pU[uvpos]);
			VICS_INT v		 = (VICS_INT)(pV[uvpos++]);

			// Lum00, u and v.

  		// Fast calculation with bit resolution conversion built in. 
			// Method 1. All in one but larger accumulated rounding errors.
			//VICS_INT cc =  (u >> 1) - 15;
			//VICS_INT cb = -(u >> 4) - (u >> 5) - (v >> 3) + 8;
			//VICS_INT ca =  (v >> 2) + (v >> 5) - 9;
			// Method 2. Calc in 6 bit with extra accuracy terms and post shift to 4 bit.
			//           Rounding is included in the const. as an offset noting that the
			//           final result is to be positive.
			VICS_INT cc =  ((u << 1) + (u >> 5) - 63) >> 2;
			VICS_INT cb =  (-(u >> 2) - (u >> 3) - (v >> 1) - (v >> 4) + 33) >> 2;
			VICS_INT ca =  (v + (v >> 3) - 35) >> 2;

			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos0]	= b | (g << 4) | (r << 8);
			rgbpos0 += rgb1row;

			// Lum01.
			VICS_INT lum01	= (VICS_INT)(pY[lumpos0++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos0]	= b | (g << 4) | (r << 8);
			rgbpos0 += rgb1row;

			// Lum10.
			VICS_INT lum10	= (VICS_INT)(pY[lumpos1++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos1]	= b | (g << 4) | (r << 8);
			rgbpos1 += rgb1row;

			// Lum11.
			VICS_INT lum11	= (VICS_INT)(pY[lumpos1++]) >> 2;	// 6 bit to 4 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			b = FYUVIRGB12C_RANGECHECK_0TO15(b);
			g = FYUVIRGB12C_RANGECHECK_0TO15(g);
			r = FYUVIRGB12C_RANGECHECK_0TO15(r);

			optr[rgbpos1]	= b | (g << 4) | (r << 8);
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.




