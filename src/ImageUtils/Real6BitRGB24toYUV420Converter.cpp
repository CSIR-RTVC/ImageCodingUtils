/** @file

MODULE						: Real6BitRGB24toYUV420Converter

TAG								: R6RGB24YUVC

FILE NAME					: Real6BitRGB24toYUV420Converter.cpp

DESCRIPTION				: Double precision floating point RGB 24 bit to YUV420 colour 
										convertion derived from the RGBtoYUV420Converter base class.
										YUV is represented with 6 bpp. Use this implementation as 
										the reference.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "Real6BitRGB24toYUV420Converter.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
#define R6RGB24YUVC_00		 0.299
#define R6RGB24YUVC_01		 0.587
#define R6RGB24YUVC_02		 0.114
#define R6RGB24YUVC_10		-0.147
#define R6RGB24YUVC_11		-0.289
#define R6RGB24YUVC_12		 0.436
#define R6RGB24YUVC_20		 0.615
#define R6RGB24YUVC_21		-0.515
#define R6RGB24YUVC_22		-0.100

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix quation is used. The YUV output is represented with
6 bits per pel and the UV components are adjusted from their -128..127 range
to 0..255.
@param pRgb	: Packed RGB 888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void Real6BitRGB24toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
	yuvType*	py = (yuvType *)pY;
	yuvType*	pu = (yuvType *)pU;
	yuvType*	pv = (yuvType *)pV;
	unsigned char* src = (unsigned char *)pRgb;

	// Y have range 0..255, U & V have range -128..127. This
	// implementation adjusts the range to Y [0..63] and
	// UV [0..63].
	double	u,v;
	double	r,g,b;

	// Step in 2x2 pel blocks. (4 pels per block).
	int xBlks = _width >> 1;
	int yBlks = _height >> 1;
	for(int yb = 0; yb < yBlks; yb++)
   for(int xb = 0; xb < xBlks; xb++)
	{
    int							chrOff	= yb*xBlks + xb;
    int							lumOff	= (yb*_width + xb) << 1;
    unsigned char*	t				= src + lumOff*3;

		// Top left pel.  255->0.999.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = (yuvType)((int)(0.5 + R6RGB24YUVC_00*r + R6RGB24YUVC_01*g + R6RGB24YUVC_02*b) >> 2);

		u = 128.0 + R6RGB24YUVC_10*r + R6RGB24YUVC_11*g + R6RGB24YUVC_12*b;
		v = 128.0 + R6RGB24YUVC_20*r + R6RGB24YUVC_21*g + R6RGB24YUVC_22*b;

		// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = (yuvType)((int)(0.5 + R6RGB24YUVC_00*r + R6RGB24YUVC_01*g + R6RGB24YUVC_02*b) >> 2);

		u += 128.0 + R6RGB24YUVC_10*r + R6RGB24YUVC_11*g + R6RGB24YUVC_12*b;
		v += 128.0 + R6RGB24YUVC_20*r + R6RGB24YUVC_21*g + R6RGB24YUVC_22*b;

    lumOff += _width;
    t = t + _width*3 - 6;
		// Bottom left pel.  255->0.999.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = (yuvType)((int)(0.5 + R6RGB24YUVC_00*r + R6RGB24YUVC_01*g + R6RGB24YUVC_02*b) >> 2);

		u += 128.0 + R6RGB24YUVC_10*r + R6RGB24YUVC_11*g + R6RGB24YUVC_12*b;
		v += 128.0 + R6RGB24YUVC_20*r + R6RGB24YUVC_21*g + R6RGB24YUVC_22*b;

		// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = (yuvType)((int)(0.5 + R6RGB24YUVC_00*r + R6RGB24YUVC_01*g + R6RGB24YUVC_02*b) >> 2);

		u += 128.0 + R6RGB24YUVC_10*r + R6RGB24YUVC_11*g + R6RGB24YUVC_12*b;
		v += 128.0 + R6RGB24YUVC_20*r + R6RGB24YUVC_21*g + R6RGB24YUVC_22*b;

		// Average the 4 chr values.
		pu[chrOff] = (yuvType)( (int)((u + 0.5)/4) >> 2 );
		pv[chrOff] = (yuvType)( (int)((v + 0.5)/4) >> 2 );
 	}//end for xb & yb...

}//end Convert.

