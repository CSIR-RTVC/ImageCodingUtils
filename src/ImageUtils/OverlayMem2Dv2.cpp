/** @file

MODULE				: OverlayMem2Dv2

TAG						: OM2DV2

FILE NAME			: OverlayMem2Dv2.cpp

DESCRIPTION		: A class to overlay a two-dimensional mem structure onto
								a contiguous block (usually larger) of memory and provide
								several operations on 2-D blocks where the data type is
								short (16-bit variables). Static proxies are used	to reduce 
								the memory footprint of the class.

COPYRIGHT			: (c)CSIR 2007-2017 all rights resevered

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

#include "OverlayMem2Dv2.h"
#include "MeasurementTable.h" 

/*
---------------------------------------------------------------------------
Struct definition.
---------------------------------------------------------------------------
*/
typedef struct _OM2DV2_COORD
{
  short int x;
  short int y;
} OM2DV2_COORD;

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define OM2DV2_UNROLL_INNER_LOOP

int OverlayMem2Dv2::OM2DV2_Sp[17] = {0, 0, 2, 2, 0, 1, 3, 3, 1, 1, 3, 0, 2, 3, 1, 2, 0};
int OverlayMem2Dv2::OM2DV2_Tp[17] = {0, 0, 2, 0, 2, 1, 3, 1, 3, 0, 2, 1, 3, 0, 2, 1, 3};

/*
---------------------------------------------------------------------------
	Macros.
---------------------------------------------------------------------------
*/
#define OM2DV2_FAST_ABS32(x) ( ((x)^((x)>>31))-((x)>>31) )
#define OM2DV2_FAST_ABS16(x) ( ((x)^((x)>>15))-((x)>>15) )

#define OM2DV2_CLIP255(x)	( (((x) <= 255)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:255 ) )

#define OM2DV2_6TAP(minus3, minus2, minus1, plus1, plus2, plus3) ( (minus3) - 5*(minus2) + 20*(minus1) + 20*(plus1) - 5*(plus2) + (plus3) )
#define OM2DV2_VERT_6TAP(ptr, x, y)  ( OM2DV2_6TAP((int)((ptr)[(y)-2][(x)]), (int)((ptr)[(y)-1][(x)]), (int)((ptr)[(y)][(x)]), (int)((ptr)[(y)+1][(x)]), (int)((ptr)[(y)+2][(x)]), (int)((ptr)[(y)+3][(x)])) )
#define OM2DV2_HORIZ_6TAP(ptr, x, y) ( OM2DV2_6TAP((int)((ptr)[(y)][(x)-2]), (int)((ptr)[(y)][(x)-1]), (int)((ptr)[(y)][(x)]), (int)((ptr)[(y)][(x)+1]), (int)((ptr)[(y)][(x)+2]), (int)((ptr)[(y)][(x)+3])) )

#define OM2DV2_GET_J(ptr, x, y) ((OM2DV2_6TAP(OM2DV2_VERT_6TAP((ptr),(x)-2,(y)), OM2DV2_VERT_6TAP((ptr),(x)-1,(y)), OM2DV2_VERT_6TAP((ptr),(x),(y)), OM2DV2_VERT_6TAP((ptr),(x)+1,(y)), OM2DV2_VERT_6TAP((ptr),(x)+2,(y)), OM2DV2_VERT_6TAP((ptr),(x)+3,(y))) + 512) >> 10)
#define OM2DV2_GET_B(ptr, x, y)   ((OM2DV2_HORIZ_6TAP((ptr),(x),(y)) + 16) >> 5)
#define OM2DV2_GET_H(ptr, x, y)   ((OM2DV2_VERT_6TAP((ptr),(x),(y)) + 16) >> 5)

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

void OverlayMem2Dv2::ResetMembers(void)
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
OverlayMem2Dv2::OverlayMem2Dv2(void* srcPtr, int srcWidth, int srcHeight, int width, int height)
{
	ResetMembers();

	if(srcPtr == NULL)
		return;

	_width			= width;
	_height			= height;
	_srcWidth		= srcWidth;
	_srcHeight	= srcHeight;
	_pMem				= (short *)srcPtr;

	/// Potentially dangerous to alloc mem in a constructor as there is no way 
	/// of determining failure.

	/// Alloc the mem block row addresses.
	_pBlock = new short *[srcHeight];
	if(_pBlock == NULL)
		return;

	/// Fill the row addresses.
	for(int row = 0; row < srcHeight; row++)
		_pBlock[row] = &(_pMem[srcWidth * row]);

}//end alt constructor.

OverlayMem2Dv2::~OverlayMem2Dv2()
{
	// Ensure no mem is left alloc.
	Destroy();
}//end destructor.

void OverlayMem2Dv2::Destroy(void)
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
int OverlayMem2Dv2::SetMem(void* srcPtr, int srcWidth, int srcHeight)
{
	if(srcPtr == NULL)
		return(0);

	_srcWidth		= srcWidth;
	_srcHeight	= srcHeight;
	_pMem				= (short *)srcPtr;

	// 2-Dim ptr must be recreated.
	if(_pBlock != NULL)
		delete[] _pBlock;
	_pBlock = NULL;

	// Alloc the mem block row addresses.
	_pBlock = new short *[srcHeight];
	if(_pBlock == NULL)
		return(0);

	// Fill the row addresses.
	for(int row = 0; row < srcHeight; row++)
		_pBlock[row] = &(_pMem[srcWidth * row]);

	return(1);
}//end SetMem.

void OverlayMem2Dv2::SetOrigin(int x, int y)
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
int OverlayMem2Dv2::Write(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock, int toCol, int toRow)
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
					 width * sizeof(short));

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
int OverlayMem2Dv2::Write(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock, 
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
					 colLimit * sizeof(short));

	return(1);
}//end Write.

/** Write the entire source block into this block.
Copy all values from the top left of the source into this block. Return if 
the source does not have the same dimensions as this block.
does not exist.
@param srcBlock		: The block to copy. 
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2Dv2::Write(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock)
{
	if( (me._width != srcBlock._width)||(me._height != srcBlock._height) )
		return(0);

	for(int row = 0; row < me._height; row++)
		memcpy((void *)(&(me._pBlock[me._yPos + row][me._xPos])), 
					 (const void *)(&(srcBlock._pBlock[srcBlock._yPos + row][srcBlock._xPos])), 
					 me._width * sizeof(short));

	return(1);
}//end Write.

/** Write the entire 4x4 source block into this block.
Copy all values from the top left of the source into this block. The source
block must have 4x4 dimensions. Note that no dimension checking is done.
does not exist.
@param srcBlock		: The 8x8 block to copy. 
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2Dv2::Write4x4(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock)
{
	register int row,y,srcy;

	int			meOffX	= me._xPos;
	short**	srcPtr	= srcBlock.Get2DSrcPtr();
	int			srcOffX	= srcBlock._xPos;

	for(row = 0, y = me._yPos, srcy = srcBlock._yPos; row < 4; row++, y++, srcy++)
	{
		memcpy((void *)(&(me._pBlock[y][meOffX])), (const void *)(&(srcPtr[srcy][srcOffX])), 8);
	}//end for row...

	return(1);
}//end Write4x4.

/** Write the entire 8x8 source block into this block.
Copy all values from the top left of the source into this block. The source
block must have 8x8 dimensions. Note that no dimension checking is done.
does not exist.
@param srcBlock		: The 8x8 block to copy. 
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2Dv2::Write8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock)
{
	register int row,y,srcy;

	int			meOffX	= me._xPos;
	short**	srcPtr	= srcBlock.Get2DSrcPtr();
	int			srcOffX	= srcBlock._xPos;

	for(row = 0, y = me._yPos, srcy = srcBlock._yPos; row < 8; row++, y++, srcy++)
	{
		memcpy((void *)(&(me._pBlock[y][meOffX])), (const void *)(&(srcPtr[srcy][srcOffX])), 16);
	}//end for row...

	return(1);
}//end Write8x8.

/** Write the entire 16x16 source block into this block.
Copy all values from the top left of the source into this block. The source
block must have 16x16 dimensions. Note that no dimension checking is done.
does not exist.
@param srcBlock		: The 16x16 block to copy. 
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2Dv2::Write16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& srcBlock)
{
	register int row,y,srcy;

	int			meOffX	= me._xPos;
	short**	srcPtr	= srcBlock.Get2DSrcPtr();
	int			srcOffX	= srcBlock._xPos;

	for(row = 0, y = me._yPos, srcy = srcBlock._yPos; row < 16; row++, y++, srcy++)
	{
		memcpy((void *)(&(me._pBlock[y][meOffX])), (const void *)(&(srcPtr[srcy][srcOffX])), 32);
	}//end for row...

	return(1);
}//end Write16x16.

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
void OverlayMem2Dv2::Read(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, 
												int toCol,				int toRow,
												int fromCol,			int fromRow,
												int cols,					int rows)
{
	short**	dstPtr = dstBlock.Get2DSrcPtr();
	toCol		+= dstBlock._xPos;
	toRow		+= dstBlock._yPos;
	fromCol += me._xPos;
	fromRow += me._yPos;
	for(int row = 0; row < rows; row++)
	{
		memcpy((void *)(&(dstPtr[toRow + row][toCol])), 
					 (const void *)(&(me._pBlock[fromRow + row][fromCol])),	
					 cols * sizeof(short));
	}//end for row...

}//end Read.

/** Read from this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Return if the dimensions are not equal.
@param dstBlock		: Destination to read to.
@return 					: 0 = failure, 1 = success.
*/
int OverlayMem2Dv2::Read(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock)
{
	if( (me._width != dstBlock._width)||(me._height != dstBlock._height) )
		return(0);

	short**	dstPtr = dstBlock.Get2DSrcPtr();
	int toCol	= dstBlock._xPos;
	int toRow	= dstBlock._yPos;

	for(int row = 0; row < me._height; row++)
	{
		memcpy((void *)(&(dstPtr[toRow + row][toCol])), 
					 (const void *)(&(me._pBlock[me._yPos + row][me._xPos])),	
					 me._width * sizeof(short));
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
void OverlayMem2Dv2::HalfRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int halfColOff,	int halfRowOff)
{
	short**	pLcl = dstBlock.Get2DSrcPtr();
	int	row,col,srcRow,dstRow;

	/// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. and 4 x Linear.

#ifdef OM2DV2_UNROLL_INNER_LOOP
	if( (halfColOff == 1) && halfRowOff )			///< Diagonal cases [1,1], [1,-1].
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcRowHalf = srcRow + halfRowOff;
			int srcCol,dstCol,y,z;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col += 2)	///< 2 at a time.
			{
				/// Loop 1.
				z =	(int)me._pBlock[srcRow][srcCol] +	(int)me._pBlock[srcRowHalf][srcCol];
				srcCol++;
				y = (int)me._pBlock[srcRow][srcCol] + (int)me._pBlock[srcRowHalf][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 2)/4;

				/// Loop 2.
				srcCol++;
				z = (int)me._pBlock[srcRow][srcCol] + (int)me._pBlock[srcRowHalf][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 2)/4;
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if( (halfColOff == -1) && halfRowOff )			///< Diagonal cases [-1,1], [-1,-1].
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcRowHalf = srcRow + halfRowOff;
			int srcCol,dstCol,y,z;
			for(col = 0, srcCol = (me._xPos-1), dstCol = dstBlock._xPos; col < me._width; col += 2)	///< 2 at a time.
			{
				/// Loop 1.
				z =	(int)me._pBlock[srcRow][srcCol] + (int)me._pBlock[srcRowHalf][srcCol];
				srcCol++;
				y =	(int)me._pBlock[srcRow][srcCol] +	(int)me._pBlock[srcRowHalf][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 2)/4;

				/// Loop 2.
				srcCol++;
				z = (int)me._pBlock[srcRow][srcCol] + (int)me._pBlock[srcRowHalf][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 2)/4;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else if( halfColOff && (halfRowOff == 0) )	///< Linear cases [1,0], [-1,0].
	{
		int start = me._xPos;
		if(halfColOff < 0)
			start--;
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol,y,z;
			for(col = 0, srcCol = start, dstCol = dstBlock._xPos; col < me._width; col += 2)
			{
				/// Loop 1.
				z =	(int)me._pBlock[srcRow][srcCol];
				srcCol++;
				y = (int)me._pBlock[srcRow][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 1)/2;

				/// Loop 2.
				srcCol++;
				z =	(int)me._pBlock[srcRow][srcCol];

				pLcl[dstRow][dstCol++] = (short)(z + y + 1)/2;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else if( (halfColOff == 0) && halfRowOff )	///< Linear cases [0,1], [0,-1].
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcRowHalf = srcRow + halfRowOff;
			int srcCol,dstCol,y,z;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col += 2)
			{
				/// Loop 1.
				z =	(int)me._pBlock[srcRow][srcCol];
				y = (int)me._pBlock[srcRowHalf][srcCol++];

				pLcl[dstRow][dstCol++] = (short)(z + y + 1)/2;

				/// Loop 2.
				z =	(int)me._pBlock[srcRow][srcCol];
				y = (int)me._pBlock[srcRowHalf][srcCol++];

				pLcl[dstRow][dstCol++] = (short)(z + y + 1)/2;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
#else
	if(halfColOff && halfRowOff)			///< Diagonal case.
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcRowHalf = srcRow + halfRowOff;
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =	((int)me._pBlock[srcRow    ][srcCol] +	
								 (int)me._pBlock[srcRow    ][srcCol+halfColOff] +
								 (int)me._pBlock[srcRowHalf][srcCol] + 
								 (int)me._pBlock[srcRowHalf][srcCol+halfColOff] + 2)/4;
		
				pLcl[dstRow][dstCol] = (short)z;
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if(halfColOff || halfRowOff)	///< Linear case.
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcRowHalf = srcRow + halfRowOff;
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =	((int)me._pBlock[srcRow    ][srcCol] +	
								 (int)me._pBlock[srcRowHalf][srcCol+halfColOff] + 1)/2;

				pLcl[dstRow][dstCol] = (short)z;
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
#endif
	else															///< Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
				pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
}//end HalfRead.

/** Read from a half location offset around this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Note that values outside of the _width and _height
need to be valid for the half calculation.
@param dstBlock		: Destination to read to.
@param halfColOff	: Half location offset from fromCol.
@param halfRowOff	: Half location offset from fromRow.
@return 					: None.
*/
void OverlayMem2Dv2::HalfReadv2(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int halfColOff,	int halfRowOff)
{
	short**	pLcl = dstBlock.Get2DSrcPtr();
	int	row,col,srcRow,dstRow;

	/// Half location calc has 3 cases: 1 x [0,0], 4 x Diag. (square) and 4 x Linear (cross).

	if(halfColOff && halfRowOff)			///< Diagonal case.
	{
		int offsetX = halfColOff; ///< Either a -1 or +1.
		if(halfColOff == 1)
			offsetX = 0;
		int offsetY = halfRowOff; ///< Either a -1 or +1.
		if(halfRowOff == 1)
			offsetY = 0;
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int lclOffY = srcRow + offsetY - 2;
				int lclOffX = srcCol + offsetX;
				int lclTemp[6];

				/// Step through 6 rows to produce the temp intermediate values of aa, bb, b, s, gg, hh into lclTmp[].
				for(int tmp = 0; tmp < 6; tmp++, lclOffY++)
				{
					lclTemp[tmp] =		 (int)me._pBlock[lclOffY][lclOffX - 2] -  5*(int)me._pBlock[lclOffY][lclOffX - 1] +
													20*(int)me._pBlock[lclOffY][lclOffX]     + 20*(int)me._pBlock[lclOffY][lclOffX + 1] -
													 5*(int)me._pBlock[lclOffY][lclOffX + 2] +    (int)me._pBlock[lclOffY][lclOffX + 3];
				}//end for tmp...

				int j = lclTemp[0] - 5*lclTemp[1] + 20*lclTemp[2] + 20*lclTemp[3] - 5*lclTemp[4] + lclTemp[5];

				int x = (j + 512) >> 10;
				pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(x);
			}//end for col...
		}//end for row...
	}//end if halfColOff...
	else if(halfColOff)	///< Linear horizontal case.
	{
		int offset = halfColOff; ///< Either a -1 or +1.
		if(halfColOff == 1)
			offset = 0;
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =		 (int)me._pBlock[srcRow][srcCol + offset - 2] -  5*(int)me._pBlock[srcRow][srcCol + offset - 1] +
								20*(int)me._pBlock[srcRow][srcCol + offset]     + 20*(int)me._pBlock[srcRow][srcCol + offset + 1] -
								 5*(int)me._pBlock[srcRow][srcCol + offset + 2] +    (int)me._pBlock[srcRow][srcCol + offset + 3];

				int x = (z + 16) >> 5;
				pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(x);
			}//end for col...
		}//end for row...
	}//end else if halfColOff...
	else if(halfRowOff)	///< Linear vertical case.
	{
		int offset = halfRowOff; ///< Either a -1 or +1.
		if(halfRowOff == 1)
			offset = 0;
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int z =	   (int)me._pBlock[srcRow + offset - 2][srcCol] -  5*(int)me._pBlock[srcRow + offset - 1][srcCol] +	
								20*(int)me._pBlock[srcRow + offset][srcCol]     + 20*(int)me._pBlock[srcRow + offset + 1][srcCol] - 
								 5*(int)me._pBlock[srcRow + offset + 2][srcCol] +    (int)me._pBlock[srcRow + offset + 3][srcCol];

				int x = (z + 16) >> 5;
				pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(x);
			}//end for col...
		}//end for row...
	}//end else if halfRowOff...
	else															///< Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
				pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
}//end HalfReadv2.

