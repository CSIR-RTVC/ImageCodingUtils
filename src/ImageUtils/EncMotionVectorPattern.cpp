/** @file

MODULE						: EncMotionVectorPattern

TAG								: EMVP

FILE NAME					: EncMotionVectorPattern.cpp

DESCRIPTION				: A class to describe and operate a pattern structure for coding
										motion vectors. This class extends MotionVectorPattern class
										to add encoding functionality for EncMotionVector objects.

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


#include	"EncMotionVectorPattern.h"

/*
---------------------------------------------------------------------------
	Public Interface.
---------------------------------------------------------------------------
*/

VICS_INT EncMotionVectorPattern::GetNumPatternBits(VICS_INT pattern)
{
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
		return(1);
	else
		return(3);
}//end GetNumPatternBits.

VICS_INT EncMotionVectorPattern::GetPatternBitCode(VICS_INT pattern)
{
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
		return(0x00000001);	// 1.
	else
	{
		if(pattern == MotionVectorPattern::HORIZ_PATTERN)
			return(0x00000002);	// 010.
		else if(pattern == MotionVectorPattern::VERT_PATTERN)
			return(0x00000004);	// 100;
		else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
			return(0x00000006);	// 110.
	}//end else...
}//end GetPatternBitCode.

// Encodes the pattern and the associated motion vectors and returns the total
// number of pattern and motion vector bits.
VICS_INT EncMotionVectorPattern::EncodePattern(EncMotionVectorPattern& emvp)
{
	VICS_INT pattern	= emvp.GetPattern();
	VICS_INT bits			= emvp.GetNumPatternBits(pattern);
	emvp.SetNumPatternBits(bits);
	emvp.SetPatternBitCode(emvp.GetPatternBitCode(pattern));

	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		bits += EncMotionVector::EncodeVector(emvp._subVector[MotionVectorPattern::SINGLE]);
	}//end if pattern...
	else
	{
		if(pattern == MotionVectorPattern::HORIZ_PATTERN)
		{
			bits += EncMotionVector::EncodeVector(emvp._subVector[MotionVectorPattern::HORIZ_TOP]);
			bits += EncMotionVector::EncodeVector(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM]);
		}//end if HORIZ_PATTERN...
		else if(pattern == MotionVectorPattern::VERT_PATTERN)
		{
			bits += EncMotionVector::EncodeVector(emvp._subVector[MotionVectorPattern::VERT_LEFT]);
			bits += EncMotionVector::EncodeVector(emvp._subVector[MotionVectorPattern::VERT_RIGHT]);
		}//end else if VERT_PATTERN...
		else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
		{
			for(VICS_INT i = 0; i < 4; i++)
				bits += EncMotionVector::EncodeVector(emvp._subVector[i]);
		}//end else...
	}//end else...

	return(bits);
}//end EncodePattern.

// Encodes the pattern and the associated differential motion vectors.
VICS_INT EncMotionVectorPattern::EncodeDiffPattern(EncMotionVectorPattern& emvp)
{
	VICS_INT pattern	= emvp.GetPattern();
	VICS_INT bits			= emvp.GetNumPatternBits(pattern);
	emvp.SetNumPatternBits(bits);
	emvp.SetPatternBitCode(emvp.GetPatternBitCode(pattern));

	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		bits += EncMotionVector::EncodeVector(emvp._diffVector[MotionVectorPattern::SINGLE]);
	}//end if pattern...
	else
	{
		if(pattern == MotionVectorPattern::HORIZ_PATTERN)
		{
			bits += EncMotionVector::EncodeVector(emvp._diffVector[MotionVectorPattern::HORIZ_TOP]);
			bits += EncMotionVector::EncodeVector(emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM]);
		}//end if HORIZ_PATTERN...
		else if(pattern == MotionVectorPattern::VERT_PATTERN)
		{
			bits += EncMotionVector::EncodeVector(emvp._diffVector[MotionVectorPattern::VERT_LEFT]);
			bits += EncMotionVector::EncodeVector(emvp._diffVector[MotionVectorPattern::VERT_RIGHT]);
		}//end else if VERT_PATTERN...
		else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
		{
			for(VICS_INT i = 0; i < 4; i++)
				bits += EncMotionVector::EncodeVector(emvp._diffVector[i]);
		}//end else...
	}//end else...

	return(bits);
}//end EncodeDiffPattern.

