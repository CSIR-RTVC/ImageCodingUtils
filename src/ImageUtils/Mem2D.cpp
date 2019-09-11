/** @file

MODULE						: Mem2D

TAG								: M2D

FILE NAME					: Mem2D.cpp

DESCRIPTION				: A class to describe and operate on a two-dimensional
										block of memory of primitive types.

REVISION HISTORY	:
									: 

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


#include	"Mem2D.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const M2D_COORD Mem2D::HALF_POSITION[M2D_HALF_POS_ARRAY_LENGTH] =
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

void Mem2D::ResetMembers(void)
{
	_width						= 0;
	_height						= 0;
	_pMem							= NULL;
	_pBlock						= NULL;
}//end ResetMembers.

Mem2D::Mem2D()
{
	ResetMembers();
}//end constructor.

Mem2D::Mem2D(int width, int height)
{
	ResetMembers();

	// Potentially dangerous to alloc mem in a constructor as there is no way 
	// of determining failure.
	Create(width,height);
	
}//end alt constructor.

Mem2D::~Mem2D()
{
	// Ensure no mem is left alloc.
	Destroy();
}//end destructor.

void Mem2D::Destroy(void)
{
	if(_pBlock != NULL)
		delete[] _pBlock;
	_pBlock = NULL;

	if(_pMem != NULL)
		delete[] _pMem;
	_pMem = NULL;

}//end Destroy.

int Mem2D::Create(int width, int height)
{
	// Clear first.
	if(_pMem != NULL)
		Destroy();
	_width  = 0;
	_height = 0;

	// Alloc the mem block.
	_pMem = new M2D_TYPE[width * height];
	if(_pMem == NULL)
		return(0);

	// Alloc the mem block row addresses.
	_pBlock = new M2D_PTYPE[height];
	if(_pBlock == NULL)
	{
		Destroy();
		return(0);
	}//end if _pBlock...

	// Successful alloc.
	_width	= width;
	_height	= height;

	// Fill the row addresses.
	for(int row = 0; row < height; row++)
		_pBlock[row] = &(_pMem[width * row]);

	return(1);
}//end Create.

/*
---------------------------------------------------------------------------
	Public input/output interface.
---------------------------------------------------------------------------
*/

/** Set this block by filling it from the input source block.
Copy all values from the top left block of the source into this block. Resize
this block if it will not fit and create this block if it does not exist. The
result leaves this block completely filled.

@param srcBlock		: Top left source data pointer. 

@return 					: 0 = Mem alloc failure, 1 = success.
*/
int Mem2D::Set(Mem2D& srcBlock)
{
	return( Mem2D::Set(srcBlock._pMem, srcBlock._width, srcBlock._height) );
}//end Set.

/** Set this block by filling it from the larger input source.
Copy all values from the top left block of the source into this block. Resize
this block if it will not fit and create this block if it does not exist. The
result leaves this block completely filled.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Source block width.

@param srcHeight	: Source block height.

@return 					: 0 = Mem alloc failure, 1 = success.
*/
int Mem2D::Set(void* srcPtr, int width, int height)
{
	// Create the mem if it doesn't exist already.
	if( (_width != width) || (_height != height) )
		Destroy();
	if(_pMem == NULL)
	{
		if(!Create(width,height))
			return(0);
	}//end if _pMem...

	// This object is filled with a _width X _height 2-D block of values from 
	// the top left corner of the source.
	M2D_PTYPE	pLcl = (M2D_PTYPE)srcPtr;

	// Slow.
	//int				col,row;
	//for(row = 0; row < _height; row++)
	//	for(col = 0; col < _width; col++)
	//		_pBlock[row][col] = pLcl[(row * width) + col];

	// Fast.
	int	pos = 0;
	for(int row = 0; row < _height; row++, pos += width)
		memcpy((void *)(_pBlock[row]), (const void *)(&(pLcl[pos])), _width * sizeof(M2D_TYPE));

	return(1);
}//end Set.

/** Write the from the input source into this block.
Copy all values from the top left of the source into this block starting
at the input specified offset. This block is filled and no bounds checking
is performed. Return if this block does not exist.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Total source width.

@param srcHeight	: Total source height.

@return 					: 0 = failure, 1 = success.
*/
int Mem2D::Write(void* srcPtr,	int	srcWidth,	int	srcHeight)
{
	// Check before copying.
	if(_pMem == NULL)
		return(0);

	// This object is filled from the top left corner of the source.
	M2D_PTYPE	pLcl = (M2D_PTYPE)srcPtr;

	// Slow.
	//int				col,row;
	//for(row = 0; row < _height; row++)
	//	for(col = 0; col < _width; col++)
	//		_pBlock[row][col] = pLcl[(row * srcWidth) + col];

	// Fast.
	int	pos = 0;
	for(int row = 0; row < _height; row++, pos += srcWidth)
		memcpy((void *)(_pBlock[row]), (const void *)(&(pLcl[pos])), _width * sizeof(M2D_TYPE));

	return(1);
}//end Write.

