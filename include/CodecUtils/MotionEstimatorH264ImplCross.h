/** @file

MODULE				: MotionEstimatorH264ImplCross

TAG						: MEH264IC

FILE NAME			: MotionEstimatorH264ImplCross.h

DESCRIPTION		: A full unrestricted motion estimator implementation for 
								Recommendation H.264 (03/2005) with both absolute error 
								difference and square error measure. Access is via an 
								IMotionEstimator interface. There are 2 mode levels of 
								execution speed vs. quality. The boundary is extended to 
								accomodate the selected motion range.

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
#ifndef _MOTIONESTIMATORH264IMPLCROSS_H
#define _MOTIONESTIMATORH264IMPLCROSS_H

#include "IMotionEstimator.h"
#include "IMotionVectorPredictor.h"
#include "VectorStructList.h"
#include "OverlayMem2Dv2.h"
#include "OverlayExtMem2Dv2.h"
#include "Fifo.h"

/*
---------------------------------------------------------------------------
	Struct definition.
---------------------------------------------------------------------------
*/
typedef struct _MEH264IC_COORD
{
	short int x;
	short int y;
} MEH264IC_COORD;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MotionEstimatorH264ImplCross : public IMotionEstimator
{
/// Construction.
public:

	MotionEstimatorH264ImplCross(	const void*             pSrc, 
																const void*             pRef, 
																int					            imgWidth, 
																int					            imgHeight,
																int					            motionRange,
                                IMotionVectorPredictor* pMVPred);

	MotionEstimatorH264ImplCross(	const void*             pSrc, 
																const void*             pRef, 
																int					            imgWidth, 
																int					            imgHeight,
																int					            motionRange,
                                IMotionVectorPredictor* pMVPred,
																void*				            pDistortionIncluded);

	virtual ~MotionEstimatorH264ImplCross(void);

/// IMotionEstimator Interface.
public:
	virtual int		Create(void);
  virtual void	Reset(void)       {}
	virtual int		Ready(void)		    { return(_ready); }
  virtual void	SetMode(int mode) { _mode = mode; }
	virtual int		GetMode(void)     { return(_mode); }

	/** Motion estimate the source within the reference.
	Do the estimation with the block sizes and image sizes defined in
	the implementation. The returned type holds the vectors.
	@param pSrc		: Input image to estimate.
	@param pRef		: Ref to estimate with.
	@return				: The list of motion vectors.
	*/
	virtual void* Estimate(const void* pSrc, const void* pRef, long* avgDistortion)
		{ return(Estimate(avgDistortion)); }
	virtual void* Estimate(long* avgDistortion);

/// Local methods.
protected:

	/// Used by constructors to reset every member.
	void ResetMembers(void);
	/// Clear alloc mem.
	void Destroy(void);
	/// Get the motion search range in the reference and limit it to the picture extended boundaries.
	void GetMotionRange(int		x,			int		y, 
											int*	xlr,		int*	xrr, 
											int*	yur,		int*	ydr, 
											int		range); 
	/// Motion search range in the reference limited to the picture extended boundaries and max vector range.
	void GetMotionRange(int		x,			int		y,
											int		offx,		int		offy,
											int*	xlr,		int*	xrr, 
											int*	yur,		int*	ydr, 
											int		range); 

	void LoadHalfQuartPelWindow(OverlayMem2Dv2* qPelWin, OverlayMem2Dv2* extRef);
	void LoadQuartPelWindow(OverlayMem2Dv2* qPelWin, int hPelColOff, int hPelRowOff);
	void QuarterRead(OverlayMem2Dv2* dstBlock, OverlayMem2Dv2* qPelWin, int qPelColOff, int qPelRowOff);

  /// Select the vector that minimised a cost function from a list of x, y and distortion ordered vectors. The
  /// euclidian distance is measured from a reference vector. Return the list index of the 
  /// smallest vector. The values of mx & my are assumed to be copies of the vector from the top
  /// of the list at pos = 0.
  int GetBestVector(Fifo* d, Fifo* x, Fifo* y, double lambda, double norm, int xref, int yref, int* mx, int* my)
  {
    int pos = 0;
    int vecLen = (int)x->GetLength();  ///< Assume d, x & y have the same length.
    double currMag = (double)((((*mx) - xref)*((*mx) - xref)) + (((*my) - yref)*((*my) - yref)));
    double currDist = d->GetItem(0);
    double currCost = (currDist / norm) + (lambda*sqrt(currMag));

    for (int ii = 1; ii < vecLen; ii++)
    {
      int xVec = (int)x->GetItem(ii);
      int yVec = (int)y->GetItem(ii);
      double newMag = (double)(((xVec - xref)*(xVec - xref)) + ((yVec - yref)*(yVec - yref)));
      double newDist = d->GetItem(ii);
      double newCost = (newDist / norm) + (lambda*sqrt(newMag));
      if (newCost < currCost)
      {
        (*mx) = xVec;
        (*my) = yVec;
        currMag = newMag;
        currDist = newDist;
        currCost = newCost;
        pos = ii;
      }//end if newCost...

    }//end for ii...

    return(pos);
  }//end GetBestVector.

protected:
  int _pathLength;
  const static MEH264IC_COORD MEH264IC_OptimalPath[128];
  const static MEH264IC_COORD MEH264IC_LinearPath[256];

protected:

	int _ready;	///< Ready to estimate.
	int _mode;	///< Motion pel resolution. [ 0 = 1/4 pel, 1 = 1/2 pel 1, 2 = full pel.]

	/// Parameters must remain const for the life time of this instantiation.
	int	_imgWidth;				///< Width of the src and ref images. 
	int	_imgHeight;				///< Height of the src and ref images.
	int	_macroBlkWidth;		///< Width of the motion block.
	int	_macroBlkHeight;	///< Height of the motion block.
	int	_motionRange;			///< (4x,4y) range of the motion vectors in 1/4 pel units.

	const void*	_pInput;	///< References to the images at construction.
	const void* _pRef;

	/// Input mem overlay members.
	OverlayMem2Dv2*		_pInOver;					///< Input overlay with motion block dim.

	/// Ref mem overlay members.
	OverlayMem2Dv2*			_pRefOver;				///< Ref overlay with whole block dim.
	short*							_pExtRef;					///< Extended ref mem created by ExtendBoundary() call.
	int									_extWidth;
	int									_extHeight;
	int									_extBoundary;			///< Extended boundary for left, right, up and down.
	OverlayExtMem2Dv2*	_pExtRefOver;			///< Extended ref overlay with motion block dim.

	/// A 1/4 pel refinement window.
	short*							_pWin;
	OverlayMem2Dv2*			_Win;

	/// Temp working block and its overlay.
	short*							_pMBlk;						///< Motion block temp mem.
	OverlayMem2Dv2*			_pMBlkOver;				///< Motion block overlay of temp mem.

  /// Fifos to store intermediate results.
  Fifo  _xVector;
  Fifo  _yVector;
  Fifo  _distVector;

  /// Hold the resulting motion vectors in a byte array.
	VectorStructList*	_pMotionVectorStruct;
  /// Attached motion vector predictor on construction.
  IMotionVectorPredictor* _pMVPred;

	/// A flag per macroblock to include it in the distortion accumulation.
	bool*							_pDistortionIncluded;
};//end MotionEstimatorH264ImplCross.

#endif // !_MOTIONESTIMATORH264IMPLCROSS_H

