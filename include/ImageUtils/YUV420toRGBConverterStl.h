/** @file

MODULE				: YUV420toRGBConverterStl

TAG						: YUVRGBC

FILE NAME			: YUV420toRGBConverterStl.h

DESCRIPTION		: Colour convertions are required on the output of
all video codecs. For embedded applications only some
combinations of colour depths and inversions are required.
This class is the base class defining the minimum interface
and properties for all derived classes.

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

/**
 * @brief YUV to RGB converter base class
 */
template <typename T>
class YUV420toRGBConverterStl
{
public:
  /// Construction and destruction.
  YUV420toRGBConverterStl() { m_uiWidth = 0; m_uiHeight = 0; _rotate = 0; _chrOff = 0; _flip = false; }
  YUV420toRGBConverterStl(uint32_t uiWidth, uint32_t uiHeight) { m_uiWidth = uiWidth; m_uiHeight = uiHeight; _rotate = 0; _chrOff = 0; _flip = false; }
  YUV420toRGBConverterStl(uint32_t uiWidth, uint32_t uiHeight, int iChrOff) { _width = uiWidth; _height = uiHeight; _rotate = 0; _chrOff = iChrOff; _flip = false; }
  virtual ~YUV420toRGBConverterStl() {}
  /**
   * @brief Getter for last error
   */
  std::string getLastError() const { return m_sLastError; }
  /// Interface.
  virtual bool Convert(T* pYuv, uint32_t uiYuvLen, uint8_t* pRgb, uint32_t uiRgbLen) = 0;

  /// Member interface.
  uint32_t	GetWidth(void)			{ return(m_uiWidth); }
  uint32_t	GetHeight(void)			{ return(m_uiHeight); }
  int	GetRotate(void)			{ return(_rotate); }
  int GetChrominanceOffset(void)	{ return(_chrOff); }
  bool GetFlip(void)      { return _flip; }

  void SetDimensions(uint32_t uiWidth, uint32_t uiHeight)	{ m_uiWidth = uiWidth; m_uiHeight = uiHeight; }
  void SetRotate(int rotate)									{ _rotate = rotate; }
  void SetChrominanceOffset(int val) { _chrOff = val; }
  void SetFlip(bool flip) { _flip = flip; }
protected:
  void setLastError(const std::string& sError) { m_sLastError = sError; }
  void resetLastError() { m_sLastError = ""; }

  /// Members.
  uint32_t m_uiWidth;
  uint32_t m_uiHeight;
  int	_rotate;
  /// Offset added to the chr values. Typically = 128 to shift all values to positive.
  int	_chrOff;
  /// Windows bitmaps are bottom to top, standard YUV formats are top to bottom
  /// for our YUV to be compatible with linux we need to flip the image
  bool _flip;
  /// last error
  std::string m_sLastError;
};//end YUV420toRGBConverterStl.