/** Read from an 1/4 location offset around this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Note that values outside of the _width and _height
need to be valid for the 1/4 calculation.
@param dstBlock				: Destination to read to.
@param quarterColOff	: 1/4 location offset from fromCol.
@param quarterRowOff	: 1/4 location offset from fromRow.
@return 							: None.
*/
void OverlayMem2Dv2::QuarterRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int quarterColOff,	int quarterRowOff)
{
	short**	pLcl = dstBlock.Get2DSrcPtr();
	int	row, col, srcRow, dstRow;

	/// Offsets are only positive numbers from the top left position so all -ve offsets are reflected
	/// to full pel locations to the left or above the current block position.
	int fullOffX	= 0;
	int xFrac			= quarterColOff;
	if(quarterColOff < 0)
	{
		fullOffX	= -1;
		xFrac			= 4 + quarterColOff;	///< e.g. -1 from full pel offset 0 = +3 from full pel offset -1. 
	}//end if quarterColOff...
	int fullOffY	= 0;
	int yFrac			= quarterRowOff;
	if(quarterRowOff < 0)
	{
		fullOffY	= -1;
		yFrac			= 4 + quarterRowOff;	 
	}//end if quarterRowOff...

	/// The quarter fractions are in the range [0..3] and for every frac value pair there is a unique calculation. The
	/// half pel positions are required first before the 1/4 and 3/4 positions can be calculated.
	int selection = (xFrac & 3) | ((yFrac << 2) & 12);
	switch(selection)
	{
		case 2:	///< = b.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);

						pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(b);
					}//end for col...
				}//end for row...
			}
			break;
		case 8:	///< = h.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);

						pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(h);
					}//end for col...
				}//end for row...
			}
			break;
		case 10:	///< = j.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
          int lclOffY = srcRow + fullOffY;
          for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
            int lclOffX = srcCol + fullOffX;
            int j = OM2DV2_GET_J(me._pBlock, lclOffX, lclOffY);

						pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(j);
					}//end for col...
				}//end for row...
			}
			break;
		case 1:	///< = a.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);

						int a = ((int)me._pBlock[lclOffY][lclOffX] + OM2DV2_CLIP255(b) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)a;
					}//end for col...
				}//end for row...
			}
			break;
		case 3:	///< = c.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);

						int c = ((int)me._pBlock[lclOffY][lclOffX+1] + OM2DV2_CLIP255(b) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)c;
					}//end for col...
				}//end for row...
			}
			break;
		case 4:	///< = d.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);

						int d = ((int)me._pBlock[lclOffY][lclOffX] + OM2DV2_CLIP255(h) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)d;
					}//end for col...
				}//end for row...
			}
			break;
		case 12:	///< = n.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);

						int n = ((int)me._pBlock[lclOffY+1][lclOffX] + OM2DV2_CLIP255(h) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)n;
					}//end for col...
				}//end for row...
			}
			break;
		case 6:	///< = f.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
          int lclOffY = srcRow + fullOffY;
          for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
            int lclOffX = srcCol + fullOffX;
            int j = OM2DV2_GET_J(me._pBlock, lclOffX, lclOffY);
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);

						int f = (OM2DV2_CLIP255(j) + OM2DV2_CLIP255(b) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)f;
					}//end for col...
				}//end for row...
			}
			break;
		case 14:	///< = q.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
            int lclOffY = srcRow + fullOffY;
            int lclOffX = srcCol + fullOffX;
            int j = OM2DV2_GET_J(me._pBlock, lclOffX, lclOffY);
						lclOffY++;
            int s = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY); ///< "s" is a "b" for the row below. 

						int q = (OM2DV2_CLIP255(j) + OM2DV2_CLIP255(s) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)q;
					}//end for col...
				}//end for row...
			}
			break;
		case 9:	///< = i.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
          int lclOffY = srcRow + fullOffY;
          for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
            int lclOffX = srcCol + fullOffX;
            int j = OM2DV2_GET_J(me._pBlock, lclOffX, lclOffY);
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);

						int i = (OM2DV2_CLIP255(j) + OM2DV2_CLIP255(h) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)i;
					}//end for col...
				}//end for row...
			}
			break;
		case 11:	///< = k.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
          int lclOffY = srcRow + fullOffY;
          for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
            int lclOffX = srcCol + fullOffX;
            int j = OM2DV2_GET_J(me._pBlock, lclOffX, lclOffY);
						lclOffX++;
            int m = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY); ///< "m" is an "h" in the next col.

						int k = (OM2DV2_CLIP255(j) + OM2DV2_CLIP255(m) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)k;
					}//end for col...
				}//end for row...
			}
			break;
		case 5:	///< = e.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);

						int e = (OM2DV2_CLIP255(b) + OM2DV2_CLIP255(h) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)e;
					}//end for col...
				}//end for row...
			}
			break;
		case 7:	///< = g.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffX = srcCol + fullOffX;
            int b = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY);
						lclOffX++;
            int m = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY); ///< "m" is an "h" in the next col.

						int g = (OM2DV2_CLIP255(b) + OM2DV2_CLIP255(m) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)g;
					}//end for col...
				}//end for row...
			}
			break;
		case 13:	///< = p.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffY = srcRow + fullOffY;
						int lclOffX = srcCol + fullOffX;
            int h = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY);
						lclOffY++;
            int s = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY); ///< "s" is a "b" for the row below. 

						int p = (OM2DV2_CLIP255(h) + OM2DV2_CLIP255(s) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)p;
					}//end for col...
				}//end for row...
			}
			break;
		case 15:	///< = r.
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					int lclOffY = srcRow + fullOffY;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
					{
						int lclOffY = srcRow + fullOffY;
						int lclOffX = srcCol + fullOffX + 1;
            int m = OM2DV2_GET_H(me._pBlock, lclOffX, lclOffY); ///< "m" is an "h" in the next col.
						lclOffY++;
						lclOffX--;
            int s = OM2DV2_GET_B(me._pBlock, lclOffX, lclOffY); ///< "s" is a "b" for the row below. 

						int r = (OM2DV2_CLIP255(m) + OM2DV2_CLIP255(s) + 1) >> 1;

						pLcl[dstRow][dstCol] = (short)r;
					}//end for col...
				}//end for row...
			}
			break;
		case 0:	///< Origin case (Shouldn't ever be used).
		default:
			{
				for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
				{
					int srcCol,dstCol;
					for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
						pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
				}//end for row...
			}//end default...
			break;
	}//end switch selection...
/*
	if(xFrac || yFrac)
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int lclOffY = srcRow + fullOffY;
				int lclOffX = srcCol + fullOffX;

				int z = (8-xFracC)*(8-yFracC)*(int)me._pBlock[lclOffY][lclOffX]     + xFracC*(8-yFracC)*(int)me._pBlock[lclOffY][lclOffX + 1] +
								(8-xFracC)*yFracC    *(int)me._pBlock[lclOffY + 1][lclOffX] + xFracC*yFracC    *(int)me._pBlock[lclOffY + 1][lclOffX + 1];

				int x = (z + 32) >> 6;
				pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(x);
			}//end for col...
		}//end for row...
	}//end if xFrac...
	else															///< Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
				pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...
*/
}//end QuarterRead.

