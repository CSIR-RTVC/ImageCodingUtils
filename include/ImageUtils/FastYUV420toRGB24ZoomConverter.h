/** @file

MODULE						: FastYUV420toRGB24ZoomConverter

TAG								: FYUVRGB24ZC

FILE NAME					: FastYUV420toRGB24ZoomConverter.h

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
#ifndef _FASTYUV420TORGB24ZOOMCONVERTER_H
#define _FASTYUV420TORGB24ZOOMCONVERTER_H

#include "VicsDefs/VicsDefs.h"
#include "FastYUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class FastYUV420toRGB24ZoomConverter: public FastYUV420toRGBConverter
{
	protected:
		// Output size.
		VICS_INT	_outWidth;
		VICS_INT	_outHeight;

	public:
		// Construction and destruction.
		FastYUV420toRGB24ZoomConverter(void) {_outWidth = 1; _outHeight = 1;}
		FastYUV420toRGB24ZoomConverter(VICS_INT w, VICS_INT h): FastYUV420toRGBConverter(w,h) {_outWidth = 1; _outHeight = 1;}
		~FastYUV420toRGB24ZoomConverter(void) {}

		// Interface.
		virtual void Convert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb) 
		{
			if(!_rotate) NonRotateConvert(pY,pU,pV,pRgb);
		};

		// Derived member interface.
		VICS_INT	GetOutputWidth(void)	{return(_outWidth);}
		VICS_INT	GetOutputHeight(void) {return(_outHeight);}
		void			SetOutputDimensions(VICS_INT width, VICS_INT height) {_outWidth = width; _outHeight = height;}


	protected:
		virtual void NonRotateConvert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb);

};//end FastYUV420toRGB24Converter.


#endif
