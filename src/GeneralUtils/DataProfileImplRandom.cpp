/** @file

MODULE						: DataProfileImplCongest

TAG								: DPIC

FILE NAME					: DataProfileImplCongest.cpp

DESCRIPTION				: A class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										represents  an entirely random process that "sticks" 
										for a selectable number of samples. It implements 
										the more general IDataProfile interface.

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
#include <ctime>
#include <math.h>
#include <stdlib.h>
#include "DataProfileImplRandom.h"

DataProfileImplRandom::DataProfileImplRandom(int granularity)
{
	_granularity = granularity;
}//end constructor.

DataProfileImplRandom::~DataProfileImplRandom(void)
{
	Destroy();
}//end destructor.

int	DataProfileImplRandom::Create(int period, int min, int max)
{
	if(!DataProfileImplBase::CreateMem(period))
		return(0);

	_nextPos = 0;	// Reset to 1st element.

	if(min < 0)
		min = 0;
	if(max <= min)
		max = min + (1000 + min);	// Anything will do.
	_maxSampleVal	= max;
	_minSampleVal	= min;

	// Seed the random variable with the this ptr.
	srand( std::time(NULL) );

	// Fill the array with the random profile.
	int x;
	int seg1end = (int)((0.1 * (double)period) + 0.5);

	// Segment 1: A short initial normal rate period.
	for(x = 0; x < seg1end; x++)
		_pSample[x] = max;

	// Generate a random num and normalise it to (max - min) range.
	for(; x < period; x++)
	{
		int stepEnd = x + _granularity;
		if(stepEnd > period)
			stepEnd = period;
		int n		= (int)(((double)rand()/(double)RAND_MAX) * (double)(max - min));
		int rv	= n + min;
		if(rv > max)
			rv = max;
		else if(rv < min)
			rv = min;
		for(; x < stepEnd; x++)
			_pSample[x] = rv;
		x--; // Prevent skipping in the loop.
	}//end for x...

	return(1);
}//end Create.




