/** @file

MODULE						: MemBlock2D

TAG								: MB2D

FILE NAME					: MemBlock2D.cpp

DESCRIPTION				: A class to describe and operate on a two-dimensional
										block of memory of primitive types.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
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


#include	"MemBlock2D.h"
#include	"EncMotionVector.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const VICS_COORD MemBlock2D::HALF_POSITION[MB2D_HALF_POS_ARRAY_LENGTH] =
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

void MemBlock2D::ResetMembers(void)
{
	_width						= 0;
	_height						= 0;
	_pMem							= NULL;
	_pBlock						= NULL;
	_candidateLength	= 0;
}//end ResetMembers.

MemBlock2D::MemBlock2D()
{
	ResetMembers();
}//end constructor.

MemBlock2D::MemBlock2D(VICS_INT width, VICS_INT height)
{
	ResetMembers();

	// Potentially dangerous to alloc mem in a constructor as there is no way 
	// of determining failure.
	Create(width,height);
	
}//end alt constructor.

MemBlock2D::~MemBlock2D()
{
	// Ensure no mem is left alloc.
	Destroy();
}//end destructor.

void MemBlock2D::Destroy(void)
{
	if(_pBlock != NULL)
		delete[] _pBlock;
	_pBlock = NULL;

	if(_pMem != NULL)
		delete[] _pMem;
	_pMem = NULL;

}//end Destroy.

VICS_INT MemBlock2D::Create(VICS_INT width, VICS_INT height)
{
	// Clear first.
	if(_pMem != NULL)
		Destroy();
	_width  = 0;
	_height = 0;

	// Alloc the mem block.
	_pMem = new MB2D_TYPE[width * height];
	if(_pMem == NULL)
		return(0);

	// Alloc the mem block row addresses.
	_pBlock = new MB2D_PTYPE[height];
	if(_pBlock == NULL)
	{
		Destroy();
		return(0);
	}//end if _pBlock...

	// Successful alloc.
	_width	= width;
	_height	= height;

	// Fill the row addresses.
	for(VICS_INT row = 0; row < height; row++)
		_pBlock[row] = &(_pMem[width * row]);

	return(1);
}//end Create.

/*
---------------------------------------------------------------------------
	Public interface.
---------------------------------------------------------------------------
*/

/** Fill this block from the larger input source.
Copy all values from the top left block of the source into this block. Resize
this block if it will not fit and create this block if it does not exist. The
result leaves this block completely filled.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Source block width.

@param srcHeight	: Source block height.

@return 					: 0 = Mem alloc failure, 1 = success.
*/
VICS_INT MemBlock2D::Fill(VICS_POINTER srcPtr, VICS_INT srcWidth, VICS_INT srcHeight)
{
	// Create the mem if it doesn't exist already or doesn't fit.
	if( (_width > srcWidth) || (_height > srcHeight) )
		Destroy();
	if(_pMem == NULL)
	{
		if(!Create(srcWidth,srcHeight))
			return(0);
	}//end if _pMem...

	// This object is filled with a _width X _height 2-D block of values from 
	// the top left corner of the source.
	MB2D_PTYPE	pLcl = (MB2D_PTYPE)srcPtr;
	VICS_INT		col,row;
	for(row = 0; row < _height; row++)
		for(col = 0; col < _width; col++)
			_pBlock[row][col] = pLcl[(row * srcWidth) + col];

	return(1);
}//end Fill.

/** Copy the entire smaller input source into an offset within this block.
Copy all values from the top left of the source into this block starting
at the input specified offset. Return if it will not fit or if this block
does not exist.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Source block width.

@param srcHeight	: Source block height.

@param toCol			: Width offset.

@param toRow			: Height offset.

@return 					: 0 = failure, 1 = success.
*/
VICS_INT MemBlock2D::Copy(VICS_POINTER srcPtr, 
													VICS_INT		 srcWidth, 
													VICS_INT		 srcHeight,
													VICS_INT		 toCol,
													VICS_INT		 toRow)
{
	// Check before copying.
	if( (_pMem == NULL) || (srcWidth > (_width - toCol)) || (srcHeight > (_height - toRow)) )
		return(0);

	// This object is filled from the to offset with a srcWidth X srcHeight 
	// 2-D block of values from the top left corner of the source.
	MB2D_PTYPE	pLcl = (MB2D_PTYPE)srcPtr;
	VICS_INT		col,row;
	for(row = 0; row < srcHeight; row++)
		for(col = 0; col < srcWidth; col++)
			_pBlock[toRow + row][toCol + col] = pLcl[(row * srcWidth) + col];

	return(1);
}//end Copy.

/** Copy the entire smaller source block into an offset within this block.
Copy all values from the top left of the source into this block starting
at the input specified offset. Return if it will not fit or if this block
does not exist.

@param smallBlock	: The block to copy. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param toCol			: Column offset to match zero column of input block.

@param toRow			: Row offset to match zero row of input block.

@return 					: 0 = failure, 1 = success.
*/
VICS_INT MemBlock2D::Copy(MemBlock2D& smallBlock, VICS_INT toCol, VICS_INT toRow)
{
	VICS_INT srcWidth	 = smallBlock._width;
	VICS_INT srcHeight = smallBlock._height;

	// Check before copying.
	if( (_pMem == NULL) || (srcWidth > (_width - toCol)) || (srcHeight > (_height - toRow)) )
		return(0);

	// This object is filled from the start offset with a srcWidth X srcHeight 
	// 2-D block of values from the top left corner of the source.
	VICS_INT col,row;
	for(row = 0; row < srcHeight; row++)
		for(col = 0; col < srcWidth; col++)
			_pBlock[toRow + row][toCol + col] = smallBlock._pBlock[row][col];

	return(1);
}//end Copy.

