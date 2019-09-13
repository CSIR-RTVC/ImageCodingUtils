#pragma once

namespace vpp {

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

} // vpp
