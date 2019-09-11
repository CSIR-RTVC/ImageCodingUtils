/** @file

MODULE						: OverlayMem2D

TAG								: OM2D

FILE NAME					: OverlayMem2D.cpp

DESCRIPTION				: A class to overlay a two-dimensional mem structure onto
										a contiguous block (usually larger) of memory of primitive 
										types.

REVISION HISTORY	:	12/01/2006 by Keith Ferguson: 
											The direct mem ptr read and write members are
											depricated for the preferred aproach of moving the origin
											of the block on top of the overlayed mem and then applying
											object transfers.
									: 19/07/2006 by KF:
											Significant methods changed to static with thier proxies
											as overloads. 

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


#include <ImageUtils/OverlayMem2D.h>

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define OM2D_UNROLL_INNER_LOOP
/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

void OverlayMem2D::ResetMembers(void)
{
	_width						= 0;
	_height						= 0;
	_srcWidth					= 0;
	_srcHeight				= 0;
	_pMem							= NULL;
	_xPos							= 0;
	_yPos							= 0;
	_pBlock						= NULL;
}//end ResetMembers.

/** Alternate constructor.
Set the overlay on top of a contiguous block of mem.
@param srcPtr		: Top left pointer of the source mem.
@param srcWidth	: Width of source as a stride.
@param srcHeight: Height of source.
@param width		: Width of the block to work with.
@param height		: Height of block.
@return					: none.
*/
OverlayMem2D::OverlayMem2D(void* srcPtr, int srcWidth, int srcHeight, int width, int height)
{
	ResetMembers();

	if(srcPtr == NULL)
		return;

	_width			= width;
	_height			= height;
	_srcWidth		= srcWidth;
	_srcHeight	= srcHeight;
	_pMem				= (OM2D_PTYPE)srcPtr;

	// Potentially dangerous to alloc mem in a constructor as there is no way 
	// of determining failure.

	// Alloc the mem block row addresses.
	_pBlock = new OM2D_PTYPE[srcHeight];
	if(_pBlock == NULL)
		return;

	// Fill the row addresses.
	for(int row = 0; row < srcHeight; row++)
		_pBlock[row] = &(_pMem[srcWidth * row]);

}//end alt constructor.

OverlayMem2D::~OverlayMem2D()
{
	// Ensure no mem is left alloc.
	Destroy();
}//end destructor.

void OverlayMem2D::Destroy(void)
{
	if(_pBlock != NULL)
		delete[] _pBlock;
	_pBlock = NULL;

}//end Destroy.

