/** @file

MODULE						: OverlayExtMem2D

TAG								: OEM2D

FILE NAME					: OverlayExtMem2D.cpp

DESCRIPTION				: A class to add extended boundary functionality to an overlay 
										OverlayMem2D class. The boundary is hidden from the calling 
										functions and is a polymorphism of a OverlayMem2D class. 

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

#include	"OverlayExtMem2D.h"

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

OverlayExtMem2D::OverlayExtMem2D(void* srcPtr, int srcWidth, int srcHeight, 
																							 int width,		 int height,
																							 int bWidth,	 int bHeight):
		OverlayMem2D(srcPtr, srcWidth, srcHeight, width, height)
{
	_bWidth		= bWidth;
	_bHeight	= bHeight;
	// Adjust for the boundary in the base class.
	OverlayMem2D::SetOrigin(bWidth, bHeight);
}//end alt constructor.

OverlayExtMem2D::~OverlayExtMem2D()
{
}//end destructor.

/** Set the top left origin of the block.
This can be in the boundary signified by neg input values. The max input
must be checked against the size of the block.
@param x	: x loc.
@param y	: y loc.
@return		: none.
*/
void OverlayExtMem2D::SetOrigin(int x, int y)
{
	x += _bWidth;

	if(x < 0)
		x = 0;
	else if(x > (_srcWidth - _width))
		x = _srcWidth - _width;
	_xPos = x;

	y += _bHeight;
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

/*
---------------------------------------------------------------------------
	Static utility methods.
---------------------------------------------------------------------------
*/

// Extend the boundary by alloc new mem. This static method must
// be called from the thread that owns the src mem to ensure it is
// thread safe.
int OverlayExtMem2D::ExtendBoundary(void* srcPtr,
																		int srcWidth,	int srcHeight, 
																		int widthBy,	int heightBy,
																		void** dstPtr)
{
	if(srcPtr == NULL)	// Nothing to extend.
		return(0);

	int					y;
	OM2D_TYPE*	lclPtr	= (OM2D_TYPE *)(srcPtr);
	int					width		= srcWidth + 2*widthBy;
	int					height	= srcHeight + 2*heightBy;

	// Create a new mem block to copy into.
	OM2D_TYPE*	newPtr = new OM2D_TYPE[width * height];
	if(newPtr == NULL)
		return(0);

	// Copy the src into the centre section of the new mem.
	for(y = 0; y < srcHeight; y++)
		memcpy((void *)(&(newPtr[(heightBy + y)*width + widthBy])),
					 (void *)(&(lclPtr[y * srcWidth])),
					 srcWidth * sizeof(OM2D_TYPE));

	// Copy the boundary.
	FillBoundary(newPtr, width, height, widthBy, heightBy);

	*dstPtr = (void *)newPtr;

	return(1);
}//end ExtendBoundary.

// The inner block is valid and requires re-filling the boundary.
void OverlayExtMem2D::FillBoundary(void* srcPtr, int srcWidth,int srcHeight, 
																								 int widthBy,	int heightBy)
{
	int	x,y;
	OM2D_TYPE* lclPtr = (OM2D_TYPE *)srcPtr;

	// Fast 2-D extended boundary mem address array.
	OM2D_TYPE** block = new OM2D_PTYPE[srcHeight];
	if(block == NULL)
		return;
	// Fill the row addresses.
	for(y = 0; y < srcHeight; y++)
		block[y] = &(lclPtr[srcWidth * y]);

	// Top left.
	for(y = 0; y < heightBy; y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[heightBy][widthBy];

	// Top right.
	for(y = 0; y < heightBy; y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[heightBy][srcWidth - widthBy - 1];

	// Bottom left.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[srcHeight - heightBy - 1][widthBy];

	// Bottom right.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[srcHeight - heightBy - 1][srcWidth - widthBy - 1];

	// Top.
	for(y = 0; y < heightBy; y++)
		memcpy((void *)(&(block[y][widthBy])),
					 (void *)(&(block[heightBy][widthBy])),
					 (srcWidth - 2*widthBy) * sizeof(OM2D_TYPE));

	// Bottom.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		memcpy((void *)(&(block[y][widthBy])),
					 (void *)(&(block[srcHeight - heightBy - 1][widthBy])),
					 (srcWidth - 2*widthBy) * sizeof(OM2D_TYPE));

	// Left.
	for(y = heightBy; y < (srcHeight - heightBy); y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[y][widthBy];

	// Right.
	for(y = heightBy; y < (srcHeight - heightBy); y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[y][srcWidth - widthBy - 1];

	if(block != NULL)
		delete[] block;

}//end FillBoundary.

void OverlayExtMem2D::FillBoundaryProxy(void)
{
	FillBoundary(_pMem, _srcWidth, _srcHeight, _bWidth, _bHeight);
}//end FillBoundaryProxy.







