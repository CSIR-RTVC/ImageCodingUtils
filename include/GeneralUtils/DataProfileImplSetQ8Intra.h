/** @file

MODULE						: DataProfileImplSetQ8Intra

TAG								: DPISQ8I

FILE NAME					: DataProfileImplSetQ8Intra.h

DESCRIPTION				: A class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										represents the bit rate of H.263+ Advanced Intra mode
										operating at QP=8 with every frame set to an I-frame.
										It implements the more general IDataProfile interface.

REVISION HISTORY	:

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _DATAPROFILEIMPLSETQ8INTRA_H
#define _DATAPROFILEIMPLSETQ8INTRA_H

#pragma once

#include "IDataProfile.h"
#include "DataProfileImplBase.h"

/**
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class DataProfileImplSetQ8Intra : public DataProfileImplBase, public IDataProfile
{
public:
	DataProfileImplSetQ8Intra(void);
	virtual ~DataProfileImplSetQ8Intra(void);

/// Interface implementation.
public:
	virtual int		Create(int period, int min, int max);
	virtual void	Destroy(void) { DataProfileImplBase::DestroyMem(); }
	virtual int		GetNext(void) { return(DataProfileImplBase::GetNextSample()); }
	virtual int		Save(char* filename) { return(DataProfileImplBase::Save(filename)); }

private:
	/// Constant array storing the profile.
	static const int	_pProfile[];

};// end class DataProfileImplSetQ8Intra.

#endif	///< _DATAPROFILEIMPLSETQ8INTRA_H
