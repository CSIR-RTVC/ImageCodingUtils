/** @file

MODULE						: MotionVectorTree

TAG								: MVT

FILE NAME					: MotionVectorTree.cpp

DESCRIPTION				: A class to describe and operate a tree structure for coding
										motion vectors.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
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


#include	"MotionVectorTree.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
	// Split locators.
const VICS_INT MotionVectorTree::VERT_LEFT					= 0;	// For _pVertSplit[].
const VICS_INT MotionVectorTree::VERT_RIGHT					= 1;
const VICS_INT MotionVectorTree::HORIZ_TOP					= 0;	// For _pHorizSplit[].
const VICS_INT MotionVectorTree::HORIZ_BOTTOM				= 1;
const VICS_INT MotionVectorTree::QUAD_TOP_LEFT			= 0;	// For _pQuadSplit[].
const VICS_INT MotionVectorTree::QUAD_TOP_RIGHT			= 1;
const VICS_INT MotionVectorTree::QUAD_BOTTOM_LEFT		= 2;
const VICS_INT MotionVectorTree::QUAD_BOTTOM_RIGHT	= 3;

	// Pattern types.
const VICS_INT MotionVectorTree::SINGLE_PATTERN = 0;
const VICS_INT MotionVectorTree::HORIZ_PATTERN	= 1;
const VICS_INT MotionVectorTree::VERT_PATTERN		= 2;
const VICS_INT MotionVectorTree::QUAD_PATTERN		= 3;

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

void MotionVectorTree::ResetMembers(void)
{
	VICS_INT i;

	_width						= 0;			// Width and height of vector block.
	_height						= 0;
	_distortion				= 0;			// Distortion of best matching block.
	_x								= 0;			// Motion vector x and y coord.
	_y								= 0;
	_numCodedBits			= 0;			// Number of coded bits for this motion vector.
	_bitCode					= 0;			// The code of length numCodedBits.

	for(i = 0; i < 2; i++)
	{
		_pVertSplit[i]	= NULL;		// Split vertically (w x h/2).
		_pHorizSplit[i]	= NULL;		// Split horiz. (w/2 x h).
		_pQuadSplit[i]	= NULL;		// Quad split (w/2 x h/2).
	}//end for i...
	for(; i < 4; i++)
		_pQuadSplit[i] = NULL;		// Quad split (w/2 x h/2).
	
	_selectedPattern	= 0;			// {singular, vert, horiz, quad} possible.
	_numPatternBits		= 0;
	_patternBitCode		= 0;

}//end ResetMembers.

MotionVectorTree::MotionVectorTree()
{
	ResetMembers();
}//end constructor.

MotionVectorTree::MotionVectorTree(VICS_INT width, VICS_INT height)
{
	ResetMembers();

	_width  = width;
	_height = height;
}//end alt constructure.

MotionVectorTree::~MotionVectorTree()
{
	DestroySplit();
}//end destruction.

void MotionVectorTree::DestroySplit(void)
{
	VICS_INT i;

	// Before deleting trees recurse the sub-trees.
	for(i = 0; i < 2; i++)
	{
		if(_pVertSplit[i] != NULL)
		{
			((MotionVectorTree*)(_pVertSplit[i]))->DestroySplit();
			delete _pVertSplit[i];
		}//end if _pVertSplit...
		_pVertSplit[i] = NULL;

		if(_pHorizSplit[i] != NULL)
		{
			((MotionVectorTree*)(_pHorizSplit[i]))->DestroySplit();
			delete _pHorizSplit[i];
		}//end if _pHorizSplit...
		_pHorizSplit[i] = NULL;

		if(_pQuadSplit[i] != NULL)
		{
			((MotionVectorTree*)(_pQuadSplit[i]))->DestroySplit();
			delete _pQuadSplit[i];
		}//end if _pQuadSplit...
		_pQuadSplit[i] = NULL;
	}//end for i...
	for(; i < 4; i++)
	{
		if(_pQuadSplit[i] != NULL)
		{
			((MotionVectorTree*)(_pQuadSplit[i]))->DestroySplit();
			delete _pQuadSplit[i];
		}//end _pQuadSplit...
		_pQuadSplit[i] = NULL;
	}//end for i...

}//end DestroySplit.

/*
---------------------------------------------------------------------------
	Public interface.
---------------------------------------------------------------------------
*/

