/** @file

MODULE						: FastYUV420toRGB24Converter

TAG								: FYUVRGB24C

FILE NAME					: FastYUV420toRGB24Converter.h

DESCRIPTION				: Fast YUV420 to RGB 24 bit colour convertion derived from
										the FastYUV420toRGBConverter base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _FASTYUV420TORGB24CONVERTER_H
#define _FASTYUV420TORGB24CONVERTER_H

#include "VicsDefs/VicsDefs.h"
#include "FastYUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class FastYUV420toRGB24Converter: public FastYUV420toRGBConverter
{
	public:
		// Construction and destruction.
		FastYUV420toRGB24Converter(void) { }
		FastYUV420toRGB24Converter(VICS_INT w, VICS_INT h): FastYUV420toRGBConverter(w,h) { }
		~FastYUV420toRGB24Converter(void) {}

		// Interface.
		virtual void Convert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb) 
		{
			if(_rotate) RotateConvert(pY,pU,pV,pRgb);
			else				NonRotateConvert(pY,pU,pV,pRgb);
		};

	protected:
		virtual void NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb);
		virtual void RotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb);

};//end FastYUV420toRGB24Converter.


#endif
