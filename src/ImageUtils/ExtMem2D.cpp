/** @file

MODULE						: ExtMem2D

TAG								: EM2D

FILE NAME					: ExtMem2D.cpp

DESCRIPTION				: A derived class to describe and operate on a two-dimensional
										block of memory with an extended boundary of primitive 
										types. Primarily constructed for suplimenting image 
										motion estimation algorithms and hiding the origin shift
										from the caller.

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


#include	"ExtMem2D.h"

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

ExtMem2D::ExtMem2D()
{
	_xOrigin			= 0;
	_yOrigin			= 0;
	_innerWidth		= 0;
	_innerHeight	= 0;
}//end constructor.

ExtMem2D::ExtMem2D(int width, int height, int widthBy, int heightBy): 
									 Mem2D(width + (2*widthBy),height + (2*heightBy))
{
	_xOrigin			= widthBy;
	_yOrigin			= heightBy;
	_innerWidth		= width;
	_innerHeight	= height;
}//end alt constructor.

ExtMem2D::~ExtMem2D()
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Public base class override interface.
---------------------------------------------------------------------------
*/

/** Set this block by filling it from the input source block.
Copy all values from the top left block of the source into this block. Resize
this block if it will not fit and create this block if it does not exist. The
result leaves this block completely filled.

@param srcBlock		: Source reference. 

@return 					: 0 = Mem alloc failure, 1 = success.
*/
int ExtMem2D::Set(Mem2D& srcBlock)
{
	return( Set( (srcBlock.Get2DPtr())[0], srcBlock.GetWidth(), srcBlock.GetHeight()) );
}//end Set.

/** Set this block by filling it from the input source.
Copy all values from the top left block of the source into this block. Extend 
the boundaries on all sides. Resize	this block if it will not fit and create 
this block if it does not exist. The result leaves this block completely filled.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Source block width.

@param srcHeight	: Source block height.

@return 					: 0 = Mem alloc failure, 1 = success.
*/
int ExtMem2D::Set(void* srcPtr, int width, int height)
{
	// Create the mem if it doesn't exist already or doesn't fit.
	if( (width < (_width - (2*_xOrigin))) || (height < (_height - (2*_yOrigin))) )
		Destroy();
	if(_pMem == NULL)
	{
		if(!Create(width + (2*_xOrigin),height + (2*_yOrigin)))
			return(0);
	}//end if _pMem...

	_innerWidth		= width;
	_innerHeight	= height;

	// Copy the centre.
	if(!Mem2D::Write(srcPtr, width, height, _xOrigin, _yOrigin, width, height))
		return(0);

	// Extend the new boundaries.
	if(!ExtendBoundary())
		return(0);

	return(1);
}//end Set.

// Part/all of the source to all of the inner block.
int	ExtMem2D::Write(void* srcPtr, int srcWidth, int srcHeight)
{
	return(Mem2D::Write(srcPtr, srcWidth, srcHeight, 
															_xOrigin, _yOrigin, 
															_innerWidth, _innerHeight));
}//end Write.

// Part/all of the source to part of the block.
int	ExtMem2D::Write(void* srcPtr, int srcWidth, int srcHeight,
																	int toCol,		int toRow,
																	int cols,	    int rows)
{
	return(Mem2D::Write(srcPtr, srcWidth,				srcHeight, 
															toCol+_xOrigin, toRow+_yOrigin,
															cols,						rows));
}//end Write.

// All of the source to part of the block.
int	ExtMem2D::Write(Mem2D&	srcBlock, int	toCol, int toRow)
{
	return(Mem2D::Write(srcBlock, toCol+_xOrigin, toRow+_yOrigin));
}//end Write.

// Part of the source to part of the block.
int	ExtMem2D::Write(Mem2D&	srcBlock,	int fromCol,	int	fromRow,
																			int toCol,		int	toRow,
																			int cols,			int	rows)
{
	return(Mem2D::Write(srcBlock, fromCol,				fromRow, 
																toCol+_xOrigin, toRow+_yOrigin, 
																cols,						rows));
}//end Write.

// All of source to all of the inner part of the block.
int ExtMem2D::Write(Mem2D&	srcBlock)
{
	if( (_innerWidth != srcBlock.GetWidth())||(_innerHeight != srcBlock.GetHeight()) )
		return(0);

	Mem2D::Write(srcBlock, _xOrigin, _yOrigin);
	return(1);
}//end Write.

// All of the inner block to part/all of the destination.
void ExtMem2D::Read(void* dstPtr, int dstWidth, int dstHeight)
{
	Mem2D::Read(dstPtr, dstWidth,	dstHeight, _xOrigin, _yOrigin, _innerWidth, _innerHeight);
}//end Read.

// Part of the block to part/all of the destination.
void ExtMem2D::Read(void* dstPtr, int dstWidth, int dstHeight,
																	int fromCol,	int fromRow,
																	int cols,			int rows)
{
	Mem2D::Read(dstPtr, dstWidth, dstHeight, fromCol+_xOrigin, fromRow+_yOrigin, cols, rows);
}//end Read.

// Part of the block to part/all of the destination.
void ExtMem2D::Read(Mem2D&	dstBlock, int toCol,		int toRow,
																			int fromCol,	int fromRow,
																			int cols,			int rows)
{
	Mem2D::Read(dstBlock, toCol, toRow, fromCol+_xOrigin, fromRow+_yOrigin,	cols,	rows);
}//end Read.

// Part of the block at half pel position to part/all of the destination.
void ExtMem2D::HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
																			int fromCol,		int fromRow,
																			int halfColOff, int halfRowOff,
																			int cols,				int rows)
{
	Mem2D::HalfRead(dstPtr, dstWidth,	dstHeight, fromCol+_xOrigin, fromRow+_yOrigin,
													halfColOff,	halfRowOff,	cols,	rows);
}//end HalfRead.

// All of the middle part to all of the destination block.
int ExtMem2D::Read(Mem2D&	dstBlock)
{
	if( (_innerWidth != dstBlock.GetWidth())||(_innerHeight != dstBlock.GetHeight()) )
		return(0);

	Mem2D::Read(dstBlock, 0, 0, _xOrigin, _yOrigin, _innerWidth, _innerHeight);
	return(1);
}//end Read.

/*
---------------------------------------------------------------------------
	Public extended interface.
---------------------------------------------------------------------------
*/

/** Extend the boundary of this 2D mem with the edge.
Repeat the edge values of the centre into the boundaries. 

@return 					: 0 = failure, 1 = success.
*/
int ExtMem2D::ExtendBoundary(void)
{
	if(_pMem == NULL)
		return(0);

	int x,y;

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
		memcpy((void *)(&(_pBlock[y][_xOrigin])),
					 (void *)(&(_pBlock[_yOrigin][_xOrigin])),
					 (_width - (2*_xOrigin))*sizeof(M2D_TYPE));

	// Bottom.
	for(y = (_height - _yOrigin); y < _height; y++)
		memcpy((void *)(&(_pBlock[y][_xOrigin])),
					 (void *)(&(_pBlock[_height - _yOrigin - 1][_xOrigin])),
					 (_width - (2*_xOrigin))*sizeof(M2D_TYPE));

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


