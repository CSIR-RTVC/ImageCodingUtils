/** @file

MODULE						: EncBlockMotionVector

TAG								: EBMV

FILE NAME					: EncBlockMotionVector.h

DESCRIPTION				: A class to describe an implementation for and encoding
										block motion vector.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _ENCBLOCKMOTIONVECTOR_H
#define _ENCBLOCKMOTIONVECTOR_H

#include "VicsDefs/VicsDefs.h"
#include "BlockMotionVector.h"
#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class EncBlockMotionVector: public BlockMotionVector
{
public:
	EncBlockMotionVector()	{_cost=0;_distortion=0;}
	EncBlockMotionVector(VICS_INT x,VICS_INT y,VICS_INT width,VICS_INT height,VICS_INT xLoc,VICS_INT yLoc):BlockMotionVector(x,y,width,height,xLoc,yLoc)	
		{_cost=0;_distortion=0;} 
	EncBlockMotionVector(EncBlockMotionVector& emv):BlockMotionVector((BlockMotionVector)emv)	
		{_cost=emv._cost;_distortion=emv._distortion;_vlcEncoder=emv._vlcEncoder;}

	virtual ~EncBlockMotionVector()	{ }
												 
	// Implementation setup typically used after construction by factories.
	static void SetVlcEncoder(IVlcEncoder*	vlcEncoder)	{_vlcEncoder=vlcEncoder;}

public:
	// Implement responsibilities for this specialised block motion vector type. 
//	virtual VICS_INT32 Estimate(void)									{return(0);}	// Returns distortion.
	virtual	VICS_INT32 GetEstimationCost(void)				{return(_cost);}
	virtual	VICS_INT32 GetEstimationDistortion(void)	{return(_distortion);}

	virtual	VICS_INT32 VlcEncode(void)			{return(_vlcEncoder->Encode2(_x,_y));}	// Returns the number of bits of the vlc.
	virtual VICS_INT32 GetVlcNumBits(void)	{return(_vlcEncoder->GetNumCodedBits());}
	virtual VICS_INT32 GetVlcCode(void)			{return(_vlcEncoder->GetCode());}

//	virtual void Compensate(void) {}

public:
	// Operator overloads.
	EncBlockMotionVector operator=(EncBlockMotionVector& emv)	
		{*((BlockMotionVector*)this)=(BlockMotionVector)emv;_cost=emv._cost;_distortion=emv._distortion;_vlcEncoder=emv._vlcEncoder;return(*this);}

protected:
	VICS_INT32 _distortion;		// Distortion of the motion estimation.
	VICS_INT32 _cost;					// Cost of the estimation, typically, a rate.

protected:
	// The encapsulated implementations must be static to prevent memory explosion
	// as EncBlockMotionVector instantiations are typically in large arrays and
	// only one is required for all.
	static IVlcEncoder*	_vlcEncoder;

};// end class EncBlockMotionVector.

#endif
