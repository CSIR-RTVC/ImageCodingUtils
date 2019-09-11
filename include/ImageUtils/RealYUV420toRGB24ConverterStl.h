/** @file

MODULE				: RealYUV420toRGB24ConverterStl

FILE NAME			: RealYUV420toRGB24ConverterStl.h

DESCRIPTION			: Floating point implementation of YUV420 (8 bpp) to RGB 24 bit 
colour convertion derived from the YUV420toRGBConverter base 
class.
					  
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2015, CSIR
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
#pragma once

#include "YUV420toRGBConverterStl.h"

#define RYUVRGB24C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

#define RYUVRGB24C_U0		 2.032
#define RYUVRGB24C_U1		-0.394
#define RYUVRGB24C_V1		-0.581
#define RYUVRGB24C_V0		 1.140

/**
 * \ingroup ImageLib
 * Floating point implementation of YUV420 (8 bpp) to RGB 24 bit 
 * colour convertion derived from the YUV420toRGBConverter base 
 * class.
 */
template <typename T>
class RealYUV420toRGB24ConverterStl: public YUV420toRGBConverterStl<T>
{
public:
  // Construction and destruction.
  RealYUV420toRGB24ConverterStl() { }
  RealYUV420toRGB24ConverterStl(uint32_t uiWidth, uint32_t uiHeight) : YUV420toRGBConverterStl(uiWidth, uiHeight) { }
  RealYUV420toRGB24ConverterStl(uint32_t uiWidth, uint32_t uiHeight, int iChrOff) : YUV420toRGBConverterStl(uiWidth, uiHeight, iChrOff) { }

  virtual ~RealYUV420toRGB24ConverterStl() {}

  // Interface.
  virtual bool Convert(T* pYuv, uint32_t uiYuvLen, uint8_t* pRgb, uint32_t uiRgbLen)
  {
    if (m_uiWidth == 0 || m_uiHeight == 0)
    {
      setLastError("Width/Height not configured.");
      return false;
    }
    uint32_t uiPixels = m_uiWidth * m_uiHeight;
    uint32_t uiSizeUV = uiPixels >> 2;

    if (uiYuvLen != (uiPixels + 2 * uiSizeUV))
    {
      setLastError("YUV input buffer size does not match oconfiguration.");
      return false;
    }

    if (uiRgbLen < uiPixels * 3)
    {
      setLastError("RGB output buffer too small");
      return false;
    }

    //set the Y, U and V pointers to point to the correct positions within the byte array
    T* pY = pYuv;
    T* pU = pYuv + uiPixels;
    T* pV = pYuv + uiPixels + uiSizeUV;

    // let invert take precedence, not catering for both
    if(_flip) 
      InvertConvert(pY, pU, pV, pRgb);
    else				
    {
      if(_rotate) RotateConvert(pY, pU, pV, pRgb);
      else				NonRotateConvert(pY, pU, pV, pRgb);
    }
    return true;
  };

protected:
  void InvertConvert(T* pY, T* pU, T* pV, uint8_t* pRgb)
  {
    unsigned char* 	optr = (unsigned char *)pRgb;
    T*				py = (T *)pY;
    T*				pu = (T *)pU;
    T*				pv = (T *)pV;
    int		lumX = m_uiWidth;
    int		lumY = m_uiHeight;
    int		uvX = m_uiWidth >> 1;

    int x, y;
    int lumposy;
    int rgbposy;
    int uvposy;
    int r, b, g;

    int tworows = lumX << 1;
    int rgb1row = (m_uiWidth * 3);
    int rgb2rows = (m_uiWidth * 3) << 1;
    int lastrow = m_uiWidth * (m_uiHeight - 1) * 3;
    for (y = 0, lumposy = 0, uvposy = 0, rgbposy = lastrow; y < lumY; y += 2, lumposy += tworows, uvposy += uvX, rgbposy -= rgb2rows)
    {
      int lumpos0 = lumposy;					// Reset to start of rows.
      int lumpos1 = lumposy + lumX;
      int uvpos = uvposy;

      int rgbpos0 = rgbposy;
      int rgbpos1 = rgbposy - rgb1row;

      for (x = 0; x < lumX; x += 2)
      {
        double dlum00 = (double)((int)(py[lumpos0++]));

        double du = (double)(((int)(pu[uvpos])));
        double dv = (double)(((int)(pv[uvpos++])));

        unsigned char lum00 = dlum00;
        signed char u = (double)(du + _chrOff);
        signed char v = (double)(dv + _chrOff);

        // Lum00, u and v.
        // Fast calculation intermediate variables. 
        double cc = (RYUVRGB24C_U0 * u) + 0.5;
        double cb = (RYUVRGB24C_U1 * u) + (RYUVRGB24C_V1 * v) + 0.5;
        double ca = (RYUVRGB24C_V0 * v) + 0.5;

        b = (int)(lum00 + cc);
        g = (int)(lum00 + cb);
        r = (int)(lum00 + ca);

        // R, G & B have range 0..255.
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum01.
        double dlum01 = (double)((int)(py[lumpos0++]));
        unsigned char lum01 = dlum01;
        b = (int)(lum01 + cc);
        g = (int)(lum01 + cb);
        r = (int)(lum01 + ca);

        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum10.
        double dlum10 = (double)((int)(py[lumpos1++]));
        unsigned char lum10 = dlum10;

        b = (int)(lum10 + cc);
        g = (int)(lum10 + cb);
        r = (int)(lum10 + ca);

        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum11.
        double dlum11 = (double)((int)(py[lumpos1++]));
        unsigned char lum11 = dlum11;

        b = (int)(lum11 + cc);
        g = (int)(lum11 + cb);
        r = (int)(lum11 + ca);

        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

      }//end for x...

    }//end for y...

  }//end NonRotateConvert.

