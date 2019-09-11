/** @file

MODULE				: MotionEstimatorH264ImplUMHS

TAG						: MEH264IUMHS

FILE NAME			: MotionEstimatorH264ImplUMHS.cpp

DESCRIPTION		: A unsymetrical-cross multi-hexagon grid search motion estimator
                implementation for Recommendation H.264 (03/2005) with both
                absolute error difference and square error measure. Access
                is via an IMotionEstimator interface. There are 2 mode levels
                of execution speed vs. quality. The boundary is extended to
                accomodate the selected motion range.

COPYRIGHT			: (c)CSIR 2007-2019 all rights resevered

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
#include <stdio.h>
#endif

#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include	"MotionEstimatorH264ImplUMHS.h"

/*
--------------------------------------------------------------------------
  Constants. 
--------------------------------------------------------------------------
*/
/// Calc = ((16[vec dim] * 16[vec dim]) * 2.
#define MEH264IUMHS_FULL_MOTION_NOISE_FLOOR				512
#define MEH264IUMHS_MOTION_NOISE_FLOOR						26 //24 //sqrt 512

/// Boundary padding past the motion vector extremes. Required for calculating
/// sub-pixel interpolations. 
#define MEH264IUMHS_PADDING													3

/// At each motion vector sub search pattern the range is limited around the 
/// previous stage's centre. 
#define MEH264IUMHS_LOCAL_RANGE											16

/// Choose between sqr err distortion or abs diff metric.
#undef MEH264IUMHS_ABS_DIFF

/// Search range coord offsets for 5x5 pattern search ordered from inner to outer.
#define MEH264IUMHS_MOTION_5X5_POS_LENGTH 	24
MEH264IUMHS_COORD MEH264IUMHS_5x5Pos[MEH264IUMHS_MOTION_5X5_POS_LENGTH] =
{
  { -1, 0 },{  1, 0 },{  0,-1 },{ 0,  1 },
  { -1,-1 },{  1,-1 },{ -1, 1 },{ 1,  1 },
  { 0, -2 },{ -2, 0 },{  2, 0 },{ 0,  2 },
  { -1,-2 },{ 1, -2 },{ -2,-1 },{ 2, -1 },
  { -2, 1 },{ 2,  1 },{ -1, 2 },{ 1,  2 },
  { -2,-2 },{ 2, -2 },{ -2, 2 },{ 2,  2 }
};

