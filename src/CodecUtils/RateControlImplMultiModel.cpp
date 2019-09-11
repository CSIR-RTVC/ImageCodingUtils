/** @file

MODULE				: RateControlImplMultiModel

TAG						: RCIMM

FILE NAME			: RateControlImplMultiModel.cpp

DESCRIPTION		: A class to hold a power function  model for the frame buffer rate rate control to
                match an average rate stream.

COPYRIGHT			: (c)CSIR 2007-2015 all rights resevered

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

=========================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#endif

/// The rate control models used by this model.
#include "RateControlImplQuad.h"
#include "RateControlImplPow.h"
#include "RateControlImplLog.h"

#include "RateControlImplMultiModel.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
RateControlImplMultiModel::RateControlImplMultiModel(double modelparms[][2], int nummodels) 
{ 
  ResetMembers(); 
  for(int i = 0; i < nummodels; i++)
  {
    _modelParam[i][0] = modelparms[i][0];
    _modelParam[i][1] = modelparms[i][1];
  }//end for i...
} // end constructor.

void RateControlImplMultiModel::ResetMembers(void)
{
  _numOfFrames  = 0;
  _rateUpper    = 24.0; 
  _rateLower    = 0.0001; 
  _distUpper    = 16777216.0; 
  _distLower    = 1024.0;

  _prevPredModelUsed   = QUAD;
  _modelParam[QUAD][0] = 40.0;
  _modelParam[QUAD][1] = 20000.0;
  _modelParam[POW][0]  = 15000.0;
  _modelParam[POW][1]  = -1.3;
  _modelParam[LOG][0]  = 0.2;
  _modelParam[LOG][1]  = 1.4;
  for(int i = 0; i < MODELS; i++)
    _model[i] = NULL;

//////////////////////////////////
  _modelCount[0] = 0;
  _modelCount[1] = 0;
  _modelCount[2] = 0;
}//end ResetMembers.

int RateControlImplMultiModel::Create(int numOfFrames)
{
  /// Clean up first.
  Destroy();

  _numOfFrames = numOfFrames;

  _outOfBoundsFlag    = false;

  /// Implementation specific construction for each model.
  _model[QUAD] = new RateControlImplQuad(_modelParam[QUAD][0], _modelParam[QUAD][1]);
  _model[POW]  = new RateControlImplPow(_modelParam[POW][0], _modelParam[POW][1]);
  _model[LOG]  = new RateControlImplLog(_modelParam[LOG][0], _modelParam[LOG][1]);
  if(!ModelsActive()) 
    return(0); ///< Mem alloc of classes failed.

  /// Create the internal model controllers.
  for(int m = 0; m < MODELS; m++)
    if(!_model[m]->Create(numOfFrames))
      return(0);

  _prevPredModelUsed = QUAD;

  _RCTableLen = 0;
  _RCTablePos = 0;

#ifdef RCIMM_DUMP_RATECNTL
  _RCTableLen = 500;
	_RCTable.Create(8, _RCTableLen);

  _RCTable.SetHeading(0, "Frame");
  _RCTable.SetDataType(0, MeasurementTable::INT);
  _RCTable.SetHeading(1, "Dmax");
  _RCTable.SetDataType(1, MeasurementTable::INT);
  _RCTable.SetHeading(2, "Rate");
  _RCTable.SetDataType(2, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(3, "Pred Rate");
  _RCTable.SetDataType(3, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(4, "Pred Mean Diff");
  _RCTable.SetDataType(4, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(5, "Mean Diff");
  _RCTable.SetDataType(5, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(6, "RD a");
  _RCTable.SetDataType(6, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(7, "RD b");
  _RCTable.SetDataType(7, MeasurementTable::DOUBLE);

  _predMD           = 0.0;
  _MD               = 0.0;
  _predDmax         = 0.0;
  _predRate         = 0.0;
  _rate             = 0.0;
#endif

  return(1);
}//end Create.

void RateControlImplMultiModel::Destroy(void)
{
  _RCTable.Destroy();

  for(int m = 0; m < MODELS; m++)
  {
    if(_model[m] != NULL)
    {
      _model[m]->Destroy();
      delete _model[m];
    }//end if _model...
    _model[m] = NULL;
  }//end for m...

  _numOfFrames = 0;
  _prevPredModelUsed = QUAD;

///////////////////////////////////
  int q = _modelCount[0];
  int p = _modelCount[1];
  int l = _modelCount[2];

}//end Destroy.

void RateControlImplMultiModel::SetRDLimits(double rateUpper, double rateLower, double distUpper, double distLower) 
{ 
  _rateUpper = rateUpper; 
  _rateLower = rateLower; 
  _distUpper = distUpper; 
  _distLower = distLower; 

  if(ModelsActive())
  {
    _model[QUAD]->SetRDLimits(rateUpper, rateLower, distUpper/4, sqrt(distLower)); ///< Quad uses abs distortion measurements.
    _model[POW]->SetRDLimits(rateUpper, rateLower, distUpper, distLower);
    _model[LOG]->SetRDLimits(rateUpper, rateLower, distUpper, distLower);
  }//end if (_model[QUAD] != NULL)...
}//end SetRDLimits.

void RateControlImplMultiModel::Reset(void)
{
  for(int m = 0; m < MODELS; m++)
    _model[m]->Reset();

  _prevPredModelUsed = QUAD;
}//end Reset.

void RateControlImplMultiModel::Dump(const char* filename)
{
  if(_RCTablePos > 0)
    _RCTable.Save(filename, ",", 1);
  _RCTablePos = 0;
}//end Dump.

/*
---------------------------------------------------------------------------
	Public Interface Methods.
---------------------------------------------------------------------------
*/

