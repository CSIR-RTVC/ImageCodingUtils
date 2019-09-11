/**
LICENSE: Software License Agreement(BSD License)

Copyright(c) 2017,CSIR Meraka Institute 
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
#include <climits>
#include <cmath>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include "Conversion.h"
#include "StringTokeniser.h"

namespace artist {

/**
 * @brief selectMostSuitableFramerate selects the most suitable framerate from a list of possible ones to meet
 * a specified target framerate, taking into account that a frameskipper will be used to achieve the target.
 */
static void selectMostSuitableFramerate(const std::set<double> availableFramerates, const double dTargedTargetFrameRate, double& dMostSuitableFramerate)
{
  double dFrameFractionTempPast = dTargedTargetFrameRate;
  for (auto it : availableFramerates)
  {
    double dFrameFractionTempNow = fabs(remainder(it, dTargedTargetFrameRate));
    // case when the frame rate in context is lower than target frame rate
    if (it < dTargedTargetFrameRate)
    {
      dMostSuitableFramerate = it;
    }
    // case when the frame rate in context is higher than target frame rate
    else
    {
      // check if the frame rate in context is better than previously selected frame rate
      if (dFrameFractionTempNow <= dFrameFractionTempPast)
      {
        if (dFrameFractionTempNow < dFrameFractionTempPast)
        {
          dFrameFractionTempPast = dFrameFractionTempNow;
          dMostSuitableFramerate = it;
        }
        else if (dFrameFractionTempNow = dFrameFractionTempPast && dMostSuitableFramerate > it && it > dTargedTargetFrameRate)
        {
          dMostSuitableFramerate = it;
        }
      }
    }
  }
}

/**
 * @brief This method takes a string representing a resolution and pulls out the width and height.
 * @param[in] sResolution The resolution
 * @return true if the width and height were extracted, and false otherwise.
 */
static bool parseResolution(const std::string& sResolution, int& iWidth, int& iHeight)
{
  iWidth = -1;
  iHeight = -1;
  std::string sWidth;
  std::string sHeight;

  static const std::regex e("(\\d+)x(\\d+)");
  std::smatch mr;
  // std::sub_match mr;
  if (std::regex_match(begin(sResolution), end(sResolution), mr, e))
  {
    sWidth = mr[1].str();
    bool bRes = false;
    iWidth = convert<int32_t>(sWidth, bRes);
    if (!bRes) return false;
    sHeight = mr[2].str();
    iHeight = convert<int32_t>(sHeight, bRes);
    if (!bRes) return false;
    return true;
  }
  else
  {
    static const std::regex e1("-(\\d+)x-(\\d+)");
    static const std::regex e2("-(\\d+)x(\\d+)");
    static const std::regex e3("(\\d+)x-(\\d+)");
    if (std::regex_match(begin(sResolution), end(sResolution), mr, e1))
    {
      sWidth = mr[1].str();
      bool bRes = false;
      iWidth = -1 * convert<int32_t>(sWidth, bRes);
      if (!bRes) return false;
      sHeight = mr[2].str();
      iHeight = -1 * convert<int32_t>(sHeight, bRes);
      if (!bRes) return false;
      return true;
    }
    else if (std::regex_match(begin(sResolution), end(sResolution), mr, e2))
    {
      sWidth = mr[1].str();
      bool bRes = false;
      iWidth = -1 * convert<int32_t>(sWidth, bRes);
      if (!bRes) return false;
      sHeight = mr[2].str();
      iHeight = convert<int32_t>(sHeight, bRes);
      if (!bRes) return false;
      return true;
    }
    else if (std::regex_match(begin(sResolution), end(sResolution), mr, e3))
    {
      sWidth = mr[1].str();
      bool bRes = false;
      iWidth = convert<int32_t>(sWidth, bRes);
      if (!bRes) return false;
      sHeight = mr[2].str();
      iHeight = -1 * convert<int32_t>(sHeight, bRes);
      if (!bRes) return false;
      return true;
    }
    return false;
  }
}

/**
 * @brief This method converts resolution into int width and int height pair from string
 * @param[in] vResolutions vector contains resolution strings in the Num x Num format
 * @return vector pair of (int width, int height)
 */
static std::vector<std::pair<int, int>> getResolutionsFromString(const std::vector<std::string>& vResolutions)
{
  std::vector<std::pair<int, int>> resolutions;
  for (auto it : vResolutions)
  {
    int tempWidth, tempHeight;
    if (parseResolution(it, tempWidth, tempHeight))
    {
      resolutions.push_back(std::make_pair(tempWidth, tempHeight));
    }
  }
  return resolutions;
}

/**
 * @brief Returns a resolution string of the form <width>x<height>
 * @param[in] iWidth Width of video
 * @param[in] iHeight Height of video
 * @return A string of the form <width>x<height>
 */
static std::string resolutionToString(const int iWidth, const int iHeight)
{
  std::ostringstream ostr;
  ostr << iWidth << "x" << iHeight;
  return ostr.str();
}

