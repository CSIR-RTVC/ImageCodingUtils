/** @file

MODULE						: DataProfileImplBase

TAG								: DPIC

FILE NAME					: DataProfileImplBase.cpp

DESCRIPTION				: A base class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										defines the common methods and members.
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

#include <stdio.h>
#include "DataProfileImplBase.h"

DataProfileImplBase::DataProfileImplBase(void)
{
	_periodInSamples	= 0;	// Length of the array.
	_nextPos					= 0;
	_maxSampleVal			= 0;
	_minSampleVal			= 0;
	_pSample					= NULL;
}//end constructor.

DataProfileImplBase::~DataProfileImplBase(void)
{
	DestroyMem();
}//end destructor.

int DataProfileImplBase::CreateMem(int periodInSamples)
{
	DestroyMem();
	_periodInSamples = periodInSamples;
	_pSample = new int[periodInSamples];
	if(_pSample == NULL)
		return(0);
	return(1);
}//end CreateMem.

void DataProfileImplBase::DestroyMem(void)
{
	if(_pSample != NULL)
		delete[] _pSample;
	_pSample = NULL;

	_periodInSamples	= 0;
	_nextPos					= 0;
}//end DestroyMem.

int DataProfileImplBase::GetNextSample(void)
{
	if(_nextPos < _periodInSamples)
		return(_pSample[_nextPos++]);

	// else wrap around.
	_nextPos = 0;
	return(_pSample[_nextPos++]);

}//end GetNextSample.

int	DataProfileImplBase::Save(char* filename)
{
  if( (_periodInSamples == 0) || (_pSample == NULL) )
    return(0);

	FILE*	stream = fopen(filename, "w");
  if(stream == NULL)
    return(0);

  // Store results.
  for(int i = 0; i < _periodInSamples; i++)
		fprintf(stream, "%d,%d\n", i, _pSample[i]);

	fclose(stream);
	return(1);
}//end Save.


