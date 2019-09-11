/** @file

MODULE			: RateControlImplQuad

TAG				: RCIQ

FILE NAME		: RateControlImplQuad.cpp

DESCRIPTION		: A class to hold the model for the frame buffer rate rate control to
                match an average rate stream. This class models the RD curve with an
                inverse quadratic function.

COPYRIGHT		: (c)CSIR 2007-2017 all rights resevered

LICENSE			: Software License Agreement (BSD License)

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
#include "RateControlImplQuad.h"

#define RCIQ_NUM_RATE_BUFF_LENGTHS  1

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
int RateControlImplQuad::Create(int numOfFrames)
{
  /// Clean up first.
  Destroy();

  if( (!_buffRateSamples.Create(numOfFrames)) || (!_meanDiffModel.Create(numOfFrames, LinearModel::timeSeriesType)) ||
      (!_headerRateModel.Create(numOfFrames, LinearModel::timeSeriesType)) || (!_ratePredError.Create(RCIQ_NUM_RATE_BUFF_LENGTHS*numOfFrames)) ||
      (!_coeffRateSamples.Create(numOfFrames))|| (!_distortionSamples.Create(numOfFrames)) )
	  return(0);

  _numOfFrames = numOfFrames; 
  _buffRateSamples.Clear();

  _coeffRateSamples.Clear(); 
  _distortionSamples.Clear();
  _ratePredError.Clear();

  _prevCoeffRate      = 0.0;
  _prevDistortion     = 0.0;

  _prevCoeffRatePred  = 0.0;
  _outOfBoundsFlag    = false;

  _modelFit = 1.0;
  _choice   = 0;
  _predRate = 0.0;

  _Sx   = 0.0;
  _Sy   = 0.0;
  _Sxy  = 0.0;
  _Sx2  = 0.0;

  _a1   = 1.0;  ///< mae linear model parameters.
  _a2   = 0.0;

  _p1   = 1.0; ///< Header rate linear model parameters.
  _p2   = 0.0;

  _mseSignal = 0.0;

  _RCTableLen = 0;
  _RCTablePos = 0;

#ifdef RCIQ_DUMP_RATECNTL
  _RCTableLen = 300;
	_RCTable.Create(16, _RCTableLen);

  _RCTable.SetHeading(0, "Frame");
  _RCTable.SetDataType(0, MeasurementTable::INT);
  _RCTable.SetHeading(1, "Dmax");
  _RCTable.SetDataType(1, MeasurementTable::INT);
  _RCTable.SetHeading(2, "Rate");
  _RCTable.SetDataType(2, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(3, "Pred Rate");
  _RCTable.SetDataType(3, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(4, "MAD/Dmax");
  _RCTable.SetDataType(4, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(5, "Coeff Rate");
  _RCTable.SetDataType(5, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(6, "Pred Coeff");
  _RCTable.SetDataType(6, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(7, "Header Rate");
  _RCTable.SetDataType(7, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(8, "Pred Header");
  _RCTable.SetDataType(8, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(9, "MAD");
  _RCTable.SetDataType(9, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(10, "Pred MAD");
  _RCTable.SetDataType(10, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(11, "a");
  _RCTable.SetDataType(11, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(12, "b");
  _RCTable.SetDataType(12, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(13, "Out Bound");
  _RCTable.SetDataType(13, MeasurementTable::INT);
  _RCTable.SetHeading(14, "Choice");
  _RCTable.SetDataType(14, MeasurementTable::INT);
  _RCTable.SetHeading(15, "Fit");
  _RCTable.SetDataType(15, MeasurementTable::DOUBLE);

  _predMD           = 0.0;
  _MD               = 0.0;
  _predHeaderRate   = 0.0;
  _headerRate       = 0.0;
  _predDmax         = 0.0;
  _predCoeffRate    = 0.0;
  _coeffRate        = 0.0;
  _rate             = 0.0;
#endif

  return(1);
}//end Create.

void RateControlImplQuad::Destroy(void)
{
  _RCTable.Destroy();
  _RCTableLen = 0;
  _RCTablePos = 0;

  _ratePredError.Destroy();
  _distortionSamples.Destroy();
  _coeffRateSamples.Destroy(); 

  _buffRateSamples.Destroy();

  _meanDiffModel.Destroy();
  _headerRateModel.Destroy();

  _numOfFrames = 0;

}//end Destroy.

void RateControlImplQuad::Reset(void)
{
  _distortionSamples.MarkAsEmpty(); 
  _coeffRateSamples.MarkAsEmpty(); 

  _buffRateSamples.MarkAsEmpty();
  _ratePredError.MarkAsEmpty();

  _meanDiffModel.MarkAsEmpty();
  _headerRateModel.MarkAsEmpty();

}//end Reset.

void RateControlImplQuad::Dump(const char* filename)
{
  if(_RCTablePos > 0)
    _RCTable.Save(filename, ",", 1);
  _RCTablePos = 0;
}//end Dump.

void RateControlImplQuad::Dump(void)
{
	std::ostringstream filename;
	filename << RCIQ_DUMP_FILENAME;  ///< Head.
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
@param coeffrate  : Rate of the frame coeff encoding without headers.
@param distortion : Implementation specific squared difference distortion of the frame.
@param mse        : Implementation specific signal mean square error of the frame.
@param mae        : Implementation specific signal mean absolute error of the frame.
@return           : none.
*/
void RateControlImplQuad::StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae)
{ 
  bool initialisationRequired = (bool)(!ValidData());  ///< Status prior to storing new measurement values.

#ifdef RCIQ_DUMP_RATECNTL
  _MD			    = mae;
  _coeffRate	= coeffrate;
  _rate			  = rate;
  _headerRate	= rate - coeffrate;

  /// This method is the last to be called after processing a frame. Therefore dump everything here.
  if (_RCTablePos < _RCTableLen)
  {
	  _RCTable.WriteItem(0, _RCTablePos, _RCTablePos);              ///< P-Frame
	  _RCTable.WriteItem(1, _RCTablePos, (int)(0.5 + distortion));  ///< Dmax
	  _RCTable.WriteItem(2, _RCTablePos, _rate);                    ///< Rate
    _RCTable.WriteItem(3, _RCTablePos, _predRate);                ///< Pred Rate
    _RCTable.WriteItem(4, _RCTablePos, _MD/distortion);           ///< MAD/Dmax
	  _RCTable.WriteItem(5, _RCTablePos, _coeffRate);               ///< Coeff Rate
	  _RCTable.WriteItem(6, _RCTablePos, _predCoeffRate);           ///< Pred Coeff Rate
	  _RCTable.WriteItem(7, _RCTablePos, _headerRate);              ///< Header Rate
	  _RCTable.WriteItem(8, _RCTablePos, _predHeaderRate);          ///< Pred Header Rate
    _RCTable.WriteItem(9, _RCTablePos, _MD);                      ///< MAD
    _RCTable.WriteItem(10, _RCTablePos, _predMD);                 ///< Pred MAD
    _RCTable.WriteItem(11, _RCTablePos, _a);                      ///< Model parameter a
	  _RCTable.WriteItem(12, _RCTablePos, _b);                      ///< Model parameter b
    _RCTable.WriteItem(13, _RCTablePos, (int)_outOfBoundsFlag);   ///< OutOfBounds flag
    _RCTable.WriteItem(14, _RCTablePos, _choice);                 ///< Parameter choice
    _RCTable.WriteItem(15, _RCTablePos, _modelFit);               ///< Model Fit

	  _RCTablePos++;
  }//end if _RCTablePos... 
#endif

   ///-------------------- Store all new measurements -------------------------------------------------------
  _buffRateSamples.AddFirstIn(rate);
  _ratePredError.AddFirstIn(rate - _predRate);

  /// Range check before storing MAD data samples.
  double sigma = mae;
  if (sigma < 1.0) sigma = 1.0;
  /// Add y as a time series sample point.
  _meanDiffModel.PutSamples(sigma, initialisationRequired);

  /// Store header rate as differnce between buffer rate and coeff rate.
  _headerRateModel.PutSamples(rate - coeffrate, initialisationRequired);

   /// Store all new d-rate model data samples.
  PutDistortion(distortion, initialisationRequired);  ///< Store 1/distortion
  PutCoeffRate(coeffrate, initialisationRequired);    ///< Internally uses _meanDiffModel, store coeffrate/MAD

  if (!initialisationRequired) ///< Update the running sums associated with the model. 
    ModelUpdate();
  else  ///< Indicates that there was no valid data in the model before storing these first samples.
  {
    /// Fill the model with dummy data to match non-linear _a and _b parameters. Faster model convergence after initialisation.
    /// 20% either side of intial rate value.
    double stepSize = (0.4*coeffrate) / (double)_numOfFrames;
    double r = 0.8*coeffrate;
    for (int i = 0; i < (_numOfFrames - 1); i++, r += stepSize)
    {
      PutCoeffRate(r, false);
      PutDistortion(ModelDistortion(r, _a, _b, sigma), false);
    }//end for i...
    /// Restore item 0 in the fifo buffers.
    PutDistortion(distortion, false);
    PutCoeffRate(coeffrate, false);

    /// Initialise the fifo buffers for the R-D model, Mean Diff and Model Diff prediction models.
    ModelInitialise();
  }//end else...

  /// Performance measure as fitness to model.
//  _modelFit = RSqr(_a, _b);  ///< Statistical R^2 model fit.
//  _modelFit = GetModelMSE(_a, _b);  ///< MSE model fit.
  _modelFit = _ratePredError.GetSum() / (double)_ratePredError.GetLength(); ///< Mean rate prediction error closeness to zero.

}//end StoreMeasurements.