/** Reset the overlayed mem to a different source.
This object does not own the mem but does own the 2-dim ptr that
must be recreated. The overlay block parameters are unchanged.
@param srcPtr			: New source pointer. 
@param srcWidth		: Source width.
@param srcHeight	: Source height.
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2D::SetMem(void* srcPtr, int srcWidth, int srcHeight)
{
	if(srcPtr == NULL)
		return(0);

	_srcWidth		= srcWidth;
	_srcHeight	= srcHeight;
	_pMem				= (OM2D_PTYPE)srcPtr;

	// 2-Dim ptr must be recreated.
	if(_pBlock != NULL)
		delete[] _pBlock;
	_pBlock = NULL;

	// Alloc the mem block row addresses.
	_pBlock = new OM2D_PTYPE[srcHeight];
	if(_pBlock == NULL)
		return(0);

	// Fill the row addresses.
	for(int row = 0; row < srcHeight; row++)
		_pBlock[row] = &(_pMem[srcWidth * row]);

	return(1);
}//end SetMem.

void OverlayMem2D::SetOrigin(int x, int y)
{
	if(x < 0)
		x = 0;
	else if(x > (_srcWidth - _width))
		x = _srcWidth - _width;
	_xPos = x;

	if(y < 0)
		y = 0;
	else if(y > (_srcHeight - _height))
		y = _srcHeight - _height;
	_yPos = y;

}//end SetOrigin.

/*
---------------------------------------------------------------------------
	Public input/output interface.
---------------------------------------------------------------------------
*/

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
int OverlayMem2D::Write(OverlayMem2D& me, OverlayMem2D& srcBlock, int toCol, int toRow)
{
	int width		= srcBlock._width;
	int height	= srcBlock._height;

	// Check before copying.
	if( (me._pMem == NULL) || (width > (me._width - toCol)) || (height > (me._height - toRow)) )
		return(0);

	// This object is filled from the start offset with a width X height 
	// 2-D block of values from the top left corner of the source.

	// Fast.
	toRow	+= me._yPos;
	toCol	+= me._xPos;
	for(int row = 0; row < height; row++)
		memcpy((void *)(&(me._pBlock[toRow + row][toCol])), 
					 (const void *)(&(srcBlock._pBlock[srcBlock._yPos + row][srcBlock._xPos])), 
					 width * sizeof(OM2D_TYPE));

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
int OverlayMem2D::Write(OverlayMem2D& me, OverlayMem2D& srcBlock, 
												int fromCol,			int fromRow, 
												int toCol,				int toRow, 
												int cols,					int rows)
{
	// Check before copying.
	if(me._pMem == NULL)
		return(0);

	int colLimit = cols;
	int rowLimit = rows;
	if((fromCol + cols) > srcBlock._width) colLimit = srcBlock._width - fromCol;
	if((fromRow + rows) > srcBlock._height) rowLimit = srcBlock._height - fromRow;
	if((toCol + colLimit) > me._width) colLimit = me._width - toCol;
	if((toRow + rowLimit) > me._height) rowLimit = me._height - toRow;

	// Adjust the offsets.
	fromRow += srcBlock._yPos;
	fromCol += srcBlock._xPos;
	toRow		+= me._yPos;
	toCol		+= me._xPos;
	for(int row = 0; row < rowLimit; row++)
		memcpy((void *)(&(me._pBlock[toRow + row][toCol])), 
					 (const void *)(&(srcBlock._pBlock[fromRow + row][fromCol])), 
					 colLimit * sizeof(OM2D_TYPE));

	return(1);
}//end Write.

/** Write the entire source block into this block.
Copy all values from the top left of the source into this block. Return if 
the source does not have the same dimensions as this block.
does not exist.

@param srcBlock		: The block to copy. 

@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2D::Write(OverlayMem2D& me, OverlayMem2D& srcBlock)
{
	if( (me._width != srcBlock._width)||(me._height != srcBlock._height) )
		return(0);

	for(int row = 0; row < me._height; row++)
		memcpy((void *)(&(me._pBlock[me._yPos + row][me._xPos])), 
					 (const void *)(&(srcBlock._pBlock[srcBlock._yPos + row][srcBlock._xPos])), 
					 me._width * sizeof(OM2D_TYPE));

	return(1);
}//end Write.

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
void OverlayMem2D::Read(OverlayMem2D& me, OverlayMem2D& dstBlock, 
												int toCol,				int toRow,
												int fromCol,			int fromRow,
												int cols,					int rows)
{
	OM2D_TYPE**	dstPtr = dstBlock.Get2DSrcPtr();
	toCol		+= dstBlock._xPos;
	toRow		+= dstBlock._yPos;
	fromCol += me._xPos;
	fromRow += me._yPos;
	for(int row = 0; row < rows; row++)
	{
		memcpy((void *)(&(dstPtr[toRow + row][toCol])), 
					 (const void *)(&(me._pBlock[fromRow + row][fromCol])),	
					 cols * sizeof(OM2D_TYPE));
	}//end for row...

}//end Read.

/** Read from this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Return if the dimensions are not equal.

@param dstBlock		: Destination to read to.

@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2D::Read(OverlayMem2D& me, OverlayMem2D& dstBlock)
{
	if( (me._width != dstBlock._width)||(me._height != dstBlock._height) )
		return(0);

	OM2D_TYPE**	dstPtr = dstBlock.Get2DSrcPtr();
	int toCol	= dstBlock._xPos;
	int toRow	= dstBlock._yPos;

	for(int row = 0; row < me._height; row++)
	{
		memcpy((void *)(&(dstPtr[toRow + row][toCol])), 
					 (const void *)(&(me._pBlock[me._yPos + row][me._xPos])),	
					 me._width * sizeof(OM2D_TYPE));
	}//end for row...

	return(1);
}//end Read.

/** Read from a half location offset around this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Note that values outside of the _width and _height
need to be valid for the half calculation.
@param dstBlock		: Destination to read to.
@param halfColOff	: Half location offset from fromCol.
@param halfRowOff	: Half location offset from fromRow.
@return 					: None.
*/
void OverlayMem2D::HalfRead(OverlayMem2D& me, OverlayMem2D& dstBlock, int halfColOff,	int halfRowOff)
{
	OM2D_TYPE**	pLcl = dstBlock.Get2DSrcPtr();
	int	row,col,srcRow,dstRow;

	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

	if(halfColOff && halfRowOff)			// Diagonal case.
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =	(int)me._pBlock[srcRow           ][srcCol] +	
								(int)me._pBlock[srcRow           ][srcCol+halfColOff] +
								(int)me._pBlock[srcRow+halfRowOff][srcCol] + 
								(int)me._pBlock[srcRow+halfRowOff][srcCol+halfColOff];
				if(z >= 0)
					z = (z + 2) >> 2;
				else
					z = (z - 2)/4;
				pLcl[dstRow][dstCol] = (OM2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	// Linear case.
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =	(int)me._pBlock[srcRow           ][srcCol] +	
								(int)me._pBlock[srcRow+halfRowOff][srcCol+halfColOff];
				if(z >= 0)
					z = (z + 1) >> 1;
				else
					z = (z - 1)/2;
				pLcl[dstRow][dstCol] = (OM2D_TYPE)z;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else															// Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
				pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
}//end HalfRead.

/*
---------------------------------------------------------------------------
	Public block operations interface.
---------------------------------------------------------------------------
*/

/** Clear the block values to zero.
Fast value zero-ing function.

@return:	none
*/
void OverlayMem2D::Clear(OverlayMem2D& me)
{
	for(int row = 0; row < me._height; row++)
	{
		memset((void *)(&(me._pBlock[me._yPos + row][me._xPos])), 0, me._width * sizeof(OM2D_TYPE));
	}//end for row...

}//end Clear.

/** Fill the block values.
Fast value set function.

@return:	none
*/
void OverlayMem2D::Fill(OverlayMem2D& me, int value)
{
	int row,col,y,x;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = (OM2D_TYPE)value;

}//end Fill.

/** Sum all the block values.
Arithmetic sum of all values.

@return:	sum
*/
int OverlayMem2D::Sum(OverlayMem2D& me)
{
	int					row,col;
	OM2D_TYPE*	pP;
	int					sum = 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		for(col = 0; col < me._width; col++)
		{
      sum += *(pP++);
		}//end for col...
	}//end for row...

	return(sum);
}//end Sum.

/** Subtract the input block from this.
The block dimensions must match else return 0.

@param b	: Input block that is subtracted.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2D::Sub(OverlayMem2D& me, OverlayMem2D& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	// Subtraction.
	int row,col,x,y,bx,by;
	OM2D_TYPE**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; row < me._height; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; col < me._width; col++, x++, bx++)
			me._pBlock[y][x] -= bPtr[by][bx];

	return(1);
}//end Sub.

/** Add the input block to this.
The block dimensions must match else return 0.

@param b	: Input block to add to.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2D::Add(OverlayMem2D& me, OverlayMem2D& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	// Addition.
	int row,col,x,y,bx,by;
	OM2D_TYPE**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; row < me._height; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; col < me._width; col++, x++, bx++)
			me._pBlock[y][x] += bPtr[by][bx];

	return(1);
}//end Add.

/** Calc the total square difference with the input block.
The block dimensions must match else return INF.

@param b	: Input block.
@return		: Total square diff.	
*/
int OverlayMem2D::Tsd(OverlayMem2D& me, OverlayMem2D& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(10000000);

	int row,col;
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
		}//end for col...
	}//end for row...

	return(acc);
}//end Tsd.

