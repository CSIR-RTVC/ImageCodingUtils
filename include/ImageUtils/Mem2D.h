/** @file

MODULE						: Mem2D

TAG								: M2D

FILE NAME					: Mem2D.h

DESCRIPTION				: A class to describe and read/write a two-dimensional
										block of memory of primitive types.

REVISION HISTORY	:
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _MEM2D_H
#define _MEM2D_H

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define M2D_HALF_POS_ARRAY_LENGTH 	8

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef short int M2D_TYPE;
typedef M2D_TYPE* M2D_PTYPE;

typedef struct
{
	int x;
	int y;
} M2D_COORD;

class Mem2D
{
	// Construction and destruction.
public:
	Mem2D();
	Mem2D(int width, int height);
	virtual ~Mem2D();

protected:
	virtual void	ResetMembers(void);
	virtual void	Destroy(void);
	virtual int		Create(int width, int height);

	// Member access.
public:
	virtual int GetWidth(void)	{ return(_width); }
	virtual int GetHeight(void)	{ return(_height); }

	virtual M2D_TYPE** Get2DPtr(void)	{ return(_pBlock); }

	// Interface: Input/output functions.
public:

	// Initialisation functions.
	virtual int		Set(Mem2D& srcBlock);

	virtual int		Set(void* srcPtr, int width, int height);

	// Part/all of the source to all of the block.
	virtual int		Write(void* srcPtr, int srcWidth, int srcHeight);

	// Part/all of the source to part of the block.
	virtual int		Write(void* srcPtr, int srcWidth, int srcHeight,
																		int toCol,		int toRow,
																		int cols,	    int rows);

	// All of the source to part of the block.
	virtual int		Write(Mem2D&	srcBlock, int	toCol, int toRow);

	// Part of the source to part of the block.
	virtual int		Write(Mem2D&	srcBlock,	int fromCol,	int	fromRow,
																				int toCol,		int	toRow,
																				int cols,			int	rows);

	// All of the source to all of the block. Must be equal mem footprint.
	virtual int		Write(Mem2D&	srcBlock);

	// All of the block to part/all of the destination.
	virtual void	Read(void* dstPtr, int dstWidth, int dstHeight);

	// Part of the block to part/all of the destination.
	virtual void	Read(void* dstPtr, int dstWidth,	int dstHeight,
																	 int fromCol,		int fromRow,
																	 int cols,			int rows);

	// Part of the block to part/all of the destination.
	virtual void	Read(Mem2D&	dstBlock, int toCol,		int toRow,
																			int fromCol,	int fromRow,
																			int cols,			int rows);

	// Part of the block at half pel position to part/all of the destination.
	virtual void	HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
																			 int fromCol,			int fromRow,
																			 int halfColOff,	int halfRowOff,
																			 int cols,				int rows);

	// All of the block to all of the destination. Must be equal footprint.
	virtual int		Read(Mem2D&	dstBlock);

	// Interface: Operation functions.
public:

	// Set block values to zero.
	virtual void Clear(void);
	// Subtract the input block from this.
	virtual int Sub(Mem2D& b);
	// Add the input block to this.
	virtual int Add(Mem2D& b);
	// Calc total square error with the input block.
	virtual int Tse(Mem2D& b);
	// The total square error with the input to improve on an input value.
	virtual int TseLessThan(Mem2D& b, int min);
	// Calc total absolute error with the input block.
	virtual int Tae(Mem2D& b);
	// The total abs error with the input to improve on an input value.
	virtual int TaeLessThan(Mem2D& b, int min);

protected:
	int	_width;							// Mem width and height.
	int	_height;

	// Mem is 2-D with row address array for speed.
	M2D_TYPE*		_pMem;			// Contiguous mem.
	M2D_TYPE**	_pBlock;		// Rows within _pMem;

	// Constants.
	static const M2D_COORD HALF_POSITION[M2D_HALF_POS_ARRAY_LENGTH];

};// end class Mem2D.

#endif