///---------------------- Pre-Encoding Predictions --------------------------

/** Predict the distortion for the next frame from the desired average rate.
The distortion measure is predicted from the coeff rate (total rate - header rate) using
an appropriate R-D model. The header rate and the signal mean diff are predicted from the
previous measured frame data using linear extrapolation.
@param  targetAvgRate : Total targeted average rate (including headers).
@param  rateLimit     : Upper limit to predicted rate.
@return               : Predicted square error distortion.
*/
int RateControlImplQuad::PredictDistortion(double targetAvgRate, double rateLimit)
{
  _outOfBoundsFlag = false;
  _upperDistortionOverflowFlag = false;
  _lowerDistortionOverflowFlag = false;
  _upperRateOverflowFlag = false;
  _lowerRateOverflowFlag = false;

  /// Default settings for the non-valid data case.
  double targetRate           = targetAvgRate;
  double targetCoeffRate      = targetAvgRate;
  double targetCoeffRateLimit = rateLimit;
  double predHeaderRate       = 0.02;

  ///------------------ Rate Prediction --------------------------------------
  if(ValidData())
  {
    /// Buffer averaging model: What must the target rate for the
    /// next N frames be to make the average over the rate buffer equal
    /// to the average target rate specified in the parameter?

    /// N past frames and recover in k frames: target rate = target avg rate*(N+k)/k - total buff/k.
    int numRecoverFrames = _numOfFrames;
    targetRate = ((targetAvgRate * (double)(_numOfFrames + numRecoverFrames))/(double)numRecoverFrames) - (_buffRateSamples.GetSum() /(double)numRecoverFrames);

    /// Check that the result is reasonable. If not then force it to some arbitrary minimum.
    if(targetRate < 0.0)  
      targetRate = _headerRateModel.GetYSample(0) + _rateLower;

    /// Predict the header rate with linear extrapolation model.
    predHeaderRate = (_p1*(double)(_numOfFrames+1)) + _p2;
     /// Check that the result is reasonable. If not, use the last entry.
    if( (predHeaderRate > targetAvgRate) || (predHeaderRate < 0.0) ) 
      predHeaderRate = _headerRateModel.GetYSample(0);

    /// Remove the pred header rate to predict the coeff rate.
    targetCoeffRate = targetRate - predHeaderRate;
    targetCoeffRateLimit = rateLimit - predHeaderRate;

  }//end if ValidData...
  else  ///< There is no valid data in the fifo buffers yet.
  {
    if(targetAvgRate > 0.02) ///< Guess header rate at 0.02 bpp.
      targetCoeffRate = targetAvgRate - 0.02;
  }//end else...

  /// Keep the target rate within the upper rate limit.
  if(targetCoeffRate > targetCoeffRateLimit)
    targetCoeffRate = targetCoeffRateLimit;

  /// No less than the coeff floor rate.
  if(targetCoeffRate < 0.00)
    targetCoeffRate = _rateLower;
  _prevCoeffRatePred = targetCoeffRate;

  ///--------------- Distortion Prediction -----------------------------------------

  /// Predict Mean Diff with linear extrapolation model. Override
  /// the prediction if a scene change was signalled externally.
  double predMD = ModelMeanDifference();

  /// Make prediction.
  double distd = 0.0;
  double core = (_a*_a) + (4.0*_b*targetCoeffRate/predMD);
  if(core >= 0.0)
    distd = ModelDistortion(targetCoeffRate, _a, _b, predMD);
  else
    distd = (predMD*_a)/targetCoeffRate;
///////////////////// Test code
  _choice = 0;
  if (core < 0.0)
    _choice = 1;
////////////////////

  /// Check bounds on distortion.
  if(distd > _distUpper)
  {
    distd = _distUpper;
    _outOfBoundsFlag = true;
	_upperDistortionOverflowFlag = true;
  }//end if distd...
  /// Cannot be less than 16x16x(2^2)
  if(distd < _distLower)
  {
    distd = _distLower;
    _outOfBoundsFlag = true;
	_lowerDistortionOverflowFlag = true;
  }//end if distd...

  /// Update the target coeff rate with changes in Dmax boundaries.
  if(_outOfBoundsFlag)
  {
    targetCoeffRate = ModelRate(distd, _a, _b, predMD);
    _prevCoeffRatePred = targetCoeffRate;
  }//end if _outOfBoundsFlag...

  /// Store the most recent rate requirement.
  _predRate = targetRate;

#ifdef RCIQ_DUMP_RATECNTL
  _predHeaderRate   = predHeaderRate;
  _predCoeffRate    = targetCoeffRate;
  _predMD           = predMD;
  _predDmax         = distd;
#endif

  /// Convert absolute distortion to square distortion.
  //return((int)((distd*distd) + 0.5));
  return((int)(distd + 0.5));
}//end PredictDistortion.

 /*
 ---------------------------------------------------------------------------
 Public Interface Methods.
 ---------------------------------------------------------------------------
 */

