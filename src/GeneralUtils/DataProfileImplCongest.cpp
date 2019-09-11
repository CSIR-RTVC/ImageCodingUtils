/** @file

MODULE						: DataProfileImplCongest

TAG								: DPIC

FILE NAME					: DataProfileImplCongest.cpp

DESCRIPTION				: A class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										represents the effective bit rate as a network enters
										and exits a congested period. It implements the more
										general IDataProfile interface.

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

#include <math.h>
#include <stdlib.h>
#include "DataProfileImplCongest.h"

DataProfileImplCongest::DataProfileImplCongest()
{
}//end constructor.

DataProfileImplCongest::~DataProfileImplCongest()
{
	Destroy();
}//end destructor.

int	DataProfileImplCongest::Create(int period, int min, int max)
{
	if(!DataProfileImplBase::CreateMem(period))
		return(0);

	_nextPos			= 0;
	_maxSampleVal	= max;
	_minSampleVal	= min;
//	srand( (unsigned int)this & 0x0000FFFF);

	// Fill the array with the profile. This congestion profile consists of 6 segments.
	int x;
	int seg1end = (int)((0.25 * (double)period) + 0.5);
	int seg2end = (int)((0.35 * (double)period) + 0.5);
	int seg3end = (int)((0.375 * (double)period) + 0.5);
	int seg4end = (int)((0.5 * (double)period) + 0.5);
	int seg5end = (int)((0.875 * (double)period) + 0.5);

	// Segment 1: Normal rate.
	for(x = 0; x < seg1end; x++)
		_pSample[x] = max;

	// Segment 2: Slow decay as congestion begins.
	// Decay from max to 0.9*(max - min).
	double m = (0.1 * (double)(min - max))/(double)(seg2end - seg1end);
	for(; x < seg2end; x++)
	{
		double normx = (double)(x - seg1end);
		_pSample[x] = (int)(m*normx) + max;
	}//end for x...

	// Segment 3: Rate collapse to min as congestion takes hold.
	int c = _pSample[x-1];
	m = (double)(min - c)/(double)(seg3end - seg2end);
	for(; x < seg3end; x++)
	{
		double normx = (double)(x - seg2end);
		_pSample[x] = (int)(m*normx) + c;
	}//end for x...

	// Segment 4: Congested period.
	for(; x < seg4end; x++)
		_pSample[x] = min;

	// Segment 5 : Gradual recovery.
	m = (double)(max - min)/(double)(seg5end - seg4end);
	for(; x < seg5end; x++)
	{
		double normx = (double)(x - seg4end);
		_pSample[x] = (int)(m*normx) + min;
	}//end for x...

	// Segment 6: Normal recovered rate.
	for(; x < period; x++)
		_pSample[x] = max;

	return(1);
}//end Create.


