/** @file

MODULE				: RGBtoYUV420ConverterStl

TAG						: RGBYUVC

FILE NAME			: RGBtoYUV420ConverterStl.h

DESCRIPTION		: Colour convertions to YUV420 are required on the input of
all video codecs. For embedded applications only some
combinations of colour depths are required. This class is
the base class defining the minimum interface	and
properties for all derived classes. The conversion method
is pure virtual and must be implemented by derived classes.

COPYRIGHT			:	(c)CSIR 2007-2015 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used
to endorse or promote products derived from this software without specific
prior written permission.

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
#include <cstdint>
#include <string>

/*
===========================================================================
Class definition.
===========================================================================
*/
template <typename T>
class RGBtoYUV420ConverterStl
{
public:
  /// Construction and destruction.
  RGBtoYUV420ConverterStl() { m_uiWidth = 0; m_uiHeight = 0; _chrOff = 0; _flip = false; }
  RGBtoYUV420ConverterStl(uint32_t uiWidth, uint32_t uiHeight) { m_uiWidth = uiWidth; m_uiHeight = uiHeight; _chrOff = 0; _flip = false; }
  RGBtoYUV420ConverterStl(uint32_t uiWidth, uint32_t uiHeight, int chrOff) { m_uiWidth = uiWidth; m_uiHeight = uiHeight; _chrOff = chrOff; _flip = false; }
  virtual ~RGBtoYUV420ConverterStl() {}
  /**
   * @brief Getter for last error
   */
  std::string getLastError() const { return m_sLastError; }

  /// Interface.
  virtual bool Convert(uint8_t* pRgb, uint32_t uiRgbLen, T* pYuv, uint32_t uiYuvLen) = 0;

  /// Member interface.
  int	GetWidth(void)			{ return(m_uiWidth); }
  int	GetHeight(void)			{ return(m_uiHeight); }
  int GetChrominanceOffset() const { return _chrOff; }
  bool GetFlip() const { return _flip; }

  void SetDimensions(int width, int height)	{ _width = width; _height = height; }
  void SetFlip(bool flip) { _flip = flip; }
  void SetChrominanceOffset(int val) { _chrOff = val; }

protected:
  void setLastError(const std::string& sError) { m_sLastError = sError; }
  void resetLastError() { m_sLastError = ""; }

  /// Members.
  uint32_t m_uiWidth;
  uint32_t m_uiHeight;
  /// Offset added to the chr values. Typically = 128 to shift all values to positive.
  int	_chrOff;
  bool _flip;
  /// last error
  std::string m_sLastError;
};//end RGBtoYUV420ConverterStl.

