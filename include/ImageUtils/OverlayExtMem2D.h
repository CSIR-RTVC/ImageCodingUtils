/** @file

MODULE						: OverlayExtMem2D

TAG								: OEM2D

FILE NAME					: OverlayExtMem2D.h

DESCRIPTION				: A class to add extended boundary functionality to an overlay 
										OverlayMem2D class. The boundary is hidden from the calling 
										functions and is a polymorphism of an OverlayMem2D class. 

REVISION HISTORY	:	
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _OVERLAYEXTMEM2D_H
#define _OVERLAYEXTMEM2D_H

/*
---------------------------------------------------------------------------
	Includes.
---------------------------------------------------------------------------
*/

#include	"OverlayMem2D.h"

class OverlayExtMem2D: public OverlayMem2D
{
	// Construction and destruction.
public:
	OverlayExtMem2D(void* srcPtr, int srcWidth, int srcHeight, 
																int width,		int height,
																int bWidth,		int bHeight);

	virtual ~OverlayExtMem2D();

	// Member access.
public:
	int	GetBoundaryWidth(void)	{ return(_bWidth); }
	int	GetBoundaryHeight(void)	{ return(_bHeight); }

	virtual void	SetOrigin(int x, int y);

	// Interface: Input/output functions.
public:
	// Extend the boundary by alloc new mem. These static methods must
	// be called from the thread that owns the src mem to ensure it is
	// thread safe.	They are utility functions and are independent of the class. 
	static int ExtendBoundary(void* srcPtr, 
														int srcWidth,int srcHeight, 
														int widthBy, int heightBy,
														void** dstPtr);
	// Fill an existing boundary.
	static void FillBoundary(void* srcPtr, int srcWidth,int srcHeight, 
																				 int widthBy,	int heightBy);

	// Proxy class method.
	void 	FillBoundaryProxy(void);

protected:
	int	_bWidth;				// Boundary width and height.
	int	_bHeight;
};// end class OverlayExtMem2D.

#endif