  virtual void NonRotateConvert(T* pY, T* pU, T* pV, uint8_t* pRgb)
  {
    unsigned char* 	optr = (unsigned char *)pRgb;
    T*				py = (T *)pY;
    T*				pu = (T *)pU;
    T*				pv = (T *)pV;
    int		lumX = m_uiWidth;
    int		lumY = m_uiHeight;
    int		uvX = m_uiWidth >> 1;

    int x, y;
    int lumposy;
    int rgbposy;
    int uvposy;
    int r, b, g;

    int tworows = lumX << 1;
    int rgb1row = (m_uiWidth * 3);
    int rgb2rows = (m_uiWidth * 3) << 1;

    for (y = 0, lumposy = 0, uvposy = 0, rgbposy = 0; y < lumY; y += 2, lumposy += tworows, uvposy += uvX, rgbposy += rgb2rows)
    {
      int lumpos0 = lumposy;					// Reset to start of rows.
      int lumpos1 = lumposy + lumX;
      int uvpos = uvposy;

      int rgbpos0 = rgbposy;
      int rgbpos1 = rgbposy + rgb1row;

      for (x = 0; x < lumX; x += 2)
      {
        double dlum00 = (double)((int)(py[lumpos0++]));

        double du = (double)(((int)(pu[uvpos])));
        double dv = (double)(((int)(pv[uvpos++])));

        unsigned char lum00 = dlum00;
        signed char u = (double)(du + _chrOff);
        signed char v = (double)(dv + _chrOff);

        // Lum00, u and v.
        // Fast calculation intermediate variables. 
        double cc = (RYUVRGB24C_U0 * u) + 0.5;
        double cb = (RYUVRGB24C_U1 * u) + (RYUVRGB24C_V1 * v) + 0.5;
        double ca = (RYUVRGB24C_V0 * v) + 0.5;

        b = (int)(lum00 + cc);
        g = (int)(lum00 + cb);
        r = (int)(lum00 + ca);

        // R, G & B have range 0..255.
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum01.
        double dlum01 = (double)((int)(py[lumpos0++]));
        unsigned char lum01 = dlum01;
        b = (int)(lum01 + cc);
        g = (int)(lum01 + cb);
        r = (int)(lum01 + ca);

        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum10.
        double dlum10 = (double)((int)(py[lumpos1++]));
        unsigned char lum10 = dlum10;

        b = (int)(lum10 + cc);
        g = (int)(lum10 + cb);
        r = (int)(lum10 + ca);

        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

        // Lum11.
        double dlum11 = (double)((int)(py[lumpos1++]));
        unsigned char lum11 = dlum11;

        b = (int)(lum11 + cc);
        g = (int)(lum11 + cb);
        r = (int)(lum11 + ca);

        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1++] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));

      }//end for x...

    }//end for y...

  }//end NonRotateConvert.

  virtual void RotateConvert(T* pY, T* pU, T* pV, uint8_t* pRgb)
  {
    unsigned char* 	optr = (unsigned char *)pRgb;
    T*				py = (T *)pY;
    T*				pu = (T *)pU;
    T*				pv = (T *)pV;
    int		lumX = m_uiWidth;
    int		lumY = m_uiHeight;
    int		uvX = m_uiWidth >> 1;

    int x, y;
    int lumposy;
    int rgbposx;
    int uvposy;
    int r, b, g;

    int tworows = lumX << 1;
    //int rgb1row  = (m_uiWidth * 3);
    int rgb1row = (m_uiHeight * 3);

    for (y = 0, lumposy = 0, uvposy = 0, rgbposx = 0; y < lumY; y += 2, lumposy += tworows, uvposy += uvX, rgbposx += 6)
    {
      int lumpos0 = lumposy;					// Reset to start of rows.
      int lumpos1 = lumposy + lumX;
      int uvpos = uvposy;

      int rgbpos0 = rgbposx;
      int rgbpos1 = rgbposx + 3;

      for (x = 0; x < lumX; x += 2)
      {
        double lum00 = (double)((int)(py[lumpos0++]));
        double u = (double)(((int)(pu[uvpos])) - 128);
        double v = (double)(((int)(pv[uvpos++])) - 128);
        //double u		 = (double)(((int)(pu[uvpos])) - 0);
        //double v		 = (double)(((int)(pv[uvpos++])) - 0);

        // Lum00, u and v.

        // Fast calculation intermediate variables. 
        double cc = (RYUVRGB24C_U0 * u) + 0.5;
        double cb = (RYUVRGB24C_U1 * u) + (RYUVRGB24C_V1 * v) + 0.5;
        double ca = (RYUVRGB24C_V0 * v) + 0.5;

        b = (int)(lum00 + cc);
        g = (int)(lum00 + cb);
        r = (int)(lum00 + ca);

        // R, G & B have range 0..255.
        optr[rgbpos0] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0 + 1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0 + 2] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));
        rgbpos0 += rgb1row;

        // Lum01.
        double lum01 = (double)((int)(py[lumpos0++]));

        b = (int)(lum01 + cc);
        g = (int)(lum01 + cb);
        r = (int)(lum01 + ca);

        optr[rgbpos0] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos0 + 1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos0 + 2] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));
        rgbpos0 += rgb1row;

        // Lum10.
        double lum10 = (double)((int)(py[lumpos1++]));

        b = (int)(lum10 + cc);
        g = (int)(lum10 + cb);
        r = (int)(lum10 + ca);

        optr[rgbpos1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1 + 1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1 + 2] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));
        rgbpos1 += rgb1row;

        // Lum11.
        double lum11 = (double)((int)(py[lumpos1++]));

        b = (int)(lum11 + cc);
        g = (int)(lum11 + cb);
        r = (int)(lum11 + ca);

        optr[rgbpos1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(b));
        optr[rgbpos1 + 1] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(g));
        optr[rgbpos1 + 2] = (unsigned char)(RYUVRGB24C_RANGECHECK_0TO255(r));
        rgbpos1 += rgb1row;

      }//end for x...

    }//end for y...

  }//end NonNonRotateConvert.

};//end RealYUV420toRGB24ConverterStl.
