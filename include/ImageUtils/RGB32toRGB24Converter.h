/** @file

MODULE						: RGB32toRGB24Converter

TAG								: RGB32RGB24C

FILE NAME					: RGB32toRGB24Converter.h

DESCRIPTION				: Simple RGB 32 bit to RGB 24 bit colour convertion 
										derived from the RGBtoRGBConverter base class.

REVISION HISTORY	:

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _REALRGB32TORGB24CONVERTER_H
#define _REALRGB32TORGB24CONVERTER_H

#include "RGBtoRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RGB32toRGB24Converter: public RGBtoRGBConverter
{
public:
	// Construction and destruction.
	RGB32toRGB24Converter(void) { }
	RGB32toRGB24Converter(int width, int height): RGBtoRGBConverter(width,height) { }
	virtual ~RGB32toRGB24Converter(void) {}

	// Interface.
	void Convert(void* pRgbIn, void* pRgbOut);

};//end RGB32toRGB24Converter.


#endif	//_REALRGB32TORGB24CONVERTER_H