///---------------------- Post-Encoding Measurements --------------------------

/// Add samples to the fifo buffers and hold the sample that is discarded. The
/// last discarded value is needed in the running sum update process.

/** Store model measurements after encoding a frame.
All rate measurements are in bpp. This implementation does not use the mae parameter.
@param rate       : Total rate of the frame including headers.
@param coeffrate  : Not used in this implementation.
@param distortion : Implementation specific distortion of the frame.
@param mse        : Implementation specific signal mean square difference between pels in the frame.
@param mae        : Implementation specific signal mean absolute difference between pels in the frame.
@return           : none.
*/
void RateControlImplMultiModel::StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae)
{ 
  /// Pass on the measurements to the all the models for initialisation.
  for(int m = 0; m < MODELS; m++)
    _model[m]->StoreMeasurements(rate, coeffrate, distortion, mse, mae);

#ifdef RCIMM_DUMP_RATECNTL
  _MD   = GetMostRecentMeanDiff();
  _rate = GetMostRecentBuffRate();

  /// This method is the last to be called after processing a frame. Therefore dump here.
  if(_RCTablePos < _RCTableLen)
  {
    _RCTable.WriteItem(0, _RCTablePos, _RCTablePos);            ///< Frame
    _RCTable.WriteItem(1, _RCTablePos, (int)(0.5 + _predDmax)); ///< Dmax
    _RCTable.WriteItem(2, _RCTablePos, _rate);                  ///< Rate
    _RCTable.WriteItem(3, _RCTablePos, _predRate);              ///< Pred Rate
    _RCTable.WriteItem(4, _RCTablePos, _predMD);                ///< Pred Mean Diff
    _RCTable.WriteItem(5, _RCTablePos, _MD);                    ///< Mean Diff
    _RCTable.WriteItem(6, _RCTablePos, _a);                     ///< Model parameter a
    _RCTable.WriteItem(7, _RCTablePos, _b);                     ///< Model parameter b

    _RCTablePos++;
  }//end if _RCTablePos... 
#endif

}//end StoreMeasurements.

///---------------------- Pre-Encoding Predictions --------------------------

/** Predict the distortion for the next frame from the desired average rate.
The distortion measure is predicted from the coeff rate (total rate - header rate) using
an appropriate R-D model. The header rate and the signal mean diff are predicted from the
previous measured frame data using linear extrapolation.
@param  targetAvgRate : Total targeted average rate (including headers).
@param  rateLimit     : Upper limit to predicted rate.
@return               : Predicted distortion.
*/
int RateControlImplMultiModel::PredictDistortion(double targetAvgRate, double rateLimit)
{
  ///------------------ Model Selection --------------------------------------------

  int DPpred  = _model[POW]->PredictDistortion(targetAvgRate, rateLimit);
  int DLpred  = _model[LOG]->PredictDistortion(targetAvgRate, rateLimit);


  /// The distortion will be predicted from the past best performing model. Default 
  /// best model is the first one in the list.
  int bestModel = 1;  ///< Skip QUAD for now.
  double bestPerf = _model[bestModel]->GetPerformanceMeasure();
  for(int m = bestModel+1; m < MODELS; m++) ///< Test against remaining models.
  {
    double perf = _model[m]->GetPerformanceMeasure();
    if(perf < bestPerf)
    {
      bestPerf = perf;
      bestModel = m;
    }//end if diff...
  }//end for m...

  double perfQuad = _model[QUAD]->GetPerformanceMeasure();
  double perfPow  = _model[POW]->GetPerformanceMeasure();
  double perfLog  = _model[LOG]->GetPerformanceMeasure();

  ///--------------- Distortion Prediction ------------------------------------------
  _prevPredModelUsed  = bestModel;
  int bestDistortion  = _model[bestModel]->PredictDistortion(targetAvgRate, rateLimit);
  _outOfBoundsFlag    = _model[bestModel]->OutOfBounds(); ///< Out of bounds is set in the PredictDistortion() method.

///////////////////////////////////
  _modelCount[bestModel]++;

#ifdef RCIMM_DUMP_RATECNTL
  _predRate = targetRate;
  _predMD   = predMD;
  _predDmax = bestDistortion;
#endif

  return(bestDistortion);
}//end PredictDistortion.

