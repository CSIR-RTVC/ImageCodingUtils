/** @file

MODULE						: EncMotionVectorPattern

TAG								: EMVP

FILE NAME					: EncMotionVectorPattern.h

DESCRIPTION				: A class to describe and operate a pattern structure for coding
										motion vectors. This class extends MotionVectorPattern class
										to add encoding functionality for EncMotionVector objects.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _ENCMOTIONVECTORPATTERN_H
#define _ENCMOTIONVECTORPATTERN_H

#include "VicsDefs/VicsDefs.h"
#include "MotionVectorPattern.h"
#include "EncMotionVector.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
// Predictor locations.
#define EMVP_BOTTOM_LEFT	0
#define EMVP_BOTTOM_RIGHT	1
#define EMVP_TOP_RIGHT		2

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class EncMotionVectorPattern : public MotionVectorPattern
{
protected:
	VICS_INT _numPatternBits;
	VICS_INT _patternBitCode;

	// Max of 4 motion vectors for quad pattern.
	EncMotionVector	_subVector[4];
	EncMotionVector	_diffVector[4];

	// Bottom, diagonal and right predictors for other blocks to use for the prediction.
	MotionVector		_predictor[3];

public:
	EncMotionVectorPattern() { _numPatternBits = 0; _patternBitCode = 0; }
	EncMotionVectorPattern(VICS_INT width, VICS_INT height) 
		{ _numPatternBits = 0; _patternBitCode = 0; _subVector[0].SetBlock(width,height); }
	EncMotionVectorPattern(VICS_INT width, VICS_INT height, VICS_INT pattern) : MotionVectorPattern(pattern) 
	{ _numPatternBits = 0; _patternBitCode = 0;
		 // Just do them all.
		for(VICS_INT i = 0; i < 4; i++) 
			{_subVector[i].SetBlock(width,height); _diffVector[i].SetBlock(width,height);} 
	}
	virtual ~EncMotionVectorPattern() { }

	// Member access.
	VICS_INT	 GetNumPatternBits(void)	{ return(_numPatternBits); }
	VICS_INT	 GetPatternBitCode(void)	{ return(_patternBitCode); }

	// Interface.
	EncMotionVector*	GetMotionVector(VICS_INT location) { return(&(_subVector[location])); }
	EncMotionVector*	GetDiffMotionVector(VICS_INT location) { return(&(_diffVector[location])); }

	virtual void SetMotionVector(VICS_INT	location,VICS_INT	mx,VICS_INT my,VICS_INT	bx,VICS_INT by,VICS_INT32	d,VICS_INT c)
		{ _subVector[location].Set(mx,my,bx,by); _subVector[location].SetDistortion(d); _subVector[location].SetCost(c);}
	virtual void SetDiffMotionVector(VICS_INT	location,VICS_INT	mx,VICS_INT my,VICS_INT	bx,VICS_INT by,VICS_INT32	d,VICS_INT c)
		{ _diffVector[location].Set(mx,my,bx,by); _diffVector[location].SetDistortion(d); _diffVector[location].SetCost(c);}

	virtual void SetMotionVector(VICS_INT	location,VICS_INT	mx,VICS_INT my,VICS_INT	bx,VICS_INT by,VICS_INT32	d)
		{ _subVector[location].Set(mx,my,bx,by); _subVector[location].SetDistortion(d); }
	virtual void SetDiffMotionVector(VICS_INT	location,VICS_INT	mx,VICS_INT my,VICS_INT	bx,VICS_INT by)
		{ _diffVector[location].Set(mx,my,bx,by); }

	virtual void SetMotionVector(VICS_INT location,VICS_INT mx,VICS_INT my,VICS_INT32 d)
		{ _subVector[location].SetMotion(mx,my); _subVector[location].SetDistortion(d); }
	virtual void SetDiffMotionVector(VICS_INT location,VICS_INT mx,VICS_INT my)
		{ _diffVector[location].SetMotion(mx,my); }

	virtual void SetMotionVector(VICS_INT location,VICS_INT mx,VICS_INT my,VICS_INT32 d,VICS_INT c)
		{ _subVector[location].SetMotion(mx,my); _subVector[location].SetDistortion(d); _subVector[location].SetCost(c);}

	VICS_INT32 GetDistortion(void)			
	{ if(_pattern == MotionVectorPattern::SINGLE_PATTERN) return(_subVector[0].GetDistortion());
		else if(_pattern == MotionVectorPattern::QUAD_PATTERN) return(_subVector[0].GetDistortion() + _subVector[1].GetDistortion() + _subVector[2].GetDistortion() + _subVector[3].GetDistortion());
		else return(_subVector[0].GetDistortion() + _subVector[1].GetDistortion());
	}

	VICS_INT32 GetDiffDistortion(void)			
	{ if(_pattern == MotionVectorPattern::SINGLE_PATTERN) return(_diffVector[0].GetDistortion());
		else if(_pattern == MotionVectorPattern::QUAD_PATTERN) return(_diffVector[0].GetDistortion() + _diffVector[1].GetDistortion() + _diffVector[2].GetDistortion() + _diffVector[3].GetDistortion());
		else return(_diffVector[0].GetDistortion() + _diffVector[1].GetDistortion());
	}

	VICS_INT GetCost(void)			
	{ if(_pattern == MotionVectorPattern::SINGLE_PATTERN) return(_subVector[0].GetCost());
		else if(_pattern == MotionVectorPattern::QUAD_PATTERN) return(_subVector[0].GetCost() + _subVector[1].GetCost() + _subVector[2].GetCost() + _subVector[3].GetCost());
		else return(_subVector[0].GetCost() + _subVector[1].GetCost());
	}

	VICS_INT GetDiffCost(void)			
	{ if(_pattern == MotionVectorPattern::SINGLE_PATTERN) return(_diffVector[0].GetCost());
		else if(_pattern == MotionVectorPattern::QUAD_PATTERN) return(_diffVector[0].GetCost() + _diffVector[1].GetCost() + _diffVector[2].GetCost() + _diffVector[3].GetCost());
		else return(_diffVector[0].GetCost() + _diffVector[1].GetCost());
	}

	MotionVector*	GetPredictor(VICS_INT location) { return(&(_predictor[location])); }

	static void SetPredictors(EncMotionVectorPattern& emvp);

	static void ToDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv);

	static void FromDifferential(EncMotionVectorPattern& emvp, MotionVector& pmv);

	static void ToDifferential(EncMotionVectorPattern& emvp,
														 EncMotionVectorPattern& above,
														 EncMotionVectorPattern& aboveRight,
														 EncMotionVectorPattern& left);

	static void FromDifferential(EncMotionVectorPattern& emvp,
															 EncMotionVectorPattern& above,
															 EncMotionVectorPattern& aboveRight,
															 EncMotionVectorPattern& left);

	static VICS_INT	 GetNumPatternBits(VICS_INT pattern);
	static VICS_INT	 GetPatternBitCode(VICS_INT pattern);

	static VICS_INT EncodePattern(EncMotionVectorPattern& emvp);
	static VICS_INT EncodeDiffPattern(EncMotionVectorPattern& emvp);

	static EncMotionVectorPattern SelectBestPattern(EncMotionVectorPattern* single,
																									EncMotionVectorPattern* horiz,
																									EncMotionVectorPattern* vert,
																									EncMotionVectorPattern* quad);
	// Operator overloads.
	EncMotionVectorPattern operator=(EncMotionVectorPattern& emvp)	
	{ _pattern = emvp._pattern; 
		for(VICS_INT i = 0; i < 4; i++) 
			{_subVector[i] = emvp._subVector[i]; _diffVector[i] = emvp._diffVector[i];} 
		for(VICS_INT j = 0; i < 3; j++) _predictor[i] = emvp._predictor[i];
		_numPatternBits = emvp._numPatternBits; _patternBitCode = emvp._patternBitCode;
		return(*this); 
	}

protected:
	// Only accessible through EncodePattern() method.
	void SetNumPatternBits(VICS_INT numPatternBits)	{ _numPatternBits = numPatternBits; }
	void SetPatternBitCode(VICS_INT patternBitCode)	{ _patternBitCode = patternBitCode; }

};// end class EncMotionVectorPattern.

#endif