/** Transfer from a source block into an offset within this block.
Transfer all values that will fit. Existance of the source is assumed.
Return if this block does not exist.

@param srcBlock	: The block to transfer from. 

@param srcCol		: Source column offset.

@param srcRow		: Source row offset.

@param toCol		: Destination column offset.

@param toRow		: Destination row offset.

@param cols			: Columns to transfer.

@param rows			: Rows to transfer.

@return 				: 0 = failure, 1 = success.
*/
VICS_INT MemBlock2D::Transfer(MemBlock2D&	srcBlock,
															VICS_INT		srcCol,
															VICS_INT		srcRow,
															VICS_INT		toCol,
															VICS_INT		toRow,
															VICS_INT		cols,
															VICS_INT		rows)
{
	// Check before copying.
	if(_pMem == NULL)
		return(0);

	VICS_INT colLimit = cols;
	VICS_INT rowLimit = rows;
	if((srcCol + cols) > srcBlock._width) colLimit = srcBlock._width - srcCol;
	if((srcRow + rows) > srcBlock._height) rowLimit = srcBlock._height - srcRow;
	if((toCol + colLimit) > _width) colLimit = _width - toCol;
	if((toRow + rowLimit) > _height) rowLimit = _height - toRow;

	VICS_INT col,row;
	for(row = 0; row < rowLimit; row++)
		for(col = 0; col < colLimit; col++)
			_pBlock[toRow + row][toCol + col] = srcBlock._pBlock[srcRow + row][srcCol + col];

	return(1);
}//end Transfer.

/** Read from an offset within this block to a 2D destination.
No mem column or row overflow checking is done during the read process. Use
this method with caution.

@param dstPtr			: Destination to read to.

@param dstWidth		: Total destination width.

@param dstHeight	: Total destination height.

@param fromCol		: Block source column offset.

@param fromRow		: Block source row offset.

@param cols				: Columns to read.

@param rows				: Rows to read.

@return 					: None.
*/
void MemBlock2D::Read(VICS_POINTER	dstPtr,
											VICS_INT			dstWidth,
											VICS_INT			dstHeight,
											VICS_INT			fromCol,
											VICS_INT			fromRow,
											VICS_INT			cols,
											VICS_INT			rows)
{
	MB2D_PTYPE	pLcl = (MB2D_PTYPE)dstPtr;
	VICS_INT		col,row,srcRow,dstRow;
	for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
	{
		VICS_INT srcCol,dstPos;
		for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
			pLcl[dstPos] = _pBlock[srcRow][srcCol];
	}//end for row...
}//end Read.

/** Read from an offset + half location within this block to a 2D destination.
No mem column or row overflow checking is done during the read process. Use
this method with caution.

@param dstPtr			: Destination to read to.

@param dstWidth		: Total destination width.

@param dstHeight	: Total destination height.

@param fromCol		: Block source column offset.

@param fromRow		: Block source row offset.

@param halfColOff	: Half location offset from fromCol.

@param halfRowOff	: Half location offset from fromRow.

@param cols				: Columns to read.

@param rows				: Rows to read.

@return 					: None.
*/
void MemBlock2D::Read(VICS_POINTER	dstPtr,
											VICS_INT			dstWidth,
											VICS_INT			dstHeight,
											VICS_INT			fromCol,
											VICS_INT			fromRow,
											VICS_INT			halfColOff,
											VICS_INT			halfRowOff,
											VICS_INT			cols,
											VICS_INT			rows)
{
	MB2D_PTYPE	pLcl = (MB2D_PTYPE)dstPtr;
	VICS_INT		col,row,srcRow,dstRow;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			VICS_INT srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
			{
				VICS_INT32 z =	((VICS_INT32)_pBlock[srcRow           ][srcCol] +	
									 			 (VICS_INT32)_pBlock[srcRow           ][srcCol+halfColOff] +
									 			 (VICS_INT32)_pBlock[srcRow+halfRowOff][srcCol] + 
									 			 (VICS_INT32)_pBlock[srcRow+halfRowOff][srcCol+halfColOff] + 2) >> 2;
				pLcl[dstPos] = (MB2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			VICS_INT srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
			{
				VICS_INT32 z =	((VICS_INT32)_pBlock[srcRow           ][srcCol] +	
									 			 (VICS_INT32)_pBlock[srcRow+halfRowOff][srcCol+halfColOff] + 1) >> 1;
				pLcl[dstPos] = (MB2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			VICS_INT srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
				pLcl[dstPos] = _pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
}//end Read.

/** Calculate the total square error with a smaller input block at an offset position.
Return the sum of the square errors with the input block values aligned at the
offset within this block specified by the input parameters. NOTE: No checking is done
to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@return 					: Sum of square error.
*/
VICS_INT32 MemBlock2D::Tse(MemBlock2D& smallBlock, VICS_INT atCol, VICS_INT atRow)
{
	VICS_INT		x,y,sx,sy;
	VICS_INT32	distortion	= 0;
	VICS_INT		width				= smallBlock._width;
	VICS_INT		height			= smallBlock._height;

	for(sy = 0, y = atRow; sy < height; sy++, y++)
		for(sx = 0, x = atCol; sx < width; sx++, x++)
	{
		VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - (VICS_INT32)_pBlock[y][x];
		distortion		 += (diff * diff);
	}//end for sy & sx...

	return(distortion);
}//end Tse.

/** Improve on the total square error with a smaller input block at an offset position.
Return the improved sum of the square errors with the input block values aligned at the
offset within this block specified by the input parameters. Jump out if the squared
error rises above the input min energy. NOTE: No checking is done
to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param minEnergy	: Energy to improve on.

@return 					: Sum of square error of improvement.
*/
VICS_INT32 MemBlock2D::ImproveTse(MemBlock2D&	smallBlock, 
																	VICS_INT			atCol, 
																	VICS_INT			atRow,
																	VICS_INT32		minEnergy)
{
	VICS_INT		x,y,sx,sy;
	VICS_INT32	distortion	= 0;
	VICS_INT		width				= smallBlock._width;
	VICS_INT		height			= smallBlock._height;

	for(sy = 0, y = atRow; sy < height; sy++, y++)
		for(sx = 0, x = atCol; sx < width; sx++, x++)
	{
		VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - (VICS_INT32)_pBlock[y][x];
		distortion		 += (diff * diff);
		if(distortion > minEnergy)
			goto MB2D_ImproveTseBreak;
	}//end for sy & sx...

	MB2D_ImproveTseBreak:

	return(distortion);
}//end ImproveTse.

/** Calculate the total square error with a smaller input block at a half offset position.
Return the sum of the square errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. 
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param halfColOff	: Further half offset from atCol.

@param halfRowOff	: Further half offset from atRow.

@return 					: Sum of square error.
*/
VICS_INT32 MemBlock2D::HalfTse(MemBlock2D&	smallBlock, 
															 VICS_INT			atCol,
															 VICS_INT			atRow,
															 VICS_INT			halfColOff,
															 VICS_INT			halfRowOff)
{
	VICS_INT		x,y,sx,sy;
	VICS_INT32	distortion	= 0;
	VICS_INT		width				= smallBlock._width;
	VICS_INT		height			= smallBlock._height;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y           ][x+halfColOff] +
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x] + 
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 2) >> 2;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			distortion		 += (diff * diff);
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 1) >> 1;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			distortion		 += (diff * diff);
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - (VICS_INT32)_pBlock[y][x];
			distortion		 += (diff * diff);
		}//end for sy & sx...
	}//end else...

	return(distortion);
}//end HalfTse.