/** Read from an 1/8th location offset around this block to a destination block.
No mem column or row overflow checking is done during the read process. Use
this method with caution. Note that values outside of the _width and _height
need to be valid for the 1/8th calculation.
@param dstBlock			: Destination to read to.
@param eighthColOff	: 1/8th location offset from fromCol.
@param eighthRowOff	: 1/8th location offset from fromRow.
@return 						: None.
*/
void OverlayMem2Dv2::EighthRead(OverlayMem2Dv2& me, OverlayMem2Dv2& dstBlock, int eighthColOff,	int eighthRowOff)
{
	short**	pLcl = dstBlock.Get2DSrcPtr();
	int	row,col,srcRow,dstRow;

	/// Offsets are only positive numbers from the top left position so all -ve offsets are reflected
	/// to full pel locations to the left or above the current block position.
	int fullOffX	= 0;
	int xFracC			= eighthColOff;
	if(eighthColOff < 0)
	{
		fullOffX	= -1;
		xFracC			= 8 + eighthColOff;	///< e.g. -1 from full pel offset 0 = +7 from full pel offset -1. 
	}//end if eighthColOff...
	int fullOffY	= 0;
	int yFracC			= eighthRowOff;
	if(eighthRowOff < 0)
	{
		fullOffY	= -1;
		yFracC			= 8 + eighthRowOff;	 
	}//end if eighthRowOff...

	if(xFracC || yFracC)
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int lclOffY = srcRow + fullOffY;
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
			{
				int lclOffX = srcCol + fullOffX;

				int z = (8-xFracC)*(8-yFracC)*(int)me._pBlock[lclOffY][lclOffX]     + xFracC*(8-yFracC)*(int)me._pBlock[lclOffY][lclOffX + 1] +
								(8-xFracC)*yFracC    *(int)me._pBlock[lclOffY + 1][lclOffX] + xFracC*yFracC    *(int)me._pBlock[lclOffY + 1][lclOffX + 1];

				int x = (z + 32) >> 6;
				/// pLcl[dstRow][dstCol] = (short)OM2DV2_CLIP255(x);
				pLcl[dstRow][dstCol] = (short)x;
			}//end for col...
		}//end for row...
	}//end if xFracC...
	else															///< Origin case (Shouldn't ever be used).
	{
		for(row = 0, srcRow = me._yPos, dstRow = dstBlock._yPos; row < me._height; row++, srcRow++, dstRow++)
		{
			int srcCol,dstCol;
			for(col = 0, srcCol = me._xPos, dstCol = dstBlock._xPos; col < me._width; col++, srcCol++, dstCol++)
				pLcl[dstRow][dstCol] = me._pBlock[srcRow][srcCol];
		}//end for row...
	}//end else...

}//end EighthRead.

/*
---------------------------------------------------------------------------
	Public block operations interface.
---------------------------------------------------------------------------
*/

/** Clear the block values to zero.
Fast value zero-ing function.
@return:	none
*/
void OverlayMem2Dv2::Clear(OverlayMem2Dv2& me)
{
	for(int row = 0; row < me._height; row++)
	{
		memset((void *)(&(me._pBlock[me._yPos + row][me._xPos])), 0, me._width * sizeof(short));
	}//end for row...

}//end Clear.

/** Fill the block values.
Fast value set function.
@return:	none
*/
void OverlayMem2Dv2::Fill(OverlayMem2Dv2& me, int value)
{
	int row,col,y,x;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = (short)value;

}//end Fill.

/** Sum all the block values.
Arithmetic sum of all values.
@return:	sum
*/
int OverlayMem2Dv2::Sum(OverlayMem2Dv2& me)
{
	int			row,col;
	short*	pP;
	int			sum = 0;
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
int OverlayMem2Dv2::Sub(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);

	/// Subtraction.
	int row,col,x,y,bx,by;
	short**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; row < me._height; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; col < me._width; col++, x++, bx++)
			me._pBlock[y][x] -= bPtr[by][bx];

	return(1);
}//end Sub.

/** Subtract the 16x16 input block from this 8x8 block.
The block dimensions must be 16x16 and are therefore not checked
within the method. Fast version of subtract
@param b	: Input block that is subtracted.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Sub16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 16; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] -= pb[0]; pMe[8]  -= pb[8];
		pMe[1] -= pb[1]; pMe[9]  -= pb[9];
		pMe[2] -= pb[2]; pMe[10] -= pb[10];
		pMe[3] -= pb[3]; pMe[11] -= pb[11];
		pMe[4] -= pb[4]; pMe[12] -= pb[12];
		pMe[5] -= pb[5]; pMe[13] -= pb[13];
		pMe[6] -= pb[6]; pMe[14] -= pb[14];
		pMe[7] -= pb[7]; pMe[15] -= pb[15];

	}//end for row...

	return(1);
}//end Sub16x16.

/** Subtract the 8x8 input block from this 8x8 block.
The block dimensions must be 8x8 and are therefore not checked
within the method. Fast version of subtract
@param b	: Input block that is subtracted.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Sub8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 8; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] -= pb[0]; pMe[4] -= pb[4];
		pMe[1] -= pb[1]; pMe[5] -= pb[5];
		pMe[2] -= pb[2]; pMe[6] -= pb[6];
		pMe[3] -= pb[3]; pMe[7] -= pb[7];

	}//end for row...

	return(1);
}//end Sub8x8.

/** Subtract the input constant from this.
@param x	: Input constant that is subtracted.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Sub(OverlayMem2Dv2& me, short z)
{
	/// Subtraction.
	int row,col,x,y;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] -= z;

	return(1);
}//end Sub.

/** Add the input block to this.
The block dimensions must match else return 0.
@param b	: Input block to add to.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Add(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	// Addition.
	int row,col,x,y,bx,by;
	short**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; row < me._height; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; col < me._width; col++, x++, bx++)
			me._pBlock[y][x] += bPtr[by][bx];

	return(1);
}//end Add.

/** Add the input block to this and clip values to [0...255].
The block dimensions must match else return 0.
@param b	: Input block to add to.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::AddWithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	/// Addition.
	int row,col,x,y,bx,by;
	short**	bPtr = b.Get2DSrcPtr();
	for(row = 0, y = me._yPos, by = b._yPos; row < me._height; row++, y++, by++)
		for(col = 0, x = me._xPos, bx = b._xPos; col < me._width; col++, x++, bx++)
			me._pBlock[y][x] = OM2DV2_CLIP255(me._pBlock[y][x] + bPtr[by][bx]);

	return(1);
}//end AddWithClip255.

/** Add the 16x16 input block to this 16x16 block.
The block dimensions must be 16x16 and are therefore not checked
within the method. Fast version of addition
@param b	: Input block that is to be added.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Add16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 16; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] += pb[0]; pMe[8]  += pb[8];
		pMe[1] += pb[1]; pMe[9]  += pb[9];
		pMe[2] += pb[2]; pMe[10] += pb[10];
		pMe[3] += pb[3]; pMe[11] += pb[11];
		pMe[4] += pb[4]; pMe[12] += pb[12];
		pMe[5] += pb[5]; pMe[13] += pb[13];
		pMe[6] += pb[6]; pMe[14] += pb[14];
		pMe[7] += pb[7]; pMe[15] += pb[15];

	}//end for row...

	return(1);
}//end Add16x16.

/** Add the 16x16 input block to this 16x16 block and clip values to [0...255].
The block dimensions must be 16x16 and are therefore not checked
within the method. Fast version of addition
@param b	: Input block that is to be added.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Add16x16WithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 16; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] = OM2DV2_CLIP255(pMe[0] + pb[0]); pMe[8]  = OM2DV2_CLIP255(pMe[8]  + pb[8]);
		pMe[1] = OM2DV2_CLIP255(pMe[1] + pb[1]); pMe[9]  = OM2DV2_CLIP255(pMe[9]  + pb[9]);
		pMe[2] = OM2DV2_CLIP255(pMe[2] + pb[2]); pMe[10] = OM2DV2_CLIP255(pMe[10] + pb[10]);
		pMe[3] = OM2DV2_CLIP255(pMe[3] + pb[3]); pMe[11] = OM2DV2_CLIP255(pMe[11] + pb[11]);
		pMe[4] = OM2DV2_CLIP255(pMe[4] + pb[4]); pMe[12] = OM2DV2_CLIP255(pMe[12] + pb[12]);
		pMe[5] = OM2DV2_CLIP255(pMe[5] + pb[5]); pMe[13] = OM2DV2_CLIP255(pMe[13] + pb[13]);
		pMe[6] = OM2DV2_CLIP255(pMe[6] + pb[6]); pMe[14] = OM2DV2_CLIP255(pMe[14] + pb[14]);
		pMe[7] = OM2DV2_CLIP255(pMe[7] + pb[7]); pMe[15] = OM2DV2_CLIP255(pMe[15] + pb[15]);

	}//end for row...

	return(1);
}//end Add16x16WithClip255.

/** Add the 8x8 input block to this 8x8 block.
The block dimensions must be 8x8 and are therefore not checked
within the method. Fast version of addition
@param b	: Input block that is to be added.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Add8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 8; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] += pb[0]; pMe[4] += pb[4];
		pMe[1] += pb[1]; pMe[5] += pb[5];
		pMe[2] += pb[2]; pMe[6] += pb[6];
		pMe[3] += pb[3]; pMe[7] += pb[7];

	}//end for row...

	return(1);
}//end Add8x8.

/** Add the 8x8 input block to this 8x8 block and clip values to [0...255].
The block dimensions must be 8x8 and are therefore not checked
within the method. Fast version of addition
@param b	: Input block that is to be added.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Add8x8WithClip255(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	register int row,y,by;

	int			meOffX	= me._xPos;
	short**	bPtr		= b.Get2DSrcPtr();
	int			bOffX		= b._xPos;

	for(row = 0, y = me._yPos, by = b._yPos; row < 8; row++, y++, by++)
	{
		short* pMe = &(me._pBlock[y][meOffX]);
		short* pb  = &(bPtr[by][bOffX]);

		pMe[0] = OM2DV2_CLIP255(pMe[0] + pb[0]); pMe[4] = OM2DV2_CLIP255(pMe[4] + pb[4]);
		pMe[1] = OM2DV2_CLIP255(pMe[1] + pb[1]); pMe[5] = OM2DV2_CLIP255(pMe[5] + pb[5]);
		pMe[2] = OM2DV2_CLIP255(pMe[2] + pb[2]); pMe[6] = OM2DV2_CLIP255(pMe[6] + pb[6]);
		pMe[3] = OM2DV2_CLIP255(pMe[3] + pb[3]); pMe[7] = OM2DV2_CLIP255(pMe[7] + pb[7]);

	}//end for row...

	return(1);
}//end Add8x8WithClip255.

/** Shift the values in this block up.
@param z	: Shift up by.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::ScaleUp(OverlayMem2Dv2& me, short z)
{
	/// Subtraction.
	int row,col,x,y;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = me._pBlock[y][x] << z;

	return(1);
}//end ScaleUp.

/** Shift the values in this block down.
@param z	: Shift down by.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::ScaleDown(OverlayMem2Dv2& me, short z)
{
	/// Subtraction.
	int row,col,x,y;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = me._pBlock[y][x] >> z;

	return(1);
}//end ScaleDown.

/** Multiply the values in this block by a constant.
@param z	: Multiply by.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Multiply(OverlayMem2Dv2& me, short z)
{
	/// Subtraction.
	int row,col,x,y;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = me._pBlock[y][x] * z;

	return(1);
}//end Multiply.

/** Divide the values in this block by a constant.
@param z	: Divide by.
@return		: Success = 1, fail = 0;	
*/
int OverlayMem2Dv2::Divide(OverlayMem2Dv2& me, short z)
{
	/// Subtraction.
	int row,col,x,y;
	for(row = 0, y = me._yPos; row < me._height; row++, y++)
		for(col = 0, x = me._xPos; col < me._width; col++, x++)
			me._pBlock[y][x] = me._pBlock[y][x] / z;

	return(1);
}//end Divide.

/** Calc the total square difference with the input block.
The block dimensions must match else return INF.
@param b	: Input block.
@return		: Total square diff.	
*/
int OverlayMem2Dv2::Tsd(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(10000000);

	int row,col;
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
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
int OverlayMem2Dv2::Tsd4x4(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
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
int OverlayMem2Dv2::Tsd8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
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
int OverlayMem2Dv2::Tsd16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
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
int OverlayMem2Dv2::TsdLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(min + 10000000);

	int row,col;
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
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
int OverlayMem2Dv2::Tsd4x4LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
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
#endif	// OM2DV2_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd4x4LessThan.

/** The total square difference with 4x4 blocks to improve on input value.
Exit early if the accumulated partial square error becomes larger	than the specified 
partial input value. The block dimensions must be 4x4 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 4x4 input block.
@param min	:	The min value to improve on.
@return			: Total square diff to the point of early exit.	
*/
int OverlayMem2Dv2::Tsd4x4PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int Dp = 0;	///< Accumulated partial square error.

	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

		int pd = 0;
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			pd += (diff * diff);
		}//end for col...

		/// Accumulated partial sqr err.
		Dp += pd;

		if( (Dp << 2) > ((row+1) * min) )
			return( Dp << 2 );	///< Early exit because exceeded min.
	}//end for row...

	return(Dp);
}//end Tsd4x4PartialLessThan.

/** The total square difference with 8x8 blocks to improve on input value.
Exit early if the accumulated square error becomes larger	than the specified 
input value. The block dimensions must be 8x8 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 8x8 input block.
@param min	:	The min value to improve on.
@return			: Total square diff to the point of early exit.	
*/
int OverlayMem2Dv2::Tsd8x8LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
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
#endif	// OM2DV2_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd8x8LessThan.

