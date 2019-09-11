/** @file

MODULE						: Fast6BitYUV420toRGB24ZoomConverter

TAG								: F6BYUVRGB24ZC

FILE NAME					: Fast6BitYUV420toRGB24ZoomConverter.cpp

DESCRIPTION				: Fast YUV420 (6 bpp) to RGB 24 bit colour convertion with zoom
										functionality derived from the YUV420toRGBConverter 
										base class.

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

#include "Fast6BitYUV420toRGB24ZoomConverter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define F6BYUVRGB24ZC_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void Fast6BitYUV420toRGB24ZoomConverter::NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb)
{
	unsigned char* 	optr			=	(unsigned char *)pRgb;
	yuvType*				py				= (yuvType *)pY;
	yuvType*				pu				= (yuvType *)pU;
	yuvType*				pv				= (yuvType *)pV;
	int		uvWidth		= _width >> 1;
	int		uvHeight	= _height >> 1;

	double scalex = ((double)_width)/((double)_outWidth);
	double scaley = ((double)_height)/((double)_outHeight);

	int x,y,i,j;
	int lumposx0,lumposy0,lumposx1,lumposy1;
	int rgbposx0,rgbposy0,rgbposx1,rgbposy1;
	int uvposx,uvposy;
  int r,b,g;

	for(y = 0; y < _outHeight; y+=2)
	{
		lumposy0	= (int)((scaley * (double)y) + 0.5);
		lumposy1	= (int)((scaley * (double)(y+1)) + 0.5);
		uvposy		= lumposy0 >> 1;	// Associated chr pel position.

		rgbposy0	=  y		* _outWidth * 3;
		rgbposy1	= (y+1) * _outWidth * 3;

		for(x = 0; x < _outWidth; x+=2)
		{
			lumposx0	= (int)((scalex * (double)x) + 0.5);
			lumposx1	= (int)((scalex * (double)(x+1)) + 0.5);
			uvposx		= lumposx0 >> 1;

			rgbposx0	=  x		* 3;
			rgbposx1	= (x+1) * 3;

			int lum00	= 7 * (int)(py[(lumposy0*_width) + lumposx0]);

			int u			= 7 * (int)(pu[(uvposy*uvWidth) + uvposx]);
			int v			= 7 * (int)(pv[(uvposy*uvWidth) + uvposx]);

			// Lum00, u and v.
			for(i = -1; i <= 1; i++)
			{
				int lumrow = lumposy0 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				int uvrow = uvposy + i;
				if(uvrow < 0)	uvrow = 0;
				if(uvrow >= uvHeight) uvrow = uvHeight-1;

				for(j = -1; j <= 1; j++)
				{
					int lumcol = lumposx0 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					int uvcol = uvposx + j;
					if(uvcol < 0) uvcol = 0;
					if(uvcol >= uvWidth) uvcol = uvWidth-1;

					lum00	+= (int)(py[(lumrow*_width) + lumcol]);

					u			+= (int)(pu[(uvrow*uvWidth) + uvcol]);
					v			+= (int)(pv[(uvrow*uvWidth) + uvcol]);
				}//end for j...
			}//end for i...

			lum00 = lum00 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			// Less accurate fast calculation with divide by 16 built in.
			int cc =  (u >> 1) - 256;
			int cb = -(u >> 3) + (u >> 5) - (v >> 3) - (v >> 6) + 120;
			int ca =  (v >> 2) + (v >> 5) - 144;
  
			b = lum00 + cc;
			g = lum00 + cb;
			r = lum00 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy0 + rgbposx0]			= (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy0 + (rgbposx0+1)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy0 + (rgbposx0+2)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum01.
			int lum01	= 7 * (int)(py[(lumposy0*_width) + lumposx1]);

			for(i = -1; i <= 1; i++)
			{
				int lumrow = lumposy0 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					int lumcol = lumposx1 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum01	+= (int)(py[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum01 = lum01 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum01 + cc;
			g = lum01 + cb;
			r = lum01 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy0 + rgbposx1]			= (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy0 + (rgbposx1+1)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy0 + (rgbposx1+2)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum10.
			int lum10	= 7 * (int)(py[(lumposy1*_width) + lumposx0]);

			for(i = -1; i <= 1; i++)
			{
				int lumrow = lumposy1 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					int lumcol = lumposx0 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum10	+= (int)(py[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum10 = lum10 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum10 + cc;
			g = lum10 + cb;
			r = lum10 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy1 + rgbposx0]			= (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy1 + (rgbposx0+1)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy1 + (rgbposx0+2)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(r));

			// Lum11.
			int lum11	= 7 * (int)(py[(lumposy1*_width) + lumposx1]);

			for(i = -1; i <= 1; i++)
			{
				int lumrow = lumposy1 + i;
				if(lumrow < 0)	lumrow = 0;
				if(lumrow >= _height) lumrow = _height-1;

				for(j = -1; j <= 1; j++)
				{
					int lumcol = lumposx1 + j;
					if(lumcol < 0) lumcol = 0;
					if(lumcol >= _width) lumcol = _width-1;

					lum11	+= (int)(py[(lumrow*_width) + lumcol]);

				}//end for j...
			}//end for i...

			lum11 = lum11 >> 2;	// Divide by 16 and 6 bit to 8 bit conversion.

			b = lum11 + cc;
			g = lum11 + cb;
			r = lum11 + ca;
  
			// R, G & B have range 0..255.
			optr[rgbposy1 + rgbposx1]			= (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(b));
			optr[rgbposy1 + (rgbposx1+1)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(g));
			optr[rgbposy1 + (rgbposx1+2)] = (unsigned char)(F6BYUVRGB24ZC_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