/** Half location total square error with worst case (Overloaded).
Return the sum of the square errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. 
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock		: The block to test with. The sum of atCol and the input block
											width must be less than this width. Similarly for atRow. 

@param atCol				: Column offset to match zero column of input block.

@param atRow				: Row offset to match zero row of input block.

@param halfColOff		: Further half offset from atCol.

@param halfRowOff		: Further half offset from atRow.

@param worstEnergy	: Return the worst case energy within the block.

@return 						: Sum of square error.
*/
VICS_INT32 MemBlock2D::HalfTse(MemBlock2D&	smallBlock, 
															 VICS_INT			atCol,
															 VICS_INT			atRow,
															 VICS_INT			halfColOff,
															 VICS_INT			halfRowOff,
															 VICS_PINT32	worstEnergy)
{
	VICS_INT		x,y,sx,sy;
	VICS_INT32	worst				= 0;
	VICS_INT32	distortion	= 0;
	VICS_INT		width				= smallBlock._width;
	VICS_INT		height			= smallBlock._height;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y           ][x+halfColOff] +
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x] + 
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 2) >> 2;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			VICS_INT32 d2 = diff * diff;
			if( d2 > worst )
				worst = d2;
			distortion += d2;
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 1) >> 1;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			VICS_INT32 d2 = diff * diff;
			if( d2 > worst )
				worst = d2;
			distortion += d2;
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - (VICS_INT32)_pBlock[y][x];
			VICS_INT32 d2 = diff * diff;
			if( d2 > worst )
				worst = d2;
			distortion += d2;
		}//end for sy & sx...
	}//end else...

	*worstEnergy = worst;
	return(distortion);
}//end HalfTse.

/** Improve on the total square error with a smaller input block at a half offset position.
Return the improved sum of the square errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. Jump out
as soon as the input min energy is exceeded. NOTE: No checking is done to ensure row 
and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param halfColOff	: Further half offset from atCol.

@param halfRowOff	: Further half offset from atRow.

@param minEnergy	: Energy to improve on.

@return 					: Sum of improved square error.
*/
VICS_INT32 MemBlock2D::ImproveHalfTse(MemBlock2D&	smallBlock, 
																			VICS_INT		atCol,
																			VICS_INT		atRow,
																			VICS_INT		halfColOff,
																			VICS_INT		halfRowOff,
																			VICS_INT32	minEnergy)
{
	VICS_INT		x,y,sx,sy;
	VICS_INT32	distortion	= 0;
	VICS_INT		width				= smallBlock._width;
	VICS_INT		height			= smallBlock._height;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y           ][x+halfColOff] +
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x] + 
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 2) >> 2;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			distortion		 += (diff * diff);
			if(distortion > minEnergy)
				goto MB2D_ImproveHalfTseBreak;
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 z		=	((VICS_INT32)_pBlock[y           ][x] +	
									 			 (VICS_INT32)_pBlock[y+halfRowOff][x+halfColOff] + 1) >> 1;
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - z;
			distortion		 += (diff * diff);
			if(distortion > minEnergy)
				goto MB2D_ImproveHalfTseBreak;
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT32 diff = (VICS_INT32)smallBlock._pBlock[sy][sx] - (VICS_INT32)_pBlock[y][x];
			distortion		 += (diff * diff);
			if(distortion > minEnergy)
				goto MB2D_ImproveHalfTseBreak;
		}//end for sy & sx...
	}//end else...

	MB2D_ImproveHalfTseBreak:
	return(distortion);
}//end ImproveHalfTse.

/** Search for an alignment that minimises the square error with a smaller input block.
Find the vector offset position (mvx,mvy) from (atCol,atRow) position of on overlaping 
block within the range that minimises the block energy. Return the sum of the square 
error with the input block values and the offset location (mvx,mvy).
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to match. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param range			: Search in the +/-range from (atCol,atRow).

@param mvx				: Returned x coord of offset from atCol.

@param mvy				: Returned y coord of offset from atRow.

@return 					: Sum of the min square error.
*/
VICS_INT32 MemBlock2D::FullSearch(MemBlock2D& smallBlock, 
																	VICS_INT		atCol,
																	VICS_INT		atRow,
																	VICS_INT		range,
																	VICS_PINT		mvx,
																	VICS_PINT		mvy)
{
	VICS_INT		x,y;
	VICS_INT		mx		= 0;
	VICS_INT		my		= 0;
	VICS_INT32	minE	= 0x7FFFFFFF;	// Largest positive number.

  // Full search over motion range.
  for(y = -(range); y <= range; y++)
  {
    for(x = -(range); x <= range; x++)
    {
			// Note: Must be base class call.
      VICS_INT32 energy = MemBlock2D::ImproveTse(smallBlock,atCol+x,atRow+y,minE);
      if(energy < minE)
      {
        minE	= energy;
        mx		= x;
        my		= y;
      }//end if energy...
    }//end for x...
  }//end for y...

	*mvx = mx;
	*mvy = my;
	return(minE);
}//end FullSearch.

/** Search for a half alignment that minimises the square error with a smaller input block.
Find the vector offset position (hmvx,hmvy) from (atCol,atRow) position of on overlaping 
block within half a position that minimises the block energy. Return the sum of the square 
error with the input block values and the offset location (hmvx,hmvy).
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to match. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param hmvx				: Returned half x coord of offset from atCol.

@param hmvy				: Returned half y coord of offset from atRow.

@return 					: Sum of the min square error.
*/
VICS_INT32 MemBlock2D::HalfSearch(MemBlock2D& smallBlock, 
																	VICS_INT		atCol,
																	VICS_INT		atRow,
																	VICS_PINT		hmvx,
																	VICS_PINT		hmvy)
{
	VICS_INT		z;
	VICS_INT		hmx		= 0;
	VICS_INT		hmy		= 0;
	VICS_INT32	minE	= 0x7FFFFFFF;	// Largest positive number.

  // Half location search around winning full location. 
	for(z = 0; z < MB2D_HALF_POS_ARRAY_LENGTH; z++)
	{
		// Note: Must be base class call.
    VICS_INT32 energy = MemBlock2D::ImproveHalfTse(smallBlock,atCol,atRow,HALF_POSITION[z].X,HALF_POSITION[z].Y,minE);
    if(energy < minE)
    {
      minE	= energy;
      hmx		= HALF_POSITION[z].X;
      hmy		= HALF_POSITION[z].Y;
    }//end if energy...
	}//end for z...

	*hmvx = hmx;
	*hmvy = hmy;
	return(minE);
}//end HalfSearch.

