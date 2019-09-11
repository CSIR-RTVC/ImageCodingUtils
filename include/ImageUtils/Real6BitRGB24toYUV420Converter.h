/** @file

MODULE						: Real6BitRGB24toYUV420Converter

TAG								: R6RGB24YUVC

FILE NAME					: Real6BitRGB24toYUV420Converter.h

DESCRIPTION				: Double precision floating point RGB 24 bit to YUV420 colour 
										convertion derived from the RGBtoYUV420Converter base class.
										Use this implementation as the reference.

REVISION HISTORY	:
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _REAL6BITRGB24TOYUV420CONVERTER_H
#define _REAL6BITRGB24TOYUV420CONVERTER_H

#pragma once

#include "RGBtoYUV420Converter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class Real6BitRGB24toYUV420Converter: public RGBtoYUV420Converter
{
public:
	// Construction and destruction.
	Real6BitRGB24toYUV420Converter(void) { }
	Real6BitRGB24toYUV420Converter(int width, int height): RGBtoYUV420Converter(width,height) { }
	virtual ~Real6BitRGB24toYUV420Converter(void) {}

	// Interface.
	void Convert(void* pRgb, void* pY, void* pU, void* pV);

};//end _REAL6BITRGB24TOYUV420CONVERTER_H.


#endif
