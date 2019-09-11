/** @file

MODULE			  : RateControlImplLog

TAG				    : RCIL

FILE NAME		  : RateControlImplLog.h

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

===========================================================================
*/
#ifndef _RATECONTROLIMPLLOG_H
#define _RATECONTROLIMPLLOG_H

#pragma once

#include  <math.h>
#include  "IRateControl.h"
#include  "LinearModel.h"
#include  "MeasurementTable.h"

//#define RCIL_DUMP_RATECNTL  1
#define RCIL_DUMP_FILENAME "C:/Users/KFerguson/Documents/Excel/VideoEvaluation/RateControlLog_"	///< File name head only

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RateControlImplLog : public IRateControl
{
public:
  RateControlImplLog(void) 
  { _a = 0.2; _b = 1.4; _numOfFrames = 0; 
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 67108864.0; _distLower = 256.0;
  }
  RateControlImplLog(double modelparam_a, double modelparam_b) 
  { _a = modelparam_a; _b = modelparam_b; _numOfFrames = 0; 
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 67108864.0; _distLower = 256.0;
  }
  virtual ~RateControlImplLog() { Destroy(); }

/// IRateControl Interface methods.
public:
  int	  Create(int numOfFrames);
  void	Destroy(void);
  int   GetFameBufferLength(void) { return(_numOfFrames); }
  void  SetRDLimits(double rateUpper, double rateLower, double distUpper, double distLower) 
        { _rateUpper = rateUpper; _rateLower = rateLower; _distUpper = distUpper; _distLower = distLower; }

  /// Pre-encoding: Apply the rate control model to predict the distortion from the target coeff rate.
  int PredictDistortion(double targetAvgRate, double rateLimit);
  /// Post-encoding: Add sample data to the models.
  void StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae);
  /// Return average rate across the buffer as the performance measure.
  double GetPerformanceMeasure(void)  { return(_modelFit); }

  /// Is the data in the fifos valid.
  int ValidData(void) 
  { if(!_meanDiffShortModel.Empty() && !_meanDiffModel.Empty() && !_logDRModel.Empty() && !_buffRateSamples.Empty() && !_ratePredError.Empty()) return(1); return(0); }
  /// Was the prediction calculation out of bounds?
  bool OutOfBounds(void) { return(_outOfBoundsFlag); }
  /// Who is the culprit?
  bool UpperDistortionOverflow(void)  { return(_upperDistortionOverflowFlag); }
  bool LowerDistortionOverflow(void)  { return(_lowerDistortionOverflowFlag); }
  bool UpperRateOverflow(void)        { return(_upperRateOverflowFlag); }
  bool LowerRateOverflow(void)        { return(_lowerRateOverflowFlag); }
  /// Reset the fifos to empty.
  void Reset(void)
  { _ratePredError.MarkAsEmpty(); _meanDiffModel.MarkAsEmpty();
    _meanDiffShortModel.MarkAsEmpty(); _logDRModel.MarkAsEmpty(); _buffRateSamples.MarkAsEmpty(); }
  /// In this implementation only the mean abs err is used to indicate the degree of scene change. It is immediately reset to zero after use.
  void SignalSceneChange(double mse, double mae) { _mseSignal = mae; }  /// Not sure why I used mae and not mse???

  /// Utility functions.
  void Dump(const char* filename);  ///< Requires RCIL_DUMP_RATECNTL to be set.
  void Dump(void);                  ///< Requires RCIL_DUMP_FILENAME to be defined.

/// Further implementation specific public methods.
public:

/// Private methods.
protected:
  /// Model equations.
  double  ModelDistortion(double rate, double a, double b, double var) { return(var*exp((b - rate)/a)); }
  double  ModelRate(double distortion, double a, double b, double var) { return((a*log(var/distortion)) + b);}
  double  ModelMeanDifference(void);
  void    ModelUpdate(void);
  double  GetGradStep(double* slope, double* yintercept);
  double  RSqr(double a, double b);
  double  StdDevRateErr(void) 
  { double std = 0.0;  for (int i = 0; i < _numOfFrames; i++) std += (_ratePredError.GetItem(i)*_ratePredError.GetItem(i)); 
    return(sqrt(std/(double)_numOfFrames)); }
  int     GetNonOutliers(int* list);  ///< Returns the locations of the non-outliers and the length of the list.

/// Constant members.
public:

/// Common members.
protected:
	int		_numOfFrames;

  /// Clip the values of the rate and distortion to these limits.
  double    _rateUpper;
  double    _rateLower;
  double    _distUpper;
  double    _distLower;

  /// Model definitions.
  LinearModel _logDRModel;          ///< Hold the liniarized r = a.ln(md/dm) + b model sample data.
  LinearModel _meanDiffModel;       ///< Mean difference between samples within picture frames.
  LinearModel _meanDiffShortModel;  ///< Short term mean difference samples.
  Fifo        _buffRateSamples;     ///< Stream rate including headers of frames.

  /// Signal when the predicted values are not valid due to out of range errors. Use
  /// this signal to prevent contributions to the model data.
  bool      _outOfBoundsFlag;
  bool      _upperDistortionOverflowFlag;
  bool      _lowerDistortionOverflowFlag;
  bool      _upperRateOverflowFlag;
  bool      _lowerRateOverflowFlag;

  /// Model parameters for r = _a*ln(mse/d) + _b.
  double    _a; ///< Log model constants.
  double    _b;

  /// The prediction of Mean Diff requires a least mean squares criterion curve fit
  /// to a linear extrapolation model with constants a1 and a2. A long term and a
  /// short term model are used.
  double    _a1;    ///< Linear model constants.
  double    _a2;
  double    _R2;    ///< Statistical R^2 value to compare performance of long term vs short term fit.
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
#ifdef RCIL_DUMP_RATECNTL
  double            _predMD;
  double            _MD;
  double            _predDmax;
  double            _rate;
#endif
};// end class RateControlImplLog.

#endif	// _RATECONTROLIMPLLOG_H
