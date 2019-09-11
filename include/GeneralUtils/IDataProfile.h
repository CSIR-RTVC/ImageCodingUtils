/** @file

MODULE						: IDataProfile

TAG								: IDP

FILE NAME					: IDataProfile.h

DESCRIPTION				: An interface to DataProfileImplxxx implementations that
										contain access to a periodic function represented by an
										array of integers. 

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _IDATAPROFILE_H
#define _IDATAPROFILE_H

#pragma once

class IDataProfile
{
public:
	virtual ~IDataProfile() {}

// Interface implementation.
public:
	/** Create the mem and set the internal members.
	@param period	: Num of samples in the array that repeats.
	@param min		: Min bound on the array values.
	@param max		: Max bound on array values.
	@return				: 1 = success, 0 = failure.
	*/
	virtual int		Create(int period, int min, int max) = 0;

	/** Destry the mem alloc.
	@return	: none.
	*/
	virtual void	Destroy(void) = 0;

	/** Get the next term in the array.
	Function is periodic so repeats indefinitely.
	@return	: Next value.
	*/
	virtual int		GetNext(void) = 0;

	/** Save the internal function to a text file.
	@param filename	: Save to file name.
	@return					: 1 = success, 0 = failure.
	*/
	virtual int		Save(char* filename) = 0;

};// end class IDataProfile.

#endif	// _IDATAPROFILE_H
