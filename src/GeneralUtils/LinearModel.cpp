/** @file

MODULE				: LinearModel

TAG						: LM

FILE NAME			: LinearModel.cpp

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
#include "LinearModel.h"

/*
---------------------------------------------------------------------------
Constants
---------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
int LinearModel::Create(int numSamples, int type)
{
  /// Clean up first.
  Destroy();

  /// Type is persistant.
  if((type <= 1)&&(type >= 0))  ///< Currently there are only 2 types of linear models.
    _type = type;

  if( (!_xSamples.Create(numSamples)) || (!_ySamples.Create(numSamples)) ) 
    return(0);

  _numSamples = numSamples;
  _xSamples.Clear();
  _ySamples.Clear();

  _prevXSample  = 0.0;
  _prevYSample  = 0.0;
  /// Running sums for Cramer's Rule solutions.
  _Sx   = 0.0;
  _Sy   = 0.0;
  _Sxy  = 0.0;
  _Sx2  = 0.0;

  _LMTableLen = 0;
  _LMTablePos = 0;

#ifdef LM_DUMP_MODEL
  _RCTableLen = 300;
	_RCTable.Create(40, _RCTableLen);

  _RCTable.SetHeading(0, "Frame");
  _RCTable.SetDataType(0, MeasurementTable::INT);
  _RCTable.SetHeading(1, "Dmax");
  _RCTable.SetDataType(1, MeasurementTable::INT);
  _RCTable.SetHeading(2, "r Rate"); ///< Required rate
  _RCTable.SetDataType(2, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(3, "Rate"); ///< Actual rate
  _RCTable.SetDataType(3, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(4, "p MSE");  ///< Predicted MSE
  _RCTable.SetDataType(4, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(5, "MSE");  ///< Actual MSE
  _RCTable.SetDataType(5, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(6, "a");
  _RCTable.SetDataType(6, MeasurementTable::DOUBLE);
  _RCTable.SetHeading(7, "b");
  _RCTable.SetDataType(7, MeasurementTable::DOUBLE);

  int i;
  char charBuffer[8];
  for(i = 0; i < 16; i++)
  {
    _RCTable.SetHeading(i+8, (const char*)_itoa(i, charBuffer, 10) );
    _RCTable.SetDataType(i+8, MeasurementTable::DOUBLE);
    _RCTable.SetHeading(i+24, (const char*)_itoa(i, charBuffer, 10) );
    _RCTable.SetDataType(i+24, MeasurementTable::DOUBLE);
  }//end for i...

  _predMD           = 0.0;
  _MD               = 0.0;
  _predDmax         = 0.0;
  _predRate         = 0.0;
  _rate             = 0.0;
#endif

  return(1);
}//end Create.

void LinearModel::Destroy(void)
{
  _LMTable.Destroy();
  _LMTableLen = 0;
  _LMTablePos = 0;

  _ySamples.Destroy();
  _xSamples.Destroy();

  _numSamples = 0;
  _type       = xyType;

}//end Destroy.

void LinearModel::Dump(const char* filename)
{
  if(_LMTablePos > 0)
    _LMTable.Save(filename, ",", 1);
  _LMTablePos = 0;
}//end Dump.

/*
---------------------------------------------------------------------------
	Public Interface Methods.
---------------------------------------------------------------------------
*/

/** Add a pair of (x,y) samples to the fifos.
After adding then update the running sums or initialise the running sums.
@param x          : Independent x-axis sample.
@param y          : Independent y-axis sample.
@param initialise : Signal initialisation required.
@return           : none.
*/
void LinearModel::PutSamples(double x, double y, bool initialise)
{
  /// For scalar types Sx and Sx2 are constant and xSamples are fixed with [1...N]. Therefore
  /// xSamples must never be updated. A further implication of xSamples being fixed is that
  /// Sxy must be recalculated every time.
  if (_type == xyType)
    Put(_xSamples, x, &_prevXSample, initialise);
  Put(_ySamples, y, &_prevYSample, initialise);

  if (!initialise) ///< Update the model parameters and running sums associated with the model.
  {
    double xt = (_xSamples.GetBuffer())[0];
    double yt = (_ySamples.GetBuffer())[0];

    _Sy = UpdateRunningSum(_prevYSample, yt, _Sy);  ///< All types require Sy to be updated.
    if (_type == xyType)
    {
      double x2 = xt*xt;
      double px2 = _prevXSample*_prevXSample;
      _Sx = UpdateRunningSum(_prevXSample, xt, _Sx);
      _Sx2 = UpdateRunningSum(px2, x2, _Sx2);
      _Sxy = UpdateRunningSum(_prevXSample*_prevYSample, xt*yt, _Sxy);
    }//end if xyType...
    else  ///< Scalar types require recalc of Sxy
    {
      _Sxy = 0.0;
      for (int t = 0; t < _numSamples; t++)
        _Sxy += (_xSamples.GetBuffer())[t] * (_ySamples.GetBuffer())[t];
    }//end else...

  }//end if !initialise...
  else  ///< Indicates that there were no valid data in the model before storing these first samples.
  {
    if (_type == timeSeriesType)
    {
      /// For a time series type, the x-samples fifo remains fixed with constant reverse order sample numbers as
      /// follows: [0] = _numSamples, [1] = (_numSamples-1), [2] = (_numSamples-2), ... , [_numSamples-1] = 1. This
      /// also implies that Sx and Sx2 will be constants with prevXSample = 1. Pre-load the xSamples:
      _xSamples.AddFirstIn(1.0);  ///< Force one item into the Fifo to initialise properly.
      for (int t = 0; t < _numSamples; t++)
        (_xSamples.GetBuffer())[t] = (double)(_numSamples - t);
      _prevXSample = 1.0;
    }//end if timeSeriesType...

    /// The fifo buffers must have some valid data in them and the model parameters must be reset prior
    /// to executing this code segment.

    /// Clear the running sums.
    _Sx   = 0.0;
    _Sy   = 0.0;
    _Sxy  = 0.0;
    _Sx2  = 0.0;
    /// Accumulate the sums required for Cramer's Rule fitting.
    for (int i = 0; i < _numSamples; i++)
    {
      double xt = (_xSamples.GetBuffer())[i];
      double yt = (_ySamples.GetBuffer())[i];

      double x2 = xt*xt;
      _Sx2  += x2;
      _Sx   += xt;
      _Sy   += yt;
      _Sxy  += xt*yt;
    }//end for i...
  }//end else...

}//end PutSamples.

