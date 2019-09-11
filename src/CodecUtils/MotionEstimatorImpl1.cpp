/** @file

MODULE						: MotionEstimatorImpl1

TAG								: MEI1

FILE NAME					: MotionEstimatorImpl1.h

DESCRIPTION				: A standard motion estimator implementation of squared
										error distortion measure.	Access via a IMotionEstimator
										interface.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
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


#include	"SimpleMotionVectorList.h"
#include	"MotionEstimatorImpl1.h"

/*
--------------------------------------------------------------------------
  Constants. 
--------------------------------------------------------------------------
*/
// Calc = ((16[vec dim] * 16[vec dim]) * 2.
#define MEI1_FULL_MOTION_NOISE_FLOOR 512

// Search range coords for centre motion vectors.
#define MEI1_MOTION_HALF_POS_LENGTH 	8
MEI1_COORD MEI1_HalfPos[MEI1_MOTION_HALF_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

// Search range coords for corner motion vectors.
#define MEI1_MOTION_TOP_LEFT_HALF_POS_LENGTH 3		// Top left search range.
MEI1_COORD MEI1_TopLeftHalfPos[MEI1_MOTION_TOP_LEFT_HALF_POS_LENGTH]	= 
{
	{1,0},{0,1},{1,1}
};

#define MEI1_MOTION_TOP_RIGHT_HALF_POS_LENGTH 3	// Top right search range.
MEI1_COORD MEI1_TopRightHalfPos[MEI1_MOTION_TOP_RIGHT_HALF_POS_LENGTH]	= 
{
	{-1,0},{-1,1},{0,1}
};

#define MEI1_MOTION_BOTTOM_LEFT_HALF_POS_LENGTH 3 // Bottom left search range.
MEI1_COORD MEI1_BottomLeftHalfPos[MEI1_MOTION_BOTTOM_LEFT_HALF_POS_LENGTH]	= 
{
	{0,-1},{1,-1},{1,0}
};

#define MEI1_MOTION_BOTTOM_RIGHT_HALF_POS_LENGTH 3	//Bottom right search range.
MEI1_COORD MEI1_BottomRightHalfPos[MEI1_MOTION_BOTTOM_RIGHT_HALF_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{-1,0}
};

// Search range coords for edge motion vectors.
#define MEI1_MOTION_LEFT_HALF_POS_LENGTH 5	// Left edge search range.
MEI1_COORD MEI1_LeftHalfPos[MEI1_MOTION_LEFT_HALF_POS_LENGTH]	= 
{
	{0,-1},{1,-1},{1,0},{0,1},{1,1}
};

#define MEI1_MOTION_RIGHT_HALF_POS_LENGTH 5	// Right edge search range.
MEI1_COORD MEI1_RightHalfPos[MEI1_MOTION_RIGHT_HALF_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{-1,0},{-1,1},{0,1}
};

#define MEI1_MOTION_TOP_HALF_POS_LENGTH 5	// Top edge search range.
MEI1_COORD MEI1_TopHalfPos[MEI1_MOTION_TOP_HALF_POS_LENGTH]	= 
{
	{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

#define MEI1_MOTION_BOTTOM_HALF_POS_LENGTH 5	// Bottom edge search range.
MEI1_COORD MEI1_BottomHalfPos[MEI1_MOTION_BOTTOM_HALF_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0}
};

/*
--------------------------------------------------------------------------
  Construction. 
--------------------------------------------------------------------------
*/

MotionEstimatorImpl1::MotionEstimatorImpl1(	const void* pSrc, 
																						const void* pRef, 
																						int					imgWidth, 
																						int					imgHeight,
																						int					macroBlkWidth,
																						int					macroBlkHeight,
																						int					motionRange)
{
	_ready	= 0;	// Ready to estimate.
	_mode		= 0;	// Speed mode or whatever.

	// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= imgWidth;					// Width of the src and ref images. 
	_imgHeight			= imgHeight;				// Height of the src and ref images.
	_macroBlkWidth	= macroBlkWidth;		// Width of the motion block.
	_macroBlkHeight	= macroBlkHeight;		// Height of the motion block.
	_motionRange		= motionRange;			// (x,y) range of the motion vectors.
	_pInput					= pSrc;
	_pRef						= pRef;

	// Input mem overlay members.
	_pInOver				= NULL;				// Input overlay with motion block dim.

	// Ref mem overlay members.
	_pRefOver				= NULL;				// Ref overlay with whole block dim.
	_pExtRef				= NULL;				// Extended ref mem created by ExtendBoundary() call.
	_extWidth				= 0;
	_extHeight			= 0;
	_pExtRefOverAll	= NULL;				// Extended ref overlay with whole block dim.
	_pExtRefOver		= NULL;				// Extended ref overlay with motion block dim.

	// Temp working block and its overlay.
	_pMBlk					= NULL;						// Motion block temp mem.
	_pMBlkOver			= NULL;				// Motion block overlay of temp mem.

	// Hold the resulting motion vectors in a byte array.
	_pMotionVectorStruct = NULL;

}//end constructor.

MotionEstimatorImpl1::~MotionEstimatorImpl1(void)
{
	Destroy();
}//end destructor.

/*
--------------------------------------------------------------------------
  Public IMotionEstimator Interface. 
--------------------------------------------------------------------------
*/

int MotionEstimatorImpl1::Create(void)
{
	// Clean out old mem.
	Destroy();

	// --------------- Configure input overlays --------------------------------
	// Put an overlay on the input with the block size set to the vector dim. This
	// is used to access input vectors.
	_pInOver = new OverlayMem2Dv2((void *)_pInput, 
															_imgWidth, 
															_imgHeight, 
															_macroBlkWidth, 
															_macroBlkHeight);
	if(_pInOver == NULL)
	{
		Destroy();
		return(0);
	}//end _pInOver...

	// --------------- Configure ref overlays --------------------------------
	// Overlay the whole reference. The reference will have an extended 
	// boundary for motion estimation and must therefore have its own mem.
	_pRefOver = new OverlayMem2Dv2((void *)_pRef, _imgWidth, _imgHeight, _imgWidth, _imgHeight);
	if(_pRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pRefOver...

	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRef, 
																			_imgWidth,						
																			_imgHeight, 
																			_motionRange + 1, // Extend left and right by...
																			_motionRange + 1,	// Extend top and bottom by...
																			(void **)(&_pExtRef)) )	// Created in the method.
  {
		Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extWidth	 = _imgWidth + (_motionRange + 1)*2;
	_extHeight = _imgHeight + (_motionRange + 1)*2;

	// Place the overlay onto the whole ext boundary mem.
	_pExtRefOverAll = new OverlayExtMem2Dv2(_pExtRef, 
																				_extWidth,				// new mem width.
																				_extHeight,				// new mem height.
																				_imgWidth,				// block width.
																				_imgHeight,				// block height.
																				_motionRange + 1,	// boundary width.
																				_motionRange + 1);// boundary height.

	// Place another overlay on the extended boundary ref with block size set to the
	// motion vec dim..
	_pExtRefOver = new OverlayExtMem2Dv2(_pExtRef,					// Src description. 
																		 _extWidth, 
																		 _extHeight,
																		 _macroBlkWidth,		// Block size description.
																		 _macroBlkHeight,
																		 _motionRange + 1,	// Boundary size for both left and right.
																		 _motionRange + 1 );
	if( (_pExtRefOverAll == NULL)||(_pExtRefOver == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pExtRefOverAll...

	// --------------- Configure temp overlays --------------------------------
	// Alloc some temp mem and overlay it to use for half pel motion estimation and 
	// compensation. The block size is the same as the mem size.
	_pMBlk = new short[_macroBlkWidth * _macroBlkHeight];
	_pMBlkOver = new OverlayMem2Dv2(_pMBlk, _macroBlkWidth, _macroBlkHeight, 
																				_macroBlkWidth, _macroBlkHeight);
	if( (_pMBlk == NULL)||(_pMBlkOver == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pMBlk...

	// --------------- Configure result ---------------------------------------
	// The structure container for the motion vectors.
	_pMotionVectorStruct = new SimpleMotionVectorList();
	if(_pMotionVectorStruct != NULL)
	{
		// How many motion vectors will there be at the block dim.
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

	_ready = 1;
	return(1);
}//end Create.

void	MotionEstimatorImpl1::Reset(void)
{
}//end Reset.

void	MotionEstimatorImpl1::SetMode(int mode)
{
	_mode = mode;
}//end SetMode.

/** Motion estimate the source within the reference.
Do the estimation with the block sizes and image sizes defined in
the implementation. The returned type holds the vectors. This is
a standard block full search algorithm with extended boundaries.
@param pSrc		: Input image to estimate (not used).
@param pRef		: Ref to estimate with (not used).
@return				: The list of motion vectors.
*/
void* MotionEstimatorImpl1::Estimate(long* avgDistortion)
{
  int		x,y,m,n;
	// Set the motion vector byte storage structure.
	char* pMv				= (char *)(_pMotionVectorStruct->GetDataPtr());
	int		maxLength	= _pMotionVectorStruct->GetLength();
	int   step			= _pMotionVectorStruct->GetPatternSize();
	int		length		= 0;

	// Accumulate the total error energy for this estimation.
	long	totalEnergy = 0;

	// Write the reference to the ext boundary mem and then fill the boundary.
	_pExtRefOverAll->Write(*_pRefOver);
	_pExtRefOverAll->FillBoundaryProxy();

  // Do estimation in the sequence order.	Gather the motion vector 
	// MSE data and choose the vector.
  for(m = 0; m < _imgHeight; m += _macroBlkHeight)
		for(n = 0; n < _imgWidth; n += _macroBlkWidth)
  {
		int mx	= 0;
		int my	= 0;
		int hmx = 0;
		int hmy = 0;

		// Set the input block to work with.
		_pInOver->SetOrigin(n,m);

		// Measure the [0,0] motion vector as a min reference to beat for 
		// all other motion vectors.
		_pExtRefOver->SetOrigin(n,m);
		int zeroVecEnergy = _pInOver->Tsd(*_pExtRefOver);

		// [0,0] motion vector is the best so far.
		int minEnergy = zeroVecEnergy;

    // Search on a full pel grid over the defined motion range.
    for(y = -(_motionRange); y < _motionRange; y++)
    {
      for(x = -(_motionRange); x < _motionRange; x++)
      {
				// Set the block to the [x,y] motion vector around the [n,m] reference
				// location.
				_pExtRefOver->SetOrigin(n+x, m+y);

				int vecEnergy = _pInOver->TsdLessThan(*_pExtRefOver, minEnergy);
				if(vecEnergy < minEnergy)
				{
					minEnergy = vecEnergy;
					mx = x;
					my = y;
				}//end if vecEnergy...
      }//end for x...
    }//end for y...

		// Set the location to the min diff motion vector [mx,my].
		_pExtRefOver->SetOrigin(n+mx, m+my);

		// Adjust the search range to stay within the global bounds.
		MEI1_COORD* pHalfPos;
		int len = GetHalfPelSearchStruct(mx, my, (MEI1_COORD **)(&pHalfPos));

    // Search around the min energy full pel motion vector on a half pel grid.
    for(x = 0; x < len; x++)
    {
			// Set the location to the min energy motion vector [mx,my].
			_pExtRefOver->SetOrigin(n+mx, m+my);
			// Read the half grid pels into temp.
			_pExtRefOver->HalfRead(*_pMBlkOver, pHalfPos[x].x, pHalfPos[x].y); 
			int vecEnergy = _pInOver->TsdLessThan(*_pMBlkOver, minEnergy);
			if(vecEnergy < minEnergy)
			{
				minEnergy = vecEnergy;
				hmx = pHalfPos[x].x;
				hmy = pHalfPos[x].y;
			}//end if vecEnergy...
    }//end for x...
  
		// Motion vectors are described in half pel units.
    int mvx = (mx << 1) + hmx;
    int mvy = (my << 1) + hmy;

		// Bounds check used for debugging.
		//if(mvx < -64) 
		//	mvx = -64;
		//else if(mvx > 63)
		//	mvx = 63;
		//if(mvy < -64) 
		//	mvy = -64;
		//else if(mvy > 63)
		//	mvy = 63;

		// Validity of the motion vector is weighted with non-linear factors.
		int weight							= 0;
		int diffWithZeroEnergy	= zeroVecEnergy - minEnergy;
		int magSqr							= (mvx * mvx) + (mvy * mvy);

		// Contribute if motion vector is small.
		if((diffWithZeroEnergy >> 2) < magSqr)
			weight++;
		// Contribute if same order as the noise.
		if(zeroVecEnergy < MEI1_FULL_MOTION_NOISE_FLOOR)
			weight++;
		// Contribute if the zero vector and min energy vector are similar.
		if((diffWithZeroEnergy * 10) < minEnergy)
			weight++;

		// Determine the basic run-length motion.
		if((minEnergy < zeroVecEnergy)&&(weight < 2))
    {
			totalEnergy += minEnergy;
    }//end if minEnergy...
    else
		{
			mvx = 0;
			mvy = 0;
			totalEnergy += zeroVecEnergy;
		}//end else...

		// Load the selected vector coord.
		if(length < maxLength)
		{
			*pMv			= (char)mvx;
			*(pMv+1)	= (char)mvy;
			pMv += step;
			length++;
		}//end if length...

  }//end for m & n...

	*avgDistortion = totalEnergy/maxLength;
	return((void *)_pMotionVectorStruct);
}//end Estimate.

/*
--------------------------------------------------------------------------
  Private methods. 
--------------------------------------------------------------------------
*/

void MotionEstimatorImpl1::Destroy(void)
{
	_ready = 0;

	if(_pInOver != NULL)
		delete _pInOver;
	_pInOver = NULL;

	if(_pRefOver != NULL)
		delete _pRefOver;
	_pRefOver	= NULL;

	if(_pExtRef != NULL)
		delete[] _pExtRef;
	_pExtRef = NULL;

	if(_pExtRefOverAll != NULL)
		delete _pExtRefOverAll;
	_pExtRefOverAll	= NULL;

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

/** Get the appropriate half pel position search struct.
Get the correct struct depending on whether the current motion vector
is at the limit of its search range. Global range = [-64..63].
@param x				: X motion vector coord.
@param y				: Y motion vector coord.
@param halfPos	: Returned correct struct.
@return					: Length of the struct.
*/
int MotionEstimatorImpl1::GetHalfPelSearchStruct(int x, int y, MEI1_COORD** ppHalfPos)
{
	bool atRightExtreme		= (x == (_motionRange - 1));
	bool atLeftExtreme		= (x == -_motionRange);
	bool atBottomExtreme	= (y == (_motionRange - 1));
	bool atTopExtreme			= (y == -_motionRange);
	// Within range most likely case.
	if( !(atRightExtreme || atLeftExtreme || atBottomExtreme || atTopExtreme) )
	{
		*ppHalfPos	= MEI1_HalfPos;
		return(MEI1_MOTION_HALF_POS_LENGTH);
	}//end if inner...
	else if( !atRightExtreme && !atLeftExtreme && !atBottomExtreme && atTopExtreme )
	{
		*ppHalfPos	= MEI1_TopHalfPos;
		return(MEI1_MOTION_TOP_HALF_POS_LENGTH);
	}//end if top...
	else if( !atRightExtreme && !atLeftExtreme && atBottomExtreme && !atTopExtreme )
	{
		*ppHalfPos	= MEI1_BottomHalfPos;
		return(MEI1_MOTION_BOTTOM_HALF_POS_LENGTH);
	}//end if bottom...
	else if( !atRightExtreme && atLeftExtreme && !atBottomExtreme && !atTopExtreme )
	{
		*ppHalfPos	= MEI1_LeftHalfPos;
		return(MEI1_MOTION_LEFT_HALF_POS_LENGTH);
	}//end if left...
	else if( atRightExtreme && !atLeftExtreme && !atBottomExtreme && !atTopExtreme )
	{
		*ppHalfPos	= MEI1_RightHalfPos;
		return(MEI1_MOTION_RIGHT_HALF_POS_LENGTH);
	}//end if right...
	else if( !atRightExtreme && atLeftExtreme && !atBottomExtreme && atTopExtreme )
	{
		*ppHalfPos	= MEI1_TopLeftHalfPos;
		return(MEI1_MOTION_TOP_LEFT_HALF_POS_LENGTH);
	}//end if top left...
	else if( atRightExtreme && !atLeftExtreme && !atBottomExtreme && atTopExtreme )
	{
		*ppHalfPos	= MEI1_TopRightHalfPos;
		return(MEI1_MOTION_TOP_RIGHT_HALF_POS_LENGTH);
	}//end if top right...
	else if( !atRightExtreme && atLeftExtreme && atBottomExtreme && !atTopExtreme )
	{
		*ppHalfPos	= MEI1_BottomLeftHalfPos;
		return(MEI1_MOTION_BOTTOM_LEFT_HALF_POS_LENGTH);
	}//end if bottom left...
	else if( atRightExtreme && !atLeftExtreme && atBottomExtreme && !atTopExtreme )
	{
		*ppHalfPos	= MEI1_BottomRightHalfPos;
		return(MEI1_MOTION_BOTTOM_RIGHT_HALF_POS_LENGTH);
	}//end if bottom right...

	*ppHalfPos	= MEI1_HalfPos;
	return(MEI1_MOTION_HALF_POS_LENGTH);
}//end GetHalfPelSearchStruct.