EncMotionVectorPattern EncMotionVectorPattern::SelectBestPattern(EncMotionVectorPattern* single,
																																 EncMotionVectorPattern* horiz,
																																 EncMotionVectorPattern* vert,
																																 EncMotionVectorPattern* quad)
{
	// Rule: Select 2-vector sub-division if min{horiz, vert} < 0.8 single.
	//			 Select 4-vector sub-division if						 quad < 0.6 single and 
	//															  min{horiz, vert} - quad > 0.15 single.
	// Note: All calc. are done on total block distortions.
	VICS_INT32 min1E  = single->GetDistortion();

	VICS_INT32 min2EH = horiz->GetDistortion();

	VICS_INT32 min2EV = vert->GetDistortion();
	VICS_INT32 min2E = min2EH;
	if(min2EV < min2E)
		min2E = min2EV;

	VICS_INT32 min4E = quad->GetDistortion();

	if( (min4E < ((6 * min1E)/10))&&((min2E - min4E) > ((15 * min1E)/100)) )
	{
		return(*quad);
	}//end if 4 vector...
	else if( min2E < ((8 * min1E)/10) )
	{
		if(min2EV < min2EH)
			return(*vert);
		else
			return(*horiz);
	}//end if 2 vector...

	return(*single);
}//end SelectBestPattern.

// The motion vectors in the pattern are assumed to be non-differential.
void EncMotionVectorPattern::SetPredictors(EncMotionVectorPattern& emvp)
{
	VICS_INT pattern	= emvp.GetPattern();

	// When these predictors are used they will favour the top left of
	// the block requiring a prediction.
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		for(VICS_INT i = 0; i < 3; i++)
			emvp._predictor[i] = (MotionVector)(emvp._subVector[MotionVectorPattern::SINGLE]);
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		emvp._predictor[EMVP_BOTTOM_LEFT]		= (MotionVector)(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM]);
		emvp._predictor[EMVP_BOTTOM_RIGHT]	= (MotionVector)(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM]);
		emvp._predictor[EMVP_TOP_RIGHT]			= (MotionVector)(emvp._subVector[MotionVectorPattern::HORIZ_TOP]);
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		emvp._predictor[EMVP_BOTTOM_LEFT]		= (MotionVector)(emvp._subVector[MotionVectorPattern::VERT_LEFT]);
		emvp._predictor[EMVP_BOTTOM_RIGHT]	= (MotionVector)(emvp._subVector[MotionVectorPattern::VERT_RIGHT]);
		emvp._predictor[EMVP_TOP_RIGHT]			= (MotionVector)(emvp._subVector[MotionVectorPattern::VERT_RIGHT]);
	}//end else if HORIZ_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		emvp._predictor[EMVP_BOTTOM_LEFT]		= (MotionVector)(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT]);
		emvp._predictor[EMVP_BOTTOM_RIGHT]	= (MotionVector)(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT]);
		emvp._predictor[EMVP_TOP_RIGHT]			= (MotionVector)(emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT]);
	}//end else if QUAD_PATTERN...
}//end SetPredictors.

