/** @file

MODULE						: ExtMemBlock2D

TAG								: EMB2D

FILE NAME					: ExtMemBlock2D.h

DESCRIPTION				: A derived class to describe and operate on a two-dimensional
										block of memory with an extended boundary of primitive 
										types. Primarily constructed for suplimenting image 
										motion estimation algorithms.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _EXTMEMBLOCK2D_H
#define _EXTMEMBLOCK2D_H

#include "VicsDefs/VicsDefs.h"
#include "MemBlock2D.h"

class ExtMemBlock2D: public MemBlock2D
{
protected:
	// Extending the boundary offsets of the block within the memory.
	VICS_INT	_xOrigin;	 	// X and y offset of the block in the mem.
	VICS_INT	_yOrigin;

public:
	ExtMemBlock2D();
	ExtMemBlock2D(VICS_INT width, VICS_INT height, VICS_INT widthBy, VICS_INT heightBy);
	virtual ~ExtMemBlock2D();

	// Interface.
	VICS_INT GetWidthOffset(void)  { return(_xOrigin); }
	VICS_INT GetHeightOffset(void) { return(_yOrigin); }

	virtual VICS_INT ExtendBoundary(void);

	// Overriden Methods.
	virtual VICS_INT		Fill(VICS_POINTER	srcPtr, 
													 VICS_INT			srcWidth, 
													 VICS_INT			srcHeight);

	virtual VICS_INT		Copy(VICS_POINTER srcPtr, 
													 VICS_INT			srcWidth, 
													 VICS_INT			srcHeight,
													 VICS_INT			toCol,
													 VICS_INT			toRow);

	virtual VICS_INT 		Copy(MemBlock2D&	smallBlock, 
													 VICS_INT			toCol,
													 VICS_INT			toRow);

	virtual VICS_INT 		Transfer(MemBlock2D&	srcBlock,
															 VICS_INT			srcCol,
															 VICS_INT			srcRow,
															 VICS_INT			toCol,
															 VICS_INT			toRow,
															 VICS_INT			cols,
															 VICS_INT			rows);

	virtual void				Read(VICS_POINTER	dstPtr,
													 VICS_INT			dstWidth,
													 VICS_INT			dstHeight,
													 VICS_INT			fromCol,
													 VICS_INT			fromRow,
													 VICS_INT			cols,
													 VICS_INT			rows);

	virtual void				Read(VICS_POINTER	dstPtr,
													 VICS_INT			dstWidth,
													 VICS_INT			dstHeight,
													 VICS_INT			fromCol,
													 VICS_INT			fromRow,
													 VICS_INT			halfColOff,
													 VICS_INT			halfRowOff,
													 VICS_INT			cols,
													 VICS_INT			rows);

	virtual VICS_INT32 	Tse(MemBlock2D& smallBlock, 
													VICS_INT		atCol,
													VICS_INT		atRow);

	virtual VICS_INT32 	ImproveTse(MemBlock2D&	smallBlock, 
																 VICS_INT			atCol,
																 VICS_INT			atRow,
																 VICS_INT32		minEnergy);

	virtual VICS_INT32 	HalfTse(MemBlock2D& smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow,
															VICS_INT		halfColOff,
															VICS_INT		halfRowOff);

	virtual VICS_INT32 	ImproveHalfTse(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT			halfColOff,
																		 VICS_INT			halfRowOff,
																		 VICS_INT32		minEnergy);

	virtual VICS_INT32 	FullSearch(MemBlock2D&	smallBlock, 
																 VICS_INT			atCol,
																 VICS_INT			atRow,
																 VICS_INT			range,
																 VICS_PINT		mvx,
																 VICS_PINT		mvy);

	virtual VICS_INT32 	HalfSearch(MemBlock2D&	smallBlock, 
																 VICS_INT			atCol,
																 VICS_INT			atRow,
																 VICS_PINT		hmvx,
																 VICS_PINT		hmvy);

	virtual VICS_INT32 	Search(MemBlock2D&	smallBlock, 
														 VICS_INT			atCol,
														 VICS_INT			atRow,
														 VICS_INT			range,
														 VICS_PINT		mvx,
														 VICS_PINT		mvy,
														 VICS_PINT32	zeroEnergy);

	virtual VICS_INT32 	CandidateSearch(MemBlock2D&	smallBlock, 
																			VICS_INT		atCol,
																			VICS_INT		atRow,
																			VICS_INT		range,
																			VICS_PINT		mvx,
																			VICS_PINT		mvy,
																			VICS_PINT32	zeroEnergy);

	virtual VICS_INT32 CandidateSearch(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT			range,
																		 VICS_PINT		mvx,
																		 VICS_PINT		mvy,
																		 VICS_INT			fvx,
																		 VICS_INT			fvy,
																		 VICS_PINT32	favouredEnergy);

	virtual VICS_INT OptimalRateSearch(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT			range,
																		 VICS_PINT		mvx,
																		 VICS_PINT		mvy,
																		 VICS_INT			biasx,
																		 VICS_INT			biasy,
																		 VICS_PINT		cost);

protected:

};// end class ExtMemBlock2D.

#endif
