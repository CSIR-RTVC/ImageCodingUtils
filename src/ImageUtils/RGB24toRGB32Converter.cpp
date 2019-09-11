/** @file

MODULE						: RGB24toRGB32Converter

TAG								: RGB24RGB32C

FILE NAME					: RGB24toRGB32Converter.cpp

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

#include "RGB24toRGB32Converter.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Simple RGB24 to RGB32 conversion implementation.
An alpha channel is added.
@param pRgbIn		: Packed RGB 888 format.
@param pRgbOut	: Packed RGB 8888 format.
@return					: none.
*/
void RGB24toRGB32Converter::Convert(void* pRgbIn, void* pRgbOut)
{
	unsigned char* src = (unsigned char *)pRgbIn;
	unsigned char* dst = (unsigned char *)pRgbOut;

	int size = _width * _height;
	for(int x = 0; x < size; x++)
	{
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = 0;
	}//end for x...

}//end Convert.

