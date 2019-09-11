/** @file

MODULE						: OverlayMem2D

TAG								: OM2D

FILE NAME					: OverlayMem2D.h

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
									: 28/11/2006 by KF:
											Added an Equals() method to determine if the elements within
											the 2D block are identical in value.

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _OVERLAYMEM2D_H
#define _OVERLAYMEM2D_H

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef short OM2D_TYPE;
typedef OM2D_TYPE*	OM2D_PTYPE;

class OverlayMem2D
{
	// Construction and destruction.
public:
	OverlayMem2D(void* srcPtr, int srcWidth, int srcHeight, int width, int height);
	virtual ~OverlayMem2D();

protected:
	void	ResetMembers(void);
	void	Destroy(void);
	int   SetMem(void* srcPtr, int srcWidth, int srcHeight);

	// Member access.
public:
	int		GetWidth(void)	{ return(_width); }
	int		GetHeight(void)	{ return(_height); }
	void	SetOverlayDim(int width, int height) { _width = width; _height = height; }

	int		GetOriginX(void) { return(_xPos); }
	int		GetOriginY(void) { return(_yPos); }
	// Overridable methods.
	virtual void	SetOrigin(int x, int y);

	OM2D_TYPE**	Get2DSrcPtr(void) { return(_pBlock); }

	// Interface: Input/output functions.
public:

	// Write a single value.
	void Write(int toCol, int toRow, int value) 
		{ _pBlock[_yPos + toRow][_xPos + toCol] = (OM2D_TYPE)value; }

	// All of the source to part of the block.
	int	Write(OverlayMem2D&	srcBlock, int	toCol, int toRow)
		{ return(Write(*this, srcBlock, toCol, toRow)); }
	static int Write(OverlayMem2D& me, OverlayMem2D&	srcBlock, int	toCol, int toRow);

	// Part of the source to part of the block.
	int	Write(OverlayMem2D&	srcBlock,	int fromCol,	int	fromRow,
																		int toCol,		int	toRow,
																		int cols,			int	rows)
		{ return(Write(*this, srcBlock,	fromCol, fromRow, toCol, toRow, cols, rows)); }
	static int Write(OverlayMem2D& me, OverlayMem2D&	srcBlock,	
									 int fromCol,			 int	fromRow,
									 int toCol,				 int	toRow,
									 int cols,				 int	rows);

	// All of the source to all of the block. Must be equal mem footprint.
	int	Write(OverlayMem2D&	srcBlock)
		{ return(Write(*this, srcBlock)); }
	static int Write(OverlayMem2D& me, OverlayMem2D&	srcBlock);

	// Read a single value.
	int	Read(int fromCol, int fromRow) 
		{ return(_pBlock[_yPos + fromRow][_xPos + fromCol]); }

	// Part of the block to part/all of the destination.
	void Read(OverlayMem2D&	dstBlock, int toCol,		int toRow,
																			int fromCol,	int fromRow,
																			int cols,			int rows)
		{ Read(*this, dstBlock, toCol, toRow, fromCol, fromRow, cols, rows); }
	static void Read(OverlayMem2D& me, OverlayMem2D&	dstBlock, 
									 int toCol,				 int toRow,
									 int fromCol,			 int fromRow,
									 int cols,				 int rows);

	// All of the block to all of the destination. Must be equal footprint.
	int Read(OverlayMem2D&	dstBlock)
		{ return(Read(*this, dstBlock)); }
	static int Read(OverlayMem2D& me, OverlayMem2D&	dstBlock);

	// All of the block at half pel position to all of the destination.
	void HalfRead(OverlayMem2D& dstBlock, int halfColOff, int halfRowOff)
		{ HalfRead(*this, dstBlock, halfColOff, halfRowOff); }
	static void HalfRead(OverlayMem2D& me, OverlayMem2D& dstBlock, int halfColOff, int halfRowOff);

	// Interface: Operation functions.
public:

	// Set block values to zero.
	void Clear(void)
		{ Clear(*this); }
	static void Clear(OverlayMem2D& me);
	// Set block values to a value.
	void Fill(int value)
		{ Fill(*this, value); }
	static void Fill(OverlayMem2D& me, int value);
	// Sum all the elements in the block.
	int Sum(void)
		{ return(Sum(*this)); }
	static int Sum(OverlayMem2D& me);
	// Subtract the input block from this.
	int Sub(OverlayMem2D& b)
		{ return( Sub(*this, b) ); }
	static int Sub(OverlayMem2D& me, OverlayMem2D& b);
	// Add the input block to this.
	int Add(OverlayMem2D& b)
		{ return( Add(*this, b) ); }
	static int Add(OverlayMem2D& me, OverlayMem2D& b);

	/// Calc total square difference with the input block.
	int Tsd(OverlayMem2D& b)
		{ return( Tsd(*this, b) ); }
	static int Tsd(OverlayMem2D& me, OverlayMem2D& b);
	int Tsd4x4(OverlayMem2D& b)
		{ return( Tsd4x4(*this, b) ); }
	static int Tsd4x4(OverlayMem2D& me, OverlayMem2D& b);			///< Fast for 4x4 blocks.
	int Tsd8x8(OverlayMem2D& b)
		{ return( Tsd8x8(*this, b) ); }
	static int Tsd8x8(OverlayMem2D& me, OverlayMem2D& b);			///< Fast for 8x8 blocks.
	int Tsd16x16(OverlayMem2D& b)
		{ return( Tsd16x16(*this, b) ); }
	static int Tsd16x16(OverlayMem2D& me, OverlayMem2D& b);		///< Fast for 16x16 blocks.

	/// The total square difference with the input to improve on an input value.
	int TsdLessThan(OverlayMem2D& b, int min)
		{ return( TsdLessThan(*this, b, min) ); }
	static int TsdLessThan(OverlayMem2D& me, OverlayMem2D& b, int min);
	int Tsd4x4LessThan(OverlayMem2D& b, int min)
		{ return( Tsd4x4LessThan(*this, b, min) ); }
	static int Tsd4x4LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);		///< Fast for 4x4 blocks.
	int Tsd8x8LessThan(OverlayMem2D& b, int min)
		{ return( Tsd8x8LessThan(*this, b, min) ); }
	static int Tsd8x8LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);		///< Fast for 8x8 blocks.
	int Tsd16x16LessThan(OverlayMem2D& b, int min)
		{ return( Tsd16x16LessThan(*this, b, min) ); }
	static int Tsd16x16LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);	///< Fast for 16x16 blocks.

	/// Calc total absolute difference with the input block.
	int Tad(OverlayMem2D& b)
		{ return( Tad(*this, b) ); }
	static int Tad(OverlayMem2D& me, OverlayMem2D& b);
	int Tad4x4(OverlayMem2D& b)
		{ return( Tad4x4(*this, b) ); }
	static int Tad4x4(OverlayMem2D& me, OverlayMem2D& b);			///< Fast for 4x4 blocks.
	int Tad8x8(OverlayMem2D& b)
		{ return( Tad8x8(*this, b) ); }
	static int Tad8x8(OverlayMem2D& me, OverlayMem2D& b);			///< Fast for 8x8 blocks.
	int Tad16x16(OverlayMem2D& b)
		{ return( Tad16x16(*this, b) ); }
	static int Tad16x16(OverlayMem2D& me, OverlayMem2D& b);		///< Fast for 16x16 blocks.

	/// The total abs difference with the input to improve on an input value.
	int TadLessThan(OverlayMem2D& b, int min)
		{ return( TadLessThan(*this, b, min) ); }
	static int TadLessThan(OverlayMem2D& me, OverlayMem2D& b, int min);
	int Tad4x4LessThan(OverlayMem2D& b, int min)
		{ return( Tad4x4LessThan(*this, b, min) ); }
	static int Tad4x4LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);		///< Fast for 4x4 blocks.
	int Tad8x8LessThan(OverlayMem2D& b, int min)
		{ return( Tad8x8LessThan(*this, b, min) ); }
	static int Tad8x8LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);		///< Fast for 8x8 blocks.
	int Tad16x16LessThan(OverlayMem2D& b, int min)
		{ return( Tad16x16LessThan(*this, b, min) ); }
	static int Tad16x16LessThan(OverlayMem2D& me, OverlayMem2D& b, int min);	///< Fast for 16x16 blocks.

	// Block test operators.
	int Equals(OverlayMem2D& b)
		{ return( Equals(*this, b) ); }
	static int Equals(OverlayMem2D& me, OverlayMem2D& b);

	// Static independent helper functions.
public:
	// Sub sample the src by half into another 2D mem block with possible offset.
	static void Half(void**	srcPtr, int srcWidth,			int srcHeight,
									 void** dstPtr, int widthOff = 0, int heightOff = 0);

protected:
	int					_width;				// Overlay mem width and height.
	int					_height;
	int					_srcWidth;		// Underlying mem width, height and ptr
	int					_srcHeight;		// initialised in the constructor.
	OM2D_TYPE*	_pMem;

	// Mem is 2-D with row address array for speed.
	OM2D_TYPE**	_pBlock;		// Rows within _srcPtr;

	// Origin of the top left corner of the overlay block within the mem.
	int					_xPos;
	int					_yPos;

};// end class OverlayMem2D.

/*
-----------------------------------------------------------------------
	Depricated Methods.
-----------------------------------------------------------------------
*/

	// Part/all of the source to all of the block.
	//virtual int		Write(void* srcPtr, int srcWidth, int srcHeight);

	// Part/all of the source to part of the block.
	//virtual int		Write(void* srcPtr, int srcWidth, int srcHeight,
	//																	int toCol,		int toRow,
	//																	int cols,	    int rows);

	// All of the block to part/all of the destination.
	//virtual void	Read(void* dstPtr, int dstWidth, int dstHeight);

	// Part of the block to part/all of the destination.
	//virtual void	Read(void* dstPtr, int dstWidth,	int dstHeight,
	//																 int fromCol,		int fromRow,
	//																 int cols,			int rows);

	// Part of the block at half pel position to part/all of the destination.
	//virtual void	HalfRead(void* dstPtr, int dstWidth,		int dstHeight,
	//																		 int fromCol,			int fromRow,
	//																		 int halfColOff,	int halfRowOff,
	//																		 int cols,				int rows);

#endif