/// Search range coords for centre motion vectors.
#define MEH264IUMHS_MOTION_SUB_POS_LENGTH 	8
MEH264IUMHS_COORD MEH264IUMHS_SubPos[MEH264IUMHS_MOTION_SUB_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

#define MEH264IUMHS_MOTION_CROSS_POS_LENGTH 	4
MEH264IUMHS_COORD MEH264IUMHS_CrossPos[MEH264IUMHS_MOTION_CROSS_POS_LENGTH] =
{
  { -1,0 },{ 1,0 },{ 0,-1 },{ 0,1 }
};

/// Search range coord offsets for uneven hexagon pattern search ordered from most to least likely.
#define MEH264IUMHS_MOTION_HEX_POS_LENGTH 	16
MEH264IUMHS_COORD MEH264IUMHS_HexPos[MEH264IUMHS_MOTION_HEX_POS_LENGTH] =
{
  { -4, 0 },{ 4, 0 },
  { -4, 1 },{ 4, 1 },{ -4, -1 },{ 4, -1 },
  { -4, 2 },{ 4, 2 },{ -4, -2 },{ 4, -2 },
  { -2, 3 },{ 2, 3 },{ -2, -3 },{ 2, -3 },
  {  0, 4 },{ 0, -4 }
};

/// Search range coord offsets for uneven hexagon pattern search ordered from most to least likely.
#define MEH264IUMHS_MOTION_EXTHEX_POS_LENGTH 	6
MEH264IUMHS_COORD MEH264IUMHS_ExtHexPos[MEH264IUMHS_MOTION_EXTHEX_POS_LENGTH] =
{
  { -2, 0 },{ -1, -2 },{ 1, -2 },{ 2, 0 },{ 1, 2 },{ -1, 2 }
};

const MEH264IUMHS_COORD MotionEstimatorH264ImplUMHS::MEH264IUMHS_OptimalPath[] = 
{
  {  5,10 },{ 13, 5 },{  2, 1 },{ 13,14 },{  1,14 },{  9, 1 },{  1, 6 },{ 15, 9 },{  8,13 },{ 14, 2 },{  7, 4 },{ 10, 8 },{  4,15 },{  0,10 },{  0, 3 },{  5, 0 },
  { 12,11 },{  4, 7 },{ 12, 0 },{ 10,14 },{  3,12 },{ 10, 5 },{ 15,13 },{  4, 3 },{  7, 8 },{ 14, 7 },{  6,13 },{ 11, 3 },{  2, 9 },{  9,10 },{ 15, 1 },{  7, 2 },
  {  3, 5 },{  8,15 },{  1,12 },{  8, 6 },{  0, 1 },{ 12, 9 },{ 14,15 },{ 15, 4 },{ 11,12 },{  6, 6 },{  7,11 },{  0, 7 },{ 14,11 },{  7, 0 },{  2,15 },{ 12, 7 },
  {  2, 3 },{  4, 9 },{ 12, 2 },{  9, 3 },{ 12,13 },{  3, 0 },{  5,14 },{  5, 4 },{  0,13 },{  8, 9 },{  2,11 },{ 11,15 },{ 10, 1 },{  2, 7 },{ 15, 6 },{  9,12 },
  {  5, 2 },{  5,12 },{  1, 4 },{ 11, 5 },{ 13, 8 },{  9, 7 },{ 13, 3 },{  5, 8 },{ 10,11 },{  6,15 },{ 13,10 },{  8, 4 },{  3,10 },{ 13, 1 },{  1, 8 },{ 14, 0 },
  {  3, 2 },{ 14,12 },{  6, 5 },{  3, 1 },{  0,15 },{  7,10 },{ 11, 6 },{  6, 1 },{  4, 6 },{  1, 1 },{ 15,14 },{ 11, 9 },{  7,14 },{  8, 2 },{ 14, 4 },{  4,11 },
  {  1, 5 },{ 10,13 },{  6, 7 },{ 15,10 },{ 11, 1 },{  0, 9 },{  9, 5 },{  4, 1 },{ 13, 6 },{ 12,15 },{  3, 4 },{  6,11 },{  3,14 },{ 10, 3 },{  9, 9 },{  9,14 },
  {  6, 3 },{ 12, 4 },{  0, 5 },{ 14, 8 },{  1,10 },{  1, 2 },{  2, 8 },{  2,12 },{  2, 6 },{  0,12 },{  3, 7 },{  8,11 },{  3, 8 },{  6, 9 },{  9, 0 },{  1, 0 },
  { 15, 3 },{ 13,12 },{ 11, 8 },{  8, 7 },{  5, 5 },{  5,13 },{ 11, 0 },{ 10,10 },{ 15, 7 },{ 11,14 },{  7,12 },{  7, 5 },{ 14,13 },{ 10, 6 },{ 15, 0 },{  9,15 },
  {  7, 1 },{ 12,10 },{  2,14 },{  4, 2 },{ 12, 5 },{ 10, 2 },{  4,13 },{  8, 8 },{ 15,15 },{ 14, 5 },{  4, 0 },{ 14, 9 },{ 13, 2 },{  5, 7 },{  8, 3 },{  0, 0 },
  {  2, 4 },{  7,15 },{ 11,11 },{  0,11 },{  6,10 },{  8, 0 },{  4, 5 },{  3,15 },{ 10, 4 },{ 15,11 },{  8,12 },{ 13,13 },{  0, 4 },{  4,10 },{ 11, 7 },{  5, 3 },
  {  1,13 },{  7, 7 },{ 14, 3 },{  2, 2 },{ 10,12 },{  0, 8 },{ 12, 1 },{  5,15 },{ 13, 7 },{ 12,14 },{  3, 9 },{  9, 6 },{  6, 2 },{  7,13 },{ 13,11 },{  5,11 },
  {  9, 8 },{ 15, 5 },{  6, 0 },{ 12, 3 },{  0, 6 },{  2,10 },{ 13, 0 },{ 13,15 },{  3, 3 },{  9, 4 },{ 15, 8 },{  5, 9 },{  2, 0 },{ 10, 9 },{ 15, 2 },{ 15,12 },
  {  8,14 },

            { 10, 0 },{  5, 1 },{  8, 1 },{ 14, 1 },{  0, 2 },{  9, 2 },{ 11, 2 },{  1, 3 },{  7, 3 },{  4, 4 },{  6, 4 },{ 11, 4 },{ 13, 4 },{  2, 5 },{  8, 5 },
  {  3, 6 },{  5, 6 },{  7, 6 },{ 12, 6 },{ 14, 6 },{  1, 7 },{ 10, 7 },{  4, 8 },{  6, 8 },{ 12, 8 },{  1, 9 },{  7, 9 },{ 13, 9 },{  8,10 },{ 11,10 },{ 14,10 },
  {  1,11 },{  3,11 },{  9,11 },{  4,12 },{  6,12 },{ 12,12 },{  2,13 },{  3,13 },{  9,13 },{ 11,13 },{  0,14 },{  4,14 },{  6,14 },{ 14,14 },{  1,15 },{ 10,15 }

};

const MEH264IUMHS_COORD MotionEstimatorH264ImplUMHS::MEH264IUMHS_LinearPath[] =
{
  {  0, 0 },{  0, 1 },{  0, 2 },{  0, 3 },{  0, 4 },{  0, 5 },{  0, 6 },{  0, 7 },{  0, 8 },{  0, 9 },{  0,10 },{  0,11 },{  0,12 },{ 0,13 },{ 0,14 },{ 0,15 }, ///< 0-15
  {  1, 0 },{  1, 1 },{  1, 2 },{  1, 3 },{  1, 4 },{  1, 5 },{  1, 6 },{  1, 7 },{  1, 8 },{  1, 9 },{  1,10 },{  1,11 },{  1,12 },{ 1,13 },{ 1,14 },{ 1,15 }, ///< 16-31
  {  2, 0 },{  2, 1 },{  2, 2 },{  2, 3 },{  2, 4 },{  2, 5 },{  2, 6 },{  2, 7 },{  2, 8 },{  2, 9 },{  2,10 },{  2,11 },{  2,12 },{ 2,13 },{ 2,14 },{ 2,15 }, ///< 32-47
  {  3, 0 },{  3, 1 },{  3, 2 },{  3, 3 },{  3, 4 },{  3, 5 },{  3, 6 },{  3, 7 },{  3, 8 },{  3, 9 },{  3,10 },{  3,11 },{  3,12 },{ 3,13 },{ 3,14 },{ 3,15 }, ///< 48-63
  {  4, 0 },{  4, 1 },{  4, 2 },{  4, 3 },{  4, 4 },{  4, 5 },{  4, 6 },{  4, 7 },{  4, 8 },{  4, 9 },{  4,10 },{  4,11 },{  4,12 },{ 4,13 },{ 4,14 },{ 4,15 }, ///< 64-79
  {  5, 0 },{  5, 1 },{  5, 2 },{  5, 3 },{  5, 4 },{  5, 5 },{  5, 6 },{  5, 7 },{  5, 8 },{  5, 9 },{  5,10 },{  5,11 },{ 5,12 },{ 5,13 },{ 5,14 },{ 5,15 }, ///< 80-95
  {  6, 0 },{  6, 1 },{  6, 2 },{  6, 3 },{  6, 4 },{  6, 5 },{  6, 6 },{  6, 7 },{  6, 8 },{  6, 9 },{  6,10 },{  6,11 },{ 6,12 },{ 6,13 },{ 6,14 },{ 6,15 }, ///< 96-111
  {  7, 0 },{  7, 1 },{  7, 2 },{  7, 3 },{  7, 4 },{  7, 5 },{  7, 6 },{  7, 7 },{  7, 8 },{  7, 9 },{  7,10 },{  7,11 },{ 7,12 },{ 7,13 },{ 7,14 },{ 7,15 }, ///< 112-127
  {  8, 0 },{  8, 1 },{  8, 2 },{  8, 3 },{  8, 4 },{  8, 5 },{  8, 6 },{  8, 7 },{  8, 8 },{  8, 9 },{  8,10 },{  8,11 },{ 8,12 },{ 8,13 },{ 8,14 },{ 8,15 }, ///< 128-143
  {  9, 0 },{  9, 1 },{  9, 2 },{  9, 3 },{  9, 4 },{  9, 5 },{  9, 6 },{  9, 7 },{  9, 8 },{  9, 9 },{  9,10 },{  9,11 },{ 9,12 },{ 9,13 },{ 9,14 },{ 9,15 }, ///< 144-159
  { 10, 0 },{ 10, 1 },{ 10, 2 },{ 10, 3 },{ 10, 4 },{ 10, 5 },{ 10, 6 },{ 10, 7 },{ 10, 8 },{ 10, 9 },{ 10,10 },{ 10,11 },{ 10,12 },{ 10,13 },{ 10,14 },{ 10,15 }, ///< 160-175
  { 11, 0 },{ 11, 1 },{ 11, 2 },{ 11, 3 },{ 11, 4 },{ 11, 5 },{ 11, 6 },{ 11, 7 },{ 11, 8 },{ 11, 9 },{ 11,10 },{ 11,11 },{ 11,12 },{ 11,13 },{ 11,14 },{ 11,15 }, ///< 176-191
  { 12, 0 },{ 12, 1 },{ 12, 2 },{ 12, 3 },{ 12, 4 },{ 12, 5 },{ 12, 6 },{ 12, 7 },{ 12, 8 },{ 12, 9 },{ 12,10 },{ 12,11 },{ 12,12 },{ 12,13 },{ 12,14 },{ 12,15 }, ///< 192-207
  { 13, 0 },{ 13, 1 },{ 13, 2 },{ 13, 3 },{ 13, 4 },{ 13, 5 },{ 13, 6 },{ 13, 7 },{ 13, 8 },{ 13, 9 },{ 13,10 },{ 13,11 },{ 13,12 },{ 13,13 },{ 13,14 },{ 13,15 }, ///< 208-223
  { 14, 0 },{ 14, 1 },{ 14, 2 },{ 14, 3 },{ 14, 4 },{ 14, 5 },{ 14, 6 },{ 14, 7 },{ 14, 8 },{ 14, 9 },{ 14,10 },{ 14,11 },{ 14,12 },{ 14,13 },{ 14,14 },{ 14,15 }, ///< 224-239
  { 15, 0 },{ 15, 1 },{ 15, 2 },{ 15, 3 },{ 15, 4 },{ 15, 5 },{ 15, 6 },{ 15, 7 },{ 15, 8 },{ 15, 9 },{ 15,10 },{ 15,11 },{ 15,12 },{ 15,13 },{ 15,14 },{ 15,15 }  ///< 240-255
};

/// Num of items in the Fifos.
#define MEH264IUMHS_FIFO_LENGTH 5

/*
--------------------------------------------------------------------------
  Macros. 
--------------------------------------------------------------------------
*/
#define MEH264IUMHS_CLIP255(x)	( (((x) <= 255)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:255 ) )

#define MEH264IUMHS_EUCLID_COST(x,y,xref,yref) ( (((x)-(xref))*((x)-(xref)))+(((y)-(yref))*((y)-(yref))) )
#define MEH264IUMHS_EUCLID_MAG(x,y,xref,yref) ( sqrt((double)((((x)-(xref))*((x)-(xref)))+(((y)-(yref))*((y)-(yref))))) )

#define MEH264IUMHS_COST(d,x,y,xref,yref) ( (int)(((double)(d)/256.0) + ( 1.05*MEH264IUMHS_EUCLID_MAG((x),(y),(xref),(yref)) )) )

/*
--------------------------------------------------------------------------
  Construction. 
--------------------------------------------------------------------------
*/

MotionEstimatorH264ImplUMHS::MotionEstimatorH264ImplUMHS(	const void*               pSrc, 
																													  const void*             pRef, 
																													  int					            imgWidth, 
																													  int					            imgHeight,
																													  int					            motionRange,
                                                            IMotionVectorPredictor* pMVPred)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= imgWidth;					///< Width of the src and ref images. 
	_imgHeight			= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth	= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight	= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange		= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput					= pSrc;
	_pRef						= pRef;
  _pMVPred        = pMVPred;

}//end constructor.

