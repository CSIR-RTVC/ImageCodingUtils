/** @file

MODULE				: VectorStructList

TAG						: VSL

FILE NAME			: VectorStructList.h

DESCRIPTION		: A generic base class to contain vectors in a contiguous 
								list of structs defined by the extended implementations.
								The structure of the vectors is determined by its type.

COPYRIGHT			: (c)CSIR 2007-2012 all rights resevered

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
#ifndef _VECTORSTRUCTLIST_H
#define _VECTORSTRUCTLIST_H

/*
---------------------------------------------------------------------------
	Vector type description.
---------------------------------------------------------------------------
*/

// Two Structs are defined for general 2-D vectors and 2-D patterns. Each 
// implementation either uses these or defines its own.

// Simple structure. _type = SIMPLE:
typedef struct _VSL_2D_TYPE
{
	short x;
	short y;
} VCL_2D_TYPE;

// Complex structure.	_type = COMPLEX:
typedef struct _VSL_COMPLEX_2D_TYPE
{
	int						pattern;		// Describes the meaning of the pattern of vectors.
	int						numVectors;	// Valid vectors in vec.
	VCL_2D_TYPE*	vec;				// Array of length _maxVecsPerStruct.
} VCL_COMPLEX_2D_TYPE;

/*
---------------------------------------------------------------------------
	Class definitions.
---------------------------------------------------------------------------
*/
class VectorStructList
{
	public:
		VectorStructList(int type, int dim = 2, int maxVecsPerStruct = 1);
		virtual ~VectorStructList(void);

		// Set mem size dependent on type and length of list.
		virtual int	SetLength(int length);
		// Get the struct at a specific position in the list.
		virtual void* GetStructPtr(int pos);

		int		GetType(void)								{ return(_type); }
		int		GetDimension(void)					{ return(_dim); }
		int		GetMaxVecsPerStruct(void)		{ return(_maxVecsPerStruct); }
		int		GetLength(void)							{ return(_length); }
		void* GetListPtr(void)						{ return(_pData); }

		// Vector element access interface.
		virtual void	SetSimpleElement(int pos, int element, int val);
		virtual int		GetSimpleElement(int pos, int element);
		virtual void	SetComplexElement(int pos, int vec, int element, int val);
		virtual int		GetComplexElement(int pos, int vec, int element);
    virtual void  Dump(char* filename, const char* title);

	public:
		static const int	SIMPLE2D;
		static const int	COMPLEX2D;

	protected:
		virtual void Delete(void);

	protected:
		int		_type;							// SIMPLE/COMPLEX or implementation defined.
		int		_dim;								// Vector dimension.
		int		_maxVecsPerStruct;	// Max vectors per pattern. (Default = 1 for SIMPLE.)
		int		_length;						// Num of patterns of type _type.	( Is num of vecs for SIMPLE).

		// The data list is stored in an array whose structure 
		// is dependent on the type and length.
		void*	_pData;	
};//end VectorStructList.


#endif // _VECTORSTRUCTLIST_H

