/** @file

MODULE						: FastYUV420toRGBConverter

TAG								: FYUVRGBC

FILE NAME					: FastYUV420toRGBConverter.h

DESCRIPTION				: Fast colour convertions are required on the output of
										all video codecs. For embedded applications only some
										combinations of colour depths and inversions are required.
										This class is the base class defining the minimum interface
										and properties for all derived classes. 

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _FASTYUV420TORGBCONVERTER_H
#define _FASTYUV420TORGBCONVERTER_H

#include "VicsDefs/VicsDefs.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class FastYUV420toRGBConverter
{
	protected:
		// Members.
		VICS_INT	_width;
		VICS_INT	_height;
		VICS_INT	_rotate;

	public:
		// Construction and destruction.
		FastYUV420toRGBConverter(void) {_width = 0; _height = 0; _rotate = 0;}
		FastYUV420toRGBConverter(VICS_INT w, VICS_INT h) {_width = w; _height = h; _rotate = 0;}
		virtual ~FastYUV420toRGBConverter(void) {}

		// Interface.
		virtual void Convert(VICS_PSBYTE pY,VICS_PSBYTE pU,VICS_PSBYTE pV,VICS_POINTER pRgb) = 0;

		// Member interface.
		VICS_INT	GetWidth(void)		{ return(_width); }
		VICS_INT	GetHeight(void)		{ return(_height); }
		VICS_INT	GetRotate(void)		{ return(_rotate); }

		void	SetDimensions(VICS_INT width, VICS_INT height)	{_width = width; _height = height;}
		void	SetRotate(VICS_INT rotate)											{ _rotate = rotate;}

};//end FastYUV420toRGBConverter.


#endif