/** Get Linear Model that passes through point (x0,y0).
Find the best curve fit from the fifo data the passes through point (x0,y0) and 
minimises the objective function. This implementation finds the best slope for
yintercept = y0 - slope*x0. (The alternative is to find the best yintercept  for
slope = (y0-yintercept)/x0.
@param slope      : Returned slope parameter.
@param yintercept : Returned yintercept parameter.
@param x0         : x point that the model must pass through.
@param y0         : y point that the model must pass through.
@return           : Objective function with this model.
*/
double LinearModel::GetModelLin(double* slope, double* yintercept, double x0, double y0)
{
  double a = *slope;  ///< Default is to leave model parameters unchanged.
  double b = *yintercept;

  /// Collect the sums.
  double Syx0 = 0.0;
  double Sx0  = 0.0;
  double Sx02 = 0.0;
  for (int i = 0; i < _numSamples; i++)
  {
    double x0diff = _xSamples.GetItem(i) - x0;
    Syx0         += (_ySamples.GetItem(i) * x0diff);
    Sx0          += x0diff;
    Sx02         += (x0diff * x0diff);
  }//end for i...

  if (Sx02 != 0.0)  ///< Prevent divide by zero error.
  {
    a = (Syx0 - (y0 * Sx0)) / Sx02;
    b = y0 - (a * x0);
  }//end if Sx02...

  *slope = a;
  *yintercept = b;
  return(GetObjFunc(a, b));
}//end GetModelLin.

double LinearModel::GetModelLin(double* slope, double* yintercept, int* inclusionlist, int listlen)
{
  int i;
  double a = *slope;  ///< Default is to leave model parameters unchanged.
  double b = *yintercept;

  /// Collect the sums required for Cramer's Rule fitting only from those fifo positions
  /// marked in the inclusion list.
  double Sx = 0.0;
  double Sy = 0.0;
  double Sxy = 0.0;
  double Sx2 = 0.0;
  for (i = 0; i < listlen; i++)
  {
    double xt = (_xSamples.GetBuffer())[inclusionlist[i]];
    double yt = (_ySamples.GetBuffer())[inclusionlist[i]];

    double x2 = xt*xt;
    Sx2 += x2;
    Sx += xt;
    Sy += yt;
    Sxy += xt*yt;
  }//end for i...

  CramersRuleSoln(Sx2, Sx, Sx, (double)listlen, Sxy, Sy, &a, &b);

  *slope = a;
  *yintercept = b;
  return(GetObjFunc(a, b, inclusionlist, listlen));
}//end GetModelLin.

/*
----------------------------------------------------------------------------------------------------
Private Methods
----------------------------------------------------------------------------------------------------
*/

/** Calculate R^2 from a sample list.
Determine the R^2 statistical model fit within an array of samples. Note
that the most recent samples are at location ySamples[0].
@param  slope       : Linear model slope.
@param  yintercept  : Linear model y-intercept.
@return             : R^2.
*/
double LinearModel::RSqr(double slope, double yintercept)
{
  double* samples = _ySamples.GetBuffer();
  double mean = 0.0;
  for (int i = 0; i < _numSamples; i++)
    mean += samples[i];
  mean = mean / _numSamples;

  double SStot = 0.0;
  double SSres = 0.0;
  for (int i = 0; i < _numSamples; i++)
  {
    double x = samples[i];
    SStot += (x - mean)*(x - mean);
    double modelDiff = x - (slope*(_numSamples - i) + yintercept);
    SSres += modelDiff*modelDiff;
  }//end for i...

  double R2 = 1.0;
  if (SStot != 0.0)
    R2 = 1.0 - (SSres / SStot);

  return(R2);
}//end RSqr.