/** Copy from an offset in the input source into an offset within this block.
Copy rows x cols from the top left of the source into this block starting
at the input specified offset. No boundary checking is done. Return if it 
does not exist.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Total source width.

@param srcHeight	: Total source height.

@param toCol			: Width offset.

@param toRow			: Height offset.

@param cols				: Columns to transfer.

@param rows				: Rows to transfer.

@return 					: 0 = failure, 1 = success.
*/
int Mem2D::Write(void* srcPtr, int srcWidth, int srcHeight, 
															 int toCol,		 int toRow,
															 int cols,		 int rows)
{
	// Check before copying.
	if(_pMem == NULL)
		return(0);

	// This object is patially filled from a srcWidth X srcHeight 2-D source 
	// with cols x rows of values into the block offset.
	M2D_PTYPE	pLcl = (M2D_PTYPE)srcPtr;

	// Slow.
	//int				col,row;
	//for(row = 0; row < rows; row++)
	//	for(col = 0; col < cols; col++)
	//		_pBlock[toRow + row][toCol + col] = pLcl[(row * srcWidth) + col];

	// Fast.
	int	pos = 0;
	for(int row = 0; row < rows; row++, pos += srcWidth)
		memcpy((void *)(&(_pBlock[toRow + row][toCol])), (const void *)(&(pLcl[pos])), cols * sizeof(M2D_TYPE));

	return(1);
}//end Write.

/** Copy the entire smaller source block into an offset within this block.
Copy all values from the top left of the source into this block starting
at the input specified offset. Return if it will not fit or if this block
does not exist.

@param srcBlock		: The block to copy. The sum of atCol and the input block
										width must be less than this width. Similarly for atRow. 

@param toCol			: Column offset to match zero column of input block.

@param toRow			: Row offset to match zero row of input block.

@return 					: 0 = failure, 1 = success.
*/
int Mem2D::Write(Mem2D& srcBlock, int toCol, int toRow)
{
	int srcWidth	= srcBlock._width;
	int srcHeight = srcBlock._height;

	// Check before copying.
	if( (_pMem == NULL) || (srcWidth > (_width - toCol)) || (srcHeight > (_height - toRow)) )
		return(0);

	// This object is filled from the start offset with a srcWidth X srcHeight 
	// 2-D block of values from the top left corner of the source.

	// Slow.
	//int col,row;
	//for(row = 0; row < srcHeight; row++)
	//	for(col = 0; col < srcWidth; col++)
	//		_pBlock[toRow + row][toCol + col] = srcBlock._pBlock[row][col];

	// Fast.
	for(int row = 0; row < srcHeight; row++)
		memcpy((void *)(&(_pBlock[toRow + row][toCol])), 
					 (const void *)(srcBlock._pBlock[row]), 
					 srcWidth * sizeof(M2D_TYPE));

	return(1);
}//end Write.

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
int Mem2D::Write(Mem2D& srcBlock, int fromCol, int fromRow, 
																	int toCol,	 int toRow, 
																	int cols,		 int rows)
{
	// Check before copying.
	if(_pMem == NULL)
		return(0);

	int colLimit = cols;
	int rowLimit = rows;
	if((fromCol + cols) > srcBlock._width) colLimit = srcBlock._width - fromCol;
	if((fromRow + rows) > srcBlock._height) rowLimit = srcBlock._height - fromRow;
	if((toCol + colLimit) > _width) colLimit = _width - toCol;
	if((toRow + rowLimit) > _height) rowLimit = _height - toRow;

	// Slow.
	//int col,row;
	//for(row = 0; row < rowLimit; row++)
	//	for(col = 0; col < colLimit; col++)
	//		_pBlock[toRow + row][toCol + col] = srcBlock._pBlock[fromRow + row][fromCol + col];

	// Fast.
	for(int row = 0; row < rowLimit; row++)
		memcpy((void *)(&(_pBlock[toRow + row][toCol])), 
					 (const void *)(&(srcBlock._pBlock[fromRow + row][fromCol])), 
					 colLimit * sizeof(M2D_TYPE));

	return(1);
}//end Write.

