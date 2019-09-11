/** @file

MODULE						: Block

TAG								: B

FILE NAME					: Block.h

DESCRIPTION				: A class to describe and operate on a two-dimensional
										block of memory of primitive types.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _BLOCK_H
#define _BLOCK_H

#include "VicsDefs/VicsDefs.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define B_HALF_POS_ARRAY_LENGTH 	8

/*
---------------------------------------------------------------------------
	Primitive type definitions.
---------------------------------------------------------------------------
*/
typedef VICS_SBYTE	B_TYPE;
typedef B_TYPE*			B_PTYPE;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class Block
{
public:
	Block();
	Block(VICS_INT width, VICS_INT height);
	virtual ~Block();

public:
	// Interface.

	// Read/Write methods for Blocks.
//	virtual VICS_INT		Fill(VICS_POINTER srcPtr, 			// Nomenclature: Fill block from
//													 VICS_INT			srcWidth, 		// larger source.
//													 VICS_INT			srcHeight);
//
//	virtual VICS_INT		Copy(VICS_POINTER srcPtr, 			// Nomenclature: Copy from smaller
//													 VICS_INT			srcWidth, 		// block sources.
//													 VICS_INT			srcHeight,
//													 VICS_INT			toCol,
//													 VICS_INT			toRow);
//
//	virtual VICS_INT 		Copy(Block&	smallBlock, 
//													 VICS_INT			toCol,
//													 VICS_INT			toRow);
//
//	virtual VICS_INT 		Transfer(Block&	srcBlock, // Nomenclature: Transfer any block
//															 VICS_INT			srcCol,		// sizes.
//															 VICS_INT			srcRow,
//															 VICS_INT			toCol,
//															 VICS_INT			toRow,
//															 VICS_INT			cols,
//															 VICS_INT			rows);
//
//	virtual void				Read(VICS_POINTER	dstPtr,
//													 VICS_INT			dstWidth,
//													 VICS_INT			dstHeight,
//													 VICS_INT			fromCol,
//													 VICS_INT			fromRow,
//													 VICS_INT			cols,
//													 VICS_INT			rows);
//
//	virtual void				Read(VICS_POINTER	dstPtr,
//													 VICS_INT			dstWidth,
//													 VICS_INT			dstHeight,
//													 VICS_INT			fromCol,
//													 VICS_INT			fromRow,
//													 VICS_INT			halfColOff,
//													 VICS_INT			halfRowOff,
//													 VICS_INT			cols,
//													 VICS_INT			rows);
//
	// Operations on Blocks.
//	virtual VICS_INT32 	Tse(Block& smallBlock, 
//													VICS_INT		atCol,
//													VICS_INT		atRow);
//
//	virtual VICS_INT32 	ImproveTse(Block& smallBlock, 
//																 VICS_INT		atCol,
//																 VICS_INT		atRow,
//																 VICS_INT32	minEnergy);
//
//	virtual VICS_INT32 	HalfTse(Block& smallBlock, 
//															VICS_INT		atCol,
//															VICS_INT		atRow,
//															VICS_INT		halfColOff,
//															VICS_INT		halfRowOff);
//
//	virtual VICS_INT32 	HalfTse(Block& smallBlock, 
//															VICS_INT		atCol,
//															VICS_INT		atRow,
//															VICS_INT		halfColOff,
//															VICS_INT		halfRowOff,
//															VICS_PINT32	worstEnergy);
//
//	virtual VICS_INT32 	ImproveHalfTse(Block&	smallBlock, 
//																		 VICS_INT			atCol,
//																		 VICS_INT			atRow,
//																		 VICS_INT			halfColOff,
//																		 VICS_INT			halfRowOff,
//																		 VICS_INT32		minEnergy);
//
//	virtual VICS_INT		ImproveDistRate(Block& smallBlock, 
//																			VICS_INT		atCol, 
//																			VICS_INT		atRow, 
//																			VICS_INT		currDist, 
//																			VICS_INT		currRate, 
//																			VICS_PINT		rate);
//
//	virtual VICS_INT	 	Tae(Block& smallBlock, 
//													VICS_INT		atCol,
//													VICS_INT		atRow,
//													VICS_PINT		distortion);
//
//	virtual VICS_INT	 	ImproveTae(Block&	smallBlock, 
//																 VICS_INT			atCol,
//																 VICS_INT			atRow,
//																 VICS_INT			minAbsErr,
//																 VICS_PINT		minSqrErr);
//
//	virtual VICS_INT	 	HalfTae(Block& smallBlock, 
//															VICS_INT		atCol,
//															VICS_INT		atRow,
//															VICS_INT		halfColOff,
//															VICS_INT		halfRowOff,
//															VICS_PINT		distortion);
//
//	virtual VICS_INT 	ImproveHalfTae(Block&	smallBlock, 
//																	 VICS_INT			atCol,
//																	 VICS_INT			atRow,
//																	 VICS_INT			halfColOff,
//																	 VICS_INT			halfRowOff,
//																	 VICS_INT			minAbsErr,
//																	 VICS_PINT 		minSqrErr);

public:
	// Member access.
	VICS_INT GetWidth(void)		{return(_width);}
	VICS_INT GetHeight(void)	{return(_height);}

	// Operation results.
	VICS_INT GetLastSqrDiff(void)	{return(_sqrDiff);}
	VICS_INT GetLastAbsDiff(void)	{return(_absDiff);}
	VICS_INT GetLastCost(void)		{return(_cost);}

	// All operations are located at these settings.
	void SetOperationLocation(VICS_INT col,VICS_INT row) {_col=col;_row=row;}
	void SetOperationLocation(VICS_INT col,VICS_INT row,VICS_INT halfColOff,VICS_INT halfRowOff)
		{_col=col;_row=row;_halfColOff=halfColOff;_halfRowOff=halfRowOff;}

protected:
	virtual void				ResetMembers(void);
	virtual void				Destroy(void);
	virtual VICS_INT		Create(VICS_INT width,VICS_INT height);

protected:
	VICS_INT	_width;							// Block width and height.
	VICS_INT	_height;

	VICS_INT	_col;								// Operation location within Block.
	VICS_INT	_row;
	VICS_INT	_halfColOff;
	VICS_INT	_halfRowOff;

	VICS_INT	_sqrDiff;						// Operational results.
	VICS_INT	_absDiff;
	VICS_INT	_cost;

	// Mem block is 2-D with row address array for speed.
	B_TYPE*		_pMem;			// Contiguous mem.
	B_TYPE**	_pBlock;		// Rows within _pMem;

	// Constants.
	static const VICS_COORD HALF_POSITION[B_HALF_POS_ARRAY_LENGTH];

};// end class Block.

#endif
