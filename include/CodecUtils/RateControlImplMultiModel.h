/** @file

MODULE				: RateControlImplMultiModel

TAG						: RCIMM

FILE NAME			: RateControlImplMultiModel.h

DESCRIPTION		: A class to hold a multi-function model for the frame buffer rate rate control to
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

===========================================================================
*/
#ifndef _RATECONTROLIMPLMULTIMODEL_H
#define _RATECONTROLIMPLMULTIMODEL_H

#pragma once

#include  <math.h>
#include  "Fifo.h"
#include  "IRateControl.h"

#include  "MeasurementTable.h"
//#define RCIMM_DUMP_RATECNTL  1

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RateControlImplMultiModel : public IRateControl
{
public:
  RateControlImplMultiModel(void) { ResetMembers(); }
  RateControlImplMultiModel(double modelparms[][2], int nummodels);
  virtual ~RateControlImplMultiModel() { Destroy(); }

/// IRateControl Interface methods.
public:
  int		Create(int numOfFrames);
  void	Destroy(void);
  int   GetFameBufferLength(void) { return(_numOfFrames); }
  void  SetRDLimits(double rateUpper, double rateLower, double distUpper, double distLower);

  /// Pre-encoding: Apply the rate control model to predict the distortion from the target coeff rate.
  int PredictDistortion(double targetAvgRate, double rateLimit);
  /// Post-encoding: Add samples to the fifo buffers and hold the sample that is discarded.
  void StoreMeasurements(double rate, double coeffrate, double distortion, double mse, double mae);

  /// Return average rate across the buffer as the performance measure.
  double GetPerformanceMeasure(void) { return(0.0); }

  /// Data is valid if models are active.
  int ValidData(void) 
  { if(ModelsActive()) 
      if( (_model[QUAD]->ValidData())&&(_model[POW]->ValidData())&&(_model[LOG]->ValidData()) )
        return(1);
    return(0); 
  }
  /// Was the prediction calculation out of bounds?
  bool OutOfBounds(void) { return(_outOfBoundsFlag); }
  /// Reset the fifos. Use after a change in frame type.
  void Reset(void);

  /// Utility functions.
  void Dump(const char* filename);  ///< Requires RCIP_DUMP_RATECNTL to be set.

/// Further implementation specific public methods.
public:

/// Private methods.
protected:
  void ResetMembers(void);

  /// Check if the models exist.
  bool ModelsActive(void) { return( (_model[QUAD] != NULL)&&(_model[POW] != NULL)&&(_model[LOG] != NULL) ); }

  /// Model equations.
  double ModelDistortion(double rate, double a, double b, double var) { return(0.0); }
  double ModelRate(double distortion, double a, double b, double var) { return(0.0); }

/// Constant members.
public:
  /// Model list array referencing.
  static const int QUAD   = 0;
  static const int POW    = 1;
  static const int LOG    = 2;
  static const int MODELS = 3;  ///< Num of models

/// Common members.
protected:
	int				_numOfFrames;
  /// Clip the values of the rate and distortion to these limits.
  double    _rateUpper;
  double    _rateLower;
  double    _distUpper;
  double    _distLower;

  /// Model list.
  IRateControl* _model[3];          ///< Pointers to the three models {QUAD, POW, LOG}.
  double        _modelParam[3][2];  ///< Store initial model parameters _a and _b for each model.
  int           _prevPredModelUsed; ///< Remember the model that was used for the last prediction.

  /// Signal when the predicted values are not valid due to out of range errors. Use
  /// this signal to prevent contributions to the model data.
  bool      _outOfBoundsFlag;

//////////////////////////////////////
  int _modelCount[3];

  MeasurementTable  _RCTable;
  int               _RCTableLen;
  int               _RCTablePos;
#ifdef RCIMM_DUMP_RATECNTL
  double            _predDmax;
  double            _predRate;
  double            _rate;
#endif
};// end class RateControlImplMultiModel.

#endif	// _RATECONTROLIMPLMULTIMODEL_H
