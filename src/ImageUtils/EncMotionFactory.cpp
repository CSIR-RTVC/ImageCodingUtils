/** @file

MODULE						: EncMotionFactory

TAG								: BMF

FILE NAME					: EncMotionFactory.cpp

DESCRIPTION				: Implement the required concrete classes for for
										all the motion encoding functions.

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

#include	"EncMotionFactory.h"

/*
---------------------------------------------------------------------------
	Protected construction and deletion.
---------------------------------------------------------------------------
*/

// Default setting.
EncMotionFactory*	EncMotionFactory::_instance = NULL;

EncMotionFactory::EncMotionFactory()
{
	_totalBlocks				= 0;
	_motionBlockWidth		= 0;
	_motionBlockHeight	= 0;
	_numXMotionVectors	= 0;
	_numYMotionVectors	= 0;
	// Required concrete objects.
	_concreteMotionVectorArray	= NULL;
	_blockMotionVectorArray			= NULL;
	_vlcEncoder						 			= NULL;
}//end constructor.

EncMotionFactory::~EncMotionFactory()
{
	Destroy();
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface implementation.
---------------------------------------------------------------------------
*/
VICS_INT EncMotionFactory::Initialise(VICS_INT pelWidth,VICS_INT pelHeight,VICS_INT blockWidth,VICS_INT blockHeight)
{
	_motionBlockWidth		= blockWidth;
	_motionBlockHeight	= blockHeight;
	// No divide by zero errors.
	if( (blockWidth==0)||(blockHeight==0) )
		return(0);

	_numXMotionVectors	= pelWidth/blockWidth;
	_numYMotionVectors	= pelHeight/blockHeight;
	_totalBlocks				= _numXMotionVectors * _numYMotionVectors;
	// Must be exact multiple.
	if( (pelWidth%blockWidth)||(pelHeight%blockHeight) )
		return(0);

	// If previously called then destroy first.
	if(_concreteMotionVectorArray != NULL) // Use this as the indicator.
		Destroy();

	// Create the array of motion vector references and set the motion vectors 
	// to a left to right and top to bottom scan order.
	_concreteMotionVectorArray	= new EncBlockMotionVector[_totalBlocks];
	_blockMotionVectorArray			= new BlockMotionVector*[_totalBlocks];
	if(!_blockMotionVectorArray || !_concreteMotionVectorArray)
	{
		Destroy();
		return(0);
	}//end if !_blockMotionVectorArray...

	VICS_INT x,y;
	VICS_INT block = 0;
	for(y = 0; y < pelHeight; y += blockHeight)
		for(x = 0; x < pelWidth; x += blockWidth, block++)
		{
			_blockMotionVectorArray[block] = &_concreteMotionVectorArray[block];
			_blockMotionVectorArray[block]->Set(0,0,blockWidth,blockHeight,x,y);
		}//end for y & x...

	// Encapsulated algorithms.
	// Create a motion vector vlc encoder.
	_vlcEncoder = NULL;
	_vlcEncoder = new MotionVectorVlcEncoder();
	if(!_vlcEncoder)
	{
		Destroy();
		return(0);
	}//end if !_vlcEncoder...
	// Set all encoder motion vectors to use it.
	EncBlockMotionVector::SetVlcEncoder(_vlcEncoder);

	return(1);	// Success.
}//end Initialise.

/*
---------------------------------------------------------------------------
	Private methods.
---------------------------------------------------------------------------
*/
void EncMotionFactory::Destroy(void)
{
	if(_blockMotionVectorArray)
		delete[] _blockMotionVectorArray;
	_blockMotionVectorArray = NULL;

	if(_concreteMotionVectorArray)
		delete[] _concreteMotionVectorArray;
	_concreteMotionVectorArray = NULL;

	if(_vlcEncoder)
		delete _vlcEncoder;
	_vlcEncoder = NULL;

}//end Destroy.



