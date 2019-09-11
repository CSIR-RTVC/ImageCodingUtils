/** @file

MODULE						: EncBlockMotionVector

TAG								: EBMV

FILE NAME					: EncBlockMotionVector.cpp

DESCRIPTION				: Implement the required concrete classes for for
										all the motion encoding functions.

REVISION HISTORY	:

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

#include	"EncBlockMotionVector.h"

/*
---------------------------------------------------------------------------
	Protected construction and deletion.
---------------------------------------------------------------------------
*/

// Default setting.
IVlcEncoder*	EncBlockMotionVector::_vlcEncoder = NULL;



