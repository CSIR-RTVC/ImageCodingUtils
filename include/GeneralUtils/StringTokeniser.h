/**
LICENSE: Software License Agreement(BSD License)

Copyright(c) 2014, Ralf Globisch
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

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "Conversion.h"

namespace artist {

class StringTokeniser
{
public:

  static std::vector<std::string> tokenise(const std::string& sText, const std::string& sTokens = " ", bool trim = false, bool bDropEmptyTokens = false)
  {
    std::vector<std::string> vTokens;
    std::string sTrimmed = sText;
    boost::algorithm::trim(sTrimmed);
    if (sTrimmed.empty()) return vTokens;

    size_t last_pos = 0;
    for (size_t pos = 0; pos < sTrimmed.length(); ++pos)
    {
      for (size_t tokenPos = 0; tokenPos != sTokens.length(); ++tokenPos)
      {
        if (sTrimmed[pos] == sTokens[tokenPos])
        {
          if (bDropEmptyTokens)
          {
            // avoid tokenising empty strings
            if (last_pos != pos)
            {
              std::string sTok = sTrimmed.substr(last_pos, pos - last_pos);
              boost::algorithm::trim(sTok);
              if (!sTok.empty())
              {
                vTokens.push_back(sTok);
              }
              last_pos = pos + 1;
            }
          }
          else
          {
            vTokens.push_back(sTrimmed.substr(last_pos, pos - last_pos));
            last_pos = pos + 1;
          }
        }
      }
    }
    // push back last token
    vTokens.push_back(sTrimmed.substr(last_pos));

    if (trim)
    {
      // remove white space
      for (auto& val : vTokens)
      {
        boost::algorithm::trim(val);
      }
    }

    return vTokens;
  }

  /**
   * @brief Tokenises string into vector of requested type. On type conversion, this method returns an empty vector
   */
  template <typename T>
  static std::vector<T> tokeniseV2(const std::string& sText, const std::string& sTokens = " ", bool trim = false)
  {
    std::vector<T> vTokens;
    size_t last_pos = 0;
    for (size_t pos = 0; pos < sText.length(); ++pos)
    {
      for (size_t tokenPos = 0; tokenPos != sTokens.length(); ++tokenPos)
      {
        if (sText[pos] == sTokens[tokenPos])
        {
          std::string sTemp = sText.substr(last_pos, pos - last_pos);
          if (trim) boost::algorithm::trim(sTemp);
          bool bSuccess = true;
          T val = convert<T>(sTemp, bSuccess);
          if (!bSuccess) return std::vector<T>();
          vTokens.push_back(val);
          last_pos = pos + 1;
        }
      }
    }
    // push back last token
    std::string sTemp = sText.substr(last_pos);
    if (trim) boost::algorithm::trim(sTemp);
    bool bSuccess = true;
    T val = convert<T>(sTemp, bSuccess);
    if (!bSuccess) return std::vector<T>();
    vTokens.push_back(val);

    return vTokens;
  }
};

} // artist