/** Predict the mean difference between pels in the frame.
Predict the frame Mean Diff with linear extrapolation model. Override the prediction 
if a scene change was signalled externally and reset the signal to zero.
@return : Predicted mean difference.
*/
double RateControlImplQuad::ModelMeanDifference(void)
{
  double predMD;
  if (_mseSignal == 0.0)
  {
    /// Predict with linear extrapolation model.
    predMD = (_a1*(double)(_numOfFrames + 1)) + _a2;
    /// Can never be negative. Use last value if prediction is negative.
    if (predMD < 0.0)
      predMD = _meanDiffModel.GetYSample(0); ///< Mean diff samples are in reverse order.
  }//end if _mseSignal...
  else
  {
    predMD = _mseSignal;
    _mseSignal = 0.0;  ///< Clear the signal.
  }//end else...

  return(predMD);
}//end ModelMeanDifference.

/** Initialise all R-D model buffers and persistant member variables.
Initialise the fifo buffers for the R-D model, Mean Diff and Model prediction members. The
fifo buffers must have some valid data in them and the model parameters must be reset prior
to calling this method. The values that are updated on every StoreMeasurement() call are
initialised here.
@return : None.
*/
void RateControlImplQuad::ModelInitialise(void)
{
  /// Initialise the fifo buffers for the R-D model, Mean Diff and Model Diff prediction models.
  _Sx = 0;
  _Sy = 0;
  _Sxy = 0;    ///< Quadratic R-D.
  _Sx2 = 0;

  for (int i = 0; i < _numOfFrames; i++)
  {
    ///< Quadratic R-D.
    double x = (_distortionSamples.GetBuffer())[i]; ///< 1.0/distortion
    double y = (_coeffRateSamples.GetBuffer())[i];  ///< coeffrate/MAD

    _Sx += x;
    _Sy += y;
    _Sxy += y / x;
    _Sx2 += (x*x);
  }//end for i...

}//end ModelInitialise. 