/** Calc the total square difference with the 4x4 input block.
The block dimensions must both be 4x4 and no checking is done
so use with caution.

@param b	: 4x4 input block.
@return		: Total square diff.	
*/
int OverlayMem2D::Tsd4x4(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][3]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tsd4x4.

/** Calc the total square difference with the 8x8 input block.
The block dimensions must both be 8x8 and no checking is done
so use with caution.

@param b	: 8x8 input block.
@return		: Total square diff.	
*/
int OverlayMem2D::Tsd8x8(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][7]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tsd8x8.

/** Calc the total square difference with the 16x16 input block.
The block dimensions must both be 16x16 and no checking is done
so use with caution.

@param b	: 16x16 input block.
@return		: Total square diff.	
*/
int OverlayMem2D::Tsd16x16(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][7]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][8]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][9]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][10]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][11]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][12]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][13]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][14]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		// [row][15]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tsd16x16.

/** The total square difference with the input to improve on an input value.
Exit early if the accumulated square diff becomes larger
than the specified input value. The block dimensions must 
match else return INF.

@param b		: Input block.
@param min	:	The min value to improve on.
@return			: Total square diff to the point of early exit.	
*/
int OverlayMem2D::TsdLessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(min + 10000000);

	int row,col;
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
	}//end for row...

	return(acc);
}//end TsdLessThan.

