/** @file

MODULE				: RealRGB32toYUV420Converter

FILE NAME			: RealRGB32toYUV420Converter.h

DESCRIPTION			: Double precision floating point RGB 32 bit to YUV420 colour 
					convertion derived from the RGBtoYUV420Converter base class.
					Use this implementation as the reference.
			  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#ifndef _REALRGB32TOYUV420CONVERTER_H
#define _REALRGB32TOYUV420CONVERTER_H

#pragma once

#include "RGBtoYUV420ConverterStl.h"

#define RRGB32YUVC_00		 0.299
#define RRGB32YUVC_01		 0.587
#define RRGB32YUVC_02		 0.114
#define RRGB32YUVC_10		-0.147
#define RRGB32YUVC_11		-0.289
#define RRGB32YUVC_12		 0.436
#define RRGB32YUVC_20		 0.615
#define RRGB32YUVC_21		-0.515
#define RRGB32YUVC_22		-0.100

/**
 * \ingroup ImageLib
 * Double precision floating point RGB 32 bit to YUV420 colour 
 * convertion derived from the RGBtoYUV420Converter base class.
 * Use this implementation as the reference.
 */
template <typename T>
class RealRGB32toYUV420ConverterStl: public RGBtoYUV420ConverterStl<T>
{
public:
	// Construction and destruction.
  RealRGB32toYUV420ConverterStl() { }
  RealRGB32toYUV420ConverterStl(uint32_t uiWidth, uint32_t uiHeight) : RGBtoYUV420ConverterStl(uiWidth, uiHeight) { }
  RealRGB32toYUV420ConverterStl(uint32_t uiWidth, uint32_t uiHeight, int chrOff) : RGBtoYUV420ConverterStl(uiWidth, uiHeight, chrOff) { }
  virtual ~RealRGB32toYUV420ConverterStl() {}

	// Interface.
  virtual bool Convert(uint8_t* pRgb, uint32_t uiRgbLen, T* pYuv, uint32_t uiYuvLen)
  {
    if (m_uiWidth == 0 || m_uiHeight == 0)
    {
      setLastError("Width/Height not configured.");
      return false;
    }
    uint32_t uiPixels = m_uiWidth * m_uiHeight;
    uint32_t uiSizeUV = uiPixels >> 2;

    if (uiRgbLen != (uiPixels * 4))
    {
      setLastError("RGB input buffer size does not match configuration.");
      return false;
    }

    if (uiYuvLen < uiPixels * 1.5 * sizeof(T))
    {
      setLastError("YUV output buffer too small");
      return false;
    }

    //Set pointer offsets into YUV array
    T* py = pYuv;
    T* pu = pYuv + uiPixels;
    T* pv = pu + uiSizeUV;

    unsigned char* src = (unsigned char *)pRgb;

    /// Y have range 0..255, U & V have range -128..127.
    double	u, v;
    double	r, g, b;

    /// Step in 2x2 pel blocks. (4 pels per block).
    int xBlks = m_uiWidth >> 1;
    int yBlks = m_uiHeight >> 1;
    for (int yb = 0; yb < yBlks; yb++)
      for (int xb = 0; xb < xBlks; xb++)
      {
        int							chrOff = yb*xBlks + xb;
        int							lumOff = (yb*m_uiWidth + xb) << 1;
        unsigned char*	t = src + lumOff * 4;

        /// Top left pel.  255->0.999.
        b = (double)(*t++);
        g = (double)(*t++);
        r = (double)(*t++);
        t++;	///< Alpha channel.
        py[lumOff] = (T)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

        u = 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
        v = 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

        /// Top right pel.
        b = (double)(*t++);
        g = (double)(*t++);
        r = (double)(*t++);
        t++;	///< Alpha channel.
        py[lumOff + 1] = (T)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

        u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
        v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

        lumOff += m_uiWidth;
        t = t + m_uiWidth * 4 - 8;
        /// Bottom left pel.  255->0.999.
        b = (double)(*t++);
        g = (double)(*t++);
        r = (double)(*t++);
        t++;	///< Alpha channel.
        py[lumOff] = (T)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

        u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
        v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

        /// Bottom right pel.
        b = (double)(*t++);
        g = (double)(*t++);
        r = (double)(*t);
        py[lumOff + 1] = (T)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

        u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
        v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

        /// Average the 4 chr values.
        pu[chrOff] = (T)((int)((u + 0.5) / 4));
        pv[chrOff] = (T)((int)((v + 0.5) / 4));
      }//end for xb & yb...
    return true;
  }

};//end RealRGB32toYUV420Converter.


#endif //_REALRGB32TOYUV420CONVERTER_H
