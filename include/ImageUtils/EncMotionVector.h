/** @file

MODULE						: EncMotionVector

TAG								: EMV

FILE NAME					: EncMotionVector.h

DESCRIPTION				: A class to extend the MotionVector class to include members
										suitable for the encoding process.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _ENCMOTIONVECTOR_H
#define _ENCMOTIONVECTOR_H

#include "VicsDefs/VicsDefs.h"
#include "MotionVector.h"

/*
---------------------------------------------------------------------------
	Class constants.
---------------------------------------------------------------------------
*/
#define EMV_TABLE_SIZE 66
#define EMV_NUM_BITS		0
#define EMV_BIT_CODE		1

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class EncMotionVector : public MotionVector
{
protected:
	VICS_INT32		_distortion;	// Block distortion of vector defined as sum of squared error.
	VICS_INT			_numCodeBits;	// Number of coded bits for this motion vector.
	VICS_INT			_bitCode;			// The code of length numCodeBits.
	VICS_INT			_cost;				// An implementation dependent cost of this vector.

	// Constants.
	static const VICS_INT NUM_ESC_BITS;
	static const VICS_INT ESC_BIT_CODE;
	static const VICS_INT ESC_LENGTH;

	static const VICS_INT VLC_TABLE[EMV_TABLE_SIZE][2];

public:
	EncMotionVector()	{ _distortion = 0; _numCodeBits = 0; _bitCode = 0; _cost = 0;}
	EncMotionVector(VICS_INT width, VICS_INT height) : MotionVector(width,height)
		{ _distortion = 0; _numCodeBits = 0; _bitCode = 0; _cost = 0;} 
	EncMotionVector(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height) : MotionVector(x,y,width,height)
		{ _distortion = 0; _numCodeBits = 0; _bitCode = 0; _cost = 0;} 
	virtual ~EncMotionVector()	{ }

	// Member access.
	VICS_INT32 GetDistortion(void)		{ return(_distortion); }
	VICS_INT	 GetNumCodedBits(void)	{ return(_numCodeBits); }
	VICS_INT	 GetBitCode(void)				{ return(_bitCode); }
	VICS_INT	 GetCost(void)					{ return(_cost); }

	void SetDistortion(VICS_INT32 distortion)	{ _distortion = distortion; }
	void SetCost(VICS_INT cost)								{ _cost = cost; }
	static VICS_INT EncodeVector(EncMotionVector& emv);

	// Operator overloads.
	EncMotionVector operator=(EncMotionVector& emv)	
	{ _width = emv._width; _height = emv._height; _x = emv._x; _y = emv._y; 
		_distortion = emv._distortion; _numCodeBits = emv._numCodeBits; 
		_bitCode = emv._bitCode; _cost = emv._cost; 
		return(*this); 
	}

protected:
	// Can only be set by calling EncodeVector() method.
	void SetNumCodedBits(VICS_INT numCodeBits)			{ _numCodeBits = numCodeBits; }
	void SetBitCode(VICS_INT bitCode)								{ _bitCode = bitCode; }

	static VICS_INT GetMotionVecBits(VICS_INT VecCoord,VICS_INT *CdeWord);

};// end class EncMotionVector.

#endif