// Set the differential vectors from the sub-vectors according to the pattern.
void EncMotionVectorPattern::ToDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv)
{
	VICS_INT pattern	= emvp.GetPattern();

	VICS_INT pmx = pmv.GetMotionX();
	VICS_INT pmy = pmv.GetMotionY();

	// The input predictor favours the top left of the pattern block. Predict 
	// all inner motion from the same input motion vector except the bottom right
	// in a quad pattern.
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		emvp._diffVector[MotionVectorPattern::SINGLE].Set(emvp._subVector[MotionVectorPattern::SINGLE].GetMotionX() - pmx,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetMotionY() - pmy,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		emvp._diffVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());

		emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() - pmx,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() - pmy,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		emvp._diffVector[MotionVectorPattern::VERT_LEFT].Set(emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());

		emvp._diffVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() - pmx,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() - pmy,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if VERT_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() - pmx,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() - pmy,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());

		emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() - pmx,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() - pmy,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());

		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() - pmx,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() - pmy,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());
		// Predict the bottom right from the median of the top left, right and bottom left.
		MotionVector newpmv = MotionVector::Median(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
																							 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT],
																							 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT]);

		pmx = newpmv.GetMotionX();
		pmy = newpmv.GetMotionY();

		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() - pmx,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() - pmy,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end ToDifferential.

// Set the differential vectors from the predicted sub-vectors in the neighboring patterns.
void EncMotionVectorPattern::ToDifferential(EncMotionVectorPattern& emvp,
																						EncMotionVectorPattern& above,
																						EncMotionVectorPattern& aboveRight,
																						EncMotionVectorPattern& left)
{
	VICS_INT pattern	= emvp.GetPattern();
	// Define a prediction vector that is established depending on the neighbor patterns. 
	MotionVector pmv(0,0,0,0);

	VICS_INT pmx,pmy;

	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		// Prediction for SINGLE is the median of the 3 closest vectors to top left corner.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_LEFT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 left._predictor[EMVP_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::SINGLE].Set(emvp._subVector[MotionVectorPattern::SINGLE].GetMotionX() - pmx,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetMotionY() - pmy,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		// Prediction for HORIZ is the vector above for TOP and left for BOTTOM (as in H.264).
		pmv = above._predictor[EMVP_BOTTOM_LEFT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());

		pmv = left._predictor[EMVP_BOTTOM_RIGHT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() - pmx,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() - pmy,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		// Prediction for VERT is the vector left for LEFT and above right for RIGHT.
		pmv = left._predictor[EMVP_TOP_RIGHT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::VERT_LEFT].Set(emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());

		pmv = aboveRight._predictor[EMVP_BOTTOM_LEFT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() - pmx,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() - pmy,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if VERT_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		// Prediction for TOP LEFT is the median vector of left, above right and above.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_LEFT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 left._predictor[EMVP_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() - pmx,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() - pmy,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());

		// Prediction for TOP RIGHT is the median vector of above, above right and internal left.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_RIGHT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() - pmx,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() - pmy,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());

		// Prediction for BOTTOM LEFT is the median vector of left, internal above and internal above right.
		pmv = MotionVector::Median(left._predictor[EMVP_BOTTOM_RIGHT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() - pmx,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() - pmy,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());
		// Prediction for BOTTOM RIGHT from the median of the internal top left, top right and bottom left.
		pmv = MotionVector::Median(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT],
															 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() - pmx,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() - pmy,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end ToDifferential.

// Set the sub-vectors from the differential vectors according to the pattern.
void EncMotionVectorPattern::FromDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv)
{
	VICS_INT pattern	= emvp.GetPattern();

	VICS_INT pmx = pmv.GetMotionX();
	VICS_INT pmy = pmv.GetMotionY();

	// The input predictor favours the top left of the pattern block. Predict 
	// all inner motion from the same input motion vector except the bottom right
	// in a quad pattern.
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		emvp._subVector[MotionVectorPattern::SINGLE].Set(emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionX() + pmx,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionY() + pmy,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		emvp._subVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());

		emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() + pmx,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() + pmy,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		emvp._subVector[MotionVectorPattern::VERT_LEFT].Set(emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());

		emvp._subVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() + pmx,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() + pmy,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if VERT_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() + pmx,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() + pmy,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());

		emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() + pmx,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() + pmy,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());
		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() + pmx,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() + pmy,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());
		// Predict the bottom right from the median of the top left, right and bottom left.
		MotionVector newpmv = MotionVector::Median(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
																							 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT],
																							 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT]);

		pmx = newpmv.GetMotionX();
		pmy = newpmv.GetMotionY();

		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() + pmx,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() + pmy,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end FromDifferential.

