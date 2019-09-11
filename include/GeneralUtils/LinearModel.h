/** @file

MODULE				: LinearModel

TAG						: LM

FILE NAME			: LinearModel.h

DESCRIPTION		: A class to hold and operate on a linear model of 2 variable functions that 
                are constituted from N number of (xi,yi) samples that are fitted to a 
                y = ax + b linear model.

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
#ifndef _LINEARMODEL_H
#define _LINEARMODEL_H

#pragma once

#include "Fifo.h"
#include "MeasurementTable.h"

//#define LM_DUMP_MODEL  1

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class LinearModel
{
public:
  LinearModel(void)       { _numSamples = 0; _type = xyType; }
  virtual ~LinearModel()  { Destroy(); }

/// Setup and interface methods.
public:
  int		Create(int numSamples, int type);
  void	Destroy(void);

  /// Add a pair of samples or scalar sample to the fifos and update the running sums or initialise for linear functions y = ax + b.
  virtual void    PutSamples(double x, double y, bool initialise);
  virtual void    PutSamples(double y, bool initialise) { PutSamples(0.0, y, initialise); } ///< For time series linear models.

  /// Get model parameters based on the data samples in the fifos.
  virtual double  GetModelLin(double* slope, double* yintercept)  ///< GetModel() and GetObjFunc()
    { CramersRuleSoln(_Sx2, _Sx, _Sx, (double)_numSamples, _Sxy, _Sy, slope, yintercept); return(GetObjFunc(*slope, *yintercept)); }
  virtual double  GetModelLin(double* slope, double* yintercept, double x0, double y0);  ///< Get Model that passes through point (x0,y0) and GetObjFunc()
  virtual void    GetModel(double* slope, double* yintercept) { CramersRuleSoln(_Sx2, _Sx, _Sx, (double)_numSamples, _Sxy, _Sy, slope, yintercept); }
  virtual double  GetObjFunc(double slope, double yintercept)
  { double objfunc = 0.0; 
    for (int i = 0; i < _numSamples; i++) { double tmp = (_ySamples.GetBuffer())[i] - (slope*(_xSamples.GetBuffer())[i]) - yintercept; objfunc += tmp*tmp; }
    return(objfunc); }

  /// Alternative get model parameters based on an inclusion list used for removing outliers.
  virtual double  GetModelLin(double* slope, double* yintercept, int* inclusionlist, int listlen);
  virtual double  GetObjFunc(double slope, double yintercept, int* inclusionlist, int listlen)
  { double objfunc = 0.0;
    for (int i = 0; i < listlen; i++) { double tmp = (_ySamples.GetBuffer())[inclusionlist[i]] - (slope*(_xSamples.GetBuffer())[inclusionlist[i]]) - yintercept; objfunc += tmp*tmp; }
    return(objfunc); }

  virtual void    MarkAsEmpty(void) { _xSamples.MarkAsEmpty(); _ySamples.MarkAsEmpty(); }

  int     GetNumSamples(void) { return(_numSamples); }
  double  GetXSample(int t)   { return( (_xSamples.GetBuffer())[t] ); }
  double  GetYSample(int t)   { return( (_ySamples.GetBuffer())[t] ); }
  Fifo*   GetXFifo(void)      { return(&_xSamples); }
  Fifo*   GetYFifo(void)      { return(&_ySamples); }
  bool    Empty(void)         { return(_xSamples.Empty() || _ySamples.Empty()); }

  double  GetSumX(void)       { return(_Sx); }
  double  GetSumY(void)       { return(_Sy); }
  double  GetSumX2(void)      { return(_Sx2); }
  double  GetSumXY(void)      { return(_Sxy); }

  /// Utility functions.
  void Dump(const char* filename);  ///< Requires LM_DUMP_MODEL to be set.
  /// R^2 statistical fit function on y samples.
  double RSqr(double slope, double yintercept);

/// Private Methods.
protected:
  /// For running sums of a buffer. Adding the new value and dropping the oldest previous value.
  double UpdateRunningSum(double prev, double curr, double currSum) { double newSum = currSum - prev; newSum += curr; return(newSum); }

  /// Solve in 2 variables returning soln in (x0, x1). If the determinant = 0 then leave (x0, x1) unchanged.
  void CramersRuleSoln(double a00, double a01, double a10, double a11, double b0, double b1, double* x0, double* x1)
  { double D = (a00 * a11) - (a10 * a01);
    if (D != 0.0) { *x0 = ((b0 * a11) - (b1 * a01)) / D; *x1 = ((a00 * b1) - (a10 * b0)) / D; } }

  /// Add to a fifo and store value that drops off the end of the fifo.
  void Put(Fifo& samples, double m, double* prev, bool initialise)
  { if (!initialise) *prev = samples.GetFirstOut(); else *prev = m; samples.AddFirstIn(m); }

/// Public constants
public:
  /// Linear model types.
  static const int xyType         = 0;  ///< Standard 2 coordinate (x,y) sample sets
  static const int timeSeriesType = 1;  ///< Fixed time series samples with one coordinate y for (t,y)

/// Common members.
protected:
  int     _type;        ///< Linear model type defined by constants.
  int     _numSamples;  ///< Length of fifos.

  Fifo    _xSamples;
  double  _prevXSample; ///< The sample value that drops off the end of the fifo.
  Fifo    _ySamples;
  double  _prevYSample;

  /// To update the constants, _a and _b, Cramer's Rule is used to solve the linear equation 
  /// for a least mean squares criterion curve fit. Running sums are used.
  double    _Sxy;
  double    _Sx2;
  double    _Sx;
  double    _Sy;

  MeasurementTable  _LMTable;
  int               _LMTableLen;
  int               _LMTablePos;
#ifdef LM_DUMP_MODEL
  double            _predMD;
  double            _MD;
  double            _predDmax;
  double            _predRate;
  double            _rate;
#endif
};// end class LinearModel.

#endif	// _LINEARMODEL_H
