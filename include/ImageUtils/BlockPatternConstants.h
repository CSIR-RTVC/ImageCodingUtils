/** @file

MODULE						: BlockPatternConstants

TAG								: BPC

FILE NAME					: BlockPatternConstants.h

DESCRIPTION				: A collection of classes to describe general constants used
										in block patterns.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _BLOCKPATTERNCONSTANTS_H
#define _BLOCKPATTERNCONSTANTS_H

#include "VicsDefs/VicsDefs.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BlockPatternType
{
public:
	static const VICS_INT SINGLE;
	static const VICS_INT HORIZ;
	static const VICS_INT VERT;
	static const VICS_INT QUAD;

};// end class BlockPatternType.

class BlockPatternLocation
{
public:
	static const VICS_INT SINGLE;							// For no Split.
	static const VICS_INT VERT_LEFT;					// For Vert Split.
	static const VICS_INT VERT_RIGHT;
	static const VICS_INT HORIZ_TOP;					// For Horiz Split.
	static const VICS_INT HORIZ_BOTTOM;
	static const VICS_INT QUAD_TOP_LEFT;			// For Quad Split.
	static const VICS_INT QUAD_TOP_RIGHT;
	static const VICS_INT QUAD_BOTTOM_LEFT;
	static const VICS_INT QUAD_BOTTOM_RIGHT;
};// end class BlockPatternLocation.

/*
---------------------------------------------------------------------------
	Constant definitions.
---------------------------------------------------------------------------
*/
// BlockPatternType.
const VICS_INT BlockPatternType::SINGLE	= 0;
const VICS_INT BlockPatternType::HORIZ	= 1;
const VICS_INT BlockPatternType::VERT		= 2;
const VICS_INT BlockPatternType::QUAD		= 3;

// BlockPatternLocation.
const VICS_INT BlockPatternLocation::SINGLE							= 0;	// For no Split.
const VICS_INT BlockPatternLocation::VERT_LEFT					= 0;	// For Vert Split.
const VICS_INT BlockPatternLocation::VERT_RIGHT					= 1;
const VICS_INT BlockPatternLocation::HORIZ_TOP					= 0;	// For Horiz Split.
const VICS_INT BlockPatternLocation::HORIZ_BOTTOM				= 1;
const VICS_INT BlockPatternLocation::QUAD_TOP_LEFT			= 0;	// For Quad Split.
const VICS_INT BlockPatternLocation::QUAD_TOP_RIGHT			= 1;
const VICS_INT BlockPatternLocation::QUAD_BOTTOM_LEFT		= 2;
const VICS_INT BlockPatternLocation::QUAD_BOTTOM_RIGHT	= 3;

#endif
