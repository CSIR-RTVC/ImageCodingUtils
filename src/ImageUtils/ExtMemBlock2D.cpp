/** @file

MODULE						: ExtMemBlock2D

TAG								: EMB2D

FILE NAME					: ExtMemBlock2D.cpp

DESCRIPTION				: A derived class to describe and operate on a two-dimensional
										block of memory with an extended boundary of primitive 
										types. Primarily constructed for suplimenting image 
										motion estimation algorithms.

REVISION HISTORY	:

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


#include	"ExtMemBlock2D.h"

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

ExtMemBlock2D::ExtMemBlock2D()
{
	_xOrigin		= 0;
	_yOrigin		= 0;
}//end constructor.

ExtMemBlock2D::ExtMemBlock2D(VICS_INT width, 
														 VICS_INT height, 
														 VICS_INT widthBy, 
														 VICS_INT heightBy): 
														 MemBlock2D(width + (2*widthBy),height + (2*heightBy))
{
	_xOrigin		= widthBy;
	_yOrigin		= heightBy;
}//end alt constructor.

ExtMemBlock2D::~ExtMemBlock2D()
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Public base class override interface.
---------------------------------------------------------------------------
*/

/** Fill the centre of this block from the larger input source.
Copy all values from the top left block of the source into the centre of this 
block. Extend the boundaries on all sides. If there is any mismatch then resize.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Source block width.

@param srcHeight	: Source block height.

@return 					: 0 = Alloc failure, 1 = success.
*/
VICS_INT ExtMemBlock2D::Fill(VICS_POINTER srcPtr, VICS_INT srcWidth, VICS_INT srcHeight)
{
	// Create the mem if it doesn't exist already or doesn't fit.
	if( (srcWidth < (_width - (2*_xOrigin))) || (srcHeight < (_height - (2*_yOrigin))) )
		Destroy();
	if(_pMem == NULL)
	{
		if(!Create(srcWidth + (2*_xOrigin),srcHeight + (2*_yOrigin)))
			return(0);
	}//end if _pMem...

	// Copy the centre.
	if(!MemBlock2D::Copy(srcPtr,srcWidth,srcHeight,_xOrigin,_yOrigin))
		return(0);

	// Extend the new boundaries.
	if(!ExtendBoundary())
		return(0);

	return(1);
}//end Fill.

VICS_INT ExtMemBlock2D::Copy(VICS_POINTER srcPtr, 
														 VICS_INT			srcWidth, 
														 VICS_INT			srcHeight,
														 VICS_INT			toCol,
														 VICS_INT			toRow)
{
	return(MemBlock2D::Copy(srcPtr,srcWidth,srcHeight,toCol+_xOrigin,toRow+_yOrigin));
}//end Copy.

VICS_INT ExtMemBlock2D::Copy(MemBlock2D&	smallBlock, 
														 VICS_INT			toCol,
														 VICS_INT			toRow)
{
	return(MemBlock2D::Copy(smallBlock,toCol+_xOrigin,toRow+_yOrigin));
}//end Copy.

VICS_INT ExtMemBlock2D::Transfer(MemBlock2D&	srcBlock,
																 VICS_INT		srcCol,
																 VICS_INT		srcRow,
																 VICS_INT		toCol,
																 VICS_INT		toRow,
																 VICS_INT		cols,
																 VICS_INT		rows)
{
	return(MemBlock2D::Transfer(srcBlock,srcCol,srcRow,toCol+_xOrigin,toRow+_yOrigin,cols,rows));
}//end Transfer.

void ExtMemBlock2D::Read(VICS_POINTER	dstPtr,
												 VICS_INT			dstWidth,
												 VICS_INT			dstHeight,
												 VICS_INT			fromCol,
												 VICS_INT			fromRow,
												 VICS_INT			cols,
												 VICS_INT			rows)
{
	MemBlock2D::Read(dstPtr,dstWidth,dstHeight,fromCol+_xOrigin,fromRow+_yOrigin,cols,rows);
}//end read.

