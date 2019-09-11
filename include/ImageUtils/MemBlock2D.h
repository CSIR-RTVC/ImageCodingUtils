/** @file

MODULE						: MemBlock2D

TAG								: MB2D

FILE NAME					: MemBlock2D.h

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
#ifndef _MEMBLOCK2D_H
#define _MEMBLOCK2D_H

#include "VicsDefs/VicsDefs.h"
#include "VideoDim16VectorQuantiser.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define MB2D_HALF_POS_ARRAY_LENGTH 	8
#define MB2D_CANDIDATE_LENGTH				4

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef VICS_SBYTE MB2D_TYPE;
typedef MB2D_TYPE* MB2D_PTYPE;

class MemBlock2D
{
protected:
	VICS_INT	_width;							// Block width and height.
	VICS_INT	_height;

	// Mem block is 2-D with row address array for speed.
	MB2D_TYPE*	_pMem;			// Contiguous mem.
	MB2D_TYPE**	_pBlock;		// Rows within _pMem;

	// Candidate search support members.
	VICS_INT		_winX[MB2D_CANDIDATE_LENGTH];	// x coord. list.
	VICS_INT		_winY[MB2D_CANDIDATE_LENGTH];	// y coord. list.
	VICS_INT32	_winD[MB2D_CANDIDATE_LENGTH];	// Distortion list.
	VICS_INT		_candidateLength;

	// Constants.
	static const VICS_COORD HALF_POSITION[MB2D_HALF_POS_ARRAY_LENGTH];

	// For optimal distortion-rate matching criteria.
	VideoDim16VectorQuantiser _vq;

public:
	MemBlock2D();
	MemBlock2D(VICS_INT width, VICS_INT height);
	virtual ~MemBlock2D();

	// Member access.
	virtual VICS_INT GetWidth(void)		{ return(_width); }
	virtual VICS_INT GetHeight(void)	{ return(_height); }

	// Interface.

	// Input/output functions.
	virtual VICS_INT		Fill(VICS_POINTER srcPtr, 			// Nomenclature: Fill block from
													 VICS_INT			srcWidth, 		// larger source.
													 VICS_INT			srcHeight);

	virtual VICS_INT		Copy(VICS_POINTER srcPtr, 			// Nomenclature: Copy from smaller
													 VICS_INT			srcWidth, 		// block sources.
													 VICS_INT			srcHeight,
													 VICS_INT			toCol,
													 VICS_INT			toRow);

	virtual VICS_INT 		Copy(MemBlock2D&	smallBlock, 
													 VICS_INT			toCol,
													 VICS_INT			toRow);

	virtual VICS_INT 		Transfer(MemBlock2D&	srcBlock, // Nomenclature: Transfer any block
															 VICS_INT			srcCol,		// sizes.
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

	// Calculation functions.
	virtual VICS_INT32 	Tse(MemBlock2D& smallBlock, 
													VICS_INT		atCol,
													VICS_INT		atRow);

	virtual VICS_INT32 	ImproveTse(MemBlock2D& smallBlock, 
																 VICS_INT		atCol,
																 VICS_INT		atRow,
																 VICS_INT32	minEnergy);

	virtual VICS_INT32 	HalfTse(MemBlock2D& smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow,
															VICS_INT		halfColOff,
															VICS_INT		halfRowOff);

	virtual VICS_INT32 	HalfTse(MemBlock2D& smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow,
															VICS_INT		halfColOff,
															VICS_INT		halfRowOff,
															VICS_PINT32	worstEnergy);

	virtual VICS_INT32 	ImproveHalfTse(MemBlock2D&	smallBlock, 
																		 VICS_INT			atCol,
																		 VICS_INT			atRow,
																		 VICS_INT			halfColOff,
																		 VICS_INT			halfRowOff,
																		 VICS_INT32		minEnergy);

	// High level search functions.
	virtual VICS_INT32 	FullSearch(MemBlock2D& smallBlock, 
																 VICS_INT		atCol,
																 VICS_INT		atRow,
																 VICS_INT		range,
																 VICS_PINT	mvx,
																 VICS_PINT	mvy);

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

	virtual VICS_INT32 	CandidateSearch(MemBlock2D&	smallBlock, 
																			VICS_INT		atCol,
																			VICS_INT		atRow,
																			VICS_INT		range,
																			VICS_PINT		mvx,
																			VICS_PINT		mvy,
																			VICS_INT		fvx,
																			VICS_INT		fvy,
																			VICS_PINT32	favouredEnergy);

	// Optimal functions.
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
	virtual void				ResetMembers(void);

	virtual void				Destroy(void);

	virtual VICS_INT		Create(VICS_INT width, VICS_INT height);

	virtual void				ResetCandidates(void);

	virtual void				AddCandidate(VICS_INT x, VICS_INT y, VICS_INT32 distortion);

	virtual VICS_INT32	GetBestCandidate(VICS_PINT x, VICS_PINT y);

	virtual VICS_INT	 	VqTse(MemBlock2D& smallBlock, 
														VICS_INT		atCol,
														VICS_INT		atRow,
														VICS_PINT		rate);

	virtual VICS_INT		ImproveDistRate(MemBlock2D& smallBlock, 
																			VICS_INT		atCol, 
																			VICS_INT		atRow, 
																			VICS_INT		currDist, 
																			VICS_INT		currRate, 
																			VICS_PINT		rate);

	virtual VICS_INT	 	VqHalfTse(MemBlock2D& smallBlock, 
																VICS_INT		atCol,
																VICS_INT		atRow,
																VICS_INT		halfColOff,
																VICS_INT		halfRowOff,
																VICS_PINT		rate);

	virtual VICS_INT	 	Tae(MemBlock2D& smallBlock, 
													VICS_INT		atCol,
													VICS_INT		atRow,
													VICS_PINT		distortion);

	virtual VICS_INT	 	ImproveTae(MemBlock2D&	smallBlock, 
																 VICS_INT			atCol,
																 VICS_INT			atRow,
																 VICS_INT			minAbsErr,
																 VICS_PINT		minSqrErr);

	virtual VICS_INT	 	HalfTae(MemBlock2D& smallBlock, 
															VICS_INT		atCol,
															VICS_INT		atRow,
															VICS_INT		halfColOff,
															VICS_INT		halfRowOff,
															VICS_PINT		distortion);

	virtual VICS_INT 	ImproveHalfTae(MemBlock2D&	smallBlock, 
																	 VICS_INT			atCol,
																	 VICS_INT			atRow,
																	 VICS_INT			halfColOff,
																	 VICS_INT			halfRowOff,
																	 VICS_INT			minAbsErr,
																	 VICS_PINT 		minSqrErr);

};// end class MemBlock2D.

#endif
