/** @file

MODULE						: VectorQuantiserDim16Imp2

TAG								: VQD16I2

FILE NAME					: VectorQuantiserDim16Imp2.h

DESCRIPTION				: A class to implement the IVectorQuantiser interface for video 
										4x4 (dim = 16) dimension 6 bit colour component values. A 
										fast implementation with a magnitude pre-search. NOTE: After 
										instantiation of an object Create() must be called.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2006  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _VECTORQUANTISERDIM16IMPL2_H
#define _VECTORQUANTISERDIM16IMPL2_H

#pragma once

#include "IVectorQuantiser.h"

/*
===========================================================================
	Class definition.
===========================================================================
*/
class VectorQuantiserDim16Impl2 : public IVectorQuantiser
{
public:
	VectorQuantiserDim16Impl2();
	virtual ~VectorQuantiserDim16Impl2();

	// IVectorQuantisation Interface.

	/** Create and destroy private mem objects.
	Methods to alloc and delete memory that is best not
	done in the constructor as failure to alloc cannot 
	be reported. (Try-catch trap is possible).
	@return	: 1 = success, 0 = failure.
	*/
	int		Create(void);
	void	Destroy(void);

	/** Get the implementation vector dimension.
	This is defined as a constant of the code book.
	@return	: Vector dimension.
	*/
	int	GetDimension(void);

	/** Get the number of vectors in the code book.
	This is defined as a constant of the code book.
	@return	: Code book length.
	*/
	int GetCodeBookLength(void);

	/** Quantise the input vector.
	Match the input vector with the implementation code book element
	that produces the smallest distortion.
	@param vector			:	Input vector to match in the code book.
	@param distortion	:	Distortion of the winning vector.
	@return						: Winning vector index.
	*/
	int Quantise(const void* vector, int* distortion);

	/** Inverse quantise the index.
	Return a pointer to the code book vector referenced by the input
	index.
	@param index	: Code book vector index.
	@return				: Reference to the code book vector.
	*/
	const void* InverseQuantise(int index);

private:
	const short** VQ_CODEBOOK;	// Fast addressing of VQ_TABLE.

	// Constants.
	static const short  VQ_TABLE[];
	static const int  TableMagnitude[];
	static const int  MagnitudeIndex[];

};// end class VectorQuantiserDim16Impl2.

#endif	// _VECTORQUANTISERDIM16IMPL2_H