MotionEstimatorH264ImplUMHS::MotionEstimatorH264ImplUMHS(	const void*             pSrc, 
																													const void*             pRef, 
																													int					            imgWidth, 
																													int					            imgHeight,
																													int					            motionRange,
                                                          IMotionVectorPredictor* pMVPred,
																													void*				            pDistortionIncluded)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth							= imgWidth;					///< Width of the src and ref images. 
	_imgHeight						= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth				= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight				= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange					= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput								= pSrc;
	_pRef									= pRef;
  _pMVPred              = pMVPred;
	_pDistortionIncluded	= (bool *)pDistortionIncluded;

}//end constructor.

MotionEstimatorH264ImplUMHS::MotionEstimatorH264ImplUMHS( const void*             pSrc,
                                                          const void*             pRef,
                                                          int					            imgWidth,
                                                          int					            imgHeight,
                                                          int					            motionRange,
                                                          IMotionVectorPredictor* pMVPred,
                                                          void*				            pDistortionIncluded,
                                                          MacroBlockH264*         pPrevFrmMBlk)
{
  ResetMembers();

  /// Parameters must remain const for the life time of this instantiation.
  _imgWidth = imgWidth;					///< Width of the src and ref images. 
  _imgHeight = imgHeight;				///< Height of the src and ref images.
  _macroBlkWidth = 16;				  ///< Width of the motion block = 16 for H.263.
  _macroBlkHeight = 16;					///< Height of the motion block = 16 for H.263.
  _motionRange = motionRange;		///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
  _pInput = pSrc;
  _pRef = pRef;
  _pMVPred = pMVPred;
  _pDistortionIncluded = (bool *)pDistortionIncluded;
  _pPrevFrmMBlk = pPrevFrmMBlk;
}//end constructor.


void MotionEstimatorH264ImplUMHS::ResetMembers(void)
{
	_ready	= 0;	///< Ready to estimate.
	_mode		= 0;	///< Default to quarterPel resolution.

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= 0;					///< Width of the src and ref images. 
	_imgHeight			= 0;					///< Height of the src and ref images.
	_macroBlkWidth	= 16;					///< Width of the motion block = 16 for H.264.
	_macroBlkHeight	= 16;					///< Height of the motion block = 16 for H.264.
	_motionRange		= 64;					///< (4x,4y) range of the motion vectors.
	_pInput					= NULL;
	_pRef						= NULL;

	/// Input mem overlay members.
	_pInOver					= NULL;			///< Input overlay with mb motion block dim.
	/// Ref mem overlay members.
	_pRefOver					= NULL;			///< Ref overlay with whole block dim.
	_pExtRef					= NULL;			///< Extended ref mem created by ExtendBoundary() call.
	_extWidth					= 0;
	_extHeight				= 0;
	_extBoundary			= 0;
	_pExtRefOver			= NULL;			///< Extended ref overlay with motion block dim.
  /// A 1/4 pel refinement window.
	_pWin							= NULL;
	_Win							= NULL;

	/// Temp working block and its overlay.
	_pMBlk						= NULL;			///< Motion block temp mem.
	_pMBlkOver				= NULL;			///< Motion block overlay of temp mem.

	/// Hold the resulting motion vectors in a byte array.
	_pMotionVectorStruct = NULL;
  /// Attached motion vector predictor on construction.
  _pMVPred             = NULL;

  /// A flag per macroblock to include it in the distortion accumulation.
	_pDistortionIncluded = NULL;

  /// Reference to encoder macroblocks from the previously encoded frame. Used for prediction.
  _pPrevFrmMBlk = NULL;

  /// Number of locations to test for partial sums along a path.
  _pathLength = 256;

}//end ResetMembers.

MotionEstimatorH264ImplUMHS::~MotionEstimatorH264ImplUMHS(void)
{
	Destroy();
}//end destructor.

/*
--------------------------------------------------------------------------
  Public IMotionEstimator Interface. 
--------------------------------------------------------------------------
*/

