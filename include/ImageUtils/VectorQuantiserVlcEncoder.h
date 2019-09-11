/** @file

MODULE						: VectorQuantiserVlcEncoder

TAG								: VQVE

FILE NAME					: VectorQuantiserVlcEncoder.h

DESCRIPTION				: A vector quantiser Vlc encoder implementation with an
										IVlcEncoder Interface.

REVISION HISTORY	:

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _VECTORQUANTISERVLCENCODER_H
#define _VECTORQUANTISERVLCENCODER_H

#include "VicsDefs\VicsDefs.h"
#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class constants.
---------------------------------------------------------------------------
*/
#define VQVE_TABLE_SIZE 256
#define VQVE_NUM_BITS		0
#define VQVE_BIT_CODE		1

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class VectorQuantiserVlcEncoder : public IVlcEncoder
{
protected:
	VICS_INT32 	_numCodeBits;	// Number of coded bits for this motion vector.
	VICS_INT32 	_bitCode;			// The code of length numCodeBits.

	// Constants.
	static const VICS_INT VLC_TABLE[VQVE_TABLE_SIZE][2];

public:
	VectorQuantiserVlcEncoder()	{_numCodeBits=0;_bitCode=0;}
	virtual ~VectorQuantiserVlcEncoder()	{ }

	// Interface implementation.
	virtual VICS_INT32 GetNumCodedBits(void)	{return(_numCodeBits);}
	virtual VICS_INT32 GetCode(void)					{return(_bitCode);}

	virtual VICS_INT32 Encode(VICS_INT symbol) 
		{
			if(symbol<VQVE_TABLE_SIZE){_numCodeBits=VLC_TABLE[symbol][VQVE_NUM_BITS];_bitCode=VLC_TABLE[symbol][VQVE_BIT_CODE];}
			else{_numCodeBits=0;_bitCode=0;}
			return(_numCodeBits);
		}//end Encode.

	// Operator overloads.
	VectorQuantiserVlcEncoder operator=(VectorQuantiserVlcEncoder& vqve)	
		{_numCodeBits=vqve._numCodeBits;_bitCode=vqve._bitCode;return(*this);}

};// end class VectorQuantiserVlcEncoder.

#endif
