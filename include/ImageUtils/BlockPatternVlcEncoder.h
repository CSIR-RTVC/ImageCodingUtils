/** @file

MODULE						: BlockPatternVlcEncoder

TAG								: BPVE

FILE NAME					: BlockPatternVlcEncoder.h

DESCRIPTION				: A block pattern Vlc encoder implementation with an
										IVlcEncoder Interface.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _BLOCKPATTERNVLCENCODER_H
#define _BLOCKPATTERNVLCENCODER_H

#include "VicsDefs/VicsDefs.h"
#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class constants.
---------------------------------------------------------------------------
*/
#define BPVE_TABLE_SIZE 4
#define BPVE_NUM_BITS		0
#define BPVE_BIT_CODE		1

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BlockPatternVlcEncoder : public IVlcEncoder
{
protected:
	VICS_INT32 	_numCodeBits;	// Number of coded bits for this motion vector.
	VICS_INT32 	_bitCode;			// The code of length numCodeBits.

	// Constants.
	static const VICS_INT VLC_TABLE[BPVE_TABLE_SIZE][2];

public:
	BlockPatternVlcEncoder()	{_numCodeBits=0;_bitCode=0;}
	virtual ~BlockPatternVlcEncoder()	{ }

	// Interface implementation.
	virtual VICS_INT32 GetNumCodedBits(void)	{return(_numCodeBits);}
	virtual VICS_INT32 GetCode(void)					{return(_bitCode);}

	virtual VICS_INT32 Encode(VICS_INT symbol) 
		{
			if(symbol<BPVE_TABLE_SIZE){_numCodeBits=VLC_TABLE[symbol][BPVE_NUM_BITS];_bitCode=VLC_TABLE[symbol][BPVE_BIT_CODE];}
			else{_numCodeBits=0;_bitCode=0;}
			return(_numCodeBits);
		}//end Encode.

	// Operator overloads.
	BlockPatternVlcEncoder operator=(BlockPatternVlcEncoder& bpve)	
		{_numCodeBits=bpve._numCodeBits;_bitCode=bpve._bitCode;return(*this);}

};// end class BlockPatternVlcEncoder.

#endif
