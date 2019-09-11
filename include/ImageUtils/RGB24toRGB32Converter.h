/** @file

MODULE						: RGB24toRGB32Converter

TAG								: RGB24RGB32C

FILE NAME					: RGB24toRGB32Converter.h

DESCRIPTION				: Simple RGB 24 bit to RGB 32 bit colour convertion 
										derived from the RGBtoRGBConverter base class.

REVISION HISTORY	:

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _REALRGB24TORGB32CONVERTER_H
#define _REALRGB24TORGB32CONVERTER_H

#include "RGBtoRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RGB24toRGB32Converter: public RGBtoRGBConverter
{
public:
	// Construction and destruction.
	RGB24toRGB32Converter(void) { }
	RGB24toRGB32Converter(int width, int height): RGBtoRGBConverter(width,height) { }
	virtual ~RGB24toRGB32Converter(void) {}

	// Interface.
	void Convert(void* pRgbIn, void* pRgbOut);

};//end RGB24toRGB32Converter.


#endif	//_REALRGB24TORGB32CONVERTER_H
