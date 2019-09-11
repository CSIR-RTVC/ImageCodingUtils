/** @file

MODULE						: MotionVectorPattern

TAG								: MVP

FILE NAME					: MotionVectorPattern.cpp

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


#include	"MotionVectorPattern.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
	// Split locators.
const VICS_INT MotionVectorPattern::SINGLE						= 0;	// For no Split.
const VICS_INT MotionVectorPattern::VERT_LEFT					= 0;	// For Vert Split.
const VICS_INT MotionVectorPattern::VERT_RIGHT				= 1;
const VICS_INT MotionVectorPattern::HORIZ_TOP					= 0;	// For Horiz Split.
const VICS_INT MotionVectorPattern::HORIZ_BOTTOM			= 1;
const VICS_INT MotionVectorPattern::QUAD_TOP_LEFT			= 0;	// For Quad Split.
const VICS_INT MotionVectorPattern::QUAD_TOP_RIGHT		= 1;
const VICS_INT MotionVectorPattern::QUAD_BOTTOM_LEFT	= 2;
const VICS_INT MotionVectorPattern::QUAD_BOTTOM_RIGHT	= 3;

	// Pattern types.
const VICS_INT MotionVectorPattern::SINGLE_PATTERN	= 0;
const VICS_INT MotionVectorPattern::HORIZ_PATTERN		= 1;
const VICS_INT MotionVectorPattern::VERT_PATTERN		= 2;
const VICS_INT MotionVectorPattern::QUAD_PATTERN		= 3;

