/** @file

MODULE						: MotionVectorTree

TAG								: MVT

FILE NAME					: MotionVectorTree.h

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
#ifndef _MOTIONVECTORTREE_H
#define _MOTIONVECTORTREE_H

#include "VicsDefs/VicsDefs.h"

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MotionVectorTree
{
protected:
	VICS_INT			_width;								// Width and height of vector block.
	VICS_INT			_height;
	VICS_INT32		_distortion;					// Distortion of best matching block.
	VICS_INT			_x;										// Motion vector x and y coord.
	VICS_INT			_y;
	VICS_INT			_numCodedBits;				// Number of coded bits for this motion vector.
	VICS_INT			_bitCode;							// The code of length numCodedBits.

	VICS_INT			_selectedPattern;			// {singular, vert, horiz, quad} possible.
	VICS_INT			_numPatternBits;
	VICS_INT			_patternBitCode;

	VICS_POINTER	_pVertSplit[2];				// Split vertically 2 x (w x h/2).
	VICS_POINTER	_pHorizSplit[2];			// Split horiz. 2 x (w/2 x h).
	VICS_POINTER	_pQuadSplit[4];				// Quad split 4 x (w/2 x h/2).

public:
	// Constants.

	// Split locators.
	static const VICS_INT VERT_LEFT;					// For _pVertSplit[].
	static const VICS_INT VERT_RIGHT;
	static const VICS_INT HORIZ_TOP;					// For _pHorizSplit[].
	static const VICS_INT HORIZ_BOTTOM;
	static const VICS_INT QUAD_TOP_LEFT;			// For _pQuadSplit[].
	static const VICS_INT QUAD_TOP_RIGHT;
	static const VICS_INT QUAD_BOTTOM_LEFT;
	static const VICS_INT QUAD_BOTTOM_RIGHT;
	// Pattern types.
	static const VICS_INT SINGLE_PATTERN;
	static const VICS_INT HORIZ_PATTERN;
	static const VICS_INT VERT_PATTERN;
	static const VICS_INT QUAD_PATTERN;

public:
	MotionVectorTree();
	MotionVectorTree(VICS_INT width, VICS_INT height); 
	~MotionVectorTree();

	// Member access.
	virtual VICS_INT			GetBlockWidth(void)			{ return(_width); }
	virtual VICS_INT			GetBlockHeight(void)		{ return(_height); }
	virtual VICS_INT32		GetDistortion(void)			{ return(_distortion); }
	virtual VICS_INT			GetMotionX(void)				{ return(_x); }
	virtual VICS_INT			GetMotionY(void)				{ return(_y); }
	virtual VICS_INT			GetNumCodedBits(void)		{ return(_numCodedBits); }
	virtual VICS_INT			GetBitCode(void)				{ return(_bitCode); }
	virtual VICS_INT			GetPattern(void)				{ return(_selectedPattern); }
	virtual VICS_INT			GetNumPatternBits(void)	{ return(_numPatternBits); }
	virtual VICS_INT			GetPatternBitCode(void)	{ return(_patternBitCode); }
	virtual	VICS_POINTER*	GetVertSplit(void)			{ return(_pVertSplit); }
	virtual	VICS_POINTER*	GetHorizSplit(void)			{ return(_pHorizSplit); }
	virtual	VICS_POINTER*	GetQuadSplit(void)			{ return(_pQuadSplit); }

	virtual void SetBlockWidth(VICS_INT width)							{ _width = width; }
	virtual void SetBlockHeight(VICS_INT height)						{ _height = height; }
	virtual void SetDistortion(VICS_INT32 distortion)				{ _distortion = distortion; }
	virtual void SetMotionX(VICS_INT x)											{ _x = x; }
	virtual void SetMotionY(VICS_INT y)											{ _y = y; }
	virtual void SetNumCodedBits(VICS_INT numCodedBits)			{ _numCodedBits = numCodedBits; }
	virtual void SetBitCode(VICS_INT bitCode)								{ _bitCode = bitCode; }
	virtual void SetPattern(VICS_INT selectedPattern)				{ _selectedPattern = selectedPattern; }
	virtual void SetNumPatternBits(VICS_INT numPatternBits)	{ _numPatternBits = numPatternBits; }
	virtual void SetPatternBitCode(VICS_INT patternBitCode)	{ _patternBitCode = patternBitCode; }

	// Interface.
	virtual VICS_INT		Split(void);
	virtual VICS_INT		AutoSelectPattern(VICS_INT32* minPatternDistortion);

protected:
	virtual void				ResetMembers(void);

	virtual void				DestroySplit(void);

};// end class MotionVectorTree.

#endif
