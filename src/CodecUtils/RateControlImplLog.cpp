/** @file

MODULE			  : RateControlImplLog

TAG				    : RCIL

FILE NAME		  : RateControlImplLog.cpp

DESCRIPTION		: A class to hold a natural log function  model for the frame buffer rate rate control to
				      match an average rate stream.

COPYRIGHT		  : (c)CSIR 2007-2017 all rights resevered

LICENSE			  : Software License Agreement (BSD License)

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

#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "RateControlImplLog.h"

/*
---------------------------------------------------------------------------
Constants
---------------------------------------------------------------------------
*/
#define RCIL_NUM_SHORT_TERM_SAMPLES 4

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
int RateControlImplLog::Create(int numOfFrames)
{
  /// Clean up first.
  Destroy();

  if ((!_logDRModel.Create(numOfFrames, LinearModel::xyType)) || (!_meanDiffModel.Create(numOfFrames, LinearModel::timeSeriesType)) ||
      (!_meanDiffShortModel.Create(RCIL_NUM_SHORT_TERM_SAMPLES, LinearModel::timeSeriesType)) || (!_buffRateSamples.Create(numOfFrames)) || 
      (!_ratePredError.Create(numOfFrames)))
    return(0);

  _numOfFrames = numOfFrames; 
  _buffRateSamples.Clear(); 
  _ratePredError.Clear();

  _outOfBoundsFlag    = false;

  /// Long term mean diff
  _a1     = 1.0;
  _a2     = 0.0;
  _R2     = 1.0;
  /// Short term
  _a1_s   = 1.0;
  _a2_s   = 0.0;
  _R2_s   = 1.0;

  _modelFit   = 1.0;
  _choice     = 0;
  _predRate   = 0.0;
  _mseSignal  = 0.0;

  _RCTableLen = 0;
  _RCTablePos = 0;

#ifdef RCIL_DUMP_RATECNTL
  _RCTableLen = 300;
	_RCTable.Create(12, _RCTableLen);

  _RCTable.SetHeading(0, "Frame");
  _RCTable.SetDataType(0, MeasurementTable::INT);
  _RCTable.SetHeading(1, "Dmax");
  _RCTable.SetDataType(1, MeasurementTable::INT);
  _RCTable.SetHeading(2, "Rate");
  _RCTable.SetDataType(2, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(3, "Pred Rate");
  _RCTable.SetDataType(3, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(4, "ln(MSE/Dmax)");
  _RCTable.SetDataType(4, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(5, "MSE");
  _RCTable.SetDataType(5, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(6, "Pred MSE");
  _RCTable.SetDataType(6, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(7, "a");
  _RCTable.SetDataType(7, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(8, "b");
  _RCTable.SetDataType(8, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(9, "Out Bound");
  _RCTable.SetDataType(9, MeasurementTable::INT);
  _RCTable.SetHeading(10, "Choice");
  _RCTable.SetDataType(10, MeasurementTable::INT);
  _RCTable.SetHeading(11, "Fit");
  _RCTable.SetDataType(11, MeasurementTable::DOUBLE);

  _predMD           = 0.0;
  _MD               = 0.0;
  _predDmax         = 0.0;
  _rate             = 0.0;
#endif

  return(1);
}//end Create.

void RateControlImplLog::Destroy(void)
{
  _RCTable.Destroy();
  _RCTableLen = 0;
  _RCTablePos = 0;

  _ratePredError.Destroy(); 
  _buffRateSamples.Destroy(); 

  _logDRModel.Destroy();
  _meanDiffModel.Destroy();
  _meanDiffShortModel.Destroy();

  _numOfFrames = 0;

}//end Destroy.

void RateControlImplLog::Dump(const char* filename)
{
  if(_RCTablePos > 0)
    _RCTable.Save(filename, ",", 1);
  _RCTablePos = 0;
}//end Dump.

void RateControlImplLog::Dump(void)
{
  std::ostringstream filename;
  filename << RCIL_DUMP_FILENAME;  ///< Head.
  filename << ".csv";              ///< Tail
  Dump(filename.str().c_str());
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
All rate measurements are in bpp.
@param rate       : Total rate of the frame including headers.
@param coeffrate  : Not used in this implementation.
@param distortion : Implementation specific distortion of the frame.
@param mse        : Implementation specific signal mean square error of the frame.
@param mae        : Implementation specific signal mean absolute error of the frame.
@return           : none.
*/
void RateControlImplLog::StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae)
{ 
  bool initialisationRequired = (bool)(!ValidData());  ///< Status prior to storing new measurement values.

#ifdef RCIL_DUMP_RATECNTL
  _MD   = mse;
  _rate = rate;

  /// This method is the last to be called after processing a frame. Therefore dump here prior to updating the params.
  if(_RCTablePos < _RCTableLen)
  {
    _RCTable.WriteItem(0, _RCTablePos, _RCTablePos);                ///< Frame
    _RCTable.WriteItem(1, _RCTablePos, (int)(0.5 + distortion));    ///< Dmax
    _RCTable.WriteItem(2, _RCTablePos, rate);                       ///< Actual Rate
    _RCTable.WriteItem(3, _RCTablePos, _predRate);                  ///< Pred Rate
    _RCTable.WriteItem(4, _RCTablePos, _logDRModel.GetXSample(0));  ///< ln(MSE/Dmax)
    _RCTable.WriteItem(5, _RCTablePos, mse);                        ///< Actual MSE
    _RCTable.WriteItem(6, _RCTablePos, _predMD);                    ///< Pred MSE
    _RCTable.WriteItem(7, _RCTablePos, _a);                         ///< Model parameter a
    _RCTable.WriteItem(8, _RCTablePos, _b);                         ///< Model parameter b
    _RCTable.WriteItem(9, _RCTablePos, (int)_outOfBoundsFlag);      ///< Out of Bounds
    _RCTable.WriteItem(10, _RCTablePos, _choice);                   ///< Model choice
    _RCTable.WriteItem(11, _RCTablePos, _modelFit);                 ///< Model R^2 fit

    _RCTablePos++;
  }//end if _RCTablePos... 
#endif

  ///-------------------- Store all new measurements -------------------------------------------------------
  _buffRateSamples.AddFirstIn(rate);
  _ratePredError.AddFirstIn(rate - _predRate); 

  /// Range check before storing MSE data samples.
  double sigma = mse;
  if (sigma < 1.0) sigma = 1.0;
  /// Add y as a time series sample point.
  _meanDiffModel.PutSamples(sigma, initialisationRequired);
  _meanDiffShortModel.PutSamples(sigma, initialisationRequired);

  /// Range checks before storing log(d)-rate model data samples.
  double d = distortion;
  if (d < 1.0) d = 1.0;
  d = mse / d;
  if (d <= 0.0) d = 0.0001;
  /// Add (x,y) = (ln(mse/d),rate) sample point to model and update the running sums associated with the model.
  _logDRModel.PutSamples(log(d), rate, initialisationRequired);

  ///-------------------- Additional processing ---------------------------------------------------------------
  /// Implement any additional initialisation or updates that are not performed during 
  /// the base LinearModel::Put...() methods above.
  if(!initialisationRequired) ///< Update the running sums associated with the model.
    ModelUpdate();
  else  ///< Initialise.
  {
    /// Fill the model with dummy data to match non-linear _a and _b parameters. Faster model convergence after initialisation.
    double x = _logDRModel.GetXSample(0); ///< Store temp initial values.
    double y = _logDRModel.GetYSample(0);
    /// 20% either side of intial rate value.
    double stepSize = (0.4*rate) / (double)_numOfFrames;
    double r = 0.8*rate;
    for (int i = 0; i < (_numOfFrames-1); i++, r += stepSize)
      _logDRModel.PutSamples(((r-_b)/_a), r, false);
    /// Restore.
    _logDRModel.PutSamples(x, y, false);
  }///end else...

  /// Long term R sqr fit
  _R2 = _meanDiffModel.RSqr(_a1, _a2);
  /// ... and short term R sqr fit
  _R2_s = _meanDiffShortModel.RSqr(_a1_s, _a2_s);
  /// Accuracy of the model over the samples in the buffer.
//  _modelFit = RSqr(_a, _b);
//  _modelFit = _logDRModel.GetObjFunc(_a, _b) / (double)_numOfFrames;
  _modelFit = _ratePredError.GetSum() / (double)_ratePredError.GetLength(); ///< Mean rate prediction error over buffer length.

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
int RateControlImplLog::PredictDistortion(double targetAvgRate, double rateLimit)
{
  _outOfBoundsFlag              = false;
  _upperDistortionOverflowFlag  = false;
  _lowerDistortionOverflowFlag  = false;
  _upperRateOverflowFlag        = false;
  _lowerRateOverflowFlag        = false;

  /// Default settings for the non-valid data case.
  double targetRate      = targetAvgRate;
  double targetRateLimit = rateLimit;

  ///------------------ Rate Prediction --------------------------------------
  if(ValidData())
  {
    /// Buffer averaging model: What must the target rate for the
    /// next N frames be to make the average over the rate buffer equal
    /// to the average target rate specified in the parameter?

    /// N past frames and recover in k frames: target rate = target avg rate*(N+k)/k - total buff/k.
    int numRecoverFrames = _numOfFrames;  ///< Make recovery same as past num of frames until further notice.
    targetRate = ((targetAvgRate * (double)(_numOfFrames + numRecoverFrames)) / (double)numRecoverFrames) - (_buffRateSamples.GetSum() / (double)numRecoverFrames);

  }//end if ValidData...

  /// Keep the target rate within the upper rate limit.
  if(targetRate > targetRateLimit)
    targetRate = targetRateLimit;

  /// No less than the coeff floor rate.
  if(targetRate < 0.00)
    targetRate = _rateLower;

  ///--------------- Distortion Prediction -----------------------------------------

  /// Predict Mean Diff with linear extrapolation model. Select between a long term or
  /// short term prediction based on the best statistical R^2 fit with the model. Override
  /// the prediction if a scene change was signalled externally.
  double predMD = ModelMeanDifference();

  /// Model function solved for distortion.
  double distd = ModelDistortion(targetRate, _a, _b, predMD);

  /// Limit a decrease change in distortion (implying limiting an increase in rate) to 25% of previous distortion.
  double prevd = _meanDiffModel.GetYSample(0) / (exp(_logDRModel.GetXSample(0)));
  double limit = prevd - (0.25*prevd);
  if((distd < limit)&&(prevd < _distUpper))
  {
    distd = limit;
    targetRate = ModelRate(distd, _a, _b, predMD);
  }//end if distd...

  /// Cannot be greater than 16x16x(256^2). In response, converge
  /// towards the upper distortion limit by halving from the previous 
  /// frame distortion.
  if(distd > _distUpper)
  {
    distd = prevd + (abs(_distUpper - prevd)*0.5);
    _outOfBoundsFlag = true;
    _upperDistortionOverflowFlag = true;
  }//end if distd...
  /// Cannot be less than 16x16x(2^2). Converge to lower distortion limit.
  if(distd < _distLower)
  {
    distd = prevd - (abs(prevd - _distLower)*0.5);
    _outOfBoundsFlag = true;
    _lowerDistortionOverflowFlag = true;
  }//end if distd...

  /// Update the target rate with changes in Dmax boundaries.
  if(_outOfBoundsFlag)
    targetRate = ModelRate(distd, _a, _b, predMD);

  /// Store the most recent rate requirement.
  _predRate = targetRate;

#ifdef RCIL_DUMP_RATECNTL
  _predMD   = predMD;
  _predDmax = distd;
#endif

  return((int)(distd + 0.5));
}//end PredictDistortion.

/*
----------------------------------------------------------------------------------------------------
  Private Methods
----------------------------------------------------------------------------------------------------
*/

/** Predict the mean difference between pels in the frame.
Predict the frame Mean Diff with linear extrapolation model. Select between a long term or
short term prediction based on the best statistical R^2 fit with the model. Override the 
prediction if a scene change was signalled externally and reset the signal to zero.
@return : Predicted mean difference.
*/
double RateControlImplLog::ModelMeanDifference(void)
{
  double predMD;
  if (_mseSignal == 0.0)
  {
    if (_R2_s > _R2)  ///< Short term is closer to 1.0 than the long term (better fit).
      predMD = (_a1_s*(double)(RCIL_NUM_SHORT_TERM_SAMPLES + 1)) + _a2_s; ///< Mean diff samples are in reverse order.
    else
      predMD = (_a1*(double)(_numOfFrames + 1)) + _a2;
    /// Can never be negative. Use last value if prediction is negative.
    if (predMD < 0.0)
      predMD = _meanDiffModel.GetYSample(0);
  }//end if _mseSignal...
  else
  {
    predMD = _mseSignal;
    _mseSignal = 0.0;  ///< Clear the signal.
  }//end else...

  return(predMD);
}//end ModelMeanDifference.

/** Update all R-D model parameters.
Update the model parameters for the R-D model (ln(md/Dmax), r), Mean Diff and the running 
sums. The latest data must have been loaded into the fifo buffers prior to calling this 
method. The values are updated on every StoreMeasurement() call.
@return : None.
*/
void RateControlImplLog::ModelUpdate(void)
{
  /// Determine new R-D model parameters.
  double tempValForSlopeBoundaryCheck = -1.0 / (log(_meanDiffModel.GetYSample(0) / _distUpper));
  /// Improvements on the current parameters applied to the objective function. Find the Cramer's Rule 
  /// solution and test it against boundary conditions. If the solution is out of bounds then use a gradient
  /// decsent algorithm to modify the current parameters. Choose only those solutions that improve on the
  /// objective function.

  /// Cramer's Solution and objective function (aa,bb) with current objective function (_a,_b)
  double aa = 0.0;
  double bb = 0.0;

  /// Three model options.
  double cramerObjFunc = _logDRModel.GetModelLin(&aa, &bb); ///< Normal
  //double cramerObjFunc = _logDRModel.GetModelLin(&aa, &bb, _logDRModel.GetXSample(0), _logDRModel.GetYSample(0)); ///< Through a point.

  /// Identify a list of sample positions that exclude outliers based on the difference between the
  /// predicted rate and actual rate.
  //int nonOutlierList[16];
  //int listLen = GetNonOutliers(nonOutlierList);
  //double cramerObjFunc = _logDRModel.GetModelLin(&aa, &bb, nonOutlierList, listLen);  ///< Outliers removed.

  double currObjFunc   = _logDRModel.GetObjFunc(_a, _b);
  //double currObjFunc = _logDRModel.GetObjFunc(_a, _b, nonOutlierList, listLen);
  /// Outside boundary conditions.
  bool cramerDisqualified = (bb <= 0.0) || (aa <= (bb*tempValForSlopeBoundaryCheck));

///////////////////////////////////////// Test code
  _choice = 0;
  if (cramerDisqualified)
    _choice = 1;
/////////////////////////////////////////

  /// Choose best improved solution.
  if (!cramerDisqualified && (cramerObjFunc <= currObjFunc))
  {
    _a = aa;
    _b = bb;
  }//end if !cramerDisqualified...
  else ///< use gradient descent change if Cramer's soln is out of bounds.
  {
    double a = _a;
    double b = _b;
    double gradObjFunc = GetGradStep(&a, &b);
    if ((gradObjFunc <= currObjFunc) && (a > (b*tempValForSlopeBoundaryCheck)) && (b > 0.0))
    {
      _a = a;
      _b = b;
    }//end if gradObjFunc...
  }//end else...

  /// Update the mean diff long term model parameters
  _meanDiffModel.GetModelLin(&_a1, &_a2);
  /// ... and short term
  _meanDiffShortModel.GetModelLin(&_a1_s, &_a2_s);

}//end ModelUpdate.

 /** Calculate a gradient descent solution to improve on the input model parameters.
 Steepest descent algorithm using the (x,y) sample points stored in the model. Note that
 this implementation only takes one step in the descent with the purpose of giving the
 model a 'nudge' towards a better solution.
 @param  slope       : Linear model slope to improve on.
 @param  yintercept  : Linear model y-intercept to improve on.
 @return             : Solution objective function.
 */
double  RateControlImplLog::GetGradStep(double* slope, double* yintercept)
{
  double aa = *slope;
  double bb = *yintercept;

  /// Get the gradient vector (dsda, dsdb) at the point (aa, bb) = (slope,yintercept).
  double dsda = 2.0*((aa*_logDRModel.GetSumX2()) + (bb*_logDRModel.GetSumX()) - _logDRModel.GetSumXY());
  double dsdb = 2.0*((aa*_logDRModel.GetSumX()) + (bb*(double)(_logDRModel.GetNumSamples())) - _logDRModel.GetSumY());

  /// Search for gamma.
  double gamma = 1.0;
  double a = aa;
  double b = bb;
  double preva, prevb;
  double gradObjFunc = _logDRModel.GetObjFunc(aa, bb) + 1.0; ///< Ensure while loop starts.
  int cnt = 0;
  bool goingDown = true;
  while ((cnt < 26) && (goingDown))
  {
    /// Temporarily store the previous position before the next descent step.
    preva = a;
    prevb = b;

    /// Get new position (a, b) by taking a single gradient descent step from point (aa, bb) along the
    /// gradient vector (dsda, dsdb) with a step fraction of gamma.
    a = aa - (gamma * dsda);
    b = bb - (gamma * dsdb);

    double lclGradObjFunc = _logDRModel.GetObjFunc(a, b);

    /// If not descending then back up to previous (a, b) and get out.
    if (cnt && (lclGradObjFunc >= gradObjFunc))
    {
      goingDown = false;
      gamma = gamma*2.0;  ///< Reverse back.
      /// Recalculate the reversed position (a, b) from (aa, bb).
      a = preva;
      b = prevb;
    }//end if lclGradObjFunc...
    else
    {
      gradObjFunc = lclGradObjFunc;
      gamma = gamma / 2.0;
    }//end else...

    cnt++;
  }//end while cnt...

   /// Update to new model parameters.
  *slope = a;
  *yintercept = b;

  return(gradObjFunc);
}//end GetGradStep.

/** Calculate R^2 for this inverse ln model.
Determine the R^2 statistical model fit within the array of rate samples. Note
that the most recent samples are at location samples[0].
@param  a  : r = a*ln(mse/d) + b.
@param  b  :
@return    : R^2.
*/
double RateControlImplLog::RSqr(double a, double b)
{
  double* samples     = _logDRModel.GetYFifo()->GetBuffer();
  double* distortion  = _logDRModel.GetXFifo()->GetBuffer(); ///< Stored as ln(d/mse).

  double mean = 0.0;
  for (int i = 0; i < _numOfFrames; i++)
    mean += samples[i];
  mean = mean / _numOfFrames;

  double SStot = 0.0;
  double SSres = 0.0;
  for (int i = 0; i < _numOfFrames; i++)
  {
    double r = samples[i];
    double d = distortion[i];

    SStot += (r - mean)*(r - mean);
    double rm = (a*d) + b;
    SSres += (r - rm)*(r - rm);
  }//end for i...

  double R2 = 1.0;
  if (SStot != 0.0)
    R2 = 1.0 - (SSres / SStot);

  return(R2);
}//end RSqr.

/** Get the locations of the non-outliers and the length of the list.
It is assumed that the list length is sufficient at least the length
of the _ratePredError fifo from which this method operates on.
@param  list  : List buffer
@return       : list length
*/
int RateControlImplLog::GetNonOutliers(int* list)
{
  int i;
  int samplesLen  = _ratePredError.GetLength();
  //int listLen     = samplesLen;
  int listLen = 0;
  double* samples = _ratePredError.GetBuffer();
  double stddev   = 2.0 * StdDevRateErr();

  /// Add the pos of those samples that are within 2 std dev to the list.
  for (i = 0; i < samplesLen; i++)
  {
    if (abs(samples[i]) <= stddev) ///< Within 2 std deviations.
      list[listLen++] = i;
  }//end for i...


/*
  /// Create an ordered list from best to worst.
  list[0] = 0;
  for (i = 1; i < samplesLen; i++)
  {
    double err = abs(samples[i]);
    int j;
    for (j = 0; j < i; j++)
      if (err < abs(samples[list[j]])) break;

    int k;
    for (k = i; k > j; k--)
      list[k] = list[k - 1];

    list[j] = i;
  }//end for i...

  /// Check that the list is ordered.
//  for (i = 1; i < samplesLen; i++)
//    if (abs(samples[list[i]])< abs(samples[list[i-1]]))
//      int check = 1;

  for (i = 0; i < samplesLen; i++)
    if(abs(samples[list[i]]) > stddev) break;

  int maxOutliers = 15;
  int room = samplesLen - i;

  if (room >= maxOutliers) listLen = samplesLen - maxOutliers;
  else listLen = samplesLen - room;
*/
  return(listLen);
}//end GetNonOutliers.


