/** @file

MODULE						: DataProfileImplBase

TAG								: DPIC

FILE NAME					: DataProfileImplBase.h

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
#ifndef _DATAPROFILEIMPLBASE_H
#define _DATAPROFILEIMPLBASE_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class DataProfileImplBase
{
public:
	DataProfileImplBase(void);
	virtual ~DataProfileImplBase();

// Base implementations.
public:
	virtual int		CreateMem(int periodInSamples);
	virtual void	DestroyMem(void);
	virtual int		GetNextSample(void);
	virtual int		Save(char* filename);

// Common members.
protected:
	int		_periodInSamples;	// Length of the array.
	int		_nextPos;
	int		_maxSampleVal;
	int		_minSampleVal;
	int*	_pSample;

};// end class DataProfileImplBase.

#endif	// _DATAPROFILEIMPLBASE_H