/** Write the entire source block into this block.
Copy all values from the top left of the source into this block. Return if 
the source does not have the same dimensions as this block.
does not exist.

@param srcBlock		: The block to copy. 

@return 					: 0 = failure, 1 = success.
*/
int Mem2D::Write(Mem2D& srcBlock)
{
	if( (_width != srcBlock._width)||(_height != srcBlock._height) )
		return(0);

	memcpy( (void *)_pMem, 
					(const void *)srcBlock._pMem, 
					_width * _height * sizeof(M2D_TYPE) );

	return(1);
}//end Write.

/** Read this block into a 2D destination.
No mem column or row overflow checking is done during the read process. Use
this method with caution.

@param dstPtr			: Destination to read to.

@param dstWidth		: Total destination width.

@param dstHeight	: Total destination height.

@return 					: None.
*/
void Mem2D::Read(void* dstPtr, int dstWidth, int dstHeight)
{
	M2D_PTYPE	pLcl = (M2D_PTYPE)dstPtr;

	// Slow.
	//int	col,row,dstRow;
	//for(row = 0, dstRow = 0; row < _height; row++, dstRow += dstWidth)
	//{
	//	int dstPos;
	//	for(col = 0, dstPos = dstRow; col < _width; col++,  dstPos++)
	//		pLcl[dstPos] = _pBlock[row][col];
	//}//end for row...

	// Fast.
	int	dstPos = 0;
	for(int row = 0; row < _height; row++, dstPos += dstWidth)
	{
		memcpy((void *)(&(pLcl[dstPos])), (const void *)(_pBlock[row]),	_width * sizeof(M2D_TYPE));
	}//end for row...

}//end Read.

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
void Mem2D::Read(void* dstPtr, int dstWidth,	int dstHeight,
															 int fromCol,		int fromRow,
															 int cols,			int rows)
{
	M2D_PTYPE	pLcl = (M2D_PTYPE)dstPtr;

	// Slow.
	//int				col,row,srcRow,dstRow;
	//for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
	//{
	//	int srcCol,dstPos;
	//	for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
	//		pLcl[dstPos] = _pBlock[srcRow][srcCol];
	//}//end for row...

	// Fast.
	int	dstPos = 0;
	for(int row = 0; row < rows; row++, dstPos += dstWidth)
	{
		memcpy((void *)(&(pLcl[dstPos])), 
					 (const void *)(&(_pBlock[fromRow + row][fromCol])),	
					 cols * sizeof(M2D_TYPE));
	}//end for row...

}//end Read.

