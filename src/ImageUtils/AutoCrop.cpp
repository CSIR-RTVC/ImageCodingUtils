/** @file

MODULE						: AutoCrop

TAG								: AC

FILE NAME					: AutoCrop.cpp

DESCRIPTION				: A class to automatically find the dark boundaries of an
										image.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#include "stdafx.h"
#include <memory.h>
#include <stdlib.h>

#include "AutoCrop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
---------------------------------------------------------------------------
	Construction/Destruction
---------------------------------------------------------------------------
*/
AutoCrop::AutoCrop()
{
	_marginLeft			= 0;
	_marginRight		= 0;
	_marginTop			= 0;
	_marginBottom		= 0;

	_width					= 0;
	_height					= 0;
	_pLeftRightAcc	= NULL;
	_pTopBottomAcc	= NULL;

	_multiple				= 1;
}//end constructor.

AutoCrop::~AutoCrop()
{
	// Close the door behind you.
	Destroy();
}//end destructor.

/** Allocate memory for projection mapping.
A memory accumulator is required for for each dimension of the image. If
this method is called and accumulators already exist then they are deleted
before reallocating. 

@param		w	: Image width.

@param		h	: Image height.

@return 		: 0 = mem alloc failure, 1 = success.
*/
VICS_INT AutoCrop::Create(VICS_INT w,VICS_INT h)
{
	// Clear out residue.
	if((_pLeftRightAcc != NULL)||(_pTopBottomAcc != NULL))
		Destroy();

	// Alloc mem for 1 row.
	_pLeftRightAcc = new VICS_INT32[w];
	if(_pLeftRightAcc == NULL)
		return(0);

	// Alloc mem for 1 col.
	_pTopBottomAcc = new VICS_INT32[h];
	if(_pTopBottomAcc == NULL)
	{
		Destroy();
		return(0);
	}//end if _pTopBottomAcc...

	// Successful alloc.
	_width	= w;
	_height	= h;

	// Zero accumulators.
	VICS_INT i;
	for(i = 0; i < _width; i++)
		_pLeftRightAcc[i] = 0;
	for(i = 0; i < _height; i++)
		_pTopBottomAcc[i] = 0;

	return(1);
}//end Create.

/** Delete the accumulators.
Deallocate the memory and clear the image dimensions.

@return 		: none.
*/
void AutoCrop::Destroy(void)
{
	if(_pLeftRightAcc != NULL)
		delete[] _pLeftRightAcc;
	_pLeftRightAcc = NULL;

	if(_pTopBottomAcc != NULL)
		delete[] _pTopBottomAcc;
	_pTopBottomAcc = NULL;

	_width				= 0;
	_height				= 0;

	_marginLeft	 	= 0;
	_marginRight 	= 0;
	_marginTop	 	= 0;
	_marginBottom	= 0;

}//end Destroy.

/*
---------------------------------------------------------------------------
	Public interface.
---------------------------------------------------------------------------
*/

/** Reset the image accumulator.
Every time ProcessImage() is called it is added to the internal image
accumulator for better edge detection. This method deletes the accumulaors.

@return 			: none.
*/
void AutoCrop::Reset()
{
	Destroy();
}//end Reset.

