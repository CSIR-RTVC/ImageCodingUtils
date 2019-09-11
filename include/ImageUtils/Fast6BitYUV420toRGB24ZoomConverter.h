/** @file

MODULE						: Fast6BitYUV420toRGB24ZoomConverter

TAG								: F6BYUVRGB24ZC

FILE NAME					: Fast6BitYUV420toRGB24ZoomConverter.h

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
#ifndef _FAST6BITYUV420TORGB24ZOOMCONVERTER_H
#define _FAST6BITYUV420TORGB24ZOOMCONVERTER_H

#include "YUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class Fast6BitYUV420toRGB24ZoomConverter: public YUV420toRGBConverter
{
	public:
		// Construction and destruction.
		Fast6BitYUV420toRGB24ZoomConverter(void) {_outWidth = 1; _outHeight = 1;}
		Fast6BitYUV420toRGB24ZoomConverter(int width, int height): YUV420toRGBConverter(width, height) {_outWidth = 1; _outHeight = 1;}
		virtual ~Fast6BitYUV420toRGB24ZoomConverter(void) {}

		// Interface.
		virtual void Convert(void* pY, void* pU, void* pV, void* pRgb) 
		{
			if(!_rotate) NonRotateConvert(pY, pU, pV, pRgb);
		}

		// Derived member interface.
		int		GetOutputWidth(void)	{return(_outWidth);}
		int		GetOutputHeight(void) {return(_outHeight);}
		void	SetOutputDimensions(int width, int height) {_outWidth = width; _outHeight = height;}


	protected:
		virtual void NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb);

	protected:
		// Output size.
		int	_outWidth;
		int	_outHeight;

};//end Fast6BitYUV420toRGB24ZoomConverter.


#endif	// _FAST6BITYUV420TORGB24ZOOMCONVERTER_H