void ExtMemBlock2D::Read(VICS_POINTER	dstPtr,
												 VICS_INT			dstWidth,
												 VICS_INT			dstHeight,
												 VICS_INT			fromCol,
												 VICS_INT			fromRow,
												 VICS_INT			halfColOff,
												 VICS_INT			halfRowOff,
												 VICS_INT			cols,
												 VICS_INT			rows)
{
	MemBlock2D::Read(dstPtr,dstWidth,dstHeight,fromCol+_xOrigin,fromRow+_yOrigin,halfColOff,halfRowOff,cols,rows);
}//end Read.

VICS_INT32 ExtMemBlock2D::Tse(MemBlock2D& smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow)
{
	return(MemBlock2D::Tse(smallBlock,atCol+_xOrigin,atRow+_yOrigin));
}//end Tse.

VICS_INT32 ExtMemBlock2D::ImproveTse(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT32		minEnergy)
{
	return(MemBlock2D::ImproveTse(smallBlock,atCol+_xOrigin,atRow+_yOrigin,minEnergy));
}//end ImproveTse.

VICS_INT32 ExtMemBlock2D::HalfTse(MemBlock2D& smallBlock, 
																	VICS_INT		atCol,
																	VICS_INT		atRow,
																	VICS_INT		halfColOff,
																	VICS_INT		halfRowOff)
{
	return(MemBlock2D::HalfTse(smallBlock,atCol+_xOrigin,atRow+_yOrigin,halfColOff,halfRowOff));
}//end HalfTse.

VICS_INT32 ExtMemBlock2D::ImproveHalfTse(MemBlock2D&	smallBlock, 
																				 VICS_INT			atCol,
																				 VICS_INT			atRow,
																				 VICS_INT			halfColOff,
																				 VICS_INT			halfRowOff,
																				 VICS_INT32		minEnergy)
{
	return(MemBlock2D::ImproveHalfTse(smallBlock,atCol+_xOrigin,atRow+_yOrigin,halfColOff,halfRowOff,minEnergy));
}//end ImproveHalfTse.

VICS_INT32 ExtMemBlock2D::FullSearch(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT			range,
																		 VICS_PINT		mvx,
																		 VICS_PINT		mvy)
{
	return(MemBlock2D::FullSearch(smallBlock,atCol+_xOrigin,atRow+_yOrigin,range,mvx,mvy));
}//end FullSearch.

VICS_INT32 ExtMemBlock2D::HalfSearch(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_PINT		hmvx,
																		 VICS_PINT		hmvy)
{
	return(MemBlock2D::HalfSearch(smallBlock,atCol+_xOrigin,atRow+_yOrigin,hmvx,hmvy));
}//end HalfSearch.

VICS_INT32 ExtMemBlock2D::Search(MemBlock2D&	smallBlock, 
																 VICS_INT			atCol,
																 VICS_INT			atRow,
																 VICS_INT			range,
																 VICS_PINT		mvx,
																 VICS_PINT		mvy,
																 VICS_PINT32	zeroEnergy)
{
	return(MemBlock2D::Search(smallBlock,atCol+_xOrigin,atRow+_yOrigin,range,mvx,mvy,zeroEnergy));
}//end Search.

VICS_INT32 ExtMemBlock2D::CandidateSearch(MemBlock2D&	smallBlock, 
																					VICS_INT		atCol,
																					VICS_INT		atRow,
																					VICS_INT		range,
																					VICS_PINT		mvx,
																					VICS_PINT		mvy,
																					VICS_PINT32	zeroEnergy)
{
	return(MemBlock2D::CandidateSearch(smallBlock,atCol+_xOrigin,atRow+_yOrigin,range,mvx,mvy,zeroEnergy));
}//end CandidateSearch.