/** Combination of FullSearch() followed by HalfSearch().
Efficient combination of the two atomic methods with the zero alignment 
favoured. NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to match. The sum of atCol+1 and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param range			: Search in the +/-range from full locations (atCol,atRow).

@param mvx				: Returned 2x + half x coord of offset from atCol.

@param mvy				: Returned 2y + half y coord of offset from atRow.

@param zeroEnergy	: Returned energy at (atCol,atRow).

@return 					: Sum of the min square error.
*/
VICS_INT32 MemBlock2D::Search(MemBlock2D&	smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow,
															VICS_INT		range,
															VICS_PINT		mvx,
															VICS_PINT		mvy,
															VICS_PINT32	zeroEnergy)
{
	VICS_INT		x,y;
	VICS_INT		mx		= 0;
	VICS_INT		my		= 0;
	VICS_INT		hmx		= 0;
	VICS_INT		hmy		= 0;
	VICS_INT32  zeroE = MemBlock2D::Tse(smallBlock,atCol,atRow);
	VICS_INT32	minE	= zeroE;

  // Full search over motion range.
  for(y = -(range); y <= range; y++)
  {
    for(x = -(range); x <= range; x++)
    {
			// Note: Must be base class call.
      VICS_INT32 energy = MemBlock2D::ImproveTse(smallBlock,atCol+x,atRow+y,minE);
      if(energy < minE)
      {
        minE	= energy;
        mx		= x;
        my		= y;
      }//end if energy...
    }//end for x...
  }//end for y...

  // Half location search around winning full location. 
	for(VICS_INT z = 0; z < MB2D_HALF_POS_ARRAY_LENGTH; z++)
	{
		// Note: Must be base class call.
    VICS_INT32 energy = MemBlock2D::ImproveHalfTse(smallBlock,atCol+mx,atRow+my,HALF_POSITION[z].X,HALF_POSITION[z].Y,minE);
    if(energy < minE)
    {
      minE	= energy;
      hmx		= HALF_POSITION[z].X;
      hmy		= HALF_POSITION[z].Y;
    }//end if energy...
	}//end for z...

	*mvx = (mx << 1) + hmx;
	*mvy = (my << 1) + hmy;
	*zeroEnergy = zeroE;
	return(minE);
}//end Search.

/** Search for the best matching candidate.
Efficient combination of the two atomic methods with the smallest distortion vector in a
neighborhood from a	set of candidates is favoured. NOTE: No checking is done to ensure 
row and col overflow.

@param smallBlock	: The block to match. The sum of atCol+1 and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param range			: Search in the +/-range from full locations (atCol,atRow).

@param mvx				: Returned 2x + half x coord of offset from atCol.

@param mvy				: Returned 2y + half y coord of offset from atRow.

@param zeroEnergy	: Returned energy at (atCol,atRow).

@return 					: Sum of the min square error.
*/
VICS_INT32 MemBlock2D::CandidateSearch(MemBlock2D&	smallBlock, 
																			 VICS_INT			atCol,
																			 VICS_INT			atRow,
																			 VICS_INT			range,
																			 VICS_PINT		mvx,
																			 VICS_PINT		mvy,
																			 VICS_PINT32	zeroEnergy)
{
	VICS_INT		x,y;
	VICS_INT		mx		= 0;
	VICS_INT		my		= 0;
	VICS_INT		hmx		= 0;
	VICS_INT		hmy		= 0;

	ResetCandidates();
	VICS_INT32  zeroE = MemBlock2D::Tse(smallBlock,atCol,atRow);
	AddCandidate(0,0,zeroE);	// Force zero vector as a candidate.
	VICS_INT32	minE	= zeroE;

  // Full search over motion range.
  for(y = -(range); y <= range; y++)
  {
    for(x = -(range); x <= range; x++)
    {
			// Note: Must be base class call.
      VICS_INT32 energy = MemBlock2D::ImproveTse(smallBlock,atCol+x,atRow+y,minE);
      if(energy < minE)	// Only full search locations are compared.
      {
        minE	= energy;
				AddCandidate(x,y,energy);
      }//end if energy...
    }//end for x...
  }//end for y...

	// Select the best full location candidate.
	minE = GetBestCandidate(&mx,&my);

  // Half location search around winning full location. 
	for(VICS_INT z = 0; z < MB2D_HALF_POS_ARRAY_LENGTH; z++)
	{
		// Note: Must be base class call.
    VICS_INT32 energy = MemBlock2D::ImproveHalfTse(smallBlock,atCol+mx,atRow+my,HALF_POSITION[z].X,HALF_POSITION[z].Y,minE);
		// Energy must be smaller.
    if(energy < minE)
    {
      minE	= energy;
      hmx		= HALF_POSITION[z].X;
      hmy		= HALF_POSITION[z].Y;
    }//end if energy...
	}//end for z...

	*mvx = (mx << 1) + hmx;
	*mvy = (my << 1) + hmy;
	*zeroEnergy = zeroE;
	return(minE);
}//end CandidateSearch.

