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
#include <cstdint>
#include <sstream>
#include <vector>
#include <algorithm>

namespace artist {

/**
 * @brief conversion utility function from string to T 
 *
 * Should be deprecated since doesn't report failure
 */
template <class T>
T convert(const std::string& sStringValue, T defaultValue )
{
  T val = defaultValue;
  std::istringstream istr(sStringValue);
  if (istr.good())
      istr >> val;
  return val;
}
/**
 * @brief conversion utility function from string to T 
 *
 * @param sStringValue The string to converted to a number
 * @param [out] bSuccess Contains true if the call was successful, false otherwise
 */
template <class T>
T convert(const std::string& sStringValue, bool& bSuccess )
{
  T val;
  std::istringstream istr(sStringValue);
  istr >> val;
  bSuccess = !istr.fail() && istr.eof();
  return val;
}
/**
 * @brief conversion utility function from string to T 
 * Should be deprecated since doesn't report failure
 *
 * HACK to get the following to work:
 * uint8_t n = convert<uint8_t>("96")
 * Without the overload, it simply forwards 
 * the first digit into the temp val
 * WARNING: these user is responsible to make sure that the value is in the valid range
 */
template <>
inline uint8_t convert<uint8_t>(const std::string& sStringValue, uint8_t defaultValue)
{
  int32_t val(0);
  std::istringstream istr(sStringValue);
  istr >> val;
  return static_cast<uint8_t>(val);
}
/**
 * @brief conversion utility function from string to T 
 * @param sStringValue The string to converted to a number
 * @param [out] bSuccess Contains true if the call was successful, false otherwise
 *
 * HACK to get the following to work:
 * uint8_t n = convert<uint8_t>("96")
 * Without the overload, it simply forwards 
 * the first digit into the temp val
 * WARNING: these user is responsible to make sure that the value is in the valid range
 */
template <>
inline uint8_t convert<uint8_t>(const std::string& sStringValue, bool& bSuccess )
{
  int32_t val(0);
  std::istringstream istr(sStringValue);
  istr >> val;
  bSuccess = !istr.fail() && istr.eof();
  return static_cast<uint8_t>(val);
}
/**
 * @brief conversion utility function from string to bool
 */
template <>
inline bool convert<bool>(const std::string& sValue, bool defaultValue)
{
  if (sValue == "1") return true;
  else if (sValue == "0" || sValue.empty()) return false;
  else
  {
    std::string sCopy(sValue);
    std::transform(sCopy.begin(), sCopy.end(), sCopy.begin(), ::tolower);
    if (sCopy == "false")
      return false;
    else 
      return true;
  }
}
/**
 * utility method to convert a numeric type to a string
 *
 * @param t The numeric type to be converted to a string
 */
template <class T>
std::string toString(T t)
{
  std::ostringstream ostr;
  ostr << t;
  return ostr.str();
}
/**
 * utility method to convert a vector to a string
 *
 * @param T The type of the vector to be converted to a string
 * @param delim The delimiter to be inserted to separate entries 
 * in the vector
 */
template <class T>
std::string toString(const std::vector<T>& vecT, char delim = ' ')
{
#if 0
  std::ostringstream ostr;
  std::for_each(vecT.begin(), vecT.end(), [&ostr,delim](const T t)
  {
    ostr << t << delim;
  });
  return ostr.str();
#else
  std::ostringstream ostr;
  if (vecT.empty()) return "";
  ostr << vecT[0];
  for (size_t i = 1; i < vecT.size(); ++i)
  {
    ostr << delim << vecT[i];
  }
  return ostr.str();

#endif
}
/**
 * @brief conversion utility function from string to bool
 *
 * Converts "1", "true", "TRUE" to true and "0", "FALSE" and "false" to false
 */
static bool stringToBool(const std::string& sValue)
{
  if (sValue == "1" || sValue == "TRUE" || sValue == "true")
  {
    return true;
  }
  else if (sValue == "0" || sValue == "FALSE" || sValue == "false")
  {
    return false;
  }
  else
  {
    // Default
    return true;
  }
}
/**
 * @brief conversion utility function from int to bool
 *
 * Converts 0 to false, and non-zero values to true
 */
static bool i2b(uint32_t uiNum)
{
  return (uiNum == 0) ? false: true;
}
/**
 * @brief conversion utility function bool to string
 *
 * Converts false to "0", and true to "1"
 */
static std::string boolToString(bool bValue)
{
  std::string sResult = bValue?"1":"0";
  return sResult;
}

} // artist
