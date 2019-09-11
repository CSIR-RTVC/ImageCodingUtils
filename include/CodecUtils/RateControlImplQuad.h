/** @file

MODULE				: RateControlImplQuad

TAG						: RCIQ

FILE NAME			: RateControlImplQuad.h

DESCRIPTION		: A class to hold an inverse quadratic model for the frame buffer rate rate control to
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
#ifndef _RATECONTROLIMPLQUAD_H
#define _RATECONTROLIMPLQUAD_H

#pragma once

#include  <math.h>
#include  "IRateControl.h"
#include  "LinearModel.h"
#include  "MeasurementTable.h"

//#define RCIQ_DUMP_RATECNTL  1
#define RCIQ_DUMP_FILENAME "C:/Users/KFerguson/Documents/Excel/VideoEvaluation/RateControlQuad_"	///< File name head only

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RateControlImplQuad : public IRateControl
{
public:
  RateControlImplQuad(void) 
  { _a = -24.0; _b = 20000.0; _numOfFrames = 0; 
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 16777216.0; _distLower = 1024.0; 
  }
  RateControlImplQuad(double modelparam_a, double modelparam_b) 
  { _a = modelparam_a; _b = modelparam_b; _numOfFrames = 0;  
    _rateUpper = 24.0; _rateLower = 0.0001; _distUpper = 16777216.0; _distLower = 1024.0;
  }
  virtual ~RateControlImplQuad() { Destroy(); }

/// IRateControl Interface methods.
public:
  int	  Create(int numOfFrames);
  void	Destroy(void);
  int   GetFameBufferLength(void) { return(_numOfFrames); }
  void  SetRDLimits(double rateUpper, double rateLower, double distUpper, double distLower) 
    { _rateUpper = rateUpper; _rateLower = rateLower; _distUpper = distUpper; _distLower = distLower; }

  /// Pre-encoding: Apply the rate control model to predict the distortion from the target coeff rate.
  int PredictDistortion(double targetAvgRate, double rateLimit);
  /// Post-encoding: Add samples to the fifo buffers and hold the sample that is discarded.
  void StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae);
  /// Return average rate across the buffer as the performance measure.
  double GetPerformanceMeasure(void) { return(_modelFit); }

  /// Is the data in the fifos valid.
  int ValidData(void) 
    { if(!_buffRateSamples.Empty() && !_meanDiffModel.Empty() && !_headerRateModel.Empty() &&
         !_distortionSamples.Empty() && !_coeffRateSamples.Empty() && !_ratePredError.Empty()) return(1); return(0); }
  /// Was the prediction calculation out of bounds?
  bool OutOfBounds(void) { return(_outOfBoundsFlag); }
  /// Who is the culprit?
  bool UpperDistortionOverflow(void) { return(_upperDistortionOverflowFlag); }
  bool LowerDistortionOverflow(void) { return(_lowerDistortionOverflowFlag); }
  bool UpperRateOverflow(void) { return(_upperRateOverflowFlag); }
  bool LowerRateOverflow(void) { return(_lowerRateOverflowFlag); }
  /// Reset the fifos to the last valid sample.
  void Reset(void);

  /// Utility functions.
  void Dump(const char* filename);  ///< Requires RCIQ_DUMP_RATECNTL to be set.
  void Dump(void);                  ///< Requires RCIQ_DUMP_FILENAME to be defined.

/// Further implementation specific public methods.
public:

/// Private methods.
protected:
  /// These following group of methods must be called in this exact order because they operate
  /// on members that are interdependant.
  inline void PutDistortion(double distortion, bool initialise) ///< Must only be called after a call to PutMeanDiff(). 
    { if(distortion < 1.0) distortion = 1.0;
      PutLinearMeasure(_distortionSamples, (1.0 /distortion), &_prevDistortion, initialise); }
  inline void PutCoeffRate(double rate, bool initialise)
    { PutLinearMeasure(_coeffRateSamples, (rate/ _meanDiffModel.GetYSample(0)), &_prevCoeffRate, initialise); }
  inline void PutLinearMeasure(Fifo& samples, double m, double* prev, bool initialise)
    { if (!initialise) *prev = samples.GetFirstOut(); else *prev = m; samples.AddFirstIn(m); }
  /// For running sums of a buffer. Adding the new value and dropping the oldest previous value.
  inline double UpdateRunningSum(double prev, double curr, double currSum) { double newSum = currSum - prev; newSum += curr; return(newSum); }

  /// Model equations.
//  double ModelDistortion(double rate, double a, double b, double var) { return( (2.0*var*b)/(sqrt((a*a) + (4.0*b*rate)) - a) ); }
//  double ModelRate(double distortion, double a, double b, double var) { return( ((a*var)/distortion) + ((b*var*var)/(distortion*distortion)) ); }
  double ModelDistortion(double rate, double a, double b, double var) { return((2.0*b) / (sqrt((a*a) + (4.0*b*rate/var)) - a)); }
  double ModelRate(double distortion, double a, double b, double var) { return(((a*var) / distortion) + ((b*var) / (distortion*distortion))); }
  double ModelMeanDifference(void);
  void   ModelInitialise(void);
  void   ModelUpdate(void);

  virtual void  GetModel(double* a, double* b) 
    { *b = (((double)_numOfFrames*_Sy) - (_Sxy*_Sx)) / (((double)_numOfFrames*_Sx2) - (_Sx*_Sx));
      *a = (_Sxy - ((*b)*_Sx)) / (double)_numOfFrames; }
  virtual void  GetModel(double* a, double* b, double x0, double y0);
  virtual void  GetModel(double* a, double* b, int* inclusionlist, int listlen);
  double        RSqr(double a, double b);          ///< Model fit R^2
  double        GetModelMSE(double a, double b);   ///< Model fit MSE
  double        StdDevRateErr(void)
  { double std = 0.0;  for (int i = 0; i < _numOfFrames; i++) std += (_ratePredError.GetItem(i)*_ratePredError.GetItem(i));
    return(sqrt(std / (double)_numOfFrames)); }
  int           GetNonOutliers(int* list);  ///< Returns the locations of the non-outliers and the length of the list.

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
  LinearModel _meanDiffModel;       ///< Mean difference between samples within picture frames.
  LinearModel _headerRateModel;     ///< Header rate for each frame.
  Fifo        _buffRateSamples;     ///< Stream rate including headers of frames.
  Fifo        _ratePredError;       ///< Performance measure of the prediction.

  Fifo      _coeffRateSamples;     ///< Coeff rate of frames.
  double    _prevCoeffRate;
  Fifo      _distortionSamples;    ///< (mad/Dmax) for the frames.
  double    _prevDistortion;
  double    _prevCoeffRatePred;

  /// Signal when the predicted values are not valid due to out of range errors. Use
  /// this signal to prevent contributions to the model data.
  bool      _outOfBoundsFlag;
  bool      _upperDistortionOverflowFlag;
  bool      _lowerDistortionOverflowFlag;
  bool      _upperRateOverflowFlag;
  bool      _lowerRateOverflowFlag;

  /// For the quadratic model x = 1/Dmax and y = R/md. To update the constants
  /// a and b, quadratic roots are used to solve the linear equation for a least
  /// squares criterion curve fit. Running sums are used.
  double    _Sxy; ///< Sum(r.Dmax/md) Note the inverse of x here.
  double    _Sx2; ///< Sum(md^2/Dmax^2)
  double    _Sx;  ///< Sum(md/Dmax)
  double    _Sy;  ///< Sum(r)
  double    _a;   ///< Quadratic model "constants".
  double    _b;

  /// The prediction of Mean Abs Diff requires a least squares criterion curve fit
  /// to a linear extrapolation model with constants a1 and a2.
  double    _a1;
  double    _a2;

  /// The prediction of frame header rate requires a least mean squares criterion curve fit
  /// to a linear extrapolation model with constants p1 and p2.
  double    _p1;
  double    _p2;

  /// Measure the performance by the model accuracy.
  double    _modelFit;
  int       _choice;
  double    _predRate;

  /// Signalling parameters.
  double    _mseSignal;

  MeasurementTable  _RCTable;
  int               _RCTableLen;
  int               _RCTablePos;
#ifdef RCIQ_DUMP_RATECNTL
  double            _predMD;
  double            _MD;
  double            _predHeaderRate;
  double            _headerRate;
  double            _predDmax;
  double            _predCoeffRate;
  double            _coeffRate;
  double            _rate;
#endif
};// end class RateControlImplQuad.

#endif	// _RATECONTROLIMPLQUAD_H
