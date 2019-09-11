/** @file

MODULE						: ExtMem2D

TAG								: EM2D

FILE NAME					: ExtMem2D.h

DESCRIPTION				: A derived class to describe and operate on a two-dimensional
										block of memory with an extended boundary of primitive 
										types. Primarily constructed for supplementing image 
										motion estimation algorithms and hiding the origin shift
										from the caller.

REVISION HISTORY	:
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _EXTMEM2D_H
#define _EXTMEM2D_H

#include "Mem2D.h"

class ExtMem2D: public Mem2D
{
public:
	ExtMem2D();
	ExtMem2D(int width, int height, int widthBy, int heightBy);
	virtual ~ExtMem2D();

	// Interface.
	int GetWidthOffset(void)  { return(_xOrigin); }
	int GetHeightOffset(void) { return(_yOrigin); }

	virtual int ExtendBoundary(void);

	// Overridden input/output functions.
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

	// All of source to all of the inner part of the block.
	virtual int Write(Mem2D&	srcBlock);

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

	// All of the block to all of the destination. Middle part must be equal footprint.
	virtual int		Read(Mem2D&	dstBlock);

protected:
	// Extending the boundary offsets of the mem within the memory.
	int	_xOrigin;	 		// X and y offset of the block in the mem.
	int	_yOrigin;
	int _innerWidth;	// Inner width and height.
	int _innerHeight;

};// end class ExtMem2D.

#endif
