/** @file

MODULE						: Real6BitYUV420toRGB24Converter

TAG								: R6BYUVRGB24C

FILE NAME					: Real6BitYUV420toRGB24Converter.h

DESCRIPTION				: Floating point implementation of YUV420 (6 bpp) to RGB 24 bit 
										colour convertion derived from	the YUV420toRGBConverter base 
										class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2006  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _REAL6BITYUV420TORGB24CONVERTER_H
#define _REAL6BITYUV420TORGB24CONVERTER_H

#include "YUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class Real6BitYUV420toRGB24Converter: public YUV420toRGBConverter
{
	public:
		// Construction and destruction.
		Real6BitYUV420toRGB24Converter(void) { }
		Real6BitYUV420toRGB24Converter(int width, int height): YUV420toRGBConverter(width,height) { }
		virtual ~Real6BitYUV420toRGB24Converter(void) {}

		// Interface.
		virtual void Convert(void* pY, void* pU, void* pV, void* pRgb) 
		{
			if(_rotate) RotateConvert(pY, pU, pV, pRgb);
			else				NonRotateConvert(pY, pU, pV, pRgb);
		};

	protected:
		virtual void NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb);
		virtual void RotateConvert(void* pY, void* pU, void* pV, void* pRgb);

};//end Real6BitYUV420toRGB24Converter.


#endif	// _REAL6BITYUV420TORGB24CONVERTER_H