int OverlayMem2Dv2::Tsd8x8PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	int Dp = 0;	/// Accumulated partial sqare error.

	for(int p = 1; p <= 16; p++)
	{
		int pd = 0;
		int meY = me._yPos + OM2DV2_Tp[p];
		int meX = me._xPos + OM2DV2_Sp[p];
		int bY = b._yPos + OM2DV2_Tp[p];
		int bX = b._xPos + OM2DV2_Sp[p];
		for(int i = 0; i < 8; i += 4)
			for(int j = 0; j < 8; j += 4)
			{
				int diff = (int)me._pBlock[meY + i][meX + j] - (int)bPtr[bY + i][bX + j];
				pd += (diff * diff);	///< pth partial sqr err
			}//end for i & j...

		/// Accumulated partial sqr err.
		Dp += pd;

		if( (Dp << 4) > (p * min) )
			return( Dp << 4 );	///< Early exit because exceeded min.
	}//end for p...

	return(Dp);
}//end Tsd8x8PartialLessThan.

/** The total square difference with 16x16 blocks to improve on input value.
Exit early if the accumulated square error becomes larger	than the specified 
input value. The block dimensions must be 16x16 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 16x16 input block.
@param min	:	The min value to improve on.
@return			: Total square error to the point of early exit.	
*/
int OverlayMem2Dv2::Tsd16x16LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
  int			acc	  = 0;
  for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
    pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][1]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][2]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][3]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
    if (acc > min)	return(acc);  // Early exit because exceeded min.
    // [row][4]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][5]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][6]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][7]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		if(acc > min)	return(acc);  // Early exit because exceeded min.
		// [row][8]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][9]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][10]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][11]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
    if (acc > min)	return(acc);  // Early exit because exceeded min.
    // [row][12]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][13]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][14]
    diff = *(pP++) - *(pI++);	acc += (diff * diff);
		// [row][15]
    diff = *(pP) - *(pI);	acc += (diff * diff);
    if (acc > min)	return(acc);

#else
		for(int col = 0; (col < 16)&&(acc <= min); col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += (diff * diff);
		}//end for col...
#endif	// OM2DV2_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tsd16x16LessThan.

/** The total square difference with 16x16 blocks to improve on input value.
Exit early if the partial accumulated square error becomes larger	than the specified 
normalised input value. The block dimensions must be 16x16 and no checking is done. Use
with caution for speed with unrolled inner loop.
@param b		: 16x16 input block.
@param min	:	The min value to improve on.
@return			: Total square error to the point of early exit.	
*/
int OverlayMem2Dv2::Tsd16x16PartialLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	int Dp = 0;	/// Accumulated partial square error.

	for(int p = 1; p <= 16; p++)
	{
		int pd = 0;
		int meY = me._yPos + OM2DV2_Tp[p];
		int meX = me._xPos + OM2DV2_Sp[p];
		int bY = b._yPos + OM2DV2_Tp[p];
		int bX = b._xPos + OM2DV2_Sp[p];
		for(int i = 0; i < 16; i += 4)
			for(int j = 0; j < 16; j += 4)
			{
				int diff = (int)me._pBlock[meY + i][meX + j] - (int)bPtr[bY + i][bX + j];
				pd += (diff * diff);	///< pth partial sqr err
			}//end for i & j...

		/// Accumulated partial sqr err.
		Dp += pd;

		if( (Dp << 4) > (p * min) )
			return( Dp << 4 );	///< Early exit because exceeded min.
	}//end for p...

	return(Dp);
}//end Tsd16x16PartialLessThan.

 /** The total square difference with 16x16 blocks to improve on input value with a partial path.
 Exit early if the partial accumulated square error becomes larger than the specified
 normalised input value along a path of coordinates. The block dimensions must be 16x16 
 and no checking is done. Use with caution for speed.
 @param b		  : 16x16 input block.
 @param path  : Two-Dim path of array type OM2DV2_COORD.
 @param len   : Length of path to check <= 16x16=256.
 @param min	  :	The min value to improve on.
 @return		  : Total square error to the point of early exit.
 */
int OverlayMem2Dv2::Tsd16x16PartialPathLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, void* path, int len, int min)
{
  short**	bPtr = b.Get2DSrcPtr();
  OM2DV2_COORD* pPath = (OM2DV2_COORD *)path;

  int Dp = 0;	///< Accumulated partial square error.

  for (int p = 0; (p < len) && (Dp <= min); ) ///< Early exit if Dp exceeded min.
  {
    int pX = (int)pPath[p].x; int pY = (int)pPath[p].y; p++;
    int diff = (int)me._pBlock[me._yPos + pY][me._xPos + pX] - (int)bPtr[b._yPos + pY][b._xPos + pX];
    Dp += (diff * diff);	///< pth partial sqr err.

    int pX2 = (int)pPath[p].x; int pY2 = (int)pPath[p].y; p++;
    int diff2 = (int)me._pBlock[me._yPos + pY2][me._xPos + pX2] - (int)bPtr[b._yPos + pY2][b._xPos + pX2];
    Dp += (diff2 * diff2);	///< pth partial sqr err.
  }//end for p...
/*
  for (int p = 0; (p < len)&&(Dp <= min); p++) ///< Early exit if Dp exceeded min.
  {
    int pX = (int)pPath[p].x;
    int pY = (int)pPath[p].y;
    int diff = (int)me._pBlock[me._yPos + pY][me._xPos + pX] - (int)bPtr[b._yPos + pY][b._xPos + pX];
    Dp += (diff * diff);	///< pth partial sqr err.
  }//end for p...
*/
  return(Dp);
}//end Tsd16x16PartialPathLessThan.

/** The total square difference with 16x16 blocks to improve on input value with a partial path.
Exit early if the predicted square error becomes larger than the specified normalised input value 
along an optimal path of coordinates. The optimal path is an input and is processed in batches. 
The block dimensions must be 16x16 and no checking is done. Use with caution for speed.
@param b		  : 16x16 input block.
@param path   : Two-Dim path of array type OM2DV2_COORD.
@param min	  :	The min value to improve on.
@return		    : Total predicted square error using the point of early exit.
*/
int OverlayMem2Dv2::Tsd16x16OptimalPathLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, void* path, int min)
{
  OM2DV2_COORD* pPath = (OM2DV2_COORD *)path;
  short**	bPtr        = b.Get2DSrcPtr();
  int meY = me._yPos; int meX = me._xPos;
  int bY  = b._yPos;  int bX  = b._xPos;

//  int upperThresh2p, lowerThresh2p;
  int Dp      = 0;  ///< Accumulated partial square error.
  int predD   = 0;  ///< Linear 'y=x' prediction of path distortion.
  int pathPos = 0;

  /// The path length is implicit and is a max of 256. All decisions are made after a batch of 16 path
  /// locations at a time.
  for (int batch = 0; batch < 5; batch++)   ///< 5 batches of 16 = 80 path locations.
  {
    for (int p = 0; p < 4; p++)  /// Accumulate 1 batch of the next 16 locations in an unrolled loop.
    {
      int pX1 = (int)pPath[pathPos].x; int pY1 = (int)pPath[pathPos].y; pathPos++;  ///< [0]
      int diff1 = (int)me._pBlock[meY + pY1][meX + pX1] - (int)bPtr[bY + pY1][bX + pX1]; Dp += (diff1 * diff1);	///< partial sqr err.

      int pX2 = (int)pPath[pathPos].x; int pY2 = (int)pPath[pathPos].y; pathPos++;  ///< [1]
      int diff2 = (int)me._pBlock[meY + pY2][meX + pX2] - (int)bPtr[bY + pY2][bX + pX2]; Dp += (diff2 * diff2);

      int pX3 = (int)pPath[pathPos].x; int pY3 = (int)pPath[pathPos].y; pathPos++;  ///< [2]
      int diff3 = (int)me._pBlock[meY + pY3][meX + pX3] - (int)bPtr[bY + pY3][bX + pX3]; Dp += (diff3 * diff3);

      int pX4 = (int)pPath[pathPos].x; int pY4 = (int)pPath[pathPos].y; pathPos++;  ///< [3]
      int diff4 = (int)me._pBlock[meY + pY4][meX + pX4] - (int)bPtr[bY + pY4][bX + pX4]; Dp += (diff4 * diff4);
    }//end for p...

    /// Predict the final result for the full 256 path locations.
    predD = (Dp << 8) / pathPos;
    if (predD > min) 
      break;
  }//end for batch...

  return(predD);
}//end Tsd16x16OptimalPathLessThan...

/** The total square difference with 16x16 blocks to improve on input value with a partial path.
Exit early if the partial accumulated square error becomes larger than the specified
normalised input value along an optimal path of coordinates. The path is hard coded and 
unrolled into 256 comparisons. The block dimensions must be 16x16 and no checking is 
done. Use with caution for speed.
@param b		  : 16x16 input block.
@param min	  :	The min value to improve on.
@return		    : Total predicted square error using the point of early exit.
*/
int OverlayMem2Dv2::Tsd16x16OptimalPathLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
  short**	bPtr = b.Get2DSrcPtr();
  int meY = me._yPos; int meX = me._xPos;
  int bY  = b._yPos;  int bX  = b._xPos;

  int diff1, diff2;
  int Dp    = 0;  ///< Accumulated partial square error.
  int predD = 0;  ///< Linear 'y=x' prediction of path distortion.
//  int upperThresh2p, lowerThresh2p;

  /// Process 16 path locations at a time before an early exit check. Also early exit on less than 2% change in prediction.
  /// 0
  diff1 = (int)me._pBlock[meY + 10][meX +  5] - (int)bPtr[bY + 10][bX +  5]; Dp += (diff1 * diff1); ///< {  5,10 }
  diff2 = (int)me._pBlock[meY +  5][meX + 13] - (int)bPtr[bY +  5][bX + 13]; Dp += (diff2 * diff2); ///< { 13, 5 }
  diff1 = (int)me._pBlock[meY +  1][meX +  2] - (int)bPtr[bY +  1][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 1 }
  diff2 = (int)me._pBlock[meY + 14][meX + 13] - (int)bPtr[bY + 14][bX + 13]; Dp += (diff2 * diff2); ///< { 13,14 }
  diff1 = (int)me._pBlock[meY + 14][meX +  1] - (int)bPtr[bY + 14][bX +  1]; Dp += (diff1 * diff1); ///< {  1,14 }
  diff2 = (int)me._pBlock[meY +  1][meX +  9] - (int)bPtr[bY +  1][bX +  9]; Dp += (diff2 * diff2); ///< {  9, 1 }
  diff1 = (int)me._pBlock[meY +  6][meX +  1] - (int)bPtr[bY +  6][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 6 }
  diff2 = (int)me._pBlock[meY +  9][meX + 15] - (int)bPtr[bY +  9][bX + 15]; Dp += (diff2 * diff2); ///< { 15, 9 }
  /// 8
  diff1 = (int)me._pBlock[meY + 13][meX +  8] - (int)bPtr[bY + 13][bX +  8]; Dp += (diff1 * diff1); ///< {  8,13 }
  diff2 = (int)me._pBlock[meY +  2][meX + 14] - (int)bPtr[bY +  2][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 2 }
  diff1 = (int)me._pBlock[meY +  4][meX +  7] - (int)bPtr[bY +  4][bX +  7]; Dp += (diff1 * diff1); ///< {  7, 4 }
  diff2 = (int)me._pBlock[meY +  8][meX + 10] - (int)bPtr[bY +  8][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 8 }
  diff1 = (int)me._pBlock[meY + 15][meX +  4] - (int)bPtr[bY + 15][bX +  4]; Dp += (diff1 * diff1); ///< {  4,15 }
  diff2 = (int)me._pBlock[meY + 10][meX +  0] - (int)bPtr[bY + 10][bX +  0]; Dp += (diff2 * diff2); ///< {  0,10 }
  diff1 = (int)me._pBlock[meY +  3][meX +  0] - (int)bPtr[bY +  3][bX +  0]; Dp += (diff1 * diff1); ///< {  0, 3 }
  diff2 = (int)me._pBlock[meY +  0][meX +  5] - (int)bPtr[bY +  0][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 0 }
//  predD = Dp * 16;  if (predD > min) return(predD);
  /// 16
  diff1 = (int)me._pBlock[meY + 11][meX + 12] - (int)bPtr[bY + 11][bX + 12]; Dp += (diff1 * diff1); ///< { 12,11 }
  diff2 = (int)me._pBlock[meY +  7][meX +  4] - (int)bPtr[bY +  7][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 7 }
  diff1 = (int)me._pBlock[meY +  0][meX + 12] - (int)bPtr[bY +  0][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 0 }
  diff2 = (int)me._pBlock[meY + 14][meX + 10] - (int)bPtr[bY + 14][bX + 10]; Dp += (diff2 * diff2); ///< { 10,14 }
  diff1 = (int)me._pBlock[meY + 12][meX +  3] - (int)bPtr[bY + 12][bX +  3]; Dp += (diff1 * diff1); ///< {  3,12 }
  diff2 = (int)me._pBlock[meY +  5][meX + 10] - (int)bPtr[bY +  5][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 5 }
  diff1 = (int)me._pBlock[meY + 13][meX + 15] - (int)bPtr[bY + 13][bX + 15]; Dp += (diff1 * diff1); ///< { 15,13 }
  diff2 = (int)me._pBlock[meY +  3][meX +  4] - (int)bPtr[bY +  3][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 3 }
