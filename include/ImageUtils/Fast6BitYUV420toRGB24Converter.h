/** @file

MODULE						: Fast6BitYUV420toRGB24Converter

TAG								: F6BYUVRGB24C

FILE NAME					: Fast6BitYUV420toRGB24Converter.h

DESCRIPTION				: Fast YUV420 ( 6 bpp)to RGB 24 bit colour convertion derived from
										the YUV420toRGBConverter base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2006  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _FAST6BITYUV420TORGB24CONVERTER_H
#define _FAST6BITYUV420TORGB24CONVERTER_H

#include "YUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class Fast6BitYUV420toRGB24Converter: public YUV420toRGBConverter
{
	public:
		// Construction and destruction.
		Fast6BitYUV420toRGB24Converter(void) { }
		Fast6BitYUV420toRGB24Converter(int width, int height): YUV420toRGBConverter(width,height) { }
		virtual ~Fast6BitYUV420toRGB24Converter(void) {}

		// Interface.
		virtual void Convert(void* pY, void* pU, void* pV, void* pRgb) 
		{
			if(_rotate) RotateConvert(pY, pU, pV, pRgb);
			else				NonRotateConvert(pY, pU, pV, pRgb);
		};

	protected:
		virtual void NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb);
		virtual void RotateConvert(void* pY, void* pU, void* pV, void* pRgb);

};//end Fast6BitYUV420toRGB24Converter.


#endif	// _FAST6BITYUV420TORGB24CONVERTER_H
