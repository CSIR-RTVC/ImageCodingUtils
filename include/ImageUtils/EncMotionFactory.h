/** @file

MODULE						: EncMotionFactory

TAG								: EMF

FILE NAME					: EncMotionFactory.h

DESCRIPTION				: A class to provide a singleton implementation of setting up
										the encoding motion parameters within a video codec. It
										exports a IMotionFactory interface

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2005  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _ENCMOTIONFACTORY_H
#define _ENCMOTIONFACTORY_H

#include "VicsDefs/VicsDefs.h"
#include "IMotionFactory.h"
#include "EncBlockMotionVector.h"
#include "MotionVectorVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class EncMotionFactory: public IMotionFactory
{
public:
	// Singleton instanciation.
	static EncMotionFactory* GetInstance(void)
		{
			if(_instance == NULL) _instance = new EncMotionFactory;
			return(_instance);
		}
	static void RemoveInstance(void) 
		{
			if(_instance) 
				delete _instance;
			_instance=NULL;
		}

public:
	// Interface abstract class implementation.
	VICS_INT						Initialise(VICS_INT pelWidth,VICS_INT pelHeight,VICS_INT blockWidth,VICS_INT blockHeight);
	BlockMotionVector**	GetBlockMotionVectorArray(void) {return(_blockMotionVectorArray);}
	VICS_INT						GetNumMotionVectors(void) {return(_totalBlocks);}
	VICS_INT						GetNumXMotionVectors(void) {return(_numXMotionVectors);}
	VICS_INT						GetNumYMotionVectors(void) {return(_numYMotionVectors);}

protected:
	EncMotionFactory();
	virtual ~EncMotionFactory();

protected:
	void Destroy(void);

protected:
	// Local settings.
	VICS_INT									_totalBlocks;
	VICS_INT									_motionBlockWidth;
	VICS_INT									_motionBlockHeight;
	VICS_INT									_numXMotionVectors;
	VICS_INT									_numYMotionVectors;

	// Aggregated objects required for specific implementations.
	EncBlockMotionVector*			_concreteMotionVectorArray;
	BlockMotionVector**				_blockMotionVectorArray;	// Interface references.
	MotionVectorVlcEncoder*		_vlcEncoder;

private:
	// Singleton instance of this class.
	static EncMotionFactory*	_instance;

};// end class EncMotionFactory.

#endif
