/** @file

MODULE						: IMotionFactory

TAG								: IMF

FILE NAME					: IMotionFactory.h

DESCRIPTION				: A IMotionFactory Interface as an abstract base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _IMOTIONFACTORY_H
#define _IMOTIONFACTORY_H

#include "BlockMotionVector.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class IMotionFactory
{
public:
	/*
	--------------------------------------------------------------------------
	Interface implementation.
	--------------------------------------------------------------------------
	*/
	virtual VICS_INT						Initialise(VICS_INT pelWidth,
																				 VICS_INT pelHeight,
																				 VICS_INT blockWidth,
																				 VICS_INT blockHeight)	= 0;

	virtual BlockMotionVector**	GetBlockMotionVectorArray(void)		= 0;
	virtual VICS_INT						GetNumMotionVectors(void)					= 0;
	virtual	VICS_INT						GetNumXMotionVectors(void)				= 0;
	virtual	VICS_INT						GetNumYMotionVectors(void)				= 0;

};// end class IMotionFactory.

#endif