//  predD = Dp * 256/24;  if (predD > min) return(predD);
  /// 24
  diff1 = (int)me._pBlock[meY +  8][meX +  7] - (int)bPtr[bY +  8][bX +  7]; Dp += (diff1 * diff1); ///< {  7, 8 }
  diff2 = (int)me._pBlock[meY +  7][meX + 14] - (int)bPtr[bY +  7][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 7 }
  diff1 = (int)me._pBlock[meY + 13][meX +  6] - (int)bPtr[bY + 13][bX +  6]; Dp += (diff1 * diff1); ///< {  6,13 }
  diff2 = (int)me._pBlock[meY +  3][meX + 11] - (int)bPtr[bY +  3][bX + 11]; Dp += (diff2 * diff2); ///< { 11, 3 }
  diff1 = (int)me._pBlock[meY +  9][meX +  2] - (int)bPtr[bY +  9][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 9 }
  diff2 = (int)me._pBlock[meY + 10][meX +  9] - (int)bPtr[bY + 10][bX +  9]; Dp += (diff2 * diff2); ///< {  9,10 }
  diff1 = (int)me._pBlock[meY +  1][meX + 15] - (int)bPtr[bY +  1][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 1 }
  diff2 = (int)me._pBlock[meY +  2][meX +  7] - (int)bPtr[bY +  2][bX +  7]; Dp += (diff2 * diff2); ///< {  7, 2 }
  predD = Dp * 8;  if (predD > min) return(predD);
//  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2%
  /// 32
  diff1 = (int)me._pBlock[meY +  5][meX +  3] - (int)bPtr[bY +  5][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 5 }
  diff2 = (int)me._pBlock[meY + 15][meX +  8] - (int)bPtr[bY + 15][bX +  8]; Dp += (diff2 * diff2); ///< {  8,15 }
  diff1 = (int)me._pBlock[meY + 12][meX +  1] - (int)bPtr[bY + 12][bX +  1]; Dp += (diff1 * diff1); ///< {  1,12 }
  diff2 = (int)me._pBlock[meY +  6][meX +  8] - (int)bPtr[bY +  6][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 6 }
  diff1 = (int)me._pBlock[meY +  1][meX +  0] - (int)bPtr[bY +  1][bX +  0]; Dp += (diff1 * diff1); ///< {  0, 1 }
  diff2 = (int)me._pBlock[meY +  9][meX + 12] - (int)bPtr[bY +  9][bX + 12]; Dp += (diff2 * diff2); ///< { 12, 9 }
  diff1 = (int)me._pBlock[meY + 15][meX + 14] - (int)bPtr[bY + 15][bX + 14]; Dp += (diff1 * diff1); ///< { 14,15 }
  diff2 = (int)me._pBlock[meY +  4][meX + 15] - (int)bPtr[bY +  4][bX + 15]; Dp += (diff2 * diff2); ///< { 15, 4 }
  predD = Dp * 256/40;  if (predD > min) return(predD);
  /// 40
  diff1 = (int)me._pBlock[meY + 12][meX + 11] - (int)bPtr[bY + 12][bX + 11]; Dp += (diff1 * diff1); ///< { 11,12 }
  diff2 = (int)me._pBlock[meY +  6][meX +  6] - (int)bPtr[bY +  6][bX +  6]; Dp += (diff2 * diff2); ///< {  6, 6 }
  diff1 = (int)me._pBlock[meY + 11][meX +  7] - (int)bPtr[bY + 11][bX +  7]; Dp += (diff1 * diff1); ///< {  7,11 }
  diff2 = (int)me._pBlock[meY +  7][meX +  0] - (int)bPtr[bY +  7][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 7 }
  diff1 = (int)me._pBlock[meY + 11][meX + 14] - (int)bPtr[bY + 11][bX + 14]; Dp += (diff1 * diff1); ///< { 14,11 }
  diff2 = (int)me._pBlock[meY +  0][meX +  7] - (int)bPtr[bY +  0][bX +  7]; Dp += (diff2 * diff2); ///< {  7, 0 }
  diff1 = (int)me._pBlock[meY + 15][meX +  2] - (int)bPtr[bY + 15][bX +  2]; Dp += (diff1 * diff1); ///< {  2,15 }
  diff2 = (int)me._pBlock[meY +  7][meX + 12] - (int)bPtr[bY +  7][bX + 12]; Dp += (diff2 * diff2); ///< { 12, 7 }
//  predD = Dp * 256/48;  if ((predD > min)||( (predD < upperThresh2p)&&(predD > lowerThresh2p) )) return(predD);
//  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2% change
  predD = Dp * 256 / 48;  if (predD > min) return(predD);
  /// 48
  diff1 = (int)me._pBlock[meY +  3][meX +  2] - (int)bPtr[bY +  3][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 3 }
  diff2 = (int)me._pBlock[meY +  9][meX +  4] - (int)bPtr[bY +  9][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 9 }
  diff1 = (int)me._pBlock[meY +  2][meX + 12] - (int)bPtr[bY +  2][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 2 }
  diff2 = (int)me._pBlock[meY +  3][meX +  9] - (int)bPtr[bY +  3][bX +  9]; Dp += (diff2 * diff2); ///< {  9, 3 }
  diff1 = (int)me._pBlock[meY + 13][meX + 12] - (int)bPtr[bY + 13][bX + 12]; Dp += (diff1 * diff1); ///< { 12,13 }
  diff2 = (int)me._pBlock[meY +  0][meX +  3] - (int)bPtr[bY +  0][bX +  3]; Dp += (diff2 * diff2); ///< {  3, 0 }
  diff1 = (int)me._pBlock[meY + 14][meX +  5] - (int)bPtr[bY + 14][bX +  5]; Dp += (diff1 * diff1); ///< {  5,14 }
  diff2 = (int)me._pBlock[meY +  4][meX +  5] - (int)bPtr[bY +  4][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 4 }
  predD = Dp * 256/56;  if (predD > min) return(predD);
  /// 56
  diff1 = (int)me._pBlock[meY + 13][meX +  0] - (int)bPtr[bY + 13][bX +  0]; Dp += (diff1 * diff1); ///< {  0,13 }
  diff2 = (int)me._pBlock[meY +  9][meX +  8] - (int)bPtr[bY +  9][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 9 }
  diff1 = (int)me._pBlock[meY + 11][meX +  2] - (int)bPtr[bY + 11][bX +  2]; Dp += (diff1 * diff1); ///< {  2,11 }
  diff2 = (int)me._pBlock[meY + 15][meX + 11] - (int)bPtr[bY + 15][bX + 11]; Dp += (diff2 * diff2); ///< { 11,15 }
  diff1 = (int)me._pBlock[meY +  1][meX + 10] - (int)bPtr[bY +  1][bX + 10]; Dp += (diff1 * diff1); ///< { 10, 1 }
  diff2 = (int)me._pBlock[meY +  7][meX +  2] - (int)bPtr[bY +  7][bX +  2]; Dp += (diff2 * diff2); ///< {  2, 7 }
  diff1 = (int)me._pBlock[meY +  6][meX + 15] - (int)bPtr[bY +  6][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 6 }
  diff2 = (int)me._pBlock[meY + 12][meX +  9] - (int)bPtr[bY + 12][bX +  9]; Dp += (diff2 * diff2); ///< {  9,12 }
//  predD = Dp * 4;  if ((predD > min) || ((predD < upperThresh2p) && (predD > lowerThresh2p))) return(predD);
//  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2% change
  predD = Dp * 4;  if (predD > min) return(predD);
  /// 64
  diff1 = (int)me._pBlock[meY +  2][meX +  5] - (int)bPtr[bY +  2][bX +  5]; Dp += (diff1 * diff1); ///< {  5, 2 }
  diff2 = (int)me._pBlock[meY + 12][meX +  5] - (int)bPtr[bY + 12][bX +  5]; Dp += (diff2 * diff2); ///< {  5,12 }
  diff1 = (int)me._pBlock[meY +  4][meX +  1] - (int)bPtr[bY +  4][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 4 }
  diff2 = (int)me._pBlock[meY +  5][meX + 11] - (int)bPtr[bY +  5][bX + 11]; Dp += (diff2 * diff2); ///< { 11, 5 }
  diff1 = (int)me._pBlock[meY +  8][meX + 13] - (int)bPtr[bY +  8][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 8 }
  diff2 = (int)me._pBlock[meY +  7][meX +  9] - (int)bPtr[bY +  7][bX +  9]; Dp += (diff2 * diff2); ///< {  9, 7 }
  diff1 = (int)me._pBlock[meY +  3][meX + 13] - (int)bPtr[bY +  3][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 3 }
  diff2 = (int)me._pBlock[meY +  8][meX +  5] - (int)bPtr[bY +  8][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 8 }
  predD = Dp * 256/72;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY + 11][meX + 10] - (int)bPtr[bY + 11][bX + 10]; Dp += (diff1 * diff1); ///< { 10,11 }
  diff2 = (int)me._pBlock[meY + 15][meX +  6] - (int)bPtr[bY + 15][bX +  6]; Dp += (diff2 * diff2); ///< {  6,15 }
  diff1 = (int)me._pBlock[meY + 10][meX + 13] - (int)bPtr[bY + 10][bX + 13]; Dp += (diff1 * diff1); ///< { 13,10 }
  diff2 = (int)me._pBlock[meY +  4][meX +  8] - (int)bPtr[bY +  4][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 4 }
  diff1 = (int)me._pBlock[meY + 10][meX +  3] - (int)bPtr[bY + 10][bX +  3]; Dp += (diff1 * diff1); ///< {  3,10 }
  diff2 = (int)me._pBlock[meY +  1][meX + 13] - (int)bPtr[bY +  1][bX + 13]; Dp += (diff2 * diff2); ///< { 13, 1 }
  diff1 = (int)me._pBlock[meY +  8][meX +  1] - (int)bPtr[bY +  8][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 8 }
  diff2 = (int)me._pBlock[meY +  0][meX + 14] - (int)bPtr[bY +  0][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 0 }
  predD = Dp * 256/80;  //if (predD > min) return(predD);
