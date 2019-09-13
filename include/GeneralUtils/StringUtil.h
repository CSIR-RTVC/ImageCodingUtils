/**
LICENSE: Software License Agreement(BSD License)

Copyright(c) 2017, Ralf Globisch
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and / or other materials provided with the distribution.
* Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#pragma once
#include <cassert>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#ifdef _WIN32
// for mbstowcs_s
#include <stdlib.h>
#endif
#include <cstdlib>
#include "Conversion.h"

namespace artist {

#ifdef _WIN32
#ifdef _CONVERT_WITH_ATL
#include <atlconv.h>
#include <strsafe.h>

#ifdef _DEBUG
# pragma comment(lib, "atls.lib")
#else
# pragma comment(lib, "atls.lib")
#endif
#endif
#endif

class StringUtil
{
public:
#ifdef _WIN32

#ifdef _CONVERT_WITH_ATL
  static std::string wideToStl(const WCHAR* wszString)
  {
    //return "";
    USES_CONVERSION;
    char *szTemp = W2A(wszString);
    return std::string(szTemp);
  }

  static WCHAR* stlToWide(std::string sString)
  {
    return(asciiToWide(sString.c_str()));
  }

  /// The caller must free the memory for the result
  static WCHAR* asciiToWide(const char* szString)
  {
    size_t nLen = strlen(szString);
    WCHAR* wszBuffer = new WCHAR[nLen + 1];
    USES_CONVERSION;
    WCHAR* wszValue = A2W(szString);
    StringCchCopyW(wszBuffer, nLen + 1, wszValue);
    return wszBuffer;
  }
#else


  static std::string wideStringToString(const wchar_t* wszString)
  {
    const unsigned max_len = 255;
    char szBuffer[max_len];
    size_t uiConverted(0);
    errno_t err = wcstombs_s(&uiConverted, szBuffer, max_len, wszString, _TRUNCATE);
    return (err == 0) ? std::string(szBuffer, uiConverted - 1) : ""; // subtract 1 for null terminator
  }

  static std::wstring stringToWideString(const std::string& sString)
  {
    std::wstring wideString(sString.size() + 1, 0);
    size_t converted = 0;
    errno_t err = mbstowcs_s(&converted, &wideString[0], sString.length() + 1, sString.c_str(), _TRUNCATE);
    assert(err == 0);
    return std::wstring(wideString.c_str()); // removes null terminator inserted by mbstowcs_s
  }

#endif

#endif

  // Converts "1", "true", "TRUE" to true and "0", "FALSE" and "false" to false
  static bool stringToBool(const std::string& sValue, bool& bValid)
  {
    if (sValue == "1" || sValue == "TRUE" || sValue == "true")
    {
      bValid = true;
      return true;
    }
    else if (sValue == "0" || sValue == "FALSE" || sValue == "false")
    {
      bValid = true;
      return false;
    }
    else
    {
      // Default
      bValid = false;
      return false;
    }
  }

  static std::string boolToString(bool bValue)
  {
    std::string sResult = bValue ? "1" : "0";
    return sResult;
  }

  static std::string doubleToString(double d, int nPrecision = 2)
  {
    std::ostringstream oss;

    oss << std::setprecision(nPrecision)
      << std::fixed
      << d;

    return oss.str();
  }

  static double stringToDouble(std::string sDouble)
  {
    std::istringstream i(sDouble);
    double x;
    if (!(i >> x))
    {
      return 0.0;
    }
    return x;
  }
#ifdef _WIN32	
  static std::string GetTimeString(std::string sFormat = "")
  {
    const unsigned uiBufferSize = 256;
    //Get current date and time and format
    time_t rawtime;
    struct tm timeinfo;
    char buffer[uiBufferSize];

    time(&rawtime);
    errno_t err = localtime_s(&timeinfo, &rawtime);
    assert(err == 0);

    if (sFormat == "")
    {
      sFormat = "%A, %d %B %Y %X";
    }
    strftime(buffer, uiBufferSize, sFormat.c_str(), &timeinfo);
    return std::string(buffer);
  }
#endif

  static std::string getZeroPaddedString(const std::string& sString, int iCount, int iTotal)
  {
    std::string sTotal = toString(iTotal);
    int iTotalLen = sTotal.length();
    std::string sCount = toString(iCount);
    int iCountLen = sCount.length();

    std::ostringstream padded;
    for (int i = 0; i < iTotalLen - iCountLen; ++i)
    {
      padded << "0";
    }
    padded << iCount << "_" << sString;
    return padded.str();
  }


};

} // artist
