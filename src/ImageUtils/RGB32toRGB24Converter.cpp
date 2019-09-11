/** @file

MODULE						: RGB32toRGB24Converter

TAG								: RGB32RGB24C

FILE NAME					: RGB32toRGB24Converter.cpp

DESCRIPTION				: Simple RGB 24 bit to RGB 32 bit colour convertion 
										derived from the RGBtoRGBConverter base class.

REVISION HISTORY	:

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#if (_MSC_VER == 1800)
#pragma warning(push)     // warning C4005: '__useHeader' : macro redefinition caused by SDK7 VS2013 combination
#pragma warning(disable:4005)
#endif
#include <Windows.h>
#if (_MSC_VER == 1800)
#pragma warning(pop)      // restore original warning level
#endif
#else
#include <stdio.h>
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "RGB32toRGB24Converter.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Simple RGB32 to RGB24 conversion implementation.
An alpha channel is added.
@param pRgbIn		: Packed RGB 8888 format.
@param pRgbOut	: Packed RGB 888 format.
@return					: none.
*/
void RGB32toRGB24Converter::Convert(void* pRgbIn, void* pRgbOut)
{
	unsigned char* src = (unsigned char *)pRgbIn;
	unsigned char* dst = (unsigned char *)pRgbOut;

	int size = _width * _height;
	for(int x = 0; x < size; x++)
	{
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		src++;
	}//end for x...

}//end Convert.

