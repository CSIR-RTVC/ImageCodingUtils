/** @file

MODULE						: MotionVectorPattern

TAG								: MVP

FILE NAME					: MotionVectorPattern.h

DESCRIPTION				: A class to describe a pattern structure for coding
										motion vectors.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _MOTIONVECTORPATTERN_H
#define _MOTIONVECTORPATTERN_H

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
class MotionVectorPattern
{
protected:
	VICS_INT			_pattern;				// {singular, vert, horiz, quad} possible.

public:
	// Constants.

	// Split locators for use with derived GetMotionVector() methods.
	static const VICS_INT SINGLE;							// For no Split.
	static const VICS_INT VERT_LEFT;					// For Vert Split.
	static const VICS_INT VERT_RIGHT;
	static const VICS_INT HORIZ_TOP;					// For Horiz Split.
	static const VICS_INT HORIZ_BOTTOM;
	static const VICS_INT QUAD_TOP_LEFT;			// For Quad Split.
	static const VICS_INT QUAD_TOP_RIGHT;
	static const VICS_INT QUAD_BOTTOM_LEFT;
	static const VICS_INT QUAD_BOTTOM_RIGHT;
	// Pattern types.
	static const VICS_INT SINGLE_PATTERN;
	static const VICS_INT HORIZ_PATTERN;
	static const VICS_INT VERT_PATTERN;
	static const VICS_INT QUAD_PATTERN;

public:
	MotionVectorPattern()										{ _pattern = SINGLE_PATTERN; }
	MotionVectorPattern(VICS_INT pattern)		{ _pattern = pattern; }
	virtual ~MotionVectorPattern()									{ }

	// Member access.
	VICS_INT GetPattern(void)							{ return(_pattern); }
	void		 SetPattern(VICS_INT pattern)	{ _pattern = pattern; }

	// Operator overloads.
	MotionVectorPattern operator=(MotionVectorPattern& mvp)	
		{ _pattern = mvp._pattern; return(*this); }

};// end class MotionVectorPattern.

#endif