/*
  /// 80
  diff1 = (int)me._pBlock[meY +  2][meX +  3] - (int)bPtr[bY +  2][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 2 }
  diff2 = (int)me._pBlock[meY + 12][meX + 14] - (int)bPtr[bY + 12][bX + 14]; Dp += (diff2 * diff2); ///< { 14,12 }
  diff1 = (int)me._pBlock[meY +  5][meX +  6] - (int)bPtr[bY +  5][bX +  6]; Dp += (diff1 * diff1); ///< {  6, 5 }
  diff2 = (int)me._pBlock[meY + 13][meX +  3] - (int)bPtr[bY + 13][bX +  3]; Dp += (diff2 * diff2); ///< {  3,13 }
  diff1 = (int)me._pBlock[meY + 15][meX +  0] - (int)bPtr[bY + 15][bX +  0]; Dp += (diff1 * diff1); ///< {  0,15 }
  diff2 = (int)me._pBlock[meY + 10][meX +  7] - (int)bPtr[bY + 10][bX +  7]; Dp += (diff2 * diff2); ///< {  7,10 }
  diff1 = (int)me._pBlock[meY +  6][meX + 11] - (int)bPtr[bY +  6][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 6 }
  diff2 = (int)me._pBlock[meY +  1][meX +  6] - (int)bPtr[bY +  1][bX +  6]; Dp += (diff2 * diff2); ///< {  6, 1 }
  predD = Dp * 256/88;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  6][meX +  4] - (int)bPtr[bY +  6][bX +  4]; Dp += (diff1 * diff1); ///< {  4, 6 }
  diff2 = (int)me._pBlock[meY +  1][meX +  1] - (int)bPtr[bY +  1][bX +  1]; Dp += (diff2 * diff2); ///< {  1, 1 }
  diff1 = (int)me._pBlock[meY + 14][meX + 15] - (int)bPtr[bY + 14][bX + 15]; Dp += (diff1 * diff1); ///< { 15,14 }
  diff2 = (int)me._pBlock[meY +  9][meX + 11] - (int)bPtr[bY +  9][bX + 11]; Dp += (diff2 * diff2); ///< { 11, 9 }
  diff1 = (int)me._pBlock[meY + 14][meX +  7] - (int)bPtr[bY + 14][bX +  7]; Dp += (diff1 * diff1); ///< {  7,14 }
  diff2 = (int)me._pBlock[meY +  2][meX +  8] - (int)bPtr[bY +  2][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 2 }
  diff1 = (int)me._pBlock[meY +  4][meX + 14] - (int)bPtr[bY +  4][bX + 14]; Dp += (diff1 * diff1); ///< { 14, 4 }
  diff2 = (int)me._pBlock[meY + 11][meX +  4] - (int)bPtr[bY + 11][bX +  4]; Dp += (diff2 * diff2); ///< {  4,11 }
  predD = Dp * 256/96;  if (predD > min) return(predD);
  /// 96
  diff1 = (int)me._pBlock[meY +  5][meX +  1] - (int)bPtr[bY +  5][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 5 }
  diff2 = (int)me._pBlock[meY + 13][meX + 10] - (int)bPtr[bY + 13][bX + 10]; Dp += (diff2 * diff2); ///< { 10,13 }
  diff1 = (int)me._pBlock[meY +  7][meX +  6] - (int)bPtr[bY +  7][bX +  6]; Dp += (diff1 * diff1); ///< {  6, 7 }
  diff2 = (int)me._pBlock[meY + 10][meX + 15] - (int)bPtr[bY + 10][bX + 15]; Dp += (diff2 * diff2); ///< { 15,10 }
  diff1 = (int)me._pBlock[meY +  1][meX + 11] - (int)bPtr[bY +  1][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 1 }
  diff2 = (int)me._pBlock[meY +  9][meX +  0] - (int)bPtr[bY +  9][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 9 }
  diff1 = (int)me._pBlock[meY +  5][meX +  9] - (int)bPtr[bY +  5][bX +  9]; Dp += (diff1 * diff1); ///< {  9, 5 }
  diff2 = (int)me._pBlock[meY +  1][meX +  4] - (int)bPtr[bY +  1][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 1 }
  predD = Dp * 256/104;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  6][meX + 13] - (int)bPtr[bY +  6][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 6 }
  diff2 = (int)me._pBlock[meY + 15][meX + 12] - (int)bPtr[bY + 15][bX + 12]; Dp += (diff2 * diff2); ///< { 12,15 }
  diff1 = (int)me._pBlock[meY +  4][meX +  3] - (int)bPtr[bY +  4][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 4 }
  diff2 = (int)me._pBlock[meY + 11][meX +  6] - (int)bPtr[bY + 11][bX +  6]; Dp += (diff2 * diff2); ///< {  6,11 }
  diff1 = (int)me._pBlock[meY + 14][meX +  3] - (int)bPtr[bY + 14][bX +  3]; Dp += (diff1 * diff1); ///< {  3,14 }
  diff2 = (int)me._pBlock[meY +  3][meX + 10] - (int)bPtr[bY +  3][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 3 }
  diff1 = (int)me._pBlock[meY +  9][meX +  9] - (int)bPtr[bY +  9][bX +  9]; Dp += (diff1 * diff1); ///< {  9, 9 }
  diff2 = (int)me._pBlock[meY + 14][meX +  9] - (int)bPtr[bY + 14][bX +  9]; Dp += (diff2 * diff2); ///< {  9,14 }
  predD = Dp * 256/112;  if (predD > min) return(predD);
  /// 112
  diff1 = (int)me._pBlock[meY +  3][meX +  6] - (int)bPtr[bY +  3][bX +  6]; Dp += (diff1 * diff1); ///< {  6, 3 }
  diff2 = (int)me._pBlock[meY +  4][meX + 12] - (int)bPtr[bY +  4][bX + 12]; Dp += (diff2 * diff2); ///< { 12, 4 }
  diff1 = (int)me._pBlock[meY +  5][meX +  0] - (int)bPtr[bY +  5][bX +  0]; Dp += (diff1 * diff1); ///< {  0, 5 }
  diff2 = (int)me._pBlock[meY +  8][meX + 14] - (int)bPtr[bY +  8][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 8 }
  diff1 = (int)me._pBlock[meY + 10][meX +  1] - (int)bPtr[bY + 10][bX +  1]; Dp += (diff1 * diff1); ///< {  1,10 }
  diff2 = (int)me._pBlock[meY +  2][meX +  1] - (int)bPtr[bY +  2][bX +  1]; Dp += (diff2 * diff2); ///< {  1, 2 }
  diff1 = (int)me._pBlock[meY +  8][meX +  2] - (int)bPtr[bY +  8][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 8 }
  diff2 = (int)me._pBlock[meY + 12][meX +  2] - (int)bPtr[bY + 12][bX +  2]; Dp += (diff2 * diff2); ///< {  2,12 }
  predD = Dp * 256/120;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  6][meX +  2] - (int)bPtr[bY +  6][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 6 }
  diff2 = (int)me._pBlock[meY + 12][meX +  0] - (int)bPtr[bY + 12][bX +  0]; Dp += (diff2 * diff2); ///< {  0,12 }
  diff1 = (int)me._pBlock[meY +  7][meX +  3] - (int)bPtr[bY +  7][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 7 }
  diff2 = (int)me._pBlock[meY + 11][meX +  8] - (int)bPtr[bY + 11][bX +  8]; Dp += (diff2 * diff2); ///< {  8,11 }
  diff1 = (int)me._pBlock[meY +  8][meX +  3] - (int)bPtr[bY +  8][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 8 }
  diff2 = (int)me._pBlock[meY +  9][meX +  6] - (int)bPtr[bY +  9][bX +  6]; Dp += (diff2 * diff2); ///< {  6, 9 }
  diff1 = (int)me._pBlock[meY +  0][meX +  9] - (int)bPtr[bY +  0][bX +  9]; Dp += (diff1 * diff1); ///< {  9, 0 }
  diff2 = (int)me._pBlock[meY +  0][meX +  1] - (int)bPtr[bY +  0][bX +  1]; Dp += (diff2 * diff2); ///< {  1, 0 }
  predD = Dp * 2;  if (predD > min) return(predD);
  /// 128
  diff1 = (int)me._pBlock[meY +  3][meX + 15] - (int)bPtr[bY +  3][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 3 }
  diff2 = (int)me._pBlock[meY + 12][meX + 13] - (int)bPtr[bY + 12][bX + 13]; Dp += (diff2 * diff2); ///< { 13,12 }
  diff1 = (int)me._pBlock[meY +  8][meX + 11] - (int)bPtr[bY +  8][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 8 }
  diff2 = (int)me._pBlock[meY +  7][meX +  8] - (int)bPtr[bY +  7][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 7 }
  diff1 = (int)me._pBlock[meY +  5][meX +  5] - (int)bPtr[bY +  5][bX +  5]; Dp += (diff1 * diff1); ///< {  5, 5 }
  diff2 = (int)me._pBlock[meY + 13][meX +  5] - (int)bPtr[bY + 13][bX +  5]; Dp += (diff2 * diff2); ///< {  5,13 }
  diff1 = (int)me._pBlock[meY +  0][meX + 11] - (int)bPtr[bY +  0][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 0 }
  diff2 = (int)me._pBlock[meY + 10][meX + 10] - (int)bPtr[bY + 10][bX + 10]; Dp += (diff2 * diff2); ///< { 10,10 }
  predD = Dp * 256/136;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  7][meX + 15] - (int)bPtr[bY +  7][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 7 }
  diff2 = (int)me._pBlock[meY + 14][meX + 11] - (int)bPtr[bY + 14][bX + 11]; Dp += (diff2 * diff2); ///< { 11,14 }
  diff1 = (int)me._pBlock[meY + 12][meX +  7] - (int)bPtr[bY + 12][bX +  7]; Dp += (diff1 * diff1); ///< {  7,12 }
  diff2 = (int)me._pBlock[meY +  5][meX +  7] - (int)bPtr[bY +  5][bX +  7]; Dp += (diff2 * diff2); ///< {  7, 5 }
  diff1 = (int)me._pBlock[meY + 13][meX + 14] - (int)bPtr[bY + 13][bX + 14]; Dp += (diff1 * diff1); ///< { 14,13 }
  diff2 = (int)me._pBlock[meY +  6][meX + 10] - (int)bPtr[bY +  6][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 6 }
  diff1 = (int)me._pBlock[meY +  0][meX + 15] - (int)bPtr[bY +  0][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 0 }
  diff2 = (int)me._pBlock[meY + 15][meX +  9] - (int)bPtr[bY + 15][bX +  9]; Dp += (diff2 * diff2); ///< {  9,15 }
  predD = Dp * 256/144;  if (predD > min) return(predD);
  /// 144
  diff1 = (int)me._pBlock[meY +  1][meX +  7] - (int)bPtr[bY +  1][bX +  7]; Dp += (diff1 * diff1); ///< {  7, 1 }
  diff2 = (int)me._pBlock[meY + 10][meX + 12] - (int)bPtr[bY + 10][bX + 12]; Dp += (diff2 * diff2); ///< { 12,10 }
  diff1 = (int)me._pBlock[meY + 14][meX +  2] - (int)bPtr[bY + 14][bX +  2]; Dp += (diff1 * diff1); ///< {  2,14 }
  diff2 = (int)me._pBlock[meY +  2][meX +  4] - (int)bPtr[bY +  2][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 2 }
  diff1 = (int)me._pBlock[meY +  5][meX + 12] - (int)bPtr[bY +  5][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 5 }
  diff2 = (int)me._pBlock[meY +  2][meX + 10] - (int)bPtr[bY +  2][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 2 }
  diff1 = (int)me._pBlock[meY + 13][meX +  4] - (int)bPtr[bY + 13][bX +  4]; Dp += (diff1 * diff1); ///< {  4,13 }
  diff2 = (int)me._pBlock[meY +  8][meX +  8] - (int)bPtr[bY +  8][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 8 }
  predD = Dp * 256/152;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY + 15][meX + 15] - (int)bPtr[bY + 15][bX + 15]; Dp += (diff1 * diff1); ///< { 15,15 }
  diff2 = (int)me._pBlock[meY +  5][meX + 14] - (int)bPtr[bY +  5][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 5 }
  diff1 = (int)me._pBlock[meY +  0][meX +  4] - (int)bPtr[bY +  0][bX +  4]; Dp += (diff1 * diff1); ///< {  4, 0 }
  diff2 = (int)me._pBlock[meY +  9][meX + 14] - (int)bPtr[bY +  9][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 9 }
  diff1 = (int)me._pBlock[meY +  2][meX + 13] - (int)bPtr[bY +  2][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 2 }
  diff2 = (int)me._pBlock[meY +  7][meX +  5] - (int)bPtr[bY +  7][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 7 }
  diff1 = (int)me._pBlock[meY +  3][meX +  8] - (int)bPtr[bY +  3][bX +  8]; Dp += (diff1 * diff1); ///< {  8, 3 }
  diff2 = (int)me._pBlock[meY +  0][meX +  0] - (int)bPtr[bY +  0][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 0 }
  predD = Dp * 256/160;  if (predD > min) return(predD);
  /// 160
  diff1 = (int)me._pBlock[meY +  4][meX +  2] - (int)bPtr[bY +  4][bX +  2]; Dp += (diff1 * diff1); ///< {  2, 4 }
  diff2 = (int)me._pBlock[meY + 15][meX +  7] - (int)bPtr[bY + 15][bX +  7]; Dp += (diff2 * diff2); ///< {  7,15 }
  diff1 = (int)me._pBlock[meY + 11][meX + 11] - (int)bPtr[bY + 11][bX + 11]; Dp += (diff1 * diff1); ///< { 11,11 }
  diff2 = (int)me._pBlock[meY + 11][meX +  0] - (int)bPtr[bY + 11][bX +  0]; Dp += (diff2 * diff2); ///< {  0,11 }
  diff1 = (int)me._pBlock[meY + 10][meX +  6] - (int)bPtr[bY + 10][bX +  6]; Dp += (diff1 * diff1); ///< {  6,10 }
  diff2 = (int)me._pBlock[meY +  0][meX +  8] - (int)bPtr[bY +  0][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 0 }
  diff1 = (int)me._pBlock[meY +  5][meX +  4] - (int)bPtr[bY +  5][bX +  4]; Dp += (diff1 * diff1); ///< {  4, 5 }
  diff2 = (int)me._pBlock[meY + 15][meX +  3] - (int)bPtr[bY + 15][bX +  3]; Dp += (diff2 * diff2); ///< {  3,15 }
  predD = Dp * 256/168;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  4][meX + 10] - (int)bPtr[bY +  4][bX + 10]; Dp += (diff1 * diff1); ///< { 10, 4 }
  diff2 = (int)me._pBlock[meY + 11][meX + 15] - (int)bPtr[bY + 11][bX + 15]; Dp += (diff2 * diff2); ///< { 15,11 }
  diff1 = (int)me._pBlock[meY + 12][meX +  8] - (int)bPtr[bY + 12][bX +  8]; Dp += (diff1 * diff1); ///< {  8,12 }
  diff2 = (int)me._pBlock[meY + 13][meX + 13] - (int)bPtr[bY + 13][bX + 13]; Dp += (diff2 * diff2); ///< { 13,13 }
  diff1 = (int)me._pBlock[meY +  4][meX +  0] - (int)bPtr[bY +  4][bX +  0]; Dp += (diff1 * diff1); ///< {  0, 4 }
  diff2 = (int)me._pBlock[meY + 10][meX +  4] - (int)bPtr[bY + 10][bX +  4]; Dp += (diff2 * diff2); ///< {  4,10 }
  diff1 = (int)me._pBlock[meY +  7][meX + 11] - (int)bPtr[bY +  7][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 7 }
  diff2 = (int)me._pBlock[meY +  3][meX +  5] - (int)bPtr[bY +  3][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 3 }
  predD = Dp * 256/176;  if (predD > min) return(predD);
  /// 176
  diff1 = (int)me._pBlock[meY + 13][meX +  1] - (int)bPtr[bY + 13][bX +  1]; Dp += (diff1 * diff1); ///< {  1,13 }
  diff2 = (int)me._pBlock[meY +  7][meX +  7] - (int)bPtr[bY +  7][bX +  7]; Dp += (diff2 * diff2); ///< {  7, 7 }
  diff1 = (int)me._pBlock[meY +  3][meX + 14] - (int)bPtr[bY +  3][bX + 14]; Dp += (diff1 * diff1); ///< { 14, 3 }
  diff2 = (int)me._pBlock[meY +  2][meX +  2] - (int)bPtr[bY +  2][bX +  2]; Dp += (diff2 * diff2); ///< {  2, 2 }
  diff1 = (int)me._pBlock[meY + 12][meX + 10] - (int)bPtr[bY + 12][bX + 10]; Dp += (diff1 * diff1); ///< { 10,12 }
  diff2 = (int)me._pBlock[meY +  8][meX +  0] - (int)bPtr[bY +  8][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 8 }
  diff1 = (int)me._pBlock[meY +  1][meX + 12] - (int)bPtr[bY +  1][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 1 }
  diff2 = (int)me._pBlock[meY + 15][meX +  5] - (int)bPtr[bY + 15][bX +  5]; Dp += (diff2 * diff2); ///< {  5,15 }
  predD = Dp * 256/184;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  7][meX + 13] - (int)bPtr[bY +  7][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 7 }
  diff2 = (int)me._pBlock[meY + 14][meX + 12] - (int)bPtr[bY + 14][bX + 12]; Dp += (diff2 * diff2); ///< { 12,14 }
  diff1 = (int)me._pBlock[meY +  9][meX +  3] - (int)bPtr[bY +  9][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 9 }
  diff2 = (int)me._pBlock[meY +  6][meX +  9] - (int)bPtr[bY +  6][bX +  9]; Dp += (diff2 * diff2); ///< {  9, 6 }
  diff1 = (int)me._pBlock[meY +  2][meX +  6] - (int)bPtr[bY +  2][bX +  6]; Dp += (diff1 * diff1); ///< {  6, 2 }
  diff2 = (int)me._pBlock[meY + 13][meX +  7] - (int)bPtr[bY + 13][bX +  7]; Dp += (diff2 * diff2); ///< {  7,13 }
  diff1 = (int)me._pBlock[meY + 11][meX + 13] - (int)bPtr[bY + 11][bX + 13]; Dp += (diff1 * diff1); ///< { 13,11 }
  diff2 = (int)me._pBlock[meY + 15][meX +  1] - (int)bPtr[bY + 15][bX +  1]; Dp += (diff2 * diff2); ///< {  1,15 }
  predD = Dp * 256/192;  if (predD > min) return(predD);
  /// 192
  diff1 = (int)me._pBlock[meY + 11][meX +  5] - (int)bPtr[bY + 11][bX +  5]; Dp += (diff1 * diff1); ///< {  5,11 }
  diff2 = (int)me._pBlock[meY +  8][meX +  9] - (int)bPtr[bY +  8][bX +  9]; Dp += (diff2 * diff2); ///< {  9, 8 }
  diff1 = (int)me._pBlock[meY +  5][meX + 15] - (int)bPtr[bY +  5][bX + 15]; Dp += (diff1 * diff1); ///< { 15, 5 }
  diff2 = (int)me._pBlock[meY +  0][meX +  6] - (int)bPtr[bY +  0][bX +  6]; Dp += (diff2 * diff2); ///< {  6, 0 }
  diff1 = (int)me._pBlock[meY +  3][meX + 12] - (int)bPtr[bY +  3][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 3 }
  diff2 = (int)me._pBlock[meY +  6][meX +  0] - (int)bPtr[bY +  6][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 6 }
  diff1 = (int)me._pBlock[meY + 10][meX +  2] - (int)bPtr[bY + 10][bX +  2]; Dp += (diff1 * diff1); ///< {  2,10 }
  diff2 = (int)me._pBlock[meY +  0][meX + 13] - (int)bPtr[bY +  0][bX + 13]; Dp += (diff2 * diff2); ///< { 13, 0 }
  predD = Dp * 256/200;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY + 15][meX + 13] - (int)bPtr[bY + 15][bX + 13]; Dp += (diff1 * diff1); ///< { 13,15 }
  diff2 = (int)me._pBlock[meY +  3][meX +  3] - (int)bPtr[bY +  3][bX +  3]; Dp += (diff2 * diff2); ///< {  3, 3 }
  diff1 = (int)me._pBlock[meY +  4][meX +  9] - (int)bPtr[bY +  4][bX +  9]; Dp += (diff1 * diff1); ///< {  9, 4 }
  diff2 = (int)me._pBlock[meY +  8][meX + 15] - (int)bPtr[bY +  8][bX + 15]; Dp += (diff2 * diff2); ///< { 15, 8 }
  diff1 = (int)me._pBlock[meY +  9][meX +  5] - (int)bPtr[bY +  9][bX +  5]; Dp += (diff1 * diff1); ///< {  5, 9 }
  diff2 = (int)me._pBlock[meY +  0][meX +  2] - (int)bPtr[bY +  0][bX +  2]; Dp += (diff2 * diff2); ///< {  2, 0 }
  diff1 = (int)me._pBlock[meY +  9][meX + 10] - (int)bPtr[bY +  9][bX + 10]; Dp += (diff1 * diff1); ///< { 10, 9 }
  diff2 = (int)me._pBlock[meY +  2][meX + 15] - (int)bPtr[bY +  2][bX + 15]; Dp += (diff2 * diff2); ///< { 15, 2 }
  predD = Dp * 256/208;  if (predD > min) return(predD);
  /// 208
  diff1 = (int)me._pBlock[meY + 12][meX + 15] - (int)bPtr[bY + 12][bX + 15]; Dp += (diff1 * diff1); ///< { 15,12 }
  diff2 = (int)me._pBlock[meY + 14][meX +  8] - (int)bPtr[bY + 14][bX +  8]; Dp += (diff2 * diff2); ///< {  8,14 }
  diff1 = (int)me._pBlock[meY +  6][meX +  7] - (int)bPtr[bY +  6][bX +  7]; Dp += (diff1 * diff1); ///< {  7, 6 }
  diff2 = (int)me._pBlock[meY + 14][meX +  0] - (int)bPtr[bY + 14][bX +  0]; Dp += (diff2 * diff2); ///< {  0,14 }
  diff1 = (int)me._pBlock[meY + 14][meX + 14] - (int)bPtr[bY + 14][bX + 14]; Dp += (diff1 * diff1); ///< { 14,14 }
  diff2 = (int)me._pBlock[meY +  2][meX +  0] - (int)bPtr[bY +  2][bX +  0]; Dp += (diff2 * diff2); ///< {  0, 2 }
  diff1 = (int)me._pBlock[meY +  0][meX + 10] - (int)bPtr[bY +  0][bX + 10]; Dp += (diff1 * diff1); ///< { 10, 0 }
  diff2 = (int)me._pBlock[meY + 12][meX +  4] - (int)bPtr[bY + 12][bX +  4]; Dp += (diff2 * diff2); ///< {  4,12 }
  predD = Dp * 256/216;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY +  6][meX +  3] - (int)bPtr[bY +  6][bX +  3]; Dp += (diff1 * diff1); ///< {  3, 6 }
  diff2 = (int)me._pBlock[meY +  8][meX + 12] - (int)bPtr[bY +  8][bX + 12]; Dp += (diff2 * diff2); ///< { 12, 8 }
  diff1 = (int)me._pBlock[meY +  4][meX +  6] - (int)bPtr[bY +  4][bX +  6]; Dp += (diff1 * diff1); ///< {  6, 4 }
  diff2 = (int)me._pBlock[meY + 15][meX + 10] - (int)bPtr[bY + 15][bX + 10]; Dp += (diff2 * diff2); ///< { 10,15 }
  diff1 = (int)me._pBlock[meY + 11][meX +  9] - (int)bPtr[bY + 11][bX +  9]; Dp += (diff1 * diff1); ///< {  9,11 }
  diff2 = (int)me._pBlock[meY +  1][meX +  8] - (int)bPtr[bY +  1][bX +  8]; Dp += (diff2 * diff2); ///< {  8, 1 }
  diff1 = (int)me._pBlock[meY + 11][meX +  1] - (int)bPtr[bY + 11][bX +  1]; Dp += (diff1 * diff1); ///< {  1,11 }
  diff2 = (int)me._pBlock[meY +  8][meX +  6] - (int)bPtr[bY +  8][bX +  6]; Dp += (diff2 * diff2); ///< {  6, 8 }
  predD = Dp * 256/224;  if (predD > min) return(predD);
  /// 224
  diff1 = (int)me._pBlock[meY +  4][meX + 11] - (int)bPtr[bY +  4][bX + 11]; Dp += (diff1 * diff1); ///< { 11, 4 }
  diff2 = (int)me._pBlock[meY + 13][meX + 11] - (int)bPtr[bY + 13][bX + 11]; Dp += (diff2 * diff2); ///< { 11,13 }
  diff1 = (int)me._pBlock[meY + 14][meX +  4] - (int)bPtr[bY + 14][bX +  4]; Dp += (diff1 * diff1); ///< {  4,14 }
  diff2 = (int)me._pBlock[meY +  7][meX +  1] - (int)bPtr[bY +  7][bX +  1]; Dp += (diff2 * diff2); ///< {  1, 7 }
  diff1 = (int)me._pBlock[meY + 10][meX + 14] - (int)bPtr[bY + 10][bX + 14]; Dp += (diff1 * diff1); ///< { 14,10 }
  diff2 = (int)me._pBlock[meY +  1][meX +  3] - (int)bPtr[bY +  1][bX +  3]; Dp += (diff2 * diff2); ///< {  3, 1 }
  diff1 = (int)me._pBlock[meY + 10][meX +  8] - (int)bPtr[bY + 10][bX +  8]; Dp += (diff1 * diff1); ///< {  8,10 }
  diff2 = (int)me._pBlock[meY +  6][meX + 14] - (int)bPtr[bY +  6][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 6 }
  predD = Dp * 256/232;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY + 12][meX +  6] - (int)bPtr[bY + 12][bX +  6]; Dp += (diff1 * diff1); ///< {  6,12 }
  diff2 = (int)me._pBlock[meY +  2][meX + 11] - (int)bPtr[bY +  2][bX + 11]; Dp += (diff2 * diff2); ///< { 11, 2 }
  diff1 = (int)me._pBlock[meY +  3][meX +  1] - (int)bPtr[bY +  3][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 3 }
  diff2 = (int)me._pBlock[meY +  1][meX + 14] - (int)bPtr[bY +  1][bX + 14]; Dp += (diff2 * diff2); ///< { 14, 1 }
  diff1 = (int)me._pBlock[meY +  6][meX +  5] - (int)bPtr[bY +  6][bX +  5]; Dp += (diff1 * diff1); ///< {  5, 6 }
  diff2 = (int)me._pBlock[meY +  3][meX +  7] - (int)bPtr[bY +  3][bX +  7]; Dp += (diff2 * diff2); ///< {  7, 3 }
  diff1 = (int)me._pBlock[meY + 13][meX +  2] - (int)bPtr[bY + 13][bX +  2]; Dp += (diff1 * diff1); ///< {  2,13 }
  diff2 = (int)me._pBlock[meY +  9][meX + 13] - (int)bPtr[bY +  9][bX + 13]; Dp += (diff2 * diff2); ///< { 13, 9 }
  predD = Dp * 256/240;  if (predD > min) return(predD);
  /// 240
  diff1 = (int)me._pBlock[meY +  9][meX +  1] - (int)bPtr[bY +  9][bX +  1]; Dp += (diff1 * diff1); ///< {  1, 9 }
  diff2 = (int)me._pBlock[meY +  4][meX +  4] - (int)bPtr[bY +  4][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 4 }
  diff1 = (int)me._pBlock[meY +  2][meX +  9] - (int)bPtr[bY +  2][bX +  9]; Dp += (diff1 * diff1); ///< {  9, 2 }
  diff2 = (int)me._pBlock[meY + 12][meX + 12] - (int)bPtr[bY + 12][bX + 12]; Dp += (diff2 * diff2); ///< { 12,12 }
  diff1 = (int)me._pBlock[meY + 14][meX +  6] - (int)bPtr[bY + 14][bX +  6]; Dp += (diff1 * diff1); ///< {  6,14 }
  diff2 = (int)me._pBlock[meY +  1][meX +  5] - (int)bPtr[bY +  1][bX +  5]; Dp += (diff2 * diff2); ///< {  5, 1 }
  diff1 = (int)me._pBlock[meY +  4][meX + 13] - (int)bPtr[bY +  4][bX + 13]; Dp += (diff1 * diff1); ///< { 13, 4 }
  diff2 = (int)me._pBlock[meY +  5][meX +  2] - (int)bPtr[bY +  5][bX +  2]; Dp += (diff2 * diff2); ///< {  2, 5 }
  predD = Dp * 256/248;  if (predD > min) return(predD);
  diff1 = (int)me._pBlock[meY + 11][meX +  3] - (int)bPtr[bY + 11][bX +  3]; Dp += (diff1 * diff1); ///< {  3,11 }
  diff2 = (int)me._pBlock[meY + 13][meX +  9] - (int)bPtr[bY + 13][bX +  9]; Dp += (diff2 * diff2); ///< {  9,13 }
  diff1 = (int)me._pBlock[meY +  6][meX + 12] - (int)bPtr[bY +  6][bX + 12]; Dp += (diff1 * diff1); ///< { 12, 6 }
  diff2 = (int)me._pBlock[meY +  8][meX +  4] - (int)bPtr[bY +  8][bX +  4]; Dp += (diff2 * diff2); ///< {  4, 8 }
  diff1 = (int)me._pBlock[meY +  5][meX +  8] - (int)bPtr[bY +  5][bX +  8]; Dp += (diff1 * diff1); ///< {  8, 5 }
  diff2 = (int)me._pBlock[meY + 10][meX + 11] - (int)bPtr[bY + 10][bX + 11]; Dp += (diff2 * diff2); ///< { 11,10 }
  diff1 = (int)me._pBlock[meY +  9][meX +  7] - (int)bPtr[bY +  9][bX +  7]; Dp += (diff1 * diff1); ///< {  7, 9 }
  diff2 = (int)me._pBlock[meY +  7][meX + 10] - (int)bPtr[bY + 7][bX + 10]; Dp += (diff2 * diff2); ///< { 10, 7 }
  predD = Dp
*/
  return(predD);

}//end Tsd16x16OptimalPathLessThan.

