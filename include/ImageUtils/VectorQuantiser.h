/** @file

MODULE						: VectorQuantiser

TAG								: VQ

FILE NAME					: VectorQuantiser.h

DESCRIPTION				: Definition of the minimum interface of a base class for
										vector quantisation objects. Variable length bit encoding
										is included.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _VECTORQUANTISER_H
#define _VECTORQUANTISER_H

#include "VicsDefs/VicsDefs.h"

/*
===========================================================================
	Class definition.
===========================================================================
*/
class VectorQuantiser
{
public:
	VectorQuantiser()		{ }
	virtual ~VectorQuantiser()	{ }

	// Member access.
	virtual const VICS_INT	GetVectorDimension(void) = 0;

	// Interface.
	virtual VICS_INT	Quantise(VICS_PINT vector, VICS_PINT squareError) = 0;
	virtual const VICS_PINT InverseQuantise(VICS_INT symbol)						= 0;
	virtual VICS_INT	Encode(VICS_INT symbol, VICS_PINT codeWord)				= 0;

};// end class VectorQuantiser.

#endif