/** The total square difference with 4x4 blocks to improve on input value.
Exit early if the accumulated square error becomes larger	than the specified 
input value. The block dimensions must be 4x4 and no checking is done. Use
with caution for speed with unrolled inner loop.

@param b		: 4x4 input block.
@param min	:	The min value to improve on.
@return			: Total square diff to the point of early exit.	
*/
int OverlayMem2D::Tsd4x4LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd4x4LessThan.

/** The total square difference with 8x8 blocks to improve on input value.
Exit early if the accumulated square error becomes larger	than the specified 
input value. The block dimensions must be 8x8 and no checking is done. Use
with caution for speed with unrolled inner loop.

@param b		: 8x8 input block.
@param min	:	The min value to improve on.
@return			: Total square diff to the point of early exit.	
*/
int OverlayMem2D::Tsd8x8LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd8x8LessThan.

/** The total square difference with 16x16 blocks to improve on input value.
Exit early if the accumulated square error becomes larger	than the specified 
input value. The block dimensions must be 16x16 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 16x16 input block.
@param min	:	The min value to improve on.
@return			: Total square error to the point of early exit.	
*/
int OverlayMem2D::Tsd16x16LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][8]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][9]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][10]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][11]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][12]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][13]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][14]
    diff = *(pP++) - *(pI++);
		acc += (diff * diff);
		if(acc > min)	return(acc);
		// [row][15]
    diff = *(pP) - *(pI);
		acc += (diff * diff);

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd16x16LessThan.

/** Calc the total absolute difference with the input block.
The block dimensions must match else return INF.

@param b	: Input block.
@return		: Total absolute diff.	
*/
int OverlayMem2D::Tad(OverlayMem2D& me, OverlayMem2D& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(10000000);

	int row,col;
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
		}//end for col...
	}//end for row...

	return(acc);
}//end Tad.

/** Calc the total absolute difference with the 4x4 input block.
The block dimensions must both be 4x4 and no checking is done
so use with caution.

@param b	: 4x4 input block.
@return		: Total absolute diff.	
*/
int OverlayMem2D::Tad4x4(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][3]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tad4x4.