int OverlayMem2Dv2::Tsd16x16PartialPathLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, void* path, int len, int min, int batchlen)
{
  short**	bPtr = b.Get2DSrcPtr();
  OM2DV2_COORD* pPath = (OM2DV2_COORD *)path;

  int totBatches  = len / batchlen;
  int Dp          = 0;	///< Accumulated partial square error.
  int expectedD   = 0; ///< The predicted value of sqr err if all batches were processed. 
  int p           = 0;
  int myPos       = me._yPos;
  int mxPos       = me._xPos;
  int byPos       = b._yPos;
  int bxPos       = b._xPos;

  for (int batch = 0; (batch < totBatches) && (expectedD <= min); batch++) ///< Early exit if pred D exceeded min.
  {
    /// Process the next batch.
    int batchEnd = (batch + 1)*batchlen;

    for (; p < batchEnd; )
    {
      int pX1 = (int)pPath[p].x;  int pY1 = (int)pPath[p].y; p++;
      int diff1 = (int)me._pBlock[myPos + pY1][mxPos + pX1] - (int)bPtr[byPos + pY1][bxPos + pX1];
      Dp += (diff1 * diff1);	///< pth partial sqr err.

      int pX2 = (int)pPath[p].x;  int pY2 = (int)pPath[p].y; p++;
      int diff2 = (int)me._pBlock[myPos + pY2][mxPos + pX2] - (int)bPtr[byPos + pY2][bxPos + pX2];
      Dp += (diff2 * diff2);	///< pth partial sqr err.
    }//end for p...
/*
    for (; p < batchEnd; p++)
    {
      int pX = (int)pPath[p].x;
      int pY = (int)pPath[p].y;
      int diff = (int)me._pBlock[myPos + pY][mxPos + pX] - (int)bPtr[byPos + pY][bxPos + pX];
      Dp += (diff * diff);	///< pth partial sqr err.
    }//end for p...
*/

    /// Predict the final distortion from past batches.
    expectedD = (Dp*totBatches) / (batch + 1);
  }//end for batch...

  return(expectedD);
}//end Tsd16x16PartialPathLessThan.