/**
 * @brief Tries to determine the most suitable resolution from available resolutions to meet the target resolution.
 * - The rules applied are as follows: check for an exact match.
 * - Check for the closest aspect ratio match
 * - Use the next file with the closest aspect ratio that is the least bigger than the target resolution.
 */
static void selectMostSuitableResolution(const std::set<std::string> availableResolutions, const std::string& sTargetResolution, std::string& sMostSuitableResolution)
{
  int iTargetWidth = 0, iTargetHeight = 0;
  if (!parseResolution(sTargetResolution, iTargetWidth, iTargetHeight))
  {
    throw std::invalid_argument("Invalid resolution");
  }
  if (iTargetHeight == 0)
  {
    throw std::invalid_argument("Invalid target resolution");
  }
  double dTargetAspectRatio = iTargetWidth / static_cast<double>(iTargetHeight);

  std::vector<std::string> resolutions;
  std::vector<int> widths; 
  std::vector<int> heights;
  std::vector<double> aspectRatios;
  widths.reserve(availableResolutions.size());
  heights.reserve(availableResolutions.size());
  aspectRatios.reserve(availableResolutions.size());
  std::vector<bool> arMatch;

  
  for (auto sRes : availableResolutions)
  {
    int iWidth = 0;
    int iHeight = 0;
    if (parseResolution(sRes, iWidth, iHeight))
    {
      // early exit on exact match
      if (iWidth == iTargetWidth && iHeight == iTargetHeight)
      {
        sMostSuitableResolution = sRes;
        return;
      }
      widths.push_back(iWidth);
      heights.push_back(iHeight);
      resolutions.push_back(sRes);
      if (iHeight == 0)
        throw std::invalid_argument("Invalid resolution");
      double dAspectRatio = iWidth / static_cast<double>(iHeight);
      aspectRatios.push_back(dAspectRatio);
      if (dTargetAspectRatio == dAspectRatio)
        arMatch.push_back(true);
      else
        arMatch.push_back(false);
    }
    else
    {
      throw std::invalid_argument("Invalid resolution");
    }
  }

  int iMinIndex = -1;
  int iMinFactor = INT_MAX;
  // look for ar match and look for integer divisibility
  for (size_t i = 0; i < widths.size(); ++i)
  {
    if (arMatch[i])
    {
      // we only want resolutions bigger than the target to not loose image quality
      if (widths[i] >= iTargetWidth)
      {
        int iRemainder = widths[i] % iTargetWidth;
        int iFactor = widths[i] / iTargetWidth;
        if (iRemainder == 0 && iFactor <= iMinFactor)
        {
          iMinIndex = i;
          iMinFactor = iFactor;
        }
      }
    }
  }
  if (iMinIndex != -1)
  {
    sMostSuitableResolution = resolutions[iMinIndex];
    return;
  }

  int iMinDifference = INT_MAX;
  // else search for the first resolution where both components are bigger than the target resolution
  // TODO: perhaps looking for closest aspect ratio would be better
  for (size_t i = 0; i < widths.size(); ++i)
  {
    // we only want resolutions bigger than the target to not loose image quality
    if (widths[i] >= iTargetWidth && heights[i] >= iTargetHeight)
    {
      int iTotalBiggerThan = (widths[i] - iTargetWidth) + (heights[i] - iTargetHeight);
      if (iTotalBiggerThan <= iMinDifference)
      {
        iMinIndex = i;
        iMinDifference = iTotalBiggerThan;
      }
    }
  }
  if (iMinIndex != -1)
  {
    sMostSuitableResolution = resolutions[iMinIndex];
    return;
  }

  // worst case: select largest image
  for (size_t i = 0; i < widths.size(); ++i)
  {
    // we only want resolutions bigger than the target to not loose image quality
    if (widths[i] >= iTargetWidth && heights[i] >= iTargetHeight)
    {
      int iTotalSmallerThan = (iTargetWidth - widths[i]) + (iTargetHeight - heights[i]);
      if (iTotalSmallerThan <= iMinDifference)
      {
        iMinIndex = i;
        iMinDifference = iTotalSmallerThan;
      }
    }
  }

  assert(!widths.empty());

  // in the worst case select any resolution
  if (iMinIndex == -1)
  {
    iMinIndex = widths.size() - 1;
  }

  sMostSuitableResolution = resolutions[iMinIndex];
}

/**
 * @brief calculate best frame skipping parameters based on actual and target framerate.
 * returns true on success, false otherwise
 */
