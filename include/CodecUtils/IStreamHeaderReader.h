/** @file

MODULE						: IStreamHeaderReader

TAG								: ISHR

FILE NAME					: IStreamHeaderReader.h

DESCRIPTION				: An IStreamHeaderReader Interface is an abstract base class 
										to access stream headers that have bit representations. 
										The access is defined by text (ASCII) strings. Derived 
										classes should include members for each header parameter
										and an associated Get() method.

COPYRIGHT			: (c)CSIR 2007-2018 all rights resevered

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
#ifndef _ISTREAMHEADERREADER_H
#define _ISTREAMHEADERREADER_H

#pragma once

class IStreamHeaderReader
{
public:
	virtual ~IStreamHeaderReader() {}

	/** Extract the header from the input stream.
	Extract the header from the input stream into the derived
	class members.
	@param pSS		: Stream to extract from.
	@param bitLen	: Bit length of input stream.
	@return				: 1 = success, 0 = failed.
	*/
	virtual int Extract(void* pSS, int bitLen) = 0;

  /** Get the header length in bits.
  @return : Length in bits.
  */
	virtual int GetHeaderBitLength(void) = 0;

	/** Get the header value by name.
	This is the primary method of getting all header variables and caters 
	for all variants by using strings. Care must be taken when unicode strings
	are used. Typical usage:
		int	val;
		if( !Impl->Get((const unsigned char*)("width"), &val) )
			errorStr = "Header value does not exist";

	@param name		:	A string of the header variable required.
	@param value	:	The header value required.
	@return				: 1 = found, 0 = no header exists with this name.
	*/
	virtual int Get(const char* name, int* value) = 0;

};// end class IStreamHeaderReader.

#endif	///<_ISTREAMHEADERREADER_H