// Set the normal vectors from the predicted differential sub-vectors in the neighboring patterns.
void EncMotionVectorPattern::FromDifferential(EncMotionVectorPattern& emvp,
																							EncMotionVectorPattern& above,
																							EncMotionVectorPattern& aboveRight,
																							EncMotionVectorPattern& left)
{
	VICS_INT pattern	= emvp.GetPattern();
	// Define a prediction vector that is established depending on the neighbor patterns. 
	MotionVector pmv(0,0,0,0);

	VICS_INT pmx,pmy;

	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		// Prediction for SINGLE is the median of the 3 closest independent vectors.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_LEFT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 left._predictor[EMVP_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the difference with the normal vector.
		emvp._subVector[MotionVectorPattern::SINGLE].Set(emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionX() + pmx,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionY() + pmy,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		// Prediction for HORIZ is the vector above for TOP and left for BOTTOM.
		pmv = above._predictor[EMVP_BOTTOM_LEFT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());

		pmv = left._predictor[EMVP_BOTTOM_RIGHT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() + pmx,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() + pmy,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		// Prediction for VERT is the vector left for LEFT and above for RIGHT.
		pmv = left._predictor[EMVP_TOP_RIGHT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::VERT_LEFT].Set(emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());

		pmv = aboveRight._predictor[EMVP_BOTTOM_LEFT];
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() + pmx,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() + pmy,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if VERT_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		// Prediction for TOP LEFT is the median vector of left, above right and above.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_LEFT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 left._predictor[EMVP_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() + pmx,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() + pmy,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());

		// Prediction for TOP RIGHT is the median vector of above, above right and internal top left.
		pmv = MotionVector::Median(above._predictor[EMVP_BOTTOM_RIGHT],
															 aboveRight._predictor[EMVP_BOTTOM_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() + pmx,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() + pmy,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());

		// Prediction for BOTTOM LEFT is the median vector of left, internal above and internal above right.
		pmv = MotionVector::Median(left._predictor[EMVP_BOTTOM_RIGHT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() + pmx,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() + pmy,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());

		// Prediction for BOTTOM RIGHT from the median of the top left, right and bottom left within current block.
		pmv = MotionVector::Median(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT],
															 emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT],
															 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT]);
		// Get the predicted vector components. 
		pmx = pmv.GetMotionX();
		pmy = pmv.GetMotionY();
		// Determine the normal vector from the difference.
		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() + pmx,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() + pmy,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end FromDifferential.

/*

	Prediction Type-1 (Not very effective)
// Set the differential vectors from the sub-vectors according to the pattern.
void EncMotionVectorPattern::ToDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv)
{
	VICS_INT pattern	= emvp.GetPattern();

	VICS_INT pmx = pmv.GetMotionX();
	VICS_INT pmy = pmv.GetMotionY();

	// The input predictor favours the top left of the pattern block.
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		emvp._diffVector[MotionVectorPattern::SINGLE].Set(emvp._subVector[MotionVectorPattern::SINGLE].GetMotionX() - pmx,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetMotionY() - pmy,
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																											emvp._subVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		// Predict the top from the input.
		emvp._diffVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());
		// Predict the bottom from the top.
		pmx = emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionY();

		emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() - pmx,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() - pmy,
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																														emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		// Predict the left from the input.
		emvp._diffVector[MotionVectorPattern::VERT_LEFT].Set(emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionX() - pmx,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionY() - pmy,
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												 emvp._subVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());
		// Predict the right from the left.
		pmx = emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionY();

		emvp._diffVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() - pmx,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() - pmy,
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																													emvp._subVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		// Predict the top left from the input.
		emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() - pmx,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() - pmy,
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														 emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());
		// Predict the top right and bottom left from the top left.
		pmx = emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY();

		emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() - pmx,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() - pmy,
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																															emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());
		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() - pmx,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() - pmy,
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																																emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());
		// Predict the bottom right from the bottom left.
		pmx = emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY();

		emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() - pmx,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() - pmy,
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																 emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end ToDifferential.

// Set the sub-vectors from the differential vectors according to the pattern.
void EncMotionVectorPattern::FromDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv)
{
	VICS_INT pattern	= emvp.GetPattern();

	VICS_INT pmx = pmv.GetMotionX();
	VICS_INT pmy = pmv.GetMotionY();

	// The input predictor favours the top left of the pattern block.
	if(pattern == MotionVectorPattern::SINGLE_PATTERN)
	{
		emvp._subVector[MotionVectorPattern::SINGLE].Set(emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionX() + pmx,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetMotionY() + pmy,
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockWidth(),
																										 emvp._diffVector[MotionVectorPattern::SINGLE].GetBlockHeight());
	}//end if SINGLE_PATTERN..
	else if(pattern == MotionVectorPattern::HORIZ_PATTERN)
	{
		// Predict the top from the input.
		emvp._subVector[MotionVectorPattern::HORIZ_TOP].Set(emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::HORIZ_TOP].GetBlockHeight());
		// Predict the bottom from the top.
		pmx = emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::HORIZ_TOP].GetMotionY();

		emvp._subVector[MotionVectorPattern::HORIZ_BOTTOM].Set(emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionX() + pmx,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetMotionY() + pmy,
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockWidth(),
																													 emvp._diffVector[MotionVectorPattern::HORIZ_BOTTOM].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else if(pattern == MotionVectorPattern::VERT_PATTERN)
	{
		// Predict the left from the input.
		emvp._subVector[MotionVectorPattern::VERT_LEFT].Set(emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionX() + pmx,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetMotionY() + pmy,
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockWidth(),
																												emvp._diffVector[MotionVectorPattern::VERT_LEFT].GetBlockHeight());
		// Predict the right from the left.
		pmx = emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::VERT_LEFT].GetMotionY();

		emvp._subVector[MotionVectorPattern::VERT_RIGHT].Set(emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionX() + pmx,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetMotionY() + pmy,
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockWidth(),
																												 emvp._diffVector[MotionVectorPattern::VERT_RIGHT].GetBlockHeight());
	}//end else if HORIZ_PATTERN...
	else //if(pattern == MotionVectorPattern::QUAD_PATTERN)
	{
		// Predict the top left from the input.
		emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX() + pmx,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY() + pmy,
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockWidth(),
																														emvp._diffVector[MotionVectorPattern::QUAD_TOP_LEFT].GetBlockHeight());
		// Predict the top right and bottom left from the top left.
		pmx = emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::QUAD_TOP_LEFT].GetMotionY();

		emvp._subVector[MotionVectorPattern::QUAD_TOP_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionX() + pmx,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetMotionY() + pmy,
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockWidth(),
																														 emvp._diffVector[MotionVectorPattern::QUAD_TOP_RIGHT].GetBlockHeight());
		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX() + pmx,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY() + pmy,
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockWidth(),
																															 emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetBlockHeight());
		// Predict the bottom right from the bottom left.
		pmx = emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionX();
		pmy = emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_LEFT].GetMotionY();

		emvp._subVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].Set(emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionX() + pmx,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetMotionY() + pmy,
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockWidth(),
																																emvp._diffVector[MotionVectorPattern::QUAD_BOTTOM_RIGHT].GetBlockHeight());
	}//end else if QUAD_PATTERN...
}//end FromDifferential.
*/