/** Search for the best matching candidate with arbitrary favoured vector (Overloaded).
Efficient combination of the two atomic methods with the smallest distortion vector from 
a	set of candidates favoured by an existing neighborhood. NOTE: No checking is done	to 
ensure row and col overflow.

@param smallBlock			: The block to match. The sum of atCol+1 and the input block
												width must be less than this width. Similarly for atRow. 

@param atCol					: Column offset to match zero column of input block.

@param atRow					: Row offset to match zero row of input block.

@param range					: Search in the +/-range from full locations (atCol,atRow).

@param mvx						: Returned 2x + half x coord of offset from atCol.

@param mvy						: Returned 2y + half y coord of offset from atRow.

@param fvx						: favoured fvx/2 + half fvx%2 coord offset from atCol.

@param fvy						: favoured fvy/2 + half fvy%2 coord offset from atRow.

@param favouredEnergy	: Returned energy at (atCol + fvx,atRow + fvy).

@return 							: Sum of the min square error.
*/
VICS_INT32 MemBlock2D::CandidateSearch(MemBlock2D&	smallBlock, 
																			 VICS_INT			atCol,
																			 VICS_INT			atRow,
																			 VICS_INT			range,
																			 VICS_PINT		mvx,
																			 VICS_PINT		mvy,
																			 VICS_INT			fvx,
																			 VICS_INT			fvy,
																			 VICS_PINT32	favouredEnergy)
{
	VICS_INT		x,y;
	VICS_INT		fx 	= fvx/2;
	VICS_INT		fy 	= fvy/2;
	VICS_INT		fhx	= fvx%2;
	VICS_INT		fhy	= fvy%2;

	ResetCandidates();
	VICS_INT32 worstE;
  VICS_INT32 favE = MemBlock2D::HalfTse(smallBlock,atCol+fx,atRow+fy,fhx,fhy,&worstE);

	// Noise margin acceptance test.
	if(worstE <= 9)
	{
		*mvx = fvx;
		*mvy = fvy;
		*favouredEnergy = favE;
		return(favE);
	}//end if worstE...

	// Force favoured full vector location as primary candidate.
	AddCandidate(fx,fy,favE);
	VICS_INT32	minE = favE;

  // Full search over motion range.
  for(y = -(range); y <= range; y++)
  {
    for(x = -(range); x <= range; x++)
    {
			// Note: Must be base class call.
      VICS_INT32 energy = MemBlock2D::ImproveTse(smallBlock,atCol+x,atRow+y,minE);
      if(energy < minE)	// Only full search locations are compared.
      {
				minE = energy;
				AddCandidate(x,y,energy);
      }//end if energy...
    }//end for x...
  }//end for y...

	// Select the best full location candidate.
	VICS_INT mx,my;
	minE = GetBestCandidate(&mx,&my);

  // Half location search around winning full location. 
	VICS_INT hmx = 0;
	VICS_INT hmy = 0;
	for(VICS_INT z = 0; z < MB2D_HALF_POS_ARRAY_LENGTH; z++)
	{
		// Note: Must be base class call.
    VICS_INT32 energy = MemBlock2D::ImproveHalfTse(smallBlock,atCol+mx,atRow+my,HALF_POSITION[z].X,HALF_POSITION[z].Y,minE);
		// Energy must be smaller.
    if(energy < minE)
    {
      minE	= energy;
      hmx		= HALF_POSITION[z].X;
      hmy		= HALF_POSITION[z].Y;
    }//end if energy...
	}//end for z...

	// Check that the best vector is at least a 5% improvement on favoured vector.
	if( ((favE - minE) * 20) < (minE + 10) )
	{
		minE = favE;
		*mvx = fvx;
		*mvy = fvy;
	}//end if favE...
	else
	{
		*mvx = (mx << 1) + hmx;
		*mvy = (my << 1) + hmy;
	}//end else...

	*favouredEnergy = favE;
	return(minE);
}//end CandidateSearch.

/** Search for the best matching vector that minimises the distortion-rate cost.
Efficient combination of the two atomic methods with the smallest distortion vector 
optimised for the lowest rate and biased towards an input vector. The cost of the
optimal vector is the rate of the difference vector from the bias point.
NOTE: No checking is done to ensure row and col overflow.

@param smallBlock	: The block to match. The sum of atCol+1 and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param range			: Search in the +/-range from full locations (atCol,atRow).

@param mvx				: Returned 2x + half x coord of offset from atCol.

@param mvy				: Returned 2y + half y coord of offset from atRow.

@param biasx			: Input bias 2x + half coord of the most likely.

@param biasy			: Input bias 2y + half coord of the most likely.

@param cost				: Cost of the selected vector.

@return 					: Squared error at the optimal vector.
*/
VICS_INT MemBlock2D::OptimalRateSearch(MemBlock2D&	smallBlock, 
																			 VICS_INT			atCol,
																			 VICS_INT			atRow,
																			 VICS_INT			range,
																			 VICS_PINT		mvx,
																			 VICS_PINT		mvy,
																			 VICS_INT			biasx,
																			 VICS_INT			biasy,
																			 VICS_PINT		cost)
{
	VICS_INT		x,y;
	VICS_INT		mx		= biasx/2;
	VICS_INT		my		= biasy/2;
	VICS_INT		hmx		= biasx%2;
	VICS_INT		hmy		= biasy%2;

	// The most likely vector is the bias vector set as (0,0) difference from (biasx,biasy).
	EncMotionVector dmv(0,0,smallBlock._width,smallBlock._height);
	VICS_INT	minSE;
	VICS_INT  minE = MemBlock2D::HalfTae(smallBlock,atCol+mx,atRow+my,hmx,hmy,&minSE);
	VICS_INT	minR = EncMotionVector::EncodeVector(dmv); // (biasx,biasy) vector.

  // Full search over motion range.
  for(y = -(range); y <= range; y++)
  {
    for(x = -(range); x <= range; x++)
    {
			// Note: Must be base class call.
			VICS_INT sqrErr;
      VICS_INT absErr = MemBlock2D::ImproveTae(smallBlock,atCol+x,atRow+y,minE,&sqrErr);
			// Only full search locations are compared.
      if(absErr <= minE)
      {
				dmv.SetMotion((x<<1)-biasx,(y<<1)-biasy);  // Full locations from bias point.
				VICS_INT dmvRate = EncMotionVector::EncodeVector(dmv);

				if( (absErr < minE) || ((absErr == minE)&&(dmvRate < minR)) )
				{
					minE			= absErr;
					minR			= dmvRate;
					minSE			= sqrErr;
					mx				= x;
					my				= y;
				}//end if absErr...
			}//end if absErr...

    }//end for x...
  }//end for y...

  // Half location search around winning full location. 
	for(VICS_INT z = 0; z < MB2D_HALF_POS_ARRAY_LENGTH; z++)
	{
		// Note: Must be base class call.
		VICS_INT sqrErr;
    VICS_INT absErr = MemBlock2D::ImproveHalfTae(smallBlock,atCol+mx,atRow+my,HALF_POSITION[z].X,HALF_POSITION[z].Y,minE,&sqrErr);
    if(absErr <= minE)
    {
			dmv.SetMotion((mx << 1) + HALF_POSITION[z].X,(my << 1) + HALF_POSITION[z].Y);
			VICS_INT dmvRate = EncMotionVector::EncodeVector(dmv);

			if( (absErr < minE) || ((absErr == minE)&&(dmvRate < minR)) )
			{
				minE 			= absErr;
				minR			= dmvRate;
				minSE			= sqrErr;
				hmx	 			= HALF_POSITION[z].X;
				hmy	 			= HALF_POSITION[z].Y;
			}//end if testE...
		}//end if absErr...
	}//end for z...

	*mvx	= (mx << 1) + hmx;
	*mvy	= (my << 1) + hmy;
	*cost = minR;
	return(minSE);
}//end OptimalRateSearch.

