/** @file

MODULE:
 
TAG: 

FILE NAME: 

DESCRIPTION: 

COPYRIGHT: (c)CSIR 2007-2018 all rights reserved

LICENSE: Software License Agreement (BSD License)

RESTRICTIONS: 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation and/or 
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may 
be used to endorse or promote products derived from this software without specific 
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================

*/
#pragma once
#include <cstdint>
#include <CodecUtils/ICodecv2.h>
#include "CodecParameters.h"

/**
 * @brief Utility method to configure the RTCV H264 codec with default parameters
 */
static bool configureDefaultH263CodecParameters(ICodecv2* pCodec)
{
  if (!pCodec) return false;
  //Used for setting fixed size frame allocation
  pCodec->SetParameter(CODEC_PARAM_MODE_OF_OPERATION, "2");
  //Set default quality to 16
  pCodec->SetParameter(CODEC_PARAM_QUALITY, "1");
  //16 = YUV
  pCodec->SetParameter(CODEC_PARAM_IN_COLOUR, "16");
  //16 = YUV
  pCodec->SetParameter(CODEC_PARAM_OUT_COLOUR, "16");
  // This multiplies the bit size of every i-frame by 2
  pCodec->SetParameter(CODEC_PARAM_I_PICTURE_MULTIPLIER, "2");
  // Use a switch frame period of ten for now
  pCodec->SetParameter(CODEC_PARAM_SWITCH_FRAME_PERIOD, "10");
  // NB: New codec settings for auto-iframe detection: These settings need to correlate to the settings of the DECODER
  pCodec->SetParameter("unrestrictedmotion", "1");
  pCodec->SetParameter("extendedpicturetype", "1");
  pCodec->SetParameter("unrestrictedmotion", "1");
  pCodec->SetParameter("advancedintracoding", "1");
  pCodec->SetParameter("alternativeintervlc", "1");
  pCodec->SetParameter("modifiedquant", "1");
  pCodec->SetParameter("autoipicture", "1");
  pCodec->SetParameter("ipicturefraction", "0");
  // On Windows we will always flip the color-converted image
  pCodec->SetParameter("flip", "1");

  return true;
}

/**
 * @brief Utility method to configure the RTCV H264 codec with default parameters
 */
static bool configureDefaultH264CodecParameters(ICodecv2* pCodec)
{
  if (!pCodec) return false;
  //Set default quality to 16
  pCodec->SetParameter(CODEC_PARAM_QUALITY, "1");
  //16 = YUV
  pCodec->SetParameter(CODEC_PARAM_IN_COLOUR, "16");
  //16 = YUV
  pCodec->SetParameter(CODEC_PARAM_OUT_COLOUR, "16");
  // This multiplies the bit size of every i-frame by 2
  pCodec->SetParameter(CODEC_PARAM_I_PICTURE_MULTIPLIER, "1");

  pCodec->SetParameter(CODEC_PARAM_SWITCH_FRAME_PERIOD, "12");

  // default rate control values
  pCodec->SetParameter(CODEC_PARAM_MAX_BITS_PER_FRAME, "1000000");
  pCodec->SetParameter(CODEC_PARAM_NUM_RATE_CONTROL_FRAMES, "25");
  pCodec->SetParameter(CODEC_PARAM_MAX_DISTORTION, "30000");
  pCodec->SetParameter(CODEC_PARAM_IPICTURE_DMAX_MULTIPLIER, "2");
  pCodec->SetParameter(CODEC_PARAM_IPICTURE_DMAX_FRACTION, "0");
  pCodec->SetParameter(CODEC_PARAM_MINIMUM_INTRA_QP, "16");
  pCodec->SetParameter(CODEC_PARAM_MINIMUM_INTER_QP, "4");
  pCodec->SetParameter(CODEC_PARAM_RATE_OVERSHOOT_PERCENT, "100");

  // always flip the image on windows to get it into the desired YUV format
  pCodec->SetParameter("flip", "1");
  // NB: New codec settings for auto-iframe detection: These settings need to correlate to the settings of the DECODER
  pCodec->SetParameter("unrestrictedmotion", "1");
  pCodec->SetParameter("extendedpicturetype", "1");
  pCodec->SetParameter("unrestrictedmotion", "1");
  pCodec->SetParameter("advancedintracoding", "1");
  pCodec->SetParameter("alternativeintervlc", "1");
  pCodec->SetParameter("modifiedquant", "1");
  pCodec->SetParameter("autoipicture", "1");
  pCodec->SetParameter("ipicturefraction", "0");

  return true;
}

/**
 * @brief utility method to generate parameter sets
 */
bool generateParameterSets(ICodecv2* pCodec, std::string& sSps, std::string& sPps)
{
  uint8_t pBuffer[512];

  pCodec->SetParameter((char *)"seq param set", "0");
  pCodec->SetParameter((char *)"pic param set", "0");

  pCodec->SetParameter((char *)("generate param set on open"), "0");
  pCodec->SetParameter((char *)("picture coding type"), "2");	///< Seq param set = H264V2_SEQ_PARAM.
  if (!pCodec->Code(NULL, pBuffer, 512 * 8))
  {
    return false;
  }
  else
  {
    uint32_t uiSeqParamSetLen = pCodec->GetCompressedByteLength();
    pBuffer[uiSeqParamSetLen] = 0;
    sSps = std::string((const char*)pBuffer, uiSeqParamSetLen);
  }

  pCodec->SetParameter((char *)("picture coding type"), "3");	///< Pic param set = H264V2_PIC_PARAM.
  if (!pCodec->Code(NULL, pBuffer, 512 * 8))
  {
    return false;
  }
  else
  {
    uint32_t uiPicParamSetLen = pCodec->GetCompressedByteLength();
    pBuffer[uiPicParamSetLen] = 0;
    sPps = std::string((const char*)pBuffer, uiPicParamSetLen);
  }
  return true;
}


