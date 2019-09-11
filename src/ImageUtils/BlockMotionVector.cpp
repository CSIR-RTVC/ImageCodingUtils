/** @file

MODULE						: BlockMotionVector

TAG								: BMV

FILE NAME					: BlockMotionVector.cpp

DESCRIPTION				: A class to describe a base block motion vector.

REVISION HISTORY	:

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

#include	"BlockMotionVector.h"

/*
---------------------------------------------------------------------------
	Static helper methods.
---------------------------------------------------------------------------
*/
BlockMotionVector BlockMotionVector::Median(BlockMotionVector& x,BlockMotionVector& y,BlockMotionVector& z)
{
	// Do the x motion vector coord 1st.
	VICS_INT min,max;
	VICS_INT xc = x.GetMotionX();
	VICS_INT yc = y.GetMotionX();
	VICS_INT zc = z.GetMotionX();

	// min = MIN(xc,MIN(yc,zc)) and max = MAX(xc,MAX(yc,zc)).
	if( (yc - zc) < 0 )	{	min = yc;	max = zc;	}
	else { min = zc; max = yc; }
	if(xc < min) min = xc;
	if(xc > max) max = xc;
	// Median.
	VICS_INT vecx = xc + yc + zc - min - max;

	// Do the y motion vector coord.
	xc = x.GetMotionY();
	yc = y.GetMotionY();
	zc = z.GetMotionY();

	// min = MIN(xc,MIN(yc,zc)) and max = MAX(xc,MAX(yc,zc)).
	if( (yc - zc) < 0 )	{	min = yc;	max = zc;	}
	else { min = zc; max = yc; }
	if(xc < min) min = xc;
	if(xc > max) max = xc;
	// Median.
	VICS_INT vecy = xc + yc + zc - min - max;

	BlockMotionVector result(x);	// As a copy of x.
	result.SetMotion(vecx,vecy);

	return(result);
}//end Median.

