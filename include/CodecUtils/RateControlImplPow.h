/** @file

MODULE				: RateControlImplPow

TAG						: RCIP

FILE NAME			: RateControlImplPow.h

DESCRIPTION		: A class to hold a power function  model for the frame buffer rate rate control to
                match an average rate stream.

COPYRIGHT			: (c)CSIR 2007-2017 all rights resevered

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
#ifndef _RATECONTROLIMPLPOW_H
#define _RATECONTROLIMPLPOW_H

#pragma once

#include  <math.h>
#include  "Fifo.h"
#include  "IRateControl.h"
#include  "LinearModel.h"
#include  "MeasurementTable.h"

//#define RCIP_DUMP_RATECNTL  1
#define RCIP_DUMP_FILENAME "c:/keithf/Excel/VideoEvaluation/RateControlPow_" ///< File name head only

/*
--------------------------------------------------------------------------------------
	Class definition: 
  Model implementation: r = b*(d/var)^a  linearised to: ln(r) = a*ln(d/var) + ln(b)
--------------------------------------------------------------------------------------
*/
class RateControlImplPow : public IRateControl
{
public:
  RateControlImplPow(void) 
  { _a = -1.3; _b = 300.0; _numOfFrames = 0;
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 67108864.0; _distLower = 256.0;
  }
  RateControlImplPow(double modelparam_a, double modelparam_b) 
  { _a = modelparam_a; _b = modelparam_b; _numOfFrames = 0; 
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 67108864.0; _distLower = 256.0;
  }
  virtual ~RateControlImplPow() { Destroy(); }

/// IRateControl Interface methods.
public:
  int		Create(int numOfFrames);
  void	Destroy(void);
  int   GetFameBufferLength(void) { return(_numOfFrames); }
  void  SetRDLimits(double rateUpper, double rateLower, double distUpper, double distLower) 
        { _rateUpper = rateUpper; _rateLower = rateLower; _distUpper = distUpper; _distLower = distLower; }

  /// Pre-encoding: Apply the rate control model to predict the distortion from the target coeff rate.
  int PredictDistortion(double targetAvgRate, double rateLimit);
  /// Post-encoding: Add samples to the fifo buffers and hold the sample that is discarded.
  void StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae);
  /// Return model mismatch as the performance measure.
  double GetPerformanceMeasure(void) { return(_modelFit); }

  /// Is the data in the fifos and sample models valid.
  int ValidData(void) 
  { if(!_powDRModel.Empty() && !_meanDiffModel.Empty() && !_meanDiffShortModel.Empty() && !_buffRateSamples.Empty() && !_ratePredError.Empty()) return(1); return(0); }

  /// Was the prediction calculation out of bounds?
  bool OutOfBounds(void) { return(_outOfBoundsFlag); }
  /// Who is the culprit?
  bool UpperDistortionOverflow(void)  { return(_upperDistortionOverflowFlag); }
  bool LowerDistortionOverflow(void)  { return(_lowerDistortionOverflowFlag); }
  bool UpperRateOverflow(void)        { return(_upperRateOverflowFlag); }
  bool LowerRateOverflow(void)        { return(_lowerRateOverflowFlag); }
  /// Reset the fifos. Use after a change in frame type.
  void Reset(void) 
  { _ratePredError.MarkAsEmpty(); _powDRModel.MarkAsEmpty();
    _meanDiffModel.MarkAsEmpty(); _meanDiffShortModel.MarkAsEmpty(); _buffRateSamples.MarkAsEmpty(); }

  /// In this implementation only the mean sqr err is used to indicate the degree of scene change. The signal is immediately reset to zero after use.
  void SignalSceneChange(double mse, double mae) { _mseSignal = mse; }

  /// Utility functions.
  void Dump(const char* filename);  ///< Requires RCIP_DUMP_RATECNTL to be set.
  void Dump(void);                  ///< Requires RCIP_DUMP_FILENAME to be defined.

/// Further implementation specific public methods.
public:

/// Private methods.
protected:
  /// Model equations.
  void   ModelUpdate(void);
  double ModelMeanDifference(void);
  double GetGradStep(double* slope, double* yintercept);

  double ModelDistortion(double rate, double a, double b, double var) { return(var*exp((log(rate / b)) / a)); }
  double ModelRate(double distortion, double a, double b, double var) { return(b*pow((distortion / var), a)); }
  bool   WithinModelBounds(double a, double b) { return((b > 1.0) && (a < 0.0)); }
  double SlopeLimit(void) { return(log(_rateLower / _rateUpper) / log(_distUpper / _meanDiffModel.GetYSample(0))); }
  double SlopeLimit(double b) { return(log(_rateLower / b) / log(_distUpper / _meanDiffModel.GetYSample(0))); }
  double RSqr(double a, double b);
  double GetModelMSE(double a, double b);
  double StdDevRateErr(void)
  { double std = 0.0;  for (int i = 0; i < _numOfFrames; i++) std += (_ratePredError.GetItem(i)*_ratePredError.GetItem(i));
    return(sqrt(std / (double)_numOfFrames)); }
  int    GetNonOutliers(int* list);  ///< Returns the locations of the non-outliers and the length of the list.

/// Constant members.
public:

/// Common members.
protected:
	int				_numOfFrames;
  /// Clip the values of the rate and distortion to these limits.
  double    _rateUpper;
  double    _rateLower;
  double    _distUpper;
  double    _distLower;

  /// Model definitions.
  LinearModel _powDRModel;          ///< Hold the liniarized ln(r) = a.ln(d/md) + ln(b) model sample data.
  LinearModel _meanDiffModel;       ///< Mean difference between samples within picture frames.
  LinearModel _meanDiffShortModel;  ///< Short term mean difference samples.

  Fifo      _buffRateSamples;     ///< Stream rate including headers of encoded frames.
  /// Signal when the predicted values are not valid due to out of range errors. Use
  /// this signal to prevent contributions to the model data.
  bool      _outOfBoundsFlag;
  bool      _upperDistortionOverflowFlag;
  bool      _lowerDistortionOverflowFlag;
  bool      _upperRateOverflowFlag;
  bool      _lowerRateOverflowFlag;

  /// To update the constants, _a and _b, Cramer's Rule is used to solve the linear equation 
  /// for a least mean squares criterion curve fit.
  double    _a; ///< Power model constants for r = b*(d/var)^a or ln(r) = a*ln(d/var) + ln(b)
  double    _b;

  /// The prediction of Mean Diff requires a least mean squares criterion curve fit
  /// to a linear extrapolation model with constants a1 and a2.
  double    _a1;  ///< Linear model constants.
  double    _a2;
  double    _R2;    ///< Statistical R^2 value to compare performance of long term vs short term fit.
  /// Short term samples. No. of samples is fixed to the most recent in the mean diff sample fifo.
  double    _a1_s;
  double    _a2_s;
  double    _R2_s;

  /// Measure the performance by the model accuracy.
  double    _modelFit;
  int       _choice;
  double    _predRate;
  Fifo      _ratePredError; ///< Diff between actual rate and predicted target rate.

  /// Signalling parameters.
  double    _mseSignal;

  MeasurementTable  _RCTable;
  int               _RCTableLen;
  int               _RCTablePos;
#ifdef RCIP_DUMP_RATECNTL
  double            _predMD;
  double            _MD;
  double            _predDmax;
  double            _rate;
#endif
};// end class RateControlImplPow.

#endif	// _RATECONTROLIMPLPOW_H