/** Calc the total absolute difference with the 8x8 input block.
The block dimensions must both be 8x8 and no checking is done
so use with caution.

@param b	: 8x8 input block.
@return		: Total absolute diff.	
*/
int OverlayMem2D::Tad8x8(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][3]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][4]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][5]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][6]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][7]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tad8x8.

/** Calc the total absolute difference with the 16x16 input block.
The block dimensions must both be 16x16 and no checking is done
so use with caution.

@param b	: 16x16 input block.
@return		: Total absolute diff.	
*/
int OverlayMem2D::Tad16x16(OverlayMem2D& me, OverlayMem2D& b)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][3]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][4]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][5]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][6]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][7]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][8]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][9]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][10]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][11]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][12]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][13]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][14]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		// [row][15]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
		}//end for col...
#endif

	}//end for row...

	return(acc);
}//end Tad16x16.

/** The total absolute difference with the input to improve on an input value.
Exit early if the accumulated absolute error becomes larger
than the specified input value. The block dimensions must 
match else return INF.

@param b		: Input block.
@param min	:	The min value to improve on.
@return			: Total absolute diff to the point of early exit.	
*/
int OverlayMem2D::TadLessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(min + 10000000);

	int row,col;
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
	}//end for row...

	return(acc);
}//end TadLessThan.

/** The total absolute difference with 4x4 blocks to improve on input value.
Exit early if the accumulated absolute error becomes larger	than the specified 
input value. The block dimensions must be 4x4 and no checking is done. Use
with caution for speed with unrolled inner loop.

@param b		: 4x4 input block.
@param min	:	The min value to improve on.
@return			: Total absolute diff to the point of early exit.	
*/
int OverlayMem2D::Tad4x4LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tad4x4LessThan.

/** The total absolute difference with 8x8 blocks to improve on input value.
Exit early if the accumulated absolute error becomes larger	than the specified 
input value. The block dimensions must be 8x8 and no checking is done. Use
with caution for speed with unrolled inner loop.

@param b		: 8x8 input block.
@param min	:	The min value to improve on.
@return			: Total absolute diff to the point of early exit.	
*/
int OverlayMem2D::Tad8x8LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tad8x8LessThan.

/** The total absolute difference with 16x16 blocks to improve on input value.
Exit early if the accumulated absolute error becomes larger	than the specified 
input value. The block dimensions must be 16x16 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 16x16 input block.
@param min	:	The min value to improve on.
@return			: Total absolute diff to the point of early exit.	
*/
int OverlayMem2D::Tad16x16LessThan(OverlayMem2D& me, OverlayMem2D& b, int min)
{
	OM2D_TYPE**	bPtr	= b.Get2DSrcPtr();
	OM2D_TYPE*	pP;
	OM2D_TYPE*	pI;
	int					acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2D_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][8]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][9]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][10]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][11]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][12]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][13]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][14]
    diff = *(pP++) - *(pI++);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;
		if(acc > min)	return(acc);
		// [row][15]
    diff = *(pP) - *(pI);
		if(diff >= 0)	acc += diff;
		else					acc -= diff;

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			if(diff >= 0)	acc += diff;
			else					acc -= diff;
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2D_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tad16x16LessThan.

/** Test the blocks for equality.
The block dimensions must match else return 0.
@param b		: Input block.
@return			: 1 = are equal, 0 = not equal.
*/
int OverlayMem2D::Equals(OverlayMem2D& me, OverlayMem2D& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	
	int	equal = 1; // Assume equality until proven different.
	int row,col,x,y,bx,by;
	OM2D_TYPE**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; (row < me._height) && equal; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; (col < me._width) && equal; col++, x++, bx++)
			if(me._pBlock[y][x] != bPtr[by][bx])
				equal = 0;

	return(equal);
}//end Equals.

