/** @file

MODULE						: MotionVector

TAG								: MV

FILE NAME					: MotionVector.cpp

DESCRIPTION				: A class to describe a base motion vector.

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


#include	"MotionVector.h"

/*
---------------------------------------------------------------------------
	Static methods.
---------------------------------------------------------------------------
*/
MotionVector MotionVector::Median(MotionVector& x,MotionVector& y,MotionVector& z)
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

	MotionVector result;
	result.SetMotion(vecx,vecy);

	return(result);
}//end Median.

