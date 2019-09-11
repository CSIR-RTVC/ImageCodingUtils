/** @file

MODULE						: MotionVector

TAG								: MV

FILE NAME					: MotionVector.h

DESCRIPTION				: A class to describe a base motion vector.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _MOTIONVECTOR_H
#define _MOTIONVECTOR_H

#include "VicsDefs/VicsDefs.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MotionVector
{
protected:
	VICS_INT _width;		// Width and height of motion block.
	VICS_INT _height;
	VICS_INT _x;				// Motion vector (_x,_y).
	VICS_INT _y;

public:
	MotionVector()																{ _width = 0; _height = 0; _x = 0; _y = 0; }
	MotionVector(VICS_INT width,VICS_INT height)	{ _width = width; _height = height; _x = 0; _y = 0; } 
	MotionVector(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height)	
		{ _width = width; _height = height; _x = x; _y = y; } 
	virtual ~MotionVector()	{ }
												 
	// Member access.
	VICS_INT GetBlockWidth(void)	{ return(_width); }
	VICS_INT GetBlockHeight(void)	{ return(_height); }
	VICS_INT GetMotionX(void)			{ return(_x); }
	VICS_INT GetMotionY(void)			{ return(_y); }

	void SetBlock(VICS_INT width,VICS_INT height)	{ _width = width; _height = height;}
	void SetMotion(VICS_INT x, VICS_INT y)				{ _x = x; _y = y;}
	void Set(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height) 
		{ _x = x; _y = y; _width = width; _height = height;}

	// Operator overloads.
	MotionVector operator=(MotionVector& mv)	
		{ _width = mv._width; _height = mv._height; _x = mv._x; _y = mv._y; return(*this); }

	// Static helper methods.
	static MotionVector Median(MotionVector& x,MotionVector& y,MotionVector& z);

};// end class MotionVector.

#endif