/*
---------------------------------------------------------------------------
	Private methods.
---------------------------------------------------------------------------
*/

void MemBlock2D::ResetCandidates(void)
{
	_candidateLength = 0;
}//end ResetCandidates.

void MemBlock2D::AddCandidate(VICS_INT x, VICS_INT y, VICS_INT32 distortion)
{
	if(_candidateLength < MB2D_CANDIDATE_LENGTH)
	{
		_winX[_candidateLength] = x;
		_winY[_candidateLength] = y;
		_winD[_candidateLength] = distortion;
		_candidateLength++;
	}//end if not full...
	else
	{
		// Replace largest distortion.
		VICS_INT32	largestD		= _winD[0];
		VICS_INT		largestPos	= 0;
		for(VICS_INT i = 1; i < MB2D_CANDIDATE_LENGTH; i++)
		{
			if(_winD[i] > largestD)
			{
				largestD		= _winD[i];
				largestPos	= i;
			}//end if _winD...
		}//end for i...

		if(distortion < largestD)
		{
			_winX[largestPos] = x;
			_winY[largestPos] = y;
			_winD[largestPos] = distortion;
		}//end if distortion...
	}//end else...

}//end AddCandidate.

VICS_INT32 MemBlock2D::GetBestCandidate(VICS_PINT x, VICS_PINT y)
{
	VICS_INT32	winD = 0;
	VICS_INT		winX = 0;
	VICS_INT		winY = 0;

	if(_candidateLength) // There must be some in the list.
	{
		// Bubble sort in ascending distortion.
		VICS_INT done = 1;
		do
		{
			done = 1;
			for(VICS_INT i = 1; i < _candidateLength; i++)
			{
				if(_winD[i] < _winD[i-1])
				{
					// Swap.
					VICS_INT32 tD = _winD[i];
					VICS_INT	 tX = _winX[i];
					VICS_INT	 tY = _winY[i];
					_winD[i]		= _winD[i-1]; 
					_winX[i]		= _winX[i-1]; 
					_winY[i]		= _winY[i-1];
					_winD[i-1]	= tD;					
					_winX[i-1]	= tX;					
					_winY[i-1]	= tY;
					done = 0;	// At least once implies incomplete.
				}//end if _winD...
			}//end for i...
		}while(!done);

		// Descend list looking for neighbors.
		VICS_INT best	= -1;
		done = 0;
		while(!done && (best < (_candidateLength - 1)))
		{
			best++;
			for(VICS_INT i = (best + 1); (i < _candidateLength)&&(!done); i++)
			{
				VICS_INT dx =	_winX[i] - _winX[best];
				VICS_INT dy =	_winY[i] - _winY[best];
				VICS_INT d2 = (dx*dx) + (dy*dy);
				if(d2 <= 13)
					done = 1;	// Neighbor found.
			}//end for i...
		}//end while !done...

		// Vectors are all over the place so choose smallest distortion.
		if(!done)
			best = 0;

		winD = _winD[best];
		winX = _winX[best];
		winY = _winY[best];

	}//end if _candidateLength...

	*x = winX;
	*y = winY;
	return(winD);
}//end GetBestCandidate.

//VICS_INT32 MemBlock2D::GetBestCandidate(VICS_PINT x, VICS_PINT y, VICS_INT bnd)
//{
//	VICS_INT32	winD = 0;
//	VICS_INT		winX = 0;
//	VICS_INT		winY = 0;
//
//	if(_candidateLength > 2)
//		_candidateLength = _candidateLength;
//
//	if(_candidateLength) // There must be some in the list.
//	{
//		// Find the smallest.
//		VICS_INT	i;
//		VICS_INT	smallestPos = 0;
//		winD = _winD[0];
//		for(i = 1; i < _candidateLength; i++)
//		{
//			if(_winD[i] < winD)
//			{
//				winD = _winD[i];
//				smallestPos = i;
//			}//end if _winD..
//		}//end for i...
//		winX = _winX[smallestPos];
//		winY = _winY[smallestPos];
//
//		// Best candidate is smallest vector distance within bnd 
//		// of smallest distortion.
//		VICS_INT best					= smallestPos;
//		VICS_INT smallestDist = (winX*winX) + (winY*winY);
//		for(i = 0; i < _candidateLength; i++)
//		{
//			if( (_winD[i] - winD) < bnd )
//			{
//				// Within the bound smallest distance is best.
//				VICS_INT dist = (_winX[i]*_winX[i]) + (_winY[i]*_winY[i]);
//				if(dist < smallestDist)
//				{
//					smallestDist = dist;
//					best = i;
//				}//end if dist...
//			}//end if _winD...
//		}//end for i...
//
//		winD = _winD[best];
//		winX = _winX[best];
//		winY = _winY[best];
//
//	}//end if _candidateLength...
//
//	*x = winX;
//	*y = winY;
//	return(winD);
//}//end GetBestCandidate.

/** Calculate the total Vq square error with a smaller input block at an offset position.
Return the sum of the vector quantiser energy distortion with the input block values 
aligned at the offset within this block specified by the input parameters. NOTE: No 
checking is done to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param rate				: Total encoded rate of the Vq vectors.

@return 					: Sum of square error.
*/
VICS_INT MemBlock2D::VqTse(MemBlock2D& smallBlock, VICS_INT atCol, VICS_INT atRow, VICS_PINT rate)
{
	VICS_INT	x,y,sx,sy,vx,vy,vpos;
	VICS_INT	distortion	= 0;
	VICS_INT	lclRate			= 0;
	VICS_INT	width				= smallBlock._width;
	VICS_INT	height			= smallBlock._height;
	VICS_INT	vector[16];

	for(sy = 0, y = atRow; sy < height; sy += 4, y += 4)
		for(sx = 0, x = atCol; sx < width; sx += 4, x += 4)
	{
		// Load the 4x4 vector for vq.
		for(vy = 0,vpos = 0; vy < 4; vy++)
			for(vx = 0; vx < 4; vx++, vpos++)
				vector[vpos] = (VICS_INT)smallBlock._pBlock[sy+vy][sx+vx] - (VICS_INT)_pBlock[y+vy][x+vx];
		// Quantise and get rate.
		VICS_INT lclD,lclC;
		lclRate			+= _vq.Encode(_vq.Quantise(vector,&lclD),&lclC);
		distortion	+= lclD;
	}//end for sy & sx...

	*rate = lclRate;
	return(distortion);
}//end VqTse.