/*
---------------------------------------------------------------------------
	Static utility methods.
---------------------------------------------------------------------------
*/
/** Sub sample the 2D src by half into a another 2D dst.
There is no bounds checking and therefore the dst must be valid on entry with
half the width and half the height of the src and possibly extended by an offset.
ASSUMPTION: All values are positive.
@param srcPtr		: Addressable as srcPtr[][].
@param srcWidth	: Src width.
@param srcHeight: Src height.
@param dstPtr		: 2D addressable dstPtr[srcHeight/2 + heightOff][srcWidth/2 + widthOff].
@param widthOff	: X offset into dst.
@param heightOff: Y offset into dst.
@return					: none.
*/
void OverlayMem2D::Half(void** srcPtr, int srcWidth, int srcHeight, 
												void** dstPtr, int widthOff, int heightOff)
{
	OM2D_TYPE** ppS = (OM2D_TYPE **)(srcPtr);
	OM2D_TYPE** ppD = (OM2D_TYPE **)(dstPtr);

	int m,n,x,y;
  for(m = 0, y = heightOff; m < srcHeight; m += 2, y++)
		for(n = 0, x = widthOff; n < srcWidth; n += 2, x++)
  {
		ppD[y][x] = (ppS[m][n] + ppS[m][n+1] + ppS[m+1][n] + ppS[m+1][n+1] + 2) >> 2;
	}//end for m & n...

}//end Half.


/*
---------------------------------------------------------------------------
	Depricated methods.
---------------------------------------------------------------------------
*/

/** Write the from the input source into this block.
Copy all values from the top left of the source into this block starting
at the input specified offset. This block is filled and no bounds checking
is performed. Return if this block does not exist.

@param srcPtr			: Top left source data pointer. 

@param srcWidth		: Total source width.

@param srcHeight	: Total source height.

@return 					: 0 = failure, 1 = success.
*/
//int OverlayMem2D::Write(void* srcPtr,	int	srcWidth,	int	srcHeight)
//{
//	// Check before copying.
//	if(_pMem == NULL)
//		return(0);
//
//	// This object is filled from the top left corner of the source.
//	OM2D_PTYPE	pLcl = (OM2D_PTYPE)srcPtr;
//
//	// Slow.
//	//int				col,row;
//	//for(row = 0; row < _height; row++)
//	//	for(col = 0; col < _width; col++)
//	//		_pBlock[row+_yPos][col+_xPos] = pLcl[(row * srcWidth) + col];
//
//	// Fast.
//	int	pos = 0;
//	for(int row = 0; row < _height; row++, pos += srcWidth)
//		memcpy((void *)(&_pBlock[row+_yPos][_xPos]), (const void *)(&(pLcl[pos])), _width * sizeof(OM2D_TYPE));
//
//	return(1);
//}//end Write.