/** Update all R-D model parameters.
Update the model parameters for the R-D model ((md/Dmax), r), Mean Diff and the running
sums. The latest data must have been loaded into the fifo buffers prior to calling this
method. The values are updated on every StoreMeasurement() call.
@return : None.
*/
void RateControlImplQuad::ModelUpdate(void)
{
  /// Update ((1/Dmax), r/md) running sums.
  double x = (_distortionSamples.GetBuffer())[0]; ///< 1/distortion
  double y = (_coeffRateSamples.GetBuffer())[0];  ///< coeffrate/MAD

  double x2 = x*x;
  double px2 = _prevDistortion*_prevDistortion;
  _Sx = UpdateRunningSum(_prevDistortion, x, _Sx);
  _Sy = UpdateRunningSum(_prevCoeffRate, y, _Sy);
  _Sxy = UpdateRunningSum(_prevCoeffRate / _prevDistortion, y / x, _Sxy);  ///< Inverse of distortion is required here.
  _Sx2 = UpdateRunningSum(px2, x2, _Sx2);

  /// Solve for a and b with least square error over the buffer frames.
  double aa, bb;
  GetModel(&aa, &bb);
  //GetModel(&aa, &bb, 1.0/_distortionSamples.GetItem(0), _coeffRateSamples.GetItem(0));

  /// Identify a list of sample positions that exclude outliers based on the difference between the
  /// predicted rate and actual rate.
  //int nonOutlierList[16];
  //int listLen = GetNonOutliers(nonOutlierList);
  //GetModel(&aa, &bb, nonOutlierList, listLen);  ///< Outliers removed.

  /// TODO: Check the validity of aa and bb before assigning.

  _a = aa;
  _b = bb;

  /// Update the mean diff long term model parameters
  _meanDiffModel.GetModelLin(&_a1, &_a2);

  /// Update the header rate model parameters.
  _headerRateModel.GetModelLin(&_p1, &_p2);

}//end ModelUpdate.