/** Find the distortion at an offset position which improves the ditortion-rate function.
Return the sum of the vector quantiser energy distortion with the input block values 
aligned at the offset within this block specified by the input parameters. Exit early
if the input current distortion-rate is exceeded. NOTE: No checking is done to ensure 
row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param currDist		: Vq distortion to improve on.

@param currRate		: Vq rate to improve on.

@param rate				: Total encoded rate of the Vq vectors.

@return 					: Vq distortion as a square error.
*/
VICS_INT MemBlock2D::ImproveDistRate(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol, 
																		 VICS_INT			atRow, 
																		 VICS_INT			currDist, 
																		 VICS_INT			currRate, 
																		 VICS_PINT		rate)
{
	VICS_INT	x,y,sx,sy,vx,vy,vpos;
	VICS_INT	distortion	= 0;
	VICS_INT	lclRate			= 0;
	VICS_INT	width				= smallBlock._width;
	VICS_INT	height			= smallBlock._height;
	VICS_INT	vector[16];

	for(sy = 0, y = atRow; sy < height; sy += 4, y += 4)
		for(sx = 0, x = atCol; sx < width; sx += 4, x += 4)
	{
		// Load the 4x4 vector for vq.
		for(vy = 0,vpos = 0; vy < 4; vy++)
			for(vx = 0; vx < 4; vx++, vpos++)
				vector[vpos] = (VICS_INT)smallBlock._pBlock[sy+vy][sx+vx] - (VICS_INT)_pBlock[y+vy][x+vx];
		// Quantise and get rate.
		VICS_INT lclD,lclC;
		lclRate			+= _vq.Encode(_vq.Quantise(vector,&lclD),&lclC);
		distortion	+= lclD;
		if((distortion > currDist)||(lclRate > currRate))
			goto MB2D_ImproveDistRateBreak;
	}//end for sy & sx...

	MB2D_ImproveDistRateBreak:
	*rate = lclRate;
	return(distortion);
}//end ImproveDistRate.

/** Calculate the total Vq square error with a smaller input block at a half offset position.
Return the sum of the Vq square errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. 
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param halfColOff	: Further half offset from atCol.

@param halfRowOff	: Further half offset from atRow.

@param rate				: Total encoded rate of the Vq vectors.

@return 					: Sum of square error.
*/
VICS_INT MemBlock2D::VqHalfTse(MemBlock2D&	smallBlock, 
															 VICS_INT			atCol,
															 VICS_INT			atRow,
															 VICS_INT			halfColOff,
															 VICS_INT			halfRowOff,
															 VICS_PINT		rate)
{
	VICS_INT	x,y,sx,sy,vx,vy,vpos;
	VICS_INT	distortion	= 0;
	VICS_INT	lclRate			= 0;
	VICS_INT	width				= smallBlock._width;
	VICS_INT	height			= smallBlock._height;
	VICS_INT	vector[16];

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy += 4, y += 4)
			for(sx = 0, x = atCol; sx < width; sx += 4, x += 4)
		{
			// Load the 4x4 vector for vq.
			for(vy = 0,vpos = 0; vy < 4; vy++)
				for(vx = 0; vx < 4; vx++, vpos++)
				{
					VICS_INT z = ((VICS_INT)_pBlock[y+vy           ][x+vx] +	
									 		  (VICS_INT)_pBlock[y+vy           ][x+vx+halfColOff] +
									 		  (VICS_INT)_pBlock[y+vy+halfRowOff][x+vx] + 
									 		  (VICS_INT)_pBlock[y+vy+halfRowOff][x+vx+halfColOff] + 2) >> 2;

					vector[vpos] = (VICS_INT)smallBlock._pBlock[sy+vy][sx+vx] - z;
				}//end for vy...
			// Quantise and get rate.
			VICS_INT lclD,lclC;
			lclRate			+= _vq.Encode(_vq.Quantise(vector,&lclD),&lclC);
			distortion	+= lclD;
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy += 4, y += 4)
			for(sx = 0, x = atCol; sx < width; sx += 4, x += 4)
		{
			// Load the 4x4 vector for vq.
			for(vy = 0,vpos = 0; vy < 4; vy++)
				for(vx = 0; vx < 4; vx++, vpos++)
				{
					VICS_INT z = ((VICS_INT)_pBlock[y+vy           ][x+vx] +	
									 		  (VICS_INT)_pBlock[y+vy+halfRowOff][x+vx+halfColOff] + 1) >> 1;

					vector[vpos] = (VICS_INT)smallBlock._pBlock[sy+vy][sx+vx] - z;
				}//end for vy...
			// Quantise and get rate.
			VICS_INT lclD,lclC;
			lclRate			+= _vq.Encode(_vq.Quantise(vector,&lclD),&lclC);
			distortion	+= lclD;
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy += 4, y += 4)
			for(sx = 0, x = atCol; sx < width; sx += 4, x += 4)
		{
			// Load the 4x4 vector for vq.
			for(vy = 0,vpos = 0; vy < 4; vy++)
				for(vx = 0; vx < 4; vx++, vpos++)
					vector[vpos] = (VICS_INT)smallBlock._pBlock[sy+vy][sx+vx] - (VICS_INT)_pBlock[y+vy][x+vx];
			// Quantise and get rate.
			VICS_INT lclD,lclC;
			lclRate			+= _vq.Encode(_vq.Quantise(vector,&lclD),&lclC);
			distortion	+= lclD;
		}//end for sy & sx...
	}//end else...

	*rate = lclRate;
	return(distortion);
}//end VqHalfTse.

