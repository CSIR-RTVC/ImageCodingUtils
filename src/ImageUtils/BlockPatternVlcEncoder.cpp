/** @file

MODULE						: BlockPatternVlcEncoder

TAG								: BPVE

FILE NAME					: BlockPatternVlcEncoder.cpp

DESCRIPTION				: A class to implement a block pattern vlc encoder derived
										from an IVlcEncoder interfac..

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

#include "BlockPatternVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const VICS_INT BlockPatternVlcEncoder::VLC_TABLE[BPVE_TABLE_SIZE][2] =
{
	{  1,	0x0001  },	//	BlockPatternType::SINGLE  
	{  3,	0x0002  },	//	BlockPatternType::HORIZ  
	{  3,	0x0004  },	//	BlockPatternType::VERT  
	{  3,	0x0006  }		//	BlockPatternType::QUAD
};

