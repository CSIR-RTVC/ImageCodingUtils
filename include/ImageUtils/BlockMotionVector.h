/** @file

MODULE						: BlockMotionVector

TAG								: BMV

FILE NAME					: BlockMotionVector.h

DESCRIPTION				: A class to describe a base block motion vector.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _BLOCKMOTIONVECTOR_H
#define _BLOCKMOTIONVECTOR_H

#include "VicsDefs/VicsDefs.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BlockMotionVector
{
public:
	BlockMotionVector()	{_x=0;_y=0;_width=0;_height=0;_xLoc=0;_yLoc=0;}
	BlockMotionVector(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height,VICS_INT xLoc,VICS_INT yLoc)	
		{_x=x;_y=y;_width=width;_height=height;_xLoc=xLoc;_yLoc=yLoc;} 
	BlockMotionVector(BlockMotionVector& mv)	
		{_width=mv._width;_height=mv._height;_x=mv._x;_y=mv._y;_xLoc=mv._xLoc;_yLoc=mv._yLoc;}

	virtual ~BlockMotionVector()	{ }
												 
public:
	// Export the public interfaces of the motion vector encapsulated 
	// responsibilities for specialised implementations. 
	virtual VICS_INT32 Estimate(void)									{return(0);}	// Returns distortion.
	virtual	VICS_INT32 GetEstimationCost(void)				{return(0);}
	virtual	VICS_INT32 GetEstimationDistortion(void)	{return(0);}

	virtual	VICS_INT32 VlcEncode(void)			{return(0);}	// Returns the number of bits of the vlc.
	virtual VICS_INT32 GetVlcNumBits(void)	{return(0);}
	virtual VICS_INT32 GetVlcCode(void)			{return(0);}

	virtual void Compensate(void) {}

public:
	// Member access.
	VICS_INT GetBlockWidth(void)	{return(_width);}
	VICS_INT GetBlockHeight(void)	{return(_height);}
	VICS_INT GetMotionX(void)			{return(_x);}
	VICS_INT GetMotionY(void)			{return(_y);}
	VICS_INT GetBlockXLoc(void)		{return(_xLoc);}
	VICS_INT GetBlockYLoc(void)		{return(_yLoc);}

	void SetBlock(VICS_INT width,VICS_INT height)		{_width=width;_height=height;}
	void SetBlockLoc(VICS_INT xLoc, VICS_INT yLoc)	{_xLoc=xLoc;_yLoc=yLoc;}
	void SetMotion(VICS_INT x,VICS_INT y)						{_x=x;_y=y;}
	void Set(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height,VICS_INT xLoc,VICS_INT yLoc)	
		{_x=x;_y=y;_width=width;_height=height;_xLoc=xLoc;_yLoc=yLoc;} 

	// Operator overloads.
	BlockMotionVector operator=(BlockMotionVector& mv)	
		{_width=mv._width;_height=mv._height;_x=mv._x;_y=mv._y;_xLoc=mv._xLoc;_yLoc=mv._yLoc;return(*this);}

	// Static public helper methods.
	static BlockMotionVector Median(BlockMotionVector& x,BlockMotionVector& y,BlockMotionVector& z);

protected:
	VICS_INT _width;		// Width and height of block.
	VICS_INT _height;
	VICS_INT _x;				// The motion vector (_x,_y).
	VICS_INT _y;
	VICS_INT _xLoc;			// Block reference location.
	VICS_INT _yLoc;

};// end class BlockMotionVector.

#endif
