/** @file

MODULE						: DataProfileImplRandom

TAG								: DPIR

FILE NAME					: DataProfileImplRandom.h

DESCRIPTION				: A class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										represents an entirely random process that "sticks" 
										for a selectable number of samples. It implements 
										the more general IDataProfile interface.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _DATAPROFILEIMPLRANDOM_H
#define _DATAPROFILEIMPLRANDOM_H

#pragma once

#include "IDataProfile.h"
#include "DataProfileImplBase.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class DataProfileImplRandom : public DataProfileImplBase, public IDataProfile
{
public:
	DataProfileImplRandom(int granularity);
	virtual ~DataProfileImplRandom(void);

// Interface implementation.
public:
	virtual int		Create(int period, int min, int max);
	virtual void	Destroy(void) { DataProfileImplBase::DestroyMem(); }
	virtual int		GetNext(void) { return(DataProfileImplBase::GetNextSample()); }
	virtual int		Save(char* filename) { return(DataProfileImplBase::Save(filename)); }

private:
	int _granularity; // Num of samples random value "sticks" for.

};// end class DataProfileImplRandom.

#endif	// _DATAPROFILEIMPLRANDOM_H