int MotionEstimatorH264ImplUMHS::Create(void)
{
	/// Clean out old mem.
	Destroy();

	/// --------------- Configure input overlays --------------------------------
	/// Put an overlay on the input image with the block size set to the mb vector 
	/// dim. This is used to access input vectors.
	_pInOver = new OverlayMem2Dv2((void *)_pInput,_imgWidth,_imgHeight,_macroBlkWidth,_macroBlkHeight);
	if(_pInOver == NULL)
	{
		Destroy();
		return(0);
	}//end _pInOver...

	/// --------------- Configure ref overlays --------------------------------
	/// Overlay the whole reference. The reference will have an extended 
	/// boundary for motion estimation and must therefore create its own mem.
	_pRefOver = new OverlayMem2Dv2((void *)_pRef, _imgWidth, _imgHeight, _imgWidth, _imgHeight);
	if(_pRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pRefOver...

	/// Create the new extended boundary ref into _pExtRef. The boundary is extended by
	/// the max dimension of the macroblock plus some padding to cater for quarter pel
  /// searches on the edges of the boundary. The mem is allocated in the method call.
	_extBoundary = _macroBlkWidth + MEH264IUMHS_PADDING;
	if(_macroBlkHeight > _macroBlkWidth)
		_extBoundary = _macroBlkHeight + MEH264IUMHS_PADDING;
	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRef, 
																				_imgWidth,						
																				_imgHeight, 
																				_extBoundary,	///< Extend left and right by...
																				_extBoundary,	///< Extend top and bottom by...
																				(void **)(&_pExtRef)) )	///< Created in the method and returned.
  {
		Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extWidth	 = _imgWidth + (2 * _extBoundary);
	_extHeight = _imgHeight + (2 * _extBoundary);

	/// Place an overlay on the extended boundary ref with block size set to the mb motion 
  /// vec dim.
	_pExtRefOver = new OverlayExtMem2Dv2(	_pExtRef,				///< Src description created in the ExtendBoundary() call. 
																				_extWidth, 
																				_extHeight,
																				_macroBlkWidth,	///< Block size description.
																				_macroBlkHeight,
																				_extBoundary,		///< Boundary size for both left and right.
																				_extBoundary  );
	if(_pExtRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pExtRefOver...

	/// --------------- Configure temp overlays --------------------------------
	/// Alloc some temp mem and overlay it to use for half/quarter pel motion 
  /// estimation and compensation. The block size is the same as the mem size.
	_pMBlk = new short[_macroBlkWidth * _macroBlkHeight];
	_pMBlkOver = new OverlayMem2Dv2(_pMBlk, _macroBlkWidth, _macroBlkHeight, 
																					_macroBlkWidth, _macroBlkHeight);
	if( (_pMBlk == NULL)||(_pMBlkOver == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pMBlk...

	/// --------------- Configure result ---------------------------------------
	/// The structure container for the motion vectors.
	_pMotionVectorStruct = new VectorStructList(VectorStructList::SIMPLE2D);
	if(_pMotionVectorStruct != NULL)
	{
		/// How many motion vectors will there be at the block dim.
		int numVecs = (_imgWidth/_macroBlkWidth) * (_imgHeight/_macroBlkHeight);
		if(!_pMotionVectorStruct->SetLength(numVecs))
		{
			Destroy();
			return(0);
		}//end _pMotionVectorStruct...
	}//end if _pMotionVectorStruct...
	else
  {
		Destroy();
	  return(0);
  }//end if else...

	/// --------------- Refinement Window ---------------------------------------
	/// Prepare a 1/4 pel search window block for motion estimation refinement.
	int winWidth	= ((6 + _macroBlkWidth) * 4);
	int winHeight = ((6 + _macroBlkHeight) * 4);
	_pWin					= new short[winWidth * winHeight];
	_Win					= new OverlayMem2Dv2((void *)_pWin, winWidth, winHeight, winWidth, winHeight);
	if( (_pWin == NULL)||(_Win == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pWin...

  /// --------------- Measurements -------------------------------------------
#ifdef MEH264IUMHS_TAKE_MEASUREMENTS
  _mtLen = 290;
  _mtPos = 0;
  _mt.Create(7, _mtLen);
  _mt.SetTitle("Central MB Full Pel Motion");

  _mt.SetHeading(0, "Dpred");
  _mt.SetHeading(1, "Dpredactual");
  _mt.SetHeading(2, "MVXpred");
  _mt.SetHeading(3, "MVYpred");
  _mt.SetHeading(4, "Dmin");
  _mt.SetHeading(5, "MVXmin");
  _mt.SetHeading(6, "MVYmin");

  for (int j = 0; j < 7; j++)
    _mt.SetDataType(j, MeasurementTable::INT);
#endif

	_ready = 1;
	return(1);
}//end Create.

/** Motion estimate the source within the reference.
Do the estimation with the block sizes and image sizes defined in the implementation. 
The returned type holds the vectors. This is a telescopic cross search algorithm with 
extended boundaries with a choice of absolute difference or squared difference criteria. 
@param avgDistortion  : Return the motion compensated distortion.
@return				        : The list of motion vectors.
*/
void* MotionEstimatorH264ImplUMHS::Estimate(long* avgDistortion)
{
  int		i,j,m,n;
	int		included = 0;
	long	totalDifference = 0;

	/// Set the motion vector struct storage structure.
	int		maxLength	= _pMotionVectorStruct->GetLength();
	int		vecPos		= 0;

	/// Write the ref and fill its extended boundary. The centre part of
	/// _pExtRefOver is copied from _pRefOver before filling the boundary.
	_pExtRefOver->SetOrigin(0, 0);
	_pExtRefOver->SetOverlayDim(_imgWidth, _imgHeight);
	_pExtRefOver->Write(*_pRefOver);	///< _pRefOver dimensions are always set to the whole image.
	_pExtRefOver->FillBoundaryProxy();
	_pExtRefOver->SetOverlayDim(_macroBlkWidth, _macroBlkHeight);

  /// _motionRange is in 1/4 pel units and must be converted to full pel units.
  int mRng = _motionRange / 4;  

  /// Gather the motion vector absolute differnce/square error data and choose the vector.
	/// m,n step level 0 vec dim = _macroBlkHeight, _macroBlkWidth.
  for(m = 0; m < _imgHeight; m += _macroBlkHeight)
		for(n = 0; n < _imgWidth; n += _macroBlkWidth)
  {
		int mx	= 0;	///< Full pel grid.
		int my	= 0;
		int hmx	= 0;	///< 1/2 pel on 1/4 pel grid.
		int hmy	= 0;
		int qmx	= 0;	///< 1/4 pel grid.
		int qmy	= 0;
		int rmx = 0;	///< Refinement motion vector centre.
		int rmy = 0;

		/// Depending on which img boundary we are on will limit the full search range.
		int xlRng, xrRng, yuRng, ydRng;
    /// Set the postiion of the input mb to work with.
    _pInOver->SetOrigin(n, m);

//    if ((m == 144) && (n == 176))  /// Central MB only.
//      int check = 1;

    ///--------------------------- Full pel reference point ---------------------------------------------------
    /// The predicted vector difference between the input and ref blocks is the most likely candidate 
    /// and is therefore the best initial starting point. The predicted distortion is referenced for
    /// early termination factors.
    int predX, predY; 
    int predD = 0;
    _pMVPred->Get16x16Prediction(NULL, vecPos, &predX, &predY, &predD);
    int predXQuart  = predX % 4;
    int predYQuart  = predY % 4;
    int predX0      = predX / 4;  ///< Nearest level 0 pred motion vector.
    int predY0      = predY / 4;
    int orgPredX0 = predX0;
    int orgPredY0 = predY0;

    /// Truncate the predicted mv to be within the extended bounds of the frame.
    if ((predX0 + n) > _imgWidth)
      predX0 = _macroBlkWidth;
    if ((predX0 + n) < -_macroBlkWidth)
      predX0 = -_macroBlkWidth;
    if ((predY0 + m) > _imgHeight)
      predY0 = _macroBlkHeight;
    if ((predY0 + m) < -_macroBlkHeight)
      predY0 = -_macroBlkHeight;
    bool isTruncated = ( (orgPredX0 != predX0)||(orgPredY0 != predY0) );

    /// From the mb [0, 0] position determine the full pel search range permitted for the mv search points.
    GetMotionRange(n, m, 0, 0, &xlRng, &xrRng, &yuRng, &ydRng, mRng);

    /// Search on the predicted full pel motion vector point. 
    _pExtRefOver->SetOrigin(n + predX0, m + predY0);    ///< [predX0, predY0]
#ifdef MEH264IUMHS_ABS_DIFF
    int predVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
    int predVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif

    /// Default the best mv to the predicted mv.
    int minDiff = predVecDiff; mx = predX0; my = predY0;
    int minCost = predVecDiff / 256;

		/// Search on the zero full pel mv point if the pred mv is not also the zero mv.
    if (predX0 || predY0)
    {
      _pExtRefOver->SetOrigin(n, m);                     ///< [0, 0]
#ifdef MEH264IUMHS_ABS_DIFF
      int zeroVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
      int zeroVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif

      /// Select the best starting point full pel motion vector.
      int zeroCost = MEH264IUMHS_COST(zeroVecDiff, 0, 0, predX0, predY0);
      if ( zeroCost < minCost)
      { minDiff = zeroVecDiff; minCost = zeroCost; mx = 0; my = 0;}//end if zeroCost...
    }//end if predX0...

    //int bestDiff = minDiff;
    //int bestX = mx;
    //int bestY = my;
    int strrmx; 
    int strrmy;
    int priorMinDiff;
    /// Search on the aligned mb previous frame mv if it is not zero or equal to the pred mv. Ignore prev
    /// intra encoded mbs. 
    if (!_pPrevFrmMBlk[vecPos]._intraFlag)
    {
      int prevX0 = _pPrevFrmMBlk[vecPos]._mvX[0] / 4; ///< Convert from 1/4 pel res to full pel res.
      int prevY0 = _pPrevFrmMBlk[vecPos]._mvY[0] / 4;
      if ((prevX0 || prevY0) && (prevX0 != predX0) && (prevY0 != predY0))
      {
        _pExtRefOver->SetOrigin(n + prevX0, m + prevY0);    ///< [prevX0, prevY0]
#ifdef MEH264IFHS_ABS_DIFF
        int prevVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
        int prevVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif
        /// Check if this is a better full pel starting point.
        int prevCost = MEH264IUMHS_COST(prevVecDiff, prevX0, prevY0, predX0, predY0);
        if (prevCost < minCost)
        { minDiff = prevVecDiff; minCost = prevCost; mx = prevX0; my = prevY0; }//end if prevCost...
      }//end if prevX0...

    }//end if !_intraFlag...

    ///------------ 1st predicted mv Early Termination exit test to full pel local refinement searchs -----------
    /// Absolute thresholding used. In addition, if the predicted mv is the best initial mv and if its distortion
    /// is within 20% of the predicted distortion (i.e. the prediction is accurate) then assume the pred mv is
    /// the most likley mv and jump to the local refinement.
    if(minDiff < 1000)  goto MEH264IUMHS_EXTENDED_DIAMOND_SEARCH;
    else if ((minDiff < 4000) || ((minDiff == predVecDiff) && (minDiff < (predD * 12 / 10)) && (minDiff >(predD * 8 / 10))))
      goto MEH264IUMHS_EXTENDED_HEX_SEARCH;      /// Go to hexigon & diamond search

    ///--------------------------- Full pel unsymmetrical cross search ------------------------------------------
    /// Reset the refinement offset from mv [mx,my].
    rmx = 0; rmy = 0;
    priorMinDiff = minDiff;

    for (int w = 1; w < 16; w += 2) ///< Cross postion multiplier from the centre. Limit the full pel offset range to +/-16.
    {
      for (int x = 0; x < MEH264IUMHS_MOTION_CROSS_POS_LENGTH; x++)
      {
        i = w * MEH264IUMHS_CrossPos[x].y;
        j = w * MEH264IUMHS_CrossPos[x].x;
        /// Check that this offset is within the range of the frame boundaries and the vertical range is within half the horiz range.
        if ( (i < mRng/2) && (i > -mRng/2) && 
             ((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng) )
        {
          /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference point.
          _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
          //minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0, predY0, &rmx, &rmy, minDiff);
          /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef MEH264IUMHS_ABS_DIFF
          int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
          int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
#endif
          if (blkDiff <= minDiff)  ///< Better candidate mv offset.
          {
            int lclCost = MEH264IUMHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
            if (lclCost < minCost)
            { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
          }//end if blkDiff...
        }//if i...
      }//end for x...
    }//end for w...

    /// Update centre of winning mv from the unsymmetrical cross search.
    mx += rmx; my += rmy;

    ///------------ 2nd unsymmetrical cross Early Termination exit test to full pel local refinement searchs ---
    /// For a successful cross search improvement mv by at least 10% and it is close to the initial mv then local refinement will
    /// find the same as the 5x5 and a wider multi-hexagon search is not necessary. An absolute base threshold is also used.
    if (minDiff < 1000) goto MEH264IUMHS_EXTENDED_DIAMOND_SEARCH;
    else if ((minDiff < 2000) || ((minDiff < (priorMinDiff * 9 / 10)) && (rmx || rmy) && (abs(rmx) <= 3) && (abs(rmy) <= 3)))
      goto MEH264IUMHS_EXTENDED_HEX_SEARCH; /// Go to hexigon & diamond search

    ///--------------------------- Full pel 5x5 rectangular full search ----------------------------------------
    /// Reset the offset from the new mv [mx,my].
    strrmx = rmx; 
    strrmy = rmy;
    priorMinDiff = minDiff;
    rmx = 0; rmy = 0;
    for (int w = 0; w < MEH264IUMHS_MOTION_5X5_POS_LENGTH; w++)
    {
      i = MEH264IUMHS_5x5Pos[w].y;
      j = MEH264IUMHS_5x5Pos[w].x;
      /// Check that this offset is within the range of the image boundaries. Exclude positions already 
      /// tested in the unsymmetrical cross search above.
      bool alreadyTested = (strrmx || strrmy) && (((strrmy == 0) && ((j == 2) || (j == -2))) || ((strrmx == 0) && ((i == 2) || (i == -2))));
      if ( (!alreadyTested) && ((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng) )
      {
        /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference point.
        _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
        //minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0, predY0, &rmx, &rmy, minDiff);
        /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef MEH264IUMHS_ABS_DIFF
        int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
        int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
#endif
        if (blkDiff <= minDiff)  ///< Better candidate mv offset.
        {
          int lclCost = MEH264IUMHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
          if (lclCost < minCost)
          { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
        }//end if blkDiff...
      }//end if !alreadyTested...
    }//end for w...

    /// The centre mv [mx,my] is only updated if there is an early exit. If the multi-hexigon search is 
    /// required then it continues from the best unsymmetrical cross search [mx,my] mv and not the 5x5 
    /// best offset.
    ///------------ 3rd 5x5 Early Termination exit test to full pel local refinement searchs ---------------------
    if (minDiff < 1000) goto MEH264IUMHS_EXTENDED_DIAMOND_SEARCH;

    ///--------------------------- Full pel uneven multi-hexagon grid search -------------------------------------

    /// Search for an improvement on the 5x5 search from the unsymmetrical cross [mx,my] position. (Can be paralleld with the 5x5 search.)
    priorMinDiff = minDiff;
    for (int w = 1; w < 4; w++) ///< Uneven hexagon multiplier from the centre. Limit the offset range to +/-16.
    {
      for (int x = 0; x < MEH264IUMHS_MOTION_HEX_POS_LENGTH; x++)
      {
        i = w * MEH264IUMHS_HexPos[x].y;
        j = w * MEH264IUMHS_HexPos[x].x;
        /// Check that this offset is within the range of the frame boundaries.
        if ( ((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng) )
        {
          /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
          _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
          //minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0, predY0, &rmx, &rmy, minDiff);
          /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef MEH264IUMHS_ABS_DIFF
          int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
          int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
#endif
          if (blkDiff <= minDiff)  ///< Better candidate mv offset.
          {
            int lclCost = MEH264IUMHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
            if (lclCost < minCost)
            { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
          }//end if blkDiff...
        }//if i...
      }//end for x...

      /// 4th early termination is tested after each scaled 16-point hexagon pattern.
      if ((rmx || rmy) && ((minDiff < 2000) || (minDiff < (priorMinDiff/5))))
        break; ///< Effectively = goto MEH264IUMHS_EXTENDED_HEX_SEARCH; /// Go to hexigon & diamond search.

    }//end for w...

    /// If there was no early exit then update centre of the best mv from the completed 5x5 and uneven hexagon searches.
    mx += rmx; my += rmy;

    ///--------------------------- Full pel local refinement extended hexagon search -------------------------------------
    MEH264IUMHS_EXTENDED_HEX_SEARCH:

    do   ///< ...until the centre mv of the small hexagon is the best choice.
    {
      rmx = 0; rmy = 0;
      for (int x = 0; x < MEH264IUMHS_MOTION_EXTHEX_POS_LENGTH; x++)
      {
        i = MEH264IUMHS_ExtHexPos[x].y;
        j = MEH264IUMHS_ExtHexPos[x].x;
        /// Check that this offset is within the range of the frame boundaries.
        if (((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng))
        {
          /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
          _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
          //minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0, predY0, &rmx, &rmy, minDiff);
          /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef MEH264IUMHS_ABS_DIFF
          int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
          int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
#endif
          if (blkDiff <= minDiff)  ///< Better candidate mv offset.
          {
            int lclCost = MEH264IUMHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
            if (lclCost < minCost)
            { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
          }//end if blkDiff...
        }//if i...
      }//end for x...
      /// Readjust the best centre mv.
      mx += rmx; my += rmy;
    } while (rmx || rmy);

    ///--------------------------- Full pel local refinement extended diamond search -------------------------------------
    MEH264IUMHS_EXTENDED_DIAMOND_SEARCH:

    do   ///< ...until the centre mv of the diamond/cross is the best choice.
    {
      rmx = 0; rmy = 0;
      for (int x = 0; x < MEH264IUMHS_MOTION_CROSS_POS_LENGTH; x++)
      {
        i = MEH264IUMHS_CrossPos[x].y;
        j = MEH264IUMHS_CrossPos[x].x;
        /// Check that this offset is within the range of the frame boundaries.
        if (((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng))
        {
          /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
          _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
          //minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0, predY0, &rmx, &rmy, minDiff);
          /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef MEH264IUMHS_ABS_DIFF
          int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
          int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
#endif
          if (blkDiff <= minDiff)  ///< Better candidate mv offset.
          {
            int lclCost = MEH264IUMHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
            if (lclCost < minCost)
            { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
          }//end if blkDiff...
        }//if i...
      }//end for x...
      /// Readjust the best centre mv.
      mx += rmx; my += rmy;
    } while (rmx || rmy);

    /// ---------------------- Full Pel Measurements ---------------------------------------------
#ifdef MEH264IUMHS_TAKE_MEASUREMENTS
    if ((m == 144) && (n == 176))  /// Central MB only.
    {
      if (_mtPos < _mtLen)
      {
        _mt.WriteItem(0, _mtPos, predD);
        _mt.WriteItem(1, _mtPos, bestDiff);
        _mt.WriteItem(2, _mtPos, bestX);
        _mt.WriteItem(3, _mtPos, bestY);
        _mt.WriteItem(4, _mtPos, minDiff);
        _mt.WriteItem(5, _mtPos, mx);
        _mt.WriteItem(6, _mtPos, my);
        _mtPos++;
      }//end if _mtPos
    }//end if m...
#endif

		///----------------------- Quarter pel refined search ----------------------------------------
    /// Search around the min diff full pel motion vector on a 1/4 pel grid firstly on the
		/// 1/2 pel positions and then refine the winner on the 1/4 pel positions. 

		int mvx = mx << 2;	///< Convert to 1/4 pel units.
		int mvy = my << 2;

    if (_mode != 2)  ///< !full pel only.
    {
      /// Set the location to the min diff motion vector (mx,my).
      _pExtRefOver->SetOrigin(n + mx, m + my);

      /// Fill the 1/4 pel window with valid values only in the 1/2 pel positions.
      LoadHalfQuartPelWindow(_Win, _pExtRefOver);

      for (int x = 0; x < MEH264IUMHS_MOTION_SUB_POS_LENGTH; x++)
      {
        int qOffX = 2 * MEH264IUMHS_SubPos[x].x;
        int qOffY = 2 * MEH264IUMHS_SubPos[x].y;

        /// Read the half grid pels into temp.
        QuarterRead(_pMBlkOver, _Win, qOffX, qOffY);

#ifdef MEH264IUMHS_ABS_DIFF
        int blkDiff = _pInOver->Tad16x16LessThan(*_pMBlkOver, minDiff);
#else
        int blkDiff = _pInOver->Tsd16x16LessThan(*_pMBlkOver, minDiff);
        //int blkDiff = _pInOver->Tsd16x16PartialLessThan(*_pMBlkOver, minDiff);
        //int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pExtRefOver, (void *)MEH264IUMHS_LinearPath, _pathLength, minDiff);
        //int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pMBlkOver, (void *)MEH264IUMHS_OptimalPath, _pathLength, minDiff, 16);
        //    int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pMBlkOver, (void *)MEH264IUMHS_OptimalPath, _pathLength, minDiff);
#endif
        if (blkDiff < minDiff)
        {
          minDiff = blkDiff;
          hmx = qOffX;
          hmy = qOffY;
        }//end if blkDiff...
      }//end for x...

      qmx = hmx;
      qmy = hmy;

      if (_mode != 1)  ///< !half pel resolution
      {
        /// Fill the 1/4 pel positions around the winning 1/2 pel position (hmx,hmy).
        LoadQuartPelWindow(_Win, hmx, hmy);

        for (int x = 0; x < MEH264IUMHS_MOTION_SUB_POS_LENGTH; x++)
        {
          int qOffX = hmx + MEH264IUMHS_SubPos[x].x;
          int qOffY = hmy + MEH264IUMHS_SubPos[x].y;

          /// Read the quarter grid pels into temp.
          QuarterRead(_pMBlkOver, _Win, qOffX, qOffY);

#ifdef MEH264IUMHS_ABS_DIFF
          int blkDiff = _pInOver->Tad16x16LessThan(*_pMBlkOver, minDiff);
#else
          int blkDiff = _pInOver->Tsd16x16LessThan(*_pMBlkOver, minDiff);
          //int blkDiff = _pInOver->Tsd16x16PartialLessThan(*_pMBlkOver, minDiff);
          //int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pExtRefOver, (void *)MEH264IUMHS_LinearPath, _pathLength, minDiff);
          //int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pMBlkOver, (void *)MEH264IUMHS_OptimalPath, _pathLength, minDiff, 16);
          //      int blkDiff = _pInOver->Tsd16x16PartialPathLessThan(*_pMBlkOver, (void *)MEH264IUMHS_OptimalPath, _pathLength, minDiff);
#endif
          if (blkDiff < minDiff)
          {
            minDiff = blkDiff;
            qmx = qOffX;
            qmy = qOffY;
          }//end if blkDiff...
        }//end for x...
      }//end if !half pel...

      /// Add the refinement in 1/4 pel units.
      mvx += qmx;
      mvy += qmy;
    }//end if !full pel...

		///----------------------- Quarter pel pred vector ----------------------------
    /// Compare this winning mv with the predicted mv. 
    int reconstructPredX = (predX0 * 4) + predXQuart;
    int reconstructPredY = (predY0 * 4) + predYQuart;

    _pExtRefOver->SetOrigin(predX0 + n, predY0 + m);

    /// Get distortion at pred mv to quarter pel resolution. The predVecDiff holds the value 
    /// of the inital full pel distortion from the test for the starting position.

    /// Quarter read first if necessary.
    if (predXQuart || predYQuart)
    {
      /// Read the quarter grid pels into temp.
      _pExtRefOver->QuarterRead(*_pMBlkOver, predXQuart, predYQuart);
      /// Absolute/square diff comparison method.
#ifdef MEH264IUMHS_ABS_DIFF
      predVecDiff = _pInOver->Tad16x16(*_pMBlkOver);
#else
      predVecDiff = _pInOver->Tsd16x16(*_pMBlkOver);
#endif
    }//end if predXQuart...
    else if(isTruncated)  /// If the full pel pred mv was not truncated then it was already tested.
    {
#ifdef MEH264IUMHS_ABS_DIFF
      predVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
      predVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif
    }//end else if isTruncated...

    /// Test the searched best mv cost against the pred mv cost at quarter pel values. Note that
    /// the weighting of the euclidian distance is 4 times greater (from quarter pel multiplier
    /// effect).
    if ((MEH264IUMHS_COST(predVecDiff, reconstructPredX, reconstructPredY, predX, predY)) <= (MEH264IUMHS_COST(minDiff, mvx, mvy, predX, predY)))
    {
      minDiff = predVecDiff;
      mvx = reconstructPredX;
      mvy = reconstructPredY;
    }//end if predVecDiff...

    /// Check for inclusion in the distortion calculation.
    if (_pDistortionIncluded != NULL)
    {
      if (_pDistortionIncluded[vecPos])
      {
        included++;
        totalDifference += minDiff;
      }//end if _pDistortionIncluded...
    }//end if _pDistortionIncluded...

		/// Load the selected vector coord.
		if(vecPos < maxLength)
		{
			_pMotionVectorStruct->SetSimpleElement(vecPos, 0, mvx);
			_pMotionVectorStruct->SetSimpleElement(vecPos, 1, mvy);
      /// Set macroblock vector for future predictions.
      _pMVPred->Set16x16MotionVector(vecPos, mvx, mvy, minDiff);
			vecPos++;
		}//end if vecPos...

  }//end for m & n...

	/// In this context avg distortion is actually avg difference.
//	*avgDistortion = totalDifference/maxLength;
	if(included)	///< Prevent divide by zero error.
		*avgDistortion = totalDifference/included;
	else
		*avgDistortion = 0;
	return((void *)_pMotionVectorStruct);

}//end Estimate.

/*
--------------------------------------------------------------------------
  Private methods. 
--------------------------------------------------------------------------
*/

void MotionEstimatorH264ImplUMHS::Destroy(void)
{
	_ready = 0;

#ifdef MEH264IUMHS_TAKE_MEASUREMENTS
  if(_mtPos > 0)
    _mt.Save("C:/Users/KFerguson/Google Drive/PC/Excel/MotionEvaluation/experiment.csv", ",", 1);
#endif

	if(_Win != NULL)
		delete _Win;
	_Win = NULL;
	if(_pWin != NULL)
		delete[] _pWin;
	_pWin = NULL;

	if(_pInOver != NULL)
		delete _pInOver;
	_pInOver = NULL;

	if(_pRefOver != NULL)
		delete _pRefOver;
	_pRefOver	= NULL;

	if(_pExtRef != NULL)
		delete[] _pExtRef;
	_pExtRef = NULL;

	if(_pExtRefOver != NULL)
		delete _pExtRefOver;
	_pExtRefOver = NULL;

	if(_pMBlk != NULL)
		delete[] _pMBlk;
	_pMBlk = NULL;

	if(_pMBlkOver != NULL)
		delete _pMBlkOver;
	_pMBlkOver = NULL;

	if(_pMotionVectorStruct != NULL)
		delete _pMotionVectorStruct;
	_pMotionVectorStruct = NULL;

}//end Destroy.

/** Test a single full pel motion vector position with offsets.
In-line code refactoring method.
*/
//minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0Rnd, predY0Rnd, *rmx, *rmy, minDiff);
int MotionEstimatorH264ImplUMHS::TestForBetterCandidateMotionVec(int currx, int curry, int testx, int testy, int basemvx, int basemvy, int* offx, int* offy, int CurrMin)
{
  int d = CurrMin;
  int rmx = *offx;
  int rmy = *offy;

  /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (early return).
#ifdef MEH264IUMHS_ABS_DIFF
  int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, d);
#else
  int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, d);
#endif
  if (blkDiff <= d)  ///< Better candidate mv offset.
  {
    if ( (MEH264IUMHS_COST(blkDiff, currx + testx, curry + testy, basemvx, basemvy)) < (MEH264IUMHS_COST(d, currx + rmx, curry + rmy, basemvx, basemvy))  )
    { d = blkDiff; rmx = testx; rmy = testy; }//end if blkDiff...
  }//end if blkDiff...

  *offx = rmx; *offy = rmy;
  return(d);
}//end TestForBetterCandidateMotionVec.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The returned values are offset limits from (x,y) image coordinate.
@param x				: X coord of block.
@param y				: Y coord of block.
@param xlr			: Returned allowed left range offset from x.
@param xrr			: Returned allowed right range offset from x.
@param yur			: Returned allowed up range offset from y.
@param ydr			: Returned allowed down range offset from y.
@param range		: Desired range of motion.
@return					: none.
*/
void MotionEstimatorH264ImplUMHS::GetMotionRange( int  x,			int  y, 
																								  int* xlr,		int* xrr, 
																									int* yur,		int* ydr,
																									int	 range)
{
	int boundary	= _extBoundary - MEH264IUMHS_PADDING;
	int	width			= _imgWidth;
	int	height		= _imgHeight;

	if( (x - range) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -range;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + range) < width )	///< Rest of block extends into the bounday region.
		*xrr = range;
	else
		*xrr = width - x;

	if( (y - range) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -range;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + range) < height )	///< Rest of block extends into the bounday region.
		*ydr = range;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The range is further checked to ensure that the motion vector is within 
its defined max range. The returned values are offset limits from the (xpos+xoff,ypos+yoff) 
image coordinates.
@param xpos			: X coord of block in the image.
@param ypos			: Y coord of block in the image.
@param xoff			: Current offset (vector) from xpos.
@param yoff			: Current offset (vector) from ypos.
@param xlr			: Returned allowed left range offset from xpos+xoff.
@param xrr			: Returned allowed right range offset from xpos+xoff.
@param yur			: Returned allowed up range offset from ypos+yoff.
@param ydr			: Returned allowed down range offset from ypos+yoff.
@param range		: Desired range of motion.
@return					: none.
*/
void MotionEstimatorH264ImplUMHS::GetMotionRange(	int  xpos,	int  ypos,
																									int	 xoff,	int  yoff,
																									int* xlr,		int* xrr, 
																									int* yur,		int* ydr,
																									int	 range)
{
	int x = xpos + xoff;
	int y = ypos + yoff;

	int xLRange, xRRange, yURange, yDRange;

	int boundary	= _extBoundary - MEH264IUMHS_PADDING;
	int	width			= _imgWidth;
	int	height		= _imgHeight;
	int	vecRange	= _motionRange/4;	///< Convert 1/4 pel range to full pel units.

	/// Limit the range of the motion vector.
	if( (xoff - range) > -vecRange )
		xLRange = range;
	else
		xLRange = (vecRange-1) + xoff;
	if( (xoff + range) < vecRange )
		xRRange = range;
	else
		xRRange = (vecRange-1) - xoff;
	if( (yoff - range) > -vecRange )
		yURange = range;
	else
		yURange = (vecRange-1) + yoff;
	if( (yoff + range) < vecRange )
		yDRange = range;
	else
		yDRange = (vecRange-1) - yoff;

	if( (x - xLRange) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -xLRange;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + xRRange) < width )	///< Rest of block extends into the bounday region.
		*xrr = xRRange;
	else
		*xrr = width - x;

	if( (y - yURange) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -yURange;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + yDRange) < height )	///< Rest of block extends into the bounday region.
		*ydr = yDRange;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** Load a 1/4 pel window with 1/2 pel values.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. This window is used in a cascading 
approach to motion estimation where the best 1/2 pel search around a winning full pel is done first followed 
by the best 1/4 pel around the winning 1/2 pel position. This window is used for the input for the first stage
and therefore only the 1/2 pel positions are valid. This method must be followed by the LoadQuartPelWindow()
method to complete the 1/4 pel values around a winning 1/2 pel position. The reference origin position is 
aligned onto the full pel (3,3) position of the 1/4 pel window.
@param qPelWin	: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param extRef		: Reference to derive the 1/4 pel window from.
@return					: none.
*/
void MotionEstimatorH264ImplUMHS::LoadHalfQuartPelWindow(OverlayMem2Dv2* qPelWin, OverlayMem2Dv2* extRef)
{
	int fullRow, fullCol, quartRow, quartCol, refRow, refCol;

	int			width		= qPelWin->GetWidth()/4;	///< Convert to full pel units.
	int			height	= qPelWin->GetHeight()/4;
	short** window	= qPelWin->Get2DSrcPtr();

	short** ref			= extRef->Get2DSrcPtr();
	int			refXPos = extRef->GetOriginX();
	int			refYPos = extRef->GetOriginY();

	/// Set all the "h" half pel values in the window only at the positions that will be required for the other calcs. No
	/// scaling or clipping is performed until "j" half pel values are completed.
	for(fullRow = 2, quartRow = 10, refRow = refYPos - 1; fullRow < (_macroBlkHeight + 3); fullRow++, quartRow += 4, refRow++)
	{
		for(fullCol = 0, quartCol = 0, refCol = refXPos - 3; fullCol < width; fullCol++, quartCol += 4, refCol++)
		{
			int h =     (int)ref[refRow-2][refCol] -  5*(int)ref[refRow-1][refCol] + 
							 20*(int)ref[refRow][refCol]   + 20*(int)ref[refRow+1][refCol] - 
							  5*(int)ref[refRow+2][refCol] +    (int)ref[refRow+3][refCol];

			window[quartRow][quartCol] = (short)(h);
		}//end for fullCol...
	}//end fullRow...
	
	/// Set all the "b" half pel values in the window only at the positions that will be required for the other calcs. No
	/// scaling or clipping is performed until "j" half pel values are completed.
	for(fullRow = 0, quartRow = 0, refRow	= refYPos - 3; fullRow < height; fullRow++, quartRow += 4, refRow++)
	{
		for(fullCol = 2, quartCol = 10, refCol = refXPos - 1; fullCol < (_macroBlkWidth + 3); fullCol++, quartCol += 4, refCol++)
		{
			int b =     (int)ref[refRow][refCol-2] -  5*(int)ref[refRow][refCol-1] + 
							 20*(int)ref[refRow][refCol]   + 20*(int)ref[refRow][refCol+1] - 
							  5*(int)ref[refRow][refCol+2] +    (int)ref[refRow][refCol+3];

			window[quartRow][quartCol] = (short)(b);
		}//end for fullCol...
	}//end fullRow...

	/// For the "j" half pel values, use the previously calculated "h" and "b" values only in the positions
	/// surrounding the centre of the reference image origin. Now scaling is included for j.
	for(fullRow = 2, quartRow = 10; fullRow < (_macroBlkHeight + 3); fullRow++, quartRow += 4)
	{
		for(fullCol = 2, quartCol = 10; fullCol < (_macroBlkWidth + 3); fullCol++, quartCol += 4)
		{
			int j = (   (int)window[quartRow][quartCol-10] -  5*(int)window[quartRow][quartCol-6] + 
							 20*(int)window[quartRow][quartCol-2]  + 20*(int)window[quartRow][quartCol+2] - 
							  5*(int)window[quartRow][quartCol+6]  +    (int)window[quartRow][quartCol+10] + 512) >> 10;

			window[quartRow][quartCol] = (short)(MEH264IUMHS_CLIP255(j));
		}//end for fullCol...
	}//end fullRow...

	/// Scale and clip the useful "h" and "b" half pel values in place.
	for(fullRow = 2, refRow	= refYPos - 1, quartRow = 8; fullRow < (_macroBlkHeight + 3); fullRow++, refRow++, quartRow += 4)
	{
		for(fullCol = 2, refCol = refXPos - 1, quartCol = 8; fullCol < (_macroBlkWidth + 3); fullCol++, refCol++, quartCol += 4)
		{
			/// "h"
			window[quartRow+2][quartCol] = MEH264IUMHS_CLIP255((window[quartRow+2][quartCol] + 16) >> 5);
			/// "b"
			window[quartRow][quartCol+2] = MEH264IUMHS_CLIP255((window[quartRow][quartCol+2] + 16) >> 5);
			/// Copy full pel "G"
			window[quartRow][quartCol] = ref[refRow][refCol];
		}//end for fullCol...

    /// One further "h" and "G" col at the end of the row.
		/// "h"
		window[quartRow+2][quartCol] = MEH264IUMHS_CLIP255((window[quartRow+2][quartCol] + 16) >> 5);
		/// Copy full pel "G"
		window[quartRow][quartCol] = ref[refRow][refCol];

	}//end fullRow...

   /// One further "b" and "G" row at the end.
	for(fullCol = 2, refCol = refXPos - 1, quartCol = 8; fullCol < (_macroBlkWidth + 3); fullCol++, refCol++, quartCol += 4)
	{
		/// "b"
		window[quartRow][quartCol+2] = MEH264IUMHS_CLIP255((window[quartRow][quartCol+2] + 16) >> 5);
		/// Copy full pel "G"
		window[quartRow][quartCol] = ref[refRow][refCol];
	}//end for fullCol...

  /// ...and one final "G" full pel at the bottom right edge.
	window[quartRow][quartCol] = ref[refRow][refCol];

}//end LoadHalfQuartPelWindow.

/** Load a 1/4 pel window with 1/4 pel values not in 1/2 pel positions.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. This window is used in a cascading 
approach to motion estimation where the best 1/2 pel search around a winning full pel is done first followed 
by the best 1/4 pel around the winning 1/2 pel position. This window is used for the input for the second stage
and therefore the 1/2 pel positions are valid from a previous (first stage) call to the LoadHalfQuartPelWindow() 
method. This method will complete the 1/4 pel values around a winning 1/2 pel position. The reference origin 
position is aligned onto the full pel (3,3) position of the 1/4 pel window. The 1/4 pel values are processed
from the values already in the window.
@param qPelWin		: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param hPelColOff	: The winning 1/2 pel X offset from the (3,3) window position in 1/4 pel units.
@param hPelRowOff	: The winning 1/2 pel Y offset from the (3,3) window position in 1/4 pel units.
@return						: none.
*/
void MotionEstimatorH264ImplUMHS::LoadQuartPelWindow(OverlayMem2Dv2* qPelWin, int hPelColOff, int hPelRowOff)
{
	int fullRow, fullCol, quartRow, quartCol;

	/// The 1/4 pels are to calculated for the 8 positions surrounding the 1/2 location at (hPelRowOff,hPelColOff). Note
	/// that hPelRowOff and hPelColOff are still in 1/4 pel units i.e. = range [-2,0,2][-2,0,2].

	short** window	= qPelWin->Get2DSrcPtr();

	for(fullRow = 0, quartRow = (12 + hPelRowOff); fullRow < _macroBlkHeight; fullRow++, quartRow += 4)
	{
		for(fullCol = 0, quartCol = (12 + hPelColOff); fullCol < _macroBlkWidth; fullCol++, quartCol += 4)
		{
			window[quartRow-1][quartCol-1]	= (window[quartRow-2][quartCol-2] + window[quartRow][quartCol] + 1) >> 1;
			window[quartRow-1][quartCol]		= (window[quartRow-2][quartCol]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow-1][quartCol+1]	= (window[quartRow-2][quartCol]		+ window[quartRow][quartCol+2] + 1) >> 1;
			window[quartRow][quartCol-1]		= (window[quartRow][quartCol-2]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow][quartCol+1]		= (window[quartRow][quartCol+2]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol-1]	= (window[quartRow][quartCol-2]		+ window[quartRow+2][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol]		= (window[quartRow+2][quartCol]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol+1]	= (window[quartRow+2][quartCol]		+ window[quartRow][quartCol+2] + 1) >> 1;
		}//end for fullCol...
	}//end fullRow...

}//end LoadQuartPelWindow.

/** Read 1/4 pels from window.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. Read a block from the window centred
on the (3,3) position with a 1/4 pel offset given by the input params into the destination block.
@param dstBlock		: Destination block.
@param qPelWin		: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param qPelColOff	: The 1/4 pel X offset from the (3,3) window position in 1/4 pel units.
@param qPelRowOff	: The 1/4 pel Y offset from the (3,3) window position in 1/4 pel units.
@return						: none.
*/
void MotionEstimatorH264ImplUMHS::QuarterRead(OverlayMem2Dv2* dstBlock, OverlayMem2Dv2* qPelWin, int qPelColOff, int qPelRowOff)
{
	int fullRow, fullCol, quartRow, quartCol, dstX;

	short** window	= qPelWin->Get2DSrcPtr();	/// The origin of the window is around full pel position (3,3).

	short** dst			= dstBlock->Get2DSrcPtr();
	int			width		= dstBlock->GetWidth();
	int			height	= dstBlock->GetHeight();
	int			dstXPos = dstBlock->GetOriginX();
	int			dstYPos = dstBlock->GetOriginY();

	/// 1/4 pel rows and cols increment in quarter pel units with (3,3) full pel offset + input offset. As
	/// in: quartRow	= ((fullRow + 3) * 4) + qPelRowOff; and quartCol	= ((fullCol + 3) * 4) + qPelColOff;

	for(fullRow = 0, quartRow = (12 + qPelRowOff); fullRow < height; fullRow++, dstYPos++, quartRow += 4)
	{
		for(fullCol = 0, quartCol = (12 + qPelColOff), dstX = dstXPos; fullCol < width; fullCol++, quartCol += 4, dstX++)
			dst[dstYPos][dstX] = window[quartRow][quartCol];
	}//end fullRow...

}//end QuarterRead.