int OverlayMem2Dv2::Tsd16x16PartialPath(OverlayMem2Dv2& me, OverlayMem2Dv2& b, void* path, int len)
{
  short**	bPtr = b.Get2DSrcPtr();
  OM2DV2_COORD* pPath = (OM2DV2_COORD *)path;

  int Dp = 0;	///< Accumulated partial square error.
  int myPos = me._yPos;
  int mxPos = me._xPos;
  int byPos = b._yPos;
  int bxPos = b._xPos;

  for (int p = 0; p < len; )
  {
    int pX = (int)pPath[p].x; int pY = (int)pPath[p].y; p++;
    int diff = (int)me._pBlock[myPos + pY][mxPos + pX] - (int)bPtr[byPos + pY][bxPos + pX];
    Dp += (diff * diff);	///< pth partial sqr err.

    int pX2 = (int)pPath[p].x; int pY2 = (int)pPath[p].y; p++;
    int diff2 = (int)me._pBlock[myPos + pY2][mxPos + pX2] - (int)bPtr[byPos + pY2][bxPos + pX2];
    Dp += (diff2 * diff2);	///< pth partial sqr err.
  }//end for p...

  return(Dp);
}//end Tsd16x16PartialPath.

 /** Calc the total absolute difference with the input block.
The block dimensions must match else return INF.
@param b	: Input block.
@return		: Total absolute diff.	
*/
int OverlayMem2Dv2::Tad(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(10000000);

	int row,col;
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
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
int OverlayMem2Dv2::Tad4x4(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][3]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
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
int OverlayMem2Dv2::Tad8x8(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int					acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][7]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
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
int OverlayMem2Dv2::Tad16x16(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][7]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][8]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][9]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][10]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][11]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][12]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][13]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][14]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		// [row][15]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
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
int OverlayMem2Dv2::TadLessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(min + 10000000);

	int row,col;
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(row = 0; row < me._height; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);
		for(col = 0; col < me._width; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
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
int OverlayMem2Dv2::Tad4x4LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(int row = 0; row < 4; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 4; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2DV2_UNROLL_INNER_LOOP

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
int OverlayMem2Dv2::Tad8x8LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(int row = 0; row < 8; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 8; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2DV2_UNROLL_INNER_LOOP

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
int OverlayMem2Dv2::Tad16x16LessThan(OverlayMem2Dv2& me, OverlayMem2Dv2& b, int min)
{
	short**	bPtr	= b.Get2DSrcPtr();
	short*	pP;
	short*	pI;
	int			acc		= 0;
	for(int row = 0; row < 16; row++)
	{
		pP = &(me._pBlock[me._yPos + row][me._xPos]);
		pI = &(bPtr[b._yPos + row][b._xPos]);

#ifdef OM2DV2_UNROLL_INNER_LOOP
		// [row][0]
    int diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);	// Early exit because exceeded min.
		// [row][1]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][2]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][3]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][4]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][5]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][6]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][7]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][8]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][9]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][10]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][11]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][12]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][13]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][14]
    diff = *(pP++) - *(pI++);
		acc += OM2DV2_FAST_ABS32(diff);
		if(acc > min)	return(acc);
		// [row][15]
    diff = *(pP) - *(pI);
		acc += OM2DV2_FAST_ABS32(diff);

#else
		for(int col = 0; col < 16; col++)
		{
      int diff = *(pP++) - *(pI++);
			acc += OM2DV2_FAST_ABS32(diff);
			if(acc > min)
				return(acc);	// Early exit because exceeded min.
		}//end for col...
#endif	// OM2DV2_UNROLL_INNER_LOOP

	}//end for row...

	return(acc);
}//end Tad16x16LessThan.

/** Test the blocks for equality.
The block dimensions must match else return 0.
@param b		: Input block.
@return			: 1 = are equal, 0 = not equal.
*/
int OverlayMem2Dv2::Equals(OverlayMem2Dv2& me, OverlayMem2Dv2& b)
{
	if( (me._width != b._width)||(me._height != b._height) )
		return(0);
	
	int	equal = 1; // Assume equality until proven different.
	int row,col,x,y,bx,by;
	short**	bPtr = b.Get2DSrcPtr();
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
void OverlayMem2Dv2::Half(void** srcPtr, int srcWidth, int srcHeight, 
												void** dstPtr, int widthOff, int heightOff)
{
	short** ppS = (short **)(srcPtr);
	short** ppD = (short **)(dstPtr);

	int m,n,x,y;
  for(m = 0, y = heightOff; m < srcHeight; m += 2, y++)
		for(n = 0, x = widthOff; n < srcWidth; n += 2, x++)
  {
		ppD[y][x] = (ppS[m][n] + ppS[m][n+1] + ppS[m+1][n] + ppS[m+1][n+1] + 2) >> 2;
	}//end for m & n...

}//end Half.

/** Dump the current block into a file.
Treat all elements as type integer.
@param pBlk     : Blk to dump.
@param filename : File name to use.
@param title    : Title to use in the text file.
@return         : None.
*/
void OverlayMem2Dv2::Dump(OverlayMem2Dv2* pBlk, char* filename, const char* title)
{
  int i,j; 

  MeasurementTable* pT = new MeasurementTable();
	pT->SetTitle(title);

  int cols = pBlk->GetWidth();
  int rows = pBlk->GetHeight();

	pT->Create(cols, rows);
  for(j = 0; j < cols; j++)
  {
    pT->SetHeading(j, "");
    pT->SetDataType(j, MeasurementTable::INT);
  }//end for j...

	for(i = 0; i < rows; i++)
		for(j = 0; j < cols; j++)
			pT->WriteItem(j, i, pBlk->Read(j, i));

  pT->Save(filename, ",", 0);

  delete pT;
}//end Dump.

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
//int OverlayMem2Dv2::Write(void* srcPtr,	int	srcWidth,	int	srcHeight)
//{
//	// Check before copying.
//	if(_pMem == NULL)
//		return(0);
//
//	// This object is filled from the top left corner of the source.
//	OM2DV2_PTYPE	pLcl = (OM2DV2_PTYPE)srcPtr;
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
//		memcpy((void *)(&_pBlock[row+_yPos][_xPos]), (const void *)(&(pLcl[pos])), _width * sizeof(short));
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
//int OverlayMem2Dv2::Write(void* srcPtr, int srcWidth, int srcHeight, 
//																			int toCol,		int toRow,
//																			int cols,			int rows)
//{
//	// Check before copying.
//	if(_pMem == NULL)
//		return(0);
//
//	// This object is patially filled from a srcWidth X srcHeight 2-D source 
//	// with cols x rows of values into the block offset.
//	OM2DV2_PTYPE	pLcl = (OM2DV2_PTYPE)srcPtr;
//
//	// Fast.
//	int	pos = 0;
//	toRow		+= _yPos;
//	toCol		+= _xPos;
//	for(int row = 0; row < rows; row++, pos += srcWidth)
//		memcpy((void *)(&(_pBlock[toRow + row][toCol])), (const void *)(&(pLcl[pos])), cols * sizeof(short));
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
//void OverlayMem2Dv2::Read(void* dstPtr, int dstWidth, int dstHeight)
//{
//	OM2DV2_PTYPE	pLcl = (OM2DV2_PTYPE)dstPtr;
//
//	int	dstPos = 0;
//	for(int row = 0; row < _height; row++, dstPos += dstWidth)
//	{
//		memcpy((void *)(&(pLcl[dstPos])), 
//					 (const void *)(&(_pBlock[_yPos + row][_xPos])), 
//					 _width * sizeof(short));
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
//void OverlayMem2Dv2::Read(void* dstPtr, int dstWidth,	int dstHeight,
//																			int fromCol,	int fromRow,
//																			int cols,			int rows)
//{
//	OM2DV2_PTYPE	pLcl = (OM2DV2_PTYPE)dstPtr;
//
//	int	dstPos = 0;
//	// Adjust for the underlying mem offsets.
//	fromRow += _yPos;
//	fromCol += _xPos;
//	for(int row = 0; row < rows; row++, dstPos += dstWidth)
//	{
//		memcpy((void *)(&(pLcl[dstPos])), 
//					 (const void *)(&(_pBlock[fromRow + row][fromCol])),	
//					 cols * sizeof(short));
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
//void OverlayMem2Dv2::HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
//																					int fromCol,		int fromRow,
//																					int halfColOff,	int halfRowOff,
//																					int cols,				int rows)
//{
//	OM2DV2_PTYPE	pLcl = (OM2DV2_PTYPE)dstPtr;
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
//				pLcl[dstPos] = (short)z;
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
//				pLcl[dstPos] = (short)z;
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