/** Approximate the boundary locations of the input image.
An image accumulator is created, if one does not exist, and loaded with
appropriate representations from the input image for boundary approximation.
If an accumulator exists then the input image representation is added to it
and the boundary approximation is repeated. Note: The input image must be
in RGB24 format. The crop is adjusted to ensure multiples of the _multiple
member of pels in both dimensions.

@param pBmih	: Input image pointer in bmp format. 

@return 			: 0 = failure, 1 = success.
*/
VICS_INT AutoCrop::ProcessImage(BITMAPINFOHEADER *pBmih)
{
	DWORD bitmapBitsOffset = pBmih->biSize + pBmih->biClrUsed * sizeof(RGBQUAD);

	// Only accept uncompressed RGB formats.
	if( (pBmih->biCompression != BI_RGB) &&	(pBmih->biCompression != BI_BITFIELDS))
	{
	  return(0);
	}//end if biCompression...

	//	Determine the palette colour size.
	VICS_INT paletteSize;
	if(pBmih->biCompression == BI_RGB)
	{
		if((pBmih->biClrUsed == 0)&&(pBmih->biBitCount <= 8))
			paletteSize = (VICS_INT)((1 << pBmih->biBitCount) * sizeof(RGBQUAD));
		else
			paletteSize = (VICS_INT)(pBmih->biClrUsed * sizeof(RGBQUAD));
	}
	else //if pBmih->biCompression == BI_BITFIELDS
	{
		paletteSize = (VICS_INT)(3 * sizeof(DWORD));
	}//end else...

	// Ensure the image size in bytes is valid. Cater for scan line DWORD multiples.
	if(pBmih->biSizeImage == 0)
	{
		pBmih->biSizeImage = (DWORD)(((((pBmih->biWidth * pBmih->biBitCount) + 31)/32) * 4) * abs(pBmih->biHeight));
	}//end if biSizeImage...

	// Locate the image pel data.
	VICS_PBYTE bmptr = (VICS_PBYTE)((VICS_PBYTE)pBmih + pBmih->biSize + paletteSize);

	// Decide here on what image formats are valid.
	if(pBmih->biBitCount != 24)
		return(0);

	VICS_INT lclWidth		= (VICS_INT)(pBmih->biWidth);
	VICS_INT lclHeight	= (VICS_INT)(abs(pBmih->biHeight));

	// Create if the dimensions do not match.
	if( (lclWidth != _width)||(lclHeight != _height) )
	{
		if(!Create(lclWidth,lclHeight))
		{
			Destroy();
			return(0);
		}//end if !Create...
	}//end if lclWidth...

	// Main loop of squaring the lum value and accumulating.
	VICS_INT row,col;
	VICS_INT normWidthSize = ((((lclWidth * pBmih->biBitCount) + 31)/32) * 4);
	for(row = 0; row < _height; row++)
	{
		VICS_PBYTE bmpRow = &(bmptr[normWidthSize * row]);
		for(col = 0; col < _width; col++)
		{
			VICS_DOUBLE b = (VICS_DOUBLE)(*bmpRow++);
			VICS_DOUBLE g = (VICS_DOUBLE)(*bmpRow++);
			VICS_DOUBLE r = (VICS_DOUBLE)(*bmpRow++);
			VICS_INT32 y = (VICS_INT32)((b * 0.114) + (g * 0.587) + (r * 0.299) + 0.5);

			_pLeftRightAcc[col] += (y * y);
			_pTopBottomAcc[row] += (y * y);
		}//end for col...
	}//end for row...

	_marginLeft		= Find1stEdge(_pLeftRightAcc,_width);
	_marginRight	= (_width - 1) - FindLastEdge(_pLeftRightAcc,_width);
	_marginBottom = Find1stEdge(_pTopBottomAcc,_height);
	_marginTop		= (_height - 1) - FindLastEdge(_pTopBottomAcc,_height);

	// Adjust the cropping to ensure multiples of _multiple pels.
	VICS_INT newWidth		= _width  - (_marginLeft	 + _marginRight);
	VICS_INT newHeight	= _height - (_marginBottom + _marginTop);

	VICS_INT extra = newWidth % _multiple;
	if(extra)
	{
		_marginLeft		+= (extra/2); 
		_marginRight	+= (extra - (extra/2));
	}//end if extra...

	// For top and bottom all but 2 rows are to be cropped from the bottom. Image
	// scene information is largely in the top 1/3 of most sequences.
	extra = newHeight % _multiple;
	if(extra)
	{
		if(extra > 4)
		{
			_marginTop		+= 2; 
			_marginBottom	+= (extra - 2);
		}//end if extra...
		else
		{
			_marginTop		+= (extra/2); 
			_marginBottom	+= (extra - (extra/2));
		}//end else...
	}//end if extra...

	return(1);
}//end ProcessImage.

/*
---------------------------------------------------------------------------
	Private methods.
---------------------------------------------------------------------------
*/

/** Find the 1st edge in an array.
Use a Sobel edge detection filter to find the post-crest of the edge.

@param		x			: Input array.

@param		len		: Input array length.

@return 				: Edge position.
*/
VICS_INT AutoCrop::Find1stEdge(VICS_PINT32 x,VICS_INT len)
{
	VICS_INT pos;

	// Start with zero position.
	VICS_INT edgePos = 0;

	VICS_INT32 min = x[0];
	VICS_INT32 max = x[0];
	for(pos = 1; pos < len; pos++)
	{
		if(x[pos] < min)
			min = x[pos];
		if(x[pos] > max)
			max = x[pos];
	}//end for pos...

	// Test if the edge is dark. Measured as 1/10 of max.
	if(x[edgePos] > (max/10))
		return(edgePos);

	VICS_INT32 black	= max/12;
	VICS_INT32 dx[3]	= {0,2*x[edgePos],0};
	VICS_INT	 found	= 0;
	pos	= 1;
	while( (!found) && (pos < (len - 1)) )
	{
		dx[0] = (-2 * x[pos-1]) + (2 * x[pos+1]);
		if((dx[1] > dx[2]) && (dx[1] > dx[0]) && (x[pos] > black))
		{
			edgePos = pos;
			found   = 1;
		}//end if diff...

		dx[2] = dx[1];
		dx[1] = dx[0];
		pos++;
	}//end while !found...

	return(edgePos);
}//end Find1stEdge.
 
/** Find the last edge in an array.
Use a Sobel edge detection filter to find the pre-crest of the edge.

@param		x			: Input array.

@param		len		: Input array length.

@return 				: Edge position.
*/
VICS_INT AutoCrop::FindLastEdge(VICS_PINT32 x,VICS_INT len)
{
	VICS_INT pos;

	// Start with zero position.
	VICS_INT edgePos = len - 1;

	VICS_INT32 min = x[0];
	VICS_INT32 max = x[0];
	for(pos = 1; pos < len; pos++)
	{
		if(x[pos] < min)
			min = x[pos];
		if(x[pos] > max)
			max = x[pos];
	}//end for pos...

	// Test if the edge is dark. Measured as 1/10 of max.
	if(x[edgePos] > (max/10))
		return(edgePos);

	VICS_INT32 black	= max/12;
	VICS_INT32 dx[3]	= {0,2*x[edgePos],0};
	VICS_INT	 found	= 0;
	pos	= len - 2;
	while( (!found) && (pos > 0) )
	{
		dx[0] = (-2 * x[pos+1]) + (2 * x[pos-1]);
		if((dx[1] > dx[2]) && (dx[1] > dx[0]) && (x[pos] > black))
		{
			edgePos = pos;
			found   = 1;
		}//end if diff...

		dx[2] = dx[1];
		dx[1] = dx[0];
		pos--;
	}//end while !found...

	return(edgePos);
}//end FindLastEdge.