/** Copy from the input source into an offset within this block.
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
//int OverlayMem2D::Write(void* srcPtr, int srcWidth, int srcHeight, 
//																			int toCol,		int toRow,
//																			int cols,			int rows)
//{
//	// Check before copying.
//	if(_pMem == NULL)
//		return(0);
//
//	// This object is patially filled from a srcWidth X srcHeight 2-D source 
//	// with cols x rows of values into the block offset.
//	OM2D_PTYPE	pLcl = (OM2D_PTYPE)srcPtr;
//
//	// Fast.
//	int	pos = 0;
//	toRow		+= _yPos;
//	toCol		+= _xPos;
//	for(int row = 0; row < rows; row++, pos += srcWidth)
//		memcpy((void *)(&(_pBlock[toRow + row][toCol])), (const void *)(&(pLcl[pos])), cols * sizeof(OM2D_TYPE));
//
//	return(1);
//}//end Write.

/** Read this block into a 2D destination.
No mem column or row overflow checking is done during the read process. Use
this method with caution.	If the dest is larger than the this block then
the top left hand corner is filled.

@param dstPtr			: Destination to read to.

@param dstWidth		: Total destination width.

@param dstHeight	: Total destination height.

@return 					: None.
*/
//void OverlayMem2D::Read(void* dstPtr, int dstWidth, int dstHeight)
//{
//	OM2D_PTYPE	pLcl = (OM2D_PTYPE)dstPtr;
//
//	int	dstPos = 0;
//	for(int row = 0; row < _height; row++, dstPos += dstWidth)
//	{
//		memcpy((void *)(&(pLcl[dstPos])), 
//					 (const void *)(&(_pBlock[_yPos + row][_xPos])), 
//					 _width * sizeof(OM2D_TYPE));
//	}//end for row...
//
//}//end Read.

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
//void OverlayMem2D::Read(void* dstPtr, int dstWidth,	int dstHeight,
//																			int fromCol,	int fromRow,
//																			int cols,			int rows)
//{
//	OM2D_PTYPE	pLcl = (OM2D_PTYPE)dstPtr;
//
//	int	dstPos = 0;
//	// Adjust for the underlying mem offsets.
//	fromRow += _yPos;
//	fromCol += _xPos;
//	for(int row = 0; row < rows; row++, dstPos += dstWidth)
//	{
//		memcpy((void *)(&(pLcl[dstPos])), 
//					 (const void *)(&(_pBlock[fromRow + row][fromCol])),	
//					 cols * sizeof(OM2D_TYPE));
//	}//end for row...
//
//}//end Read.

/** Read from an offset + half location within this block to a 2D destination.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Note that values outside of the _width and _height
need to be valid for the half calculation.

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
//void OverlayMem2D::HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
//																					int fromCol,		int fromRow,
//																					int halfColOff,	int halfRowOff,
//																					int cols,				int rows)
//{
//	OM2D_PTYPE	pLcl = (OM2D_PTYPE)dstPtr;
//	int					col,row,srcRow,dstRow;
//	// Adjust overlay offset.
//	fromRow += _yPos;
//	fromCol += _xPos;
//
//	// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.
//
//	if(halfColOff && halfRowOff)			// Diagonal case.
//	{
//		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
//		{
//			int srcCol,dstPos;
//			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
//			{
//				int z =	(int)_pBlock[srcRow           ][srcCol] +	
//								(int)_pBlock[srcRow           ][srcCol+halfColOff] +
//								(int)_pBlock[srcRow+halfRowOff][srcCol] + 
//								(int)_pBlock[srcRow+halfRowOff][srcCol+halfColOff];
//				if(z >= 0)
//					z = (z + 2) >> 2;
//				else
//					z = (z - 2)/4;
//				pLcl[dstPos] = (OM2D_TYPE)z;
//			}//end for col...
//		}//end for row...
//	}//end if halfColOff...
//	else if(halfColOff || halfRowOff)	// Linear case.
//	{
//		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
//		{
//			int srcCol,dstPos;
//			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
//			{
//				int z =	(int)_pBlock[srcRow           ][srcCol] +	
//								(int)_pBlock[srcRow+halfRowOff][srcCol+halfColOff];
//				if(z >= 0)
//					z = (z + 1) >> 1;
//				else
//					z = (z - 1)/2;
//				pLcl[dstPos] = (OM2D_TYPE)z;
//			}//end for col...
//		}//end for row...
//	}//end else if halfColOff...
//	else															// Origin case (Shouldn't ever be used).
//	{
//		for(row = 0, srcRow = fromRow, dstRow = 0; row < rows; row++, srcRow++, dstRow += dstWidth)
//		{
//			int srcCol,dstPos;
//			for(col = 0, srcCol = fromCol, dstPos = dstRow; col < cols; col++, srcCol++, dstPos++)
//				pLcl[dstPos] = _pBlock[srcRow][srcCol];
//		}//end for row...
//	}//end else...
//}//end HalfRead.






