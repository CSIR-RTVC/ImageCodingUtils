/** @file

MODULE						: FastYUV420toRGB24ZoomConverter

TAG								: FYUVRGB24ZC

FILE NAME					: FastYUV420toRGB24ZoomConverter.cpp

DESCRIPTION				: Fast YUV420 to RGB 24 bit colour convertion with zoom
										functionality derived from the FastYUV420toRGBConverter 
										base class.

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

#include "FastYUV420toRGB24ZoomConverter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define FYUVRGB24ZC_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void FastYUV420toRGB24ZoomConverter::NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb)
{
	VICS_BYTE 	*optr			=	(VICS_BYTE*)pRgb;
	VICS_INT		uvWidth		= _width >> 1;
	VICS_INT		uvHeight	= _height >> 1;

	VICS_DOUBLE scalex = ((VICS_DOUBLE)_width)/((VICS_DOUBLE)_outWidth);
	VICS_DOUBLE scaley = ((VICS_DOUBLE)_height)/((VICS_DOUBLE)_outHeight);

	VICS_INT x,y,i,j;
	VICS_INT lumposx0,lumposy0,lumposx1,lumposy1;
	VICS_INT rgbposx0,rgbposy0,rgbposx1,rgbposy1;
	VICS_INT uvposx,uvposy;
  VICS_INT r,b,g;

	for(y = 0; y < _outHeight; y+=2)
	{
		lumposy0	= (VICS_INT)((scaley * (VICS_DOUBLE)y) + 0.5);
		lumposy1	= (VICS_INT)((scaley * (VICS_DOUBLE)(y+1)) + 0.5);
		uvposy		= lumposy0 >> 1;	// Associated chr pel position.

		rgbposy0	=  y		* _outWidth * 3;
		rgbposy1	= (y+1) * _outWidth * 3;

		for(x = 0; x < _outWidth; x+=2)
		{
			lumposx0	= (VICS_INT)((scalex * (VICS_DOUBLE)x) + 0.5);
			lumposx1	= (VICS_INT)((scalex * (VICS_DOUBLE)(x+1)) + 0.5);
			uvposx		= lumposx0 >> 1;

			rgbposx0	=  x		* 3;
			rgbposx1	= (x+1) * 3;

			VICS_INT lum00	= 7 * (VICS_INT)(pY[(lumposy0*_width) + lumposx0]);

			VICS_INT u			= 7 * (VICS_INT)(pU[(uvposy*uvWidth) + uvposx]);
			VICS_INT v			= 7 * (VICS_INT)(pV[(uvposy*uvWidth) + uvposx]);

			// Lum00, u and v.
			for(i = -1; i <= 1; i++)
			{
				VICS_INT lumrow = lumposy0 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				VICS_INT uvrow = uvposy + i;
				if(uvrow < 0)	uvrow = 0;
				if(uvrow >= uvHeight) uvrow = uvHeight-1;

				for(j = -1; j <= 1; j++)
				{
					VICS_INT lumcol = lumposx0 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					VICS_INT uvcol = uvposx + j;
					if(uvcol < 0) uvcol = 0;
					if(uvcol >= uvWidth) uvcol = uvWidth-1;

					lum00	+= (VICS_INT)(pY[(lumrow*_width) + lumcol]);

					u			+= (VICS_INT)(pU[(uvrow*uvWidth) + uvcol]);
					v			+= (VICS_INT)(pV[(uvrow*uvWidth) + uvcol]);
				}//end for j...
			}//end for i...

			lum00 = lum00 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			// Less accurate fast calculation with divide by 16 built in.
			VICS_INT cc =  (u >> 1) - 256;
			VICS_INT cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
			VICS_INT ca =  (v >> 2) + (v >> 5) - 144;
  
			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy0 + rgbposx0]			= (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy0 + (rgbposx0+1)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy0 + (rgbposx0+2)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum01.
			VICS_INT lum01	= 7 * (VICS_INT)(pY[(lumposy0*_width) + lumposx1]);

			for(i = -1; i <= 1; i++)
			{
				VICS_INT lumrow = lumposy0 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					VICS_INT lumcol = lumposx1 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum01	+= (VICS_INT)(pY[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum01 = lum01 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy0 + rgbposx1]			= (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy0 + (rgbposx1+1)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy0 + (rgbposx1+2)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum10.
			VICS_INT lum10	= 7 * (VICS_INT)(pY[(lumposy1*_width) + lumposx0]);

			for(i = -1; i <= 1; i++)
			{
				VICS_INT lumrow = lumposy1 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					VICS_INT lumcol = lumposx0 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum10	+= (VICS_INT)(pY[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum10 = lum10 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy1 + rgbposx0]			= (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy1 + (rgbposx0+1)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy1 + (rgbposx0+2)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum11.
			VICS_INT lum11	= 7 * (VICS_INT)(pY[(lumposy1*_width) + lumposx1]);

			for(i = -1; i <= 1; i++)
			{
				VICS_INT lumrow = lumposy1 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					VICS_INT lumcol = lumposx1 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum11	+= (VICS_INT)(pY[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum11 = lum11 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy1 + rgbposx1]			= (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy1 + (rgbposx1+1)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy1 + (rgbposx1+2)] = (VICS_BYTE)(FYUVRGB24ZC_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.