/** Split the tree into horiz, vert and quad sub-trees.
Create new sub-trees for each possible block dimension by splitting this
tree dimension into horiz, vert and quad partitions.

@return : 0 = failure, 1 = success.
*/
VICS_INT MotionVectorTree::Split(void)
{
	// Remove any existing sub-divisions.
	DestroySplit();

	// Check if sub-division is possible.
	if( (_width < 2) || (_height < 2) )
		return(0);

	// Split along a horizontal axis to give a top and bottom tree.
	_pHorizSplit[HORIZ_TOP]		 = (VICS_POINTER)(new MotionVectorTree(_width,_height >> 1));
	_pHorizSplit[HORIZ_BOTTOM] = (VICS_POINTER)(new MotionVectorTree(_width,_height >> 1));

	// Split along a verticle axis to give a left and right tree.
	_pVertSplit[VERT_LEFT]  = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height));
	_pVertSplit[VERT_RIGHT] = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height));

	// Split into four trees.
	_pQuadSplit[QUAD_TOP_LEFT]		 = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height >> 1));
	_pQuadSplit[QUAD_TOP_RIGHT]		 = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height >> 1));
	_pQuadSplit[QUAD_BOTTOM_LEFT]  = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height >> 1));
	_pQuadSplit[QUAD_BOTTOM_RIGHT] = (VICS_POINTER)(new MotionVectorTree(_width >> 1,_height >> 1));

	// Check alloc success.
	if( (_pHorizSplit[HORIZ_TOP]				== NULL) || (_pHorizSplit[HORIZ_BOTTOM]			== NULL) ||
			(_pVertSplit[VERT_LEFT]					== NULL) || (_pVertSplit[VERT_RIGHT]				== NULL) ||
			(_pQuadSplit[QUAD_TOP_LEFT]			== NULL) || (_pQuadSplit[QUAD_TOP_RIGHT]		== NULL) ||
			(_pQuadSplit[QUAD_BOTTOM_LEFT]	== NULL) || (_pQuadSplit[QUAD_BOTTOM_RIGHT] == NULL) )
	{
		DestroySplit();
		return(0);
	}//end if ...

	return(1);
}//end Split.

/** Automatically choose the most appropriate pattern from the sub-tree distortions.
Use a rule based mechanism to select and set the selected pattern based on the
relationship between the total block distortions of each of the split blocks.

@param	minPatternDistortion	: Total block distortion of the selected pattern.
@return												: The selected pattern.
*/
VICS_INT MotionVectorTree::AutoSelectPattern(VICS_INT32* minPatternDistortion)
{
	// Pick any one to test that the block is split.
	if(_pQuadSplit[QUAD_TOP_LEFT] == NULL)
	{
		// Default to SINGLE pattern.
		_selectedPattern			= SINGLE_PATTERN;
		*minPatternDistortion = _distortion;
		return(_selectedPattern);
	}//end if _pQuadSplit...

	// Rule: Select 2-vector sub-division if min{horiz, vert} < 0.8 single.
	//			 Select 4-vector sub-division if						 quad < 0.6 single and 
	//															  min{horiz, vert} - quad > 0.15 single.
	// Note: All calc. are done on total block distortions.
	VICS_INT32 min1E  = _distortion;

	VICS_INT32 min2EH = ((MotionVectorTree*)_pHorizSplit[HORIZ_TOP])->_distortion + 
											((MotionVectorTree*)_pHorizSplit[HORIZ_BOTTOM])->_distortion;

	VICS_INT32 min2EV = ((MotionVectorTree*)_pVertSplit[VERT_LEFT])->_distortion + 
											((MotionVectorTree*)_pVertSplit[VERT_RIGHT])->_distortion;
	VICS_INT32 min2E = min2EH;
	if(min2EV < min2E)
		min2E = min2EV;

	VICS_INT32 min4E = ((MotionVectorTree*)_pQuadSplit[QUAD_TOP_LEFT])->_distortion + 
										 ((MotionVectorTree*)_pQuadSplit[QUAD_TOP_RIGHT])->_distortion +
										 ((MotionVectorTree*)_pQuadSplit[QUAD_BOTTOM_LEFT])->_distortion + 
										 ((MotionVectorTree*)_pQuadSplit[QUAD_BOTTOM_RIGHT])->_distortion;

  VICS_INT32 minE;
	if( (min4E < ((6 * min1E)/10))&&((min2E - min4E) > ((15 * min1E)/100)) )
	{
		_selectedPattern = QUAD_PATTERN;
		minE						 = min4E;
	}//end if 4 vector...
	else if( min2E < ((8 * min1E)/10) )
	{
		if(min2EV < min2EH)
			_selectedPattern = VERT_PATTERN;
		else
			_selectedPattern = HORIZ_PATTERN;
		minE						 = min2E;
	}//end if 2 vector...
	else
	{
		_selectedPattern = SINGLE_PATTERN;
		minE						 = min1E;
	}//end else...

	*minPatternDistortion = minE;
	return(_selectedPattern);
}//end AutoSelectPattern.

/*
---------------------------------------------------------------------------
	Private Methods.
---------------------------------------------------------------------------
*/