/** Read from an offset within this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution.

@param dstBlock		: Destination to read to.

@param toCol			: Destination col.

@param toRow			: Destination row.

@param fromCol		: Block source column offset.

@param fromRow		: Block source row offset.

@param cols				: Columns to read.

@param rows				: Rows to read.

@return 					: None.
*/
void Mem2D::Read(Mem2D& dstBlock, int toCol,		int toRow,
																	int fromCol,	int fromRow,
																	int cols,			int rows)
{
	// Fast.
	M2D_TYPE**	dstPtr = dstBlock.Get2DPtr();
	for(int row = 0; row < rows; row++)
	{
		memcpy((void *)(&(dstPtr[toRow + row][toCol])), 
					 (const void *)(&(_pBlock[fromRow + row][fromCol])),	
					 cols * sizeof(M2D_TYPE));
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
void Mem2D::HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
																	 int fromCol,			int fromRow,
																	 int halfColOff,	int halfRowOff,
																	 int cols,				int rows)
{
	M2D_PTYPE	pLcl = (M2D_PTYPE)dstPtr;
	int				col,row,srcRow,dstRow;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			int srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
			{
				int z =	(int)_pBlock[srcRow           ][srcCol] +	
								(int)_pBlock[srcRow           ][srcCol+halfColOff] +
								(int)_pBlock[srcRow+halfRowOff][srcCol] + 
								(int)_pBlock[srcRow+halfRowOff][srcCol+halfColOff];
				if(z >= 0)
					z = (z + 2) >> 2;
				else
					z = (z - 2)/4;
				pLcl[dstPos] = (M2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			int srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
			{
				int z =	(int)_pBlock[srcRow           ][srcCol] +	
								(int)_pBlock[srcRow+halfRowOff][srcCol+halfColOff];
				if(z >= 0)
					z = (z + 1) >> 1;
				else
					z = (z - 1)/2;
				pLcl[dstPos] = (M2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
		{
			int srcCol,dstPos;
			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
				pLcl[dstPos] = _pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
}//end HalfRead.

/** Read from this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Return if the dimensions are not equal.

@param dstBlock		: Destination to read to.

@return 					: 0 = failure, 1 = success.
*/
int Mem2D::Read(Mem2D& dstBlock)
{
	if( (_width != dstBlock._width)||(_height != dstBlock._height) )
		return(0);

	memcpy( (void *)dstBlock._pMem,
					(const void *)_pMem,
					_width * _height * sizeof(M2D_TYPE));

	return(1);
}//end Read.

/*
---------------------------------------------------------------------------
	Public block operations interface.
---------------------------------------------------------------------------
*/

/** Clear the block values to zero.
Fast value zero-ing function.

@return:	none
*/
void Mem2D::Clear(void)
{
	memset((void *)_pMem, 0, _width * _height * sizeof(M2D_TYPE));
}//end Clear.

/** Subtract the input block from this.
The block dimensions must match else return 0.

@param b	: Input block to subtract from.
@return		: Success = 1, fail = 0;	
*/
int Mem2D::Sub(Mem2D& b)
{
	if( (_width != b._width)||(_height != b._height) )
		return(0);
	// Subtraction.
	int len = _width * _height;
	for(int i = 0; i < len; i++)
		_pMem[i] -= b._pMem[i];

	return(1);
}//end Sub.

/** Add the input block to this.
The block dimensions must match else return 0.

@param b	: Input block to add to.
@return		: Success = 1, fail = 0;	
*/
int Mem2D::Add(Mem2D& b)
{
	if( (_width != b._width)||(_height != b._height) )
		return(0);
	// Addition.
	int len = _width * _height;
	for(int i = 0; i < len; i++)
		_pMem[i] += b._pMem[i];

	return(1);
}//end Add.

/** Calc the total square error with the input block.
The block dimensions must match else return INF.

@param b	: Input block.
@return		: Total square error.	
*/
int Mem2D::Tse(Mem2D& b)
{
	if( (_width != b._width)||(_height != b._height) )
		return(10000000);

	int len = _width * _height;
	int acc = 0;
	for(int i = 0; i < len; i++)
	{
		int diff = (_pMem[i] - b._pMem[i]);
		acc += (diff * diff);
	}//end for i...

	return(acc);
}//end Tse.

/** The total square error with the input to improve on an input value.
Exit early if the accumulated square error becomes larger
than the specified input value. The block dimensions must 
match else return INF.

@param b		: Input block.
@param min	:	The min value to improve on.
@return			: Total square error to the point of early exit.	
*/
int Mem2D::TseLessThan(Mem2D& b, int min)
{
	if( (_width != b._width)||(_height != b._height) )
		return(min + 10000000);

	int len = _width * _height;
	int acc = 0;
	for(int i = 0; i < len; i++)
	{
		int diff = (_pMem[i] - b._pMem[i]);
		acc += (diff * diff);
		if(acc > min)
			return(acc);
	}//end for i...

	return(acc);
}//end TseLessThan.

/** Calc the total absolute error with the input block.
The block dimensions must match else return INF.

@param b	: Input block.
@return		: Total absolute error.	
*/
int Mem2D::Tae(Mem2D& b)
{
	if( (_width != b._width)||(_height != b._height) )
		return(10000000);

	int len = _width * _height;
	int acc = 0;
	for(int i = 0; i < len; i++)
	{
		int diff = (_pMem[i] - b._pMem[i]);
		if(diff >= 0)
			acc += diff;
		else
			acc -= diff;
	}//end for i...

	return(acc);
}//end Tae.

/** The total absolute error with the input to improve on an input value.
Exit early if the accumulated absolute error becomes larger
than the specified input value. The block dimensions must 
match else return INF.

@param b		: Input block.
@param min	:	The min value to improve on.
@return			: Total absolute error to the point of early exit.	
*/
int Mem2D::TaeLessThan(Mem2D& b, int min)
{
	if( (_width != b._width)||(_height != b._height) )
		return(min + 10000000);

	int len = _width * _height;
	int acc = 0;
	for(int i = 0; i < len; i++)
	{
		int diff = (_pMem[i] - b._pMem[i]);
		if(diff >= 0)
			acc += diff;
		else
			acc -= diff;
		if(acc > min)
			return(acc);
	}//end for i...

	return(acc);
}//end TaeLessThan.

/*
---------------------------------------------------------------------------
	Private methods.
---------------------------------------------------------------------------
*/