/** Get the model parameters passing through point (x0,y0).
@return : None.
*/
void RateControlImplQuad::GetModel(double* a, double* b, double x0, double y0)
{
  double Sxy = 0.0;
  double Sxx = 0.0;
  double Sx2 = 0.0;
  for (int i = 0; i < _numOfFrames; i++)
  {
    double x = _distortionSamples.GetItem(i); ///< 1/distortion
    double x2 = x*x;
    double y = _coeffRateSamples.GetItem(i);  ///< r/mad
    double t = x - (x0*x2);
    /// Accumulate
    Sxy += y*t;
    Sxx += x2*t;
    Sx2 += t*t;
  }//end for i...

  *a = (Sxy - (y0*x0*x0*Sxx)) / Sx2;
  *b = x0*((y0*x0) - (*a));
}//end GetModel.

/** Get the model parameters from a subset of sample points.
@param  a             : Reference to parameter a returned
@param  b             : Reference to parameter b returned
@param  inclusionlist : Positions in fifo to include
@param  listlen       : Num of items in the list
@return               : None.
*/
void RateControlImplQuad::GetModel(double* a, double* b, int* inclusionlist, int listlen)
{
  double Sx = 0;
  double Sy = 0;
  double Sxy = 0;
  double Sx2 = 0;

  for (int i = 0; i < listlen; i++)
  {
    ///< Quadratic R-D.
    double x = _distortionSamples.GetItem(inclusionlist[i]); ///< 1.0/distortion
    double y = _coeffRateSamples.GetItem(inclusionlist[i]);  ///< coeffrate/MAD
    Sx += x;
    Sy += y;
    Sxy += y / x;
    Sx2 += (x*x);
  }//end for i...

  *b = (((double)listlen*Sy) - (Sxy*Sx)) / (((double)listlen*Sx2) - (Sx*Sx));
  *a = (Sxy - ((*b)*Sx)) / (double)listlen;
}//end GetModel.


