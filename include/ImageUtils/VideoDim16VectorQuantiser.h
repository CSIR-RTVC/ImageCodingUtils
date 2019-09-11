/** @file

MODULE						: VideoDim16VectorQuantiser

TAG								: VD16VQ

FILE NAME					: VideoDim16VectorQuantiser.h

DESCRIPTION				: A class to extend the VectorQuantiser class for video 4x4
										dimension 6 bit colour component values.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _VIDEODIM16VECTORQUANTISER_H
#define _VIDEODIM16VECTORQUANTISER_H

#include "../VicsDefs/VicsDefs.h"
#include "VectorQuantiser.h"

/*
===========================================================================
	Class constants.
===========================================================================
*/
#define VD16VQ_TABLE_SIZE	256

/*
===========================================================================
	Class definition.
===========================================================================
*/
class VideoDim16VectorQuantiser : public VectorQuantiser
{
protected:

	// Constants.
	static const VICS_INT  VECTOR_DIMENSION;	// Read only and implementation dependent.
	static const VICS_INT  VLC_TABLE[VD16VQ_TABLE_SIZE][2];
	static const VICS_INT  VQ_TABLE[VD16VQ_TABLE_SIZE*16];
	static const VICS_PINT VQ_CODEBOOK[VD16VQ_TABLE_SIZE];	// Fast addressing of VQ_TABLE.

public:
	VideoDim16VectorQuantiser()		{ }
	virtual ~VideoDim16VectorQuantiser()	{ }

	// Member access.
	virtual const VICS_INT	GetVectorDimension(void) { return(VECTOR_DIMENSION); }

	// Interface.
	virtual VICS_INT	Quantise(VICS_PINT vector, VICS_PINT squareError);

	/** Reference the codebook vector.

	@param symbol				: Codebook reference.

	@return 						: Symbol vector of dimension VECTOR_DIMENSION.
	*/
	virtual const VICS_PINT InverseQuantise(VICS_INT symbol) 
		{ return(VQ_CODEBOOK[symbol]); }

	/** Find the VLC of the codebook symbol.

	@param symbol				: Codebook reference.

	@param codeWord			: Return the bit code of the symbol.

	@return 						: Number of bits in the bit code.
	*/
	virtual VICS_INT Encode(VICS_INT symbol, VICS_PINT codeWord) 
		{ *codeWord = VLC_TABLE[symbol][1];	return(VLC_TABLE[symbol][0]); }

};// end class VideoDim16VectorQuantiser.

#endif
