/** @file

MODULE				: RunBeforeH264VlcEncoder

TAG						: RBH264VE

FILE NAME			: RunBeforeH264VlcEncoder.h

DESCRIPTION		: A run before Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.10 page 206. The 
								zero coeffs left defines which vlc array to use. This 
								implementation is implemented with an IVlcEncoder 
								Interface.

COPYRIGHT			: (c)CSIR 2007-2010 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
								modification, are permitted provided that the following conditions 
								are met:

								* Redistributions of source code must retain the above copyright notice, 
								this list of conditions and the following disclaimer.
								* Redistributions in binary form must reproduce the above copyright notice, 
								this list of conditions and the following disclaimer in the documentation 
								and/or other materials provided with the distribution.
								* Neither the name of the CSIR nor the names of its contributors may be used 
								to endorse or promote products derived from this software without specific 
								prior written permission.

								THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
								"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
								LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
								A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
								CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
								EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
								PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
								PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
								LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
								NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
								SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#ifndef _RUNBEFOREH264VLCENCODER_H
#define _RUNBEFOREH264VLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef struct _RBH264VE_VlcCodeType
{
  int		numBits;	///< Num of bits in code word.
  int		codeWord;	///< Code word bit representation.
} RBH264VE_VlcCodeType;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RunBeforeH264VlcEncoder : public IVlcEncoder
{
public:
	RunBeforeH264VlcEncoder(void);
	virtual ~RunBeforeH264VlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface implementation.
	/// The 2 symbols represent run before and zeros left, respectively.
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

	/// Constants of table sets.
	static const RBH264VE_VlcCodeType table[15][8];

};// end class RunBeforeH264VlcEncoder.

#endif	// end _RUNBEFOREH264VLCENCODER_H.