/*
 ---------------------------------------------------------------------------
 Private Methods.
 ---------------------------------------------------------------------------
*/
/** Calculate R^2 for this inverse quadratic model.
Determine the R^2 statistical model fit within the array of rate samples. Note
that the most recent samples are at location samples[0].
@param  a  : r = a(mad/d) + b(mad/d)^2 + rheader.
@param  b  : 
@return    : R^2.
*/
double RateControlImplQuad::RSqr(double a, double b)
{
  double* samples     = _buffRateSamples.GetBuffer();
  double* distortion  = _distortionSamples.GetBuffer();
  double* header      = _headerRateModel.GetYFifo()->GetBuffer();
  double* mad         = _meanDiffModel.GetYFifo()->GetBuffer();

  double mean = 0.0;
  for (int i = 0; i < _numOfFrames; i++)
    mean += samples[i];
  mean = mean / (double)_numOfFrames;

  double SStot = 0.0;
  double SSres = 0.0;
  for (int i = 0; i < _numOfFrames; i++)
  {
    double r = samples[i];
    double d = distortion[i];
    double m = mad[i];

    SStot += ((r - mean)*(r - mean));
    double rm = m*((a*d) + (b*d*d)) + header[i];
    SSres += ((r - rm)*(r - rm));
  }//end for i...

  double R2 = 1.0;
  if (SStot > 0.0)
    R2 = 1.0 - (SSres / SStot);

  return(R2);
}//end RSqr.

/** Calculate MSE for this inverse quadratic model.
Determine the MSE statistical model fit within the array of rate samples. Note
that the most recent samples are at location samples[0].
@param  a  : r = a(mad/d) + b(mad/d)^2 + rheader.
@param  b  :
@return    : mse.
*/
double RateControlImplQuad::GetModelMSE(double a, double b)
{
  double mse = 0.0;
  double* samples     = _buffRateSamples.GetBuffer();
  double* distortion  = _distortionSamples.GetBuffer();
  double* header      = _headerRateModel.GetYFifo()->GetBuffer();
  double* mad         = _meanDiffModel.GetYFifo()->GetBuffer();

  for (int i = 0; i < _numOfFrames; i++)
  { 
    double r = samples[i];
    double d = distortion[i];
    double m = mad[i];

    double rm = m*((a*d) + (b*d*d)) + header[i];
    mse += (r - rm)*(r - rm); 
  }//end for i...
  return(mse/(double)_numOfFrames);
}//end GetModelMSE.

 /** Get the locations of the non-outliers and the length of the list.
 It is assumed that the list length is sufficient at least the length
 of the _ratePredError fifo from which this method operates on.
 @param  list  : List buffer
 @return       : list length
 */
int RateControlImplQuad::GetNonOutliers(int* list)
{
  int i;
  int samplesLen = _ratePredError.GetLength();
  //int listLen     = samplesLen;
  int listLen = 0;
  double* samples = _ratePredError.GetBuffer();
  double stddev = 1.0 * StdDevRateErr();

  /// Add the pos of those samples that are within 1 std dev to the list.
  for (i = 0; i < samplesLen; i++)
  {
    if (abs(samples[i]) <= stddev) ///< Within 1 std deviations.
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

  int maxOutliers = 1;
  int room = samplesLen - i;

  if (room >= maxOutliers) listLen = samplesLen - maxOutliers;
  else listLen = samplesLen - room;
*/  
  return(listLen);
}//end GetNonOutliers.