VICS_INT32 ExtMemBlock2D::CandidateSearch(MemBlock2D&	smallBlock, 
																					VICS_INT		atCol,
																					VICS_INT		atRow,
																					VICS_INT		range,
																					VICS_PINT		mvx,
																					VICS_PINT		mvy,
																					VICS_INT		fvx,
																					VICS_INT		fvy,
																					VICS_PINT32	favouredEnergy)
{
	return(MemBlock2D::CandidateSearch(smallBlock,atCol+_xOrigin,atRow+_yOrigin,range,mvx,mvy,fvx,fvy,favouredEnergy));
}//end CandidateSearch.

VICS_INT ExtMemBlock2D::OptimalRateSearch(MemBlock2D&	smallBlock, 
																					VICS_INT		atCol,
																					VICS_INT		atRow,
																					VICS_INT		range,
																					VICS_PINT		mvx,
																					VICS_PINT		mvy,
																					VICS_INT		biasx,
																					VICS_INT		biasy,
																					VICS_PINT		cost)
{
	return(MemBlock2D::OptimalRateSearch(smallBlock,atCol+_xOrigin,atRow+_yOrigin,range,mvx,mvy,biasx,biasy,cost));
}//end OptimalRateSearch.

/*
---------------------------------------------------------------------------
	Public extended interface.
---------------------------------------------------------------------------
*/

/** Extend the boundary of this block with the edge.
Repeat the edge values of the centre into the boundaries. 

@return 					: 0 = failure, 1 = success.
*/
VICS_INT ExtMemBlock2D::ExtendBoundary(void)
{
	if(_pMem == NULL)
		return(0);

	VICS_INT x,y;

	// Top left.
	for(y = 0; y < _yOrigin; y++)
		for(x = 0; x < _xOrigin; x++)
			_pBlock[y][x] = _pBlock[_yOrigin][_xOrigin];

	// Top right.
	for(y = 0; y < _yOrigin; y++)
		for(x = (_width - _xOrigin); x < _width; x++)
			_pBlock[y][x] = _pBlock[_yOrigin][_width - _xOrigin - 1];

	// Bottom left.
	for(y = (_height - _yOrigin); y < _height; y++)
		for(x = 0; x < _xOrigin; x++)
			_pBlock[y][x] = _pBlock[_height - _yOrigin - 1][_xOrigin];

	// Bottom right.
	for(y = (_height - _yOrigin); y < _height; y++)
		for(x = (_width - _xOrigin); x < _width; x++)
			_pBlock[y][x] = _pBlock[_height - _yOrigin - 1][_width - _xOrigin - 1];

	// Top.
	for(y = 0; y < _yOrigin; y++)
		memcpy((VICS_POINTER)(&(_pBlock[y][_xOrigin])),
					 (VICS_POINTER)(&(_pBlock[_yOrigin][_xOrigin])),
					 (_width - (2*_xOrigin))*sizeof(MB2D_TYPE));

	// Bottom.
	for(y = (_height - _yOrigin); y < _height; y++)
		memcpy((VICS_POINTER)(&(_pBlock[y][_xOrigin])),
					 (VICS_POINTER)(&(_pBlock[_height - _yOrigin - 1][_xOrigin])),
					 (_width - (2*_xOrigin))*sizeof(MB2D_TYPE));

	// Left.
	for(y = _yOrigin; y < (_height - _yOrigin); y++)
		for(x = 0; x < _xOrigin; x++)
			_pBlock[y][x] = _pBlock[y][_xOrigin];

	// Right.
	for(y = _yOrigin; y < (_height - _yOrigin); y++)
		for(x = (_width - _xOrigin); x < _width; x++)
			_pBlock[y][x] = _pBlock[y][_width - _xOrigin - 1];

	return(1);
}//end ExtendBoundary.

/*
---------------------------------------------------------------------------
	Private Methods.
---------------------------------------------------------------------------
*/