static bool calcFrameSkippingParameters(double dActualFrameRate, double dTargetFrameRate, int& iSkipFrame, int& tTotalFrames)
{
  if (dTargetFrameRate > dActualFrameRate)
  {
    return false;
  }
  const double EPSILON = 0.0001;
  if (dActualFrameRate - dTargetFrameRate < EPSILON)
  {
    iSkipFrame = 0;
    tTotalFrames = 0;
    return true;
  }

  // find brute force min diff
  int numerators[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  int denominators[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10 };

  int iMinI = -1, iMinJ = -1;
  double dMinDiff = std::abs(dActualFrameRate - dTargetFrameRate);

  for (size_t i = 0; i < sizeof(numerators) / sizeof(int); ++i)
  {
    for (size_t j = 0; j < sizeof(denominators) / sizeof(int); ++j)
    {
      if (denominators[j] <= numerators[i]) continue;
      // double dSkipFraction = (double)numerators[i] / denominators[i];
      double dRemainingFraction = (denominators[j] - numerators[i]) / (double)denominators[j];
      double dFrameRateAfterSkipping = dActualFrameRate * dRemainingFraction;
      double dDiff = std::abs(dFrameRateAfterSkipping - dTargetFrameRate);
      if (dDiff < dMinDiff)
      {
        iMinI = i;
        iMinJ = j;
        dMinDiff = dDiff;
      }
    }
  }
  if (iMinI != -1)
  {
    iSkipFrame = numerators[iMinI];
    tTotalFrames = denominators[iMinJ];
  }
  return true;
}

static bool lowestRatio(double SourceFrameRate, double targetFrameRate, int& iSkipFrame, int& tTotalFrames)
{

  if (targetFrameRate > SourceFrameRate)
  {
    return false;
  }
  const double EPSILON = 0.0001;
  if (SourceFrameRate - targetFrameRate < EPSILON)
  {
    iSkipFrame = 0;
    tTotalFrames = 0;
    return true;
  }

  //get rid of the floating point
  //limited to 1 decimal for now
  if (fmod(targetFrameRate, 1) != 0 || fmod(SourceFrameRate, 1) != 0)
  {
    targetFrameRate = targetFrameRate * 10;
    SourceFrameRate = SourceFrameRate * 10;
  }

  double targetFrameRateTemp(round(targetFrameRate)), SourceFrameRateTemp(round(SourceFrameRate));
  //Logic to find the greatest common factor
  while (true)
  {
    (targetFrameRateTemp > SourceFrameRateTemp) ? targetFrameRateTemp = remainder(targetFrameRateTemp, SourceFrameRateTemp) :
      SourceFrameRateTemp = remainder(SourceFrameRateTemp, targetFrameRateTemp);
    if (targetFrameRateTemp < 0)
    {
      targetFrameRateTemp += SourceFrameRateTemp;
    }
    else if (SourceFrameRateTemp < 0)
    {
      SourceFrameRateTemp += targetFrameRateTemp;
    }
    if (targetFrameRateTemp <= 0 || SourceFrameRateTemp <= 0)
    {
      break;
    }
  }
  // Divide by the GCF to get the lowest ratio
  if (targetFrameRateTemp == 0)
  {
    iSkipFrame = (unsigned int)((SourceFrameRate - targetFrameRate) / SourceFrameRateTemp);
    tTotalFrames = (unsigned int)(SourceFrameRate / SourceFrameRateTemp);
  }
  else if (SourceFrameRateTemp == 0)
  {
    iSkipFrame = (unsigned int)((SourceFrameRate - targetFrameRate) / targetFrameRateTemp);
    tTotalFrames = (unsigned int)(SourceFrameRate / targetFrameRateTemp);
  }
  else
  {
    //The previous loop prevent this from happening
  }
  return true;
}

/**
 * @brief Determines crop parameters according to https://docs.google.com/spreadsheets/d/15DdT4W7hPqMzM0zRSu78HJPMAU5YRPFSjrB1HLzYcfY/edit#gid=0
 *
 * Target resolution must be multiples of 32 pel in both dimensions
 * @param[in] uiWidth Actual width
 * @param[in] uiHeight Actual height
 * @param[in] uiDesiredWidth Desired width
 * @param[in] uiDesiredHeight Desired height
 * @param[out] uiTopBottomCrop Pixels to be cropped on top/bottom
 * @param[out] uiLeftRightCrop Pixels to be cropped on left/right
 * @return true if the method is able to determine suitable crop parameters, false otherwise.
 */
static bool determineCropParameters(const uint32_t uiWidth, const uint32_t uiHeight, const uint32_t uiDesiredWidth, const uint32_t uiDesiredHeight, uint32_t& uiTopBottomCrop, uint32_t& uiLeftRightCrop)
{
  // if (uiDesiredWidth > uiWidth || uiDesiredHeight > uiDesiredHeight) return false;
  if ((uiDesiredWidth % 32) != 0 || (uiDesiredHeight % 32) != 0) return false;
  double dAspectRatio = uiWidth / static_cast<double>(uiHeight);
  double dDesiredAspectRatio = uiDesiredWidth / static_cast<double>(uiDesiredHeight);
  const double EPSILON = 0.00001;
  if (std::abs(dAspectRatio - dDesiredAspectRatio) < EPSILON)
  {
    // no difference in aspect ratio, scale
    return true;
  }
  if (dAspectRatio > dDesiredAspectRatio)
  {
    uiLeftRightCrop = static_cast<uint32_t>(std::round((uiWidth - (uiHeight * dDesiredAspectRatio))));
  }
  else
  {
    uiTopBottomCrop = static_cast<uint32_t>(std::round((uiHeight - (uiWidth / dDesiredAspectRatio))));
  }
  return true;
}

} // artist

