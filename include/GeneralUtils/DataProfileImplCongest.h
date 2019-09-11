/** @file

MODULE						: DataProfileImplCongest

TAG								: DPIC

FILE NAME					: DataProfileImplCongest.h

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
#ifndef _DATAPROFILEIMPLCONGEST_H
#define _DATAPROFILEIMPLCONGEST_H

#pragma once

#include "IDataProfile.h"
#include "DataProfileImplBase.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class DataProfileImplCongest : public DataProfileImplBase, public IDataProfile
{
public:
	DataProfileImplCongest(void);
	virtual ~DataProfileImplCongest(void);

// Interface implementation.
public:
	virtual int		Create(int period, int min, int max);
	virtual void	Destroy(void) { DataProfileImplBase::DestroyMem(); }
	virtual int		GetNext(void) { return(DataProfileImplBase::GetNextSample()); }
	virtual int		Save(char* filename) { return(DataProfileImplBase::Save(filename)); }

};// end class DataProfileImplCongest.

#endif	// _DATAPROFILEIMPLCONGEST_H
