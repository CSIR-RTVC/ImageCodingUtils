/** @file

MODULE						: IExistance

TAG								: IE

FILE NAME					: IExistance.h

DESCRIPTION				: A Microsoft Managed interface that provides a hiding
										mechanism such that a header class does not need to
										know about the implementation.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#pragma once

using namespace System;

__gc __interface IExistance
{
	int			Exists(void);	// Return 1 if object is open for business.
	String* GetErrorStr(void);
};//end IExistance.