/** Calculate the total absolute error with a smaller input block at an offset position.
Return the sum of the absolute errors with the input block values aligned at the
offset within this block specified by the input parameters. NOTE: No checking is done
to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param distortion	: Sum of squared error.

@return 					: Sum of absolute error.
*/
VICS_INT MemBlock2D::Tae(MemBlock2D& smallBlock,VICS_INT atCol,VICS_INT atRow,VICS_PINT distortion)
{
	VICS_INT	x,y,sx,sy;
	VICS_INT	sqrErr	= 0;
	VICS_INT	absErr	= 0;
	VICS_INT	width		= smallBlock._width;
	VICS_INT	height	= smallBlock._height;

	for(sy = 0, y = atRow; sy < height; sy++, y++)
		for(sx = 0, x = atCol; sx < width; sx++, x++)
	{
		VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - (VICS_INT)_pBlock[y][x];
		sqrErr += (diff * diff);
		if(diff < 0)	absErr -= diff;
		else					absErr += diff;
	}//end for sy & sx...
	*distortion = sqrErr;
	return(absErr);
}//end Tae.

/** Improve on the total absolute error with a smaller input block at an offset position.
Return the improved sum of the absolute errors with the input block values aligned at the
offset within this block specified by the input parameters. Jump out if the absolute
error rises above the input min error. NOTE: No checking is done
to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param minAbsErr	: Absolute error to improve on.

@param minSqrErr	: Sum of squared error of improvement.

@return 					: Sum of absolute error of improvement.
*/
VICS_INT MemBlock2D::ImproveTae(MemBlock2D&	smallBlock, 
																VICS_INT		atCol, 
																VICS_INT		atRow,
																VICS_INT		minAbsErr,
																VICS_PINT		minSqrErr)
{
	VICS_INT	x,y,sx,sy;
	VICS_INT	sqrErr	= 0;
	VICS_INT	absErr	= 0;
	VICS_INT	width		= smallBlock._width;
	VICS_INT	height	= smallBlock._height;

	for(sy = 0, y = atRow; sy < height; sy++, y++)
		for(sx = 0, x = atCol; sx < width; sx++, x++)
	{
		VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - (VICS_INT)_pBlock[y][x];
		sqrErr += (diff * diff);
		if(diff < 0)	absErr -= diff;
		else					absErr += diff;

		if(absErr > minAbsErr)
			goto MB2D_ImproveTaeBreak;
	}//end for sy & sx...

	MB2D_ImproveTaeBreak:

	*minSqrErr = sqrErr;
	return(absErr);
}//end ImproveTae.

/** Calculate the total absolute error with a smaller input block at a half offset position.
Return the sum of the absolute errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. 
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param halfColOff	: Further half offset from atCol.

@param halfRowOff	: Further half offset from atRow.

@param distortion	: Sum of squared error.

@return 					: Sum of absolute error.
*/
VICS_INT32 MemBlock2D::HalfTae(MemBlock2D&	smallBlock, 
															 VICS_INT			atCol,
															 VICS_INT			atRow,
															 VICS_INT			halfColOff,
															 VICS_INT			halfRowOff,
															 VICS_PINT		distortion)
{
	VICS_INT	x,y,sx,sy;
	VICS_INT	sqrErr	= 0;
	VICS_INT	absErr	= 0;
	VICS_INT	width		= smallBlock._width;
	VICS_INT	height	= smallBlock._height;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT z		=	((VICS_INT)_pBlock[y           ][x] +	
									 		 (VICS_INT)_pBlock[y           ][x+halfColOff] +
									 		 (VICS_INT)_pBlock[y+halfRowOff][x] + 
									 		 (VICS_INT)_pBlock[y+halfRowOff][x+halfColOff] + 2) >> 2;
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - z;
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT z		=	((VICS_INT)_pBlock[y           ][x] +	
									 		 (VICS_INT)_pBlock[y+halfRowOff][x+halfColOff] + 1) >> 1;
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - z;
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - (VICS_INT)_pBlock[y][x];
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;
		}//end for sy & sx...
	}//end else...

	*distortion = sqrErr;
	return(absErr);
}//end HalfTae.

/** Improve the total absolute error with a smaller input block at a half offset position.
Return the sum of the absolute errors with the input block values aligned at the
offset plus half offset within this block specified by the input parameters. 
NOTE: No checking is done	to ensure row and col overflow.

@param smallBlock	: The block to test with. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param atCol			: Column offset to match zero column of input block.

@param atRow			: Row offset to match zero row of input block.

@param halfColOff	: Further half offset from atCol.

@param halfRowOff	: Further half offset from atRow.

@param minAbsErr	: Absolute error to improve on.

@param minSqrErr	: Absolute error to improve on.

@return 					: Sum of absolute error.
*/
VICS_INT32 MemBlock2D::ImproveHalfTae(MemBlock2D&	smallBlock, 
																			VICS_INT		atCol,
																			VICS_INT		atRow,
																			VICS_INT		halfColOff,
																			VICS_INT		halfRowOff,
																			VICS_INT		minAbsErr,
																			VICS_PINT		minSqrErr)
{
	VICS_INT	x,y,sx,sy;
	VICS_INT	sqrErr	= 0;
	VICS_INT	absErr	= 0;
	VICS_INT	width		= smallBlock._width;
	VICS_INT	height	= smallBlock._height;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT z		=	((VICS_INT)_pBlock[y           ][x] +	
									 		 (VICS_INT)_pBlock[y           ][x+halfColOff] +
									 		 (VICS_INT)_pBlock[y+halfRowOff][x] + 
									 		 (VICS_INT)_pBlock[y+halfRowOff][x+halfColOff] + 2) >> 2;
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - z;
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;

			if(absErr > minAbsErr)
				goto MB2D_ImproveHalfTaeBreak;
		}//end for sy & sx...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT z		=	((VICS_INT)_pBlock[y           ][x] +	
									 		 (VICS_INT)_pBlock[y+halfRowOff][x+halfColOff] + 1) >> 1;
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - z;
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;

			if(absErr > minAbsErr)
				goto MB2D_ImproveHalfTaeBreak;
		}//end for sy & sx...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(sy = 0, y = atRow; sy < height; sy++, y++)
			for(sx = 0, x = atCol; sx < width; sx++, x++)
		{
			VICS_INT diff = (VICS_INT)smallBlock._pBlock[sy][sx] - (VICS_INT)_pBlock[y][x];
			sqrErr += (diff * diff);
			if(diff < 0)	absErr -= diff;
			else					absErr += diff;

			if(absErr > minAbsErr)
				goto MB2D_ImproveHalfTaeBreak;
		}//end for sy & sx...
	}//end else...

	MB2D_ImproveHalfTaeBreak:

	*minSqrErr = sqrErr;
	return(absErr);
}//end ImproveHalfTae.





