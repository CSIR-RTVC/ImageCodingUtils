/** @file

MODULE						: EncMotionVector

TAG								: EMV

FILE NAME					: EncMotionVector.cpp

DESCRIPTION				: A class to extend the MotionVector class to include members
										suitable for the encoding process.

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

#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "EncMotionVector.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const VICS_INT EncMotionVector::NUM_ESC_BITS	= 5;
const VICS_INT EncMotionVector::ESC_BIT_CODE	= 0x000A;
const VICS_INT EncMotionVector::ESC_LENGTH		= 8;

const VICS_INT EncMotionVector::VLC_TABLE[EMV_TABLE_SIZE][2] =
{
	{ 2 					,	0x0001			 },		//	0   
	{ 3 					, 0x0006			 },		//	1
	{ 3 					, 0x0004			 },  	// -1
	{ 4 					, 0x0000			 },		//	2
	{ 4 					, 0x000b			 },  	// -2
	{ 5 					, 0x0018			 },		//	3
	{ 5 					, 0x0013			 },  	// -3
	{ 6 					, 0x003a			 },		//	4
	{ 6 					, 0x001a			 },  
	{ 6 					, 0x0012			 },		//	5
	{ 6 					, 0x0002			 },  
	{ 6 					, 0x0008			 },		//	6
	{ 6 					, 0x0023			 },  
	{ 6 					, 0x0003			 },		//	7
	{ 6 					, 0x001f			 },  
	{ 6 					, 0x0027			 },		//	8
	{ 7 					, 0x0032			 },  
	{ 7 					, 0x0062			 },		//	9
	{ 7 					, 0x0068			 },  
	{ 7 					, 0x0047			 },		//	10
	{ 8 					, 0x00f2			 },  
	{ 8 					, 0x0072			 },		//	11
	{ 8 					, 0x0022			 },  
	{ 8 					, 0x00a8			 },		//	12
	{ 8 					, 0x0028			 },  
	{ 8 					, 0x0097			 },		//	13
	{ 8 					, 0x0087			 },  
	{ 8 					, 0x0017			 },		//	14
	{ 8 					, 0x0007			 },  
	{ 9 					, 0x01a2			 },		//	15
	{ 9 					, 0x00a2			 },  
	{ 9 					, 0x00ff			 },		//	16
	{ 9 					, 0x0057			 },  
	{ 9 					, 0x0157			 },		//	17
	{ 9 					, 0x00d7			 },  
	{ 9 					, 0x01d7			 },		//	18
	{ 9 					, 0x0037			 },  
	{ 9 					, 0x0137			 },		//	19
	{ 9 					, 0x00b7			 },  
	{ 9 					, 0x01b7			 },		//	20
	{ 9 					, 0x0077			 },  
	{ 9 					, 0x0177			 },		//	21
	{ 9 					, 0x00f7			 },  
	{ 9 					, 0x01f7			 },		//	22
	{ 9 					, 0x000f			 },  
	{ 9 					, 0x010f			 },		//	23
	{ 9 					, 0x008f			 },  
	{ 9 					, 0x018f			 },		//	24
	{ 9 					, 0x004f			 },  
	{ 9 					, 0x014f			 },		//	25
	{ 9 					, 0x00cf			 },  
	{ 9 					, 0x01cf			 },		//	26
	{ 9 					, 0x002f			 },  
	{ 9 					, 0x012f			 },		//	27
	{ 9 					, 0x00af			 },  
	{ 9 					, 0x01af			 },		//	28
	{ 9 					, 0x006f			 },  
	{ 9 					, 0x016f			 },		//	29
	{ 9 					, 0x00ef			 },  
	{ 9 					, 0x01ef			 },		//	30
	{ 9 					, 0x003f			 },  
	{ 9 					, 0x013f			 },		//	31
	{ 9 					, 0x00bf			 },  
	{ 9 					, 0x01bf			 },		//	32
	{ 9						,	0x007f			 }
};

/*
---------------------------------------------------------------------------
	Public Interface.
---------------------------------------------------------------------------
*/
VICS_INT EncMotionVector::EncodeVector(EncMotionVector& emv)
{
  VICS_INT xbits,ybits;
  VICS_INT xcode = 0;
  VICS_INT ycode = 0;

  xbits = GetMotionVecBits(emv.GetMotionX(),&xcode);
  if(!xbits)
    return(0);

  ybits = GetMotionVecBits(emv.GetMotionY(),&ycode);
  if(!ybits)
    return(0);

  emv.SetBitCode(xcode | (ycode << xbits));
	emv.SetNumCodedBits(xbits + ybits);
  return(xbits + ybits);
}//end GetMotionVecBits.

/*
---------------------------------------------------------------------------
	Private Methods.
---------------------------------------------------------------------------
*/
VICS_INT EncMotionVector::GetMotionVecBits(VICS_INT VecCoord,VICS_INT *CdeWord)
{
  VICS_INT abs_motion,bits;
  VICS_INT code;
	VICS_INT neg;

	neg = 0;
  if(VecCoord <= 0)
    abs_motion = -(VecCoord);
  else
	{
    abs_motion = VecCoord;
		neg = 1;
	}//end else...

  VICS_INT pos;
	if ((abs_motion * 2) < EMV_TABLE_SIZE)
	{
		pos = (abs_motion * 2) - neg;  
		bits = VLC_TABLE[pos][EMV_NUM_BITS];
		code = VLC_TABLE[pos][EMV_BIT_CODE];
	}//end if abs_motion...
	else
	{
		pos = VecCoord + 128;
		if (pos > 255)
			return(0);

		bits = NUM_ESC_BITS + ESC_LENGTH;
		code = (pos << NUM_ESC_BITS) | ESC_BIT_CODE;
	}//end else...
	
	*CdeWord = code;
	return(bits);
}//end GetMotionVecBits.

