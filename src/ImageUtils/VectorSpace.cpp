/*
===========================================================================
MODULE          : VectorSpace

TAG             : VS

FILE NAME       : VectorSpace.cpp

DESCRIPTIONS    : An object to provide suitable loading and sampling of a 
									vector space.

                  
REVISION HISTORY: 
                : $Log: VectorSpace.cpp,v $
                : Revision 1.1  2004/08/18 08:03:13  keithf
                : A vector space utility used for vector quantisation training applications.
                : Status: Build-18-08-2004
                :

COPYRIGHT       : (c)VICS 2002  all rights resevered - info@videocoding.com
===========================================================================
*/

#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include "VectorSpace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
----------------------------------------------------------------------------
	Definitions and constants.
----------------------------------------------------------------------------
*/
typedef double* doublePtr;

#define VS_ZERO_ELEMENT		(double)0.00001

/*
----------------------------------------------------------------------------
	Construction and destruction.
----------------------------------------------------------------------------
*/

VectorSpace::VectorSpace(void)
{
	/** Properties.*/
//	_hSpace				= NULL;
  _pSpace				= NULL;							// Vector space pointer.
//  _Space				= NULL;							// Vector address space pointer.
	_length				= 0;								// Number of vectors in the vector space.
	_vectorDim		= 0;								// Dimension of the vectors.
	_zeroElement	= VS_ZERO_ELEMENT;	// Value to consider as zero.

	/** Lingering status members.*/
  _lastPos			= 0;
	_lastError		= "VectorSpace: No error";

  /** Seed the random number generator.*/
  _irandom = (unsigned)time(NULL);

}//end default constructor.

VectorSpace::~VectorSpace(void)
{
	Destroy();
}//end destructor.

VectorSpace::VectorSpace(int length, int vectorDim)
{
	/** Properties.*/
//	_hSpace				= NULL;
  _pSpace				= NULL;							// Vector space pointer.
//  _Space				= NULL;							// Vector address space pointer.
	_length				= length;						// Number of vectors in the vector space.
	_vectorDim		= vectorDim;				// Dimension of the vectors.
	_zeroElement	= VS_ZERO_ELEMENT;	// Value to consider as zero.

	/** Lingering status members.*/
  _lastPos			= 0;
	_lastError		= "VectorSpace: No error";

  /** Seed the random number generator.*/
  _irandom = (unsigned)time(NULL);

	Create();

}//end constructor.

int	VectorSpace::Create(void)
{
//	if(_hSpace != NULL)
//		Destroy();
//
//	int size = _length * _vectorDim * sizeof(double);
//
//	_hSpace = GlobalAlloc(GMEM_FIXED,(_length * _vectorDim * sizeof(double)));
//	if(_hSpace == NULL)
//  {
//		_lastError	= "VectorSpace: Unable to allocate vector space memory";
//	  return(0);
//  }//end if !_hSpace...
//	_pSpace = (double *)GlobalLock(_hSpace);


	if(_pSpace != NULL)
		Destroy();

	_pSpace = new double[_length * _vectorDim];
	if(_pSpace == NULL)
	{
		_lastError	= "VectorSpace: Unable to allocate vector space memory";
		return(0);
	}//end if !_pSpace...

//	_Space = new doublePtr[_length];
//	if(_Space == NULL)
//	{
//		_lastError	= "VectorSpace: Unable to allocate vector space address memory";
//		Destroy();
//		return(0);
//	}//end if !_Space...
//
//	/** Load the address space and zero the data.*/
//	int i,elements;
//
//	for(i = 0; i < _length; i++)
//		_Space[i] = &(_pSpace[i * _vectorDim]);
//
//	elements = _length * _vectorDim;
//	for(i = 0; i < elements; i++)
//		_pSpace[i] = 0.0;

	return(1);
}//end Create.

void VectorSpace::Destroy(void)
{
	/** Remove the memory allocations.*/
//  if(_hSpace != NULL)
//  {
//		GlobalUnlock(_hSpace);
// 		GlobalFree(_hSpace);
//  }//end if _hSpace...
// 	_hSpace = NULL;
//  _pSpace = NULL;


//	if(_Space != NULL)
//		delete[] _Space;
//	_Space = NULL;

	if(_pSpace != NULL)
		delete[] _pSpace;
	_pSpace = NULL;

}//end Destroy.

/*
----------------------------------------------------------------------------
	Vector space public methods.
----------------------------------------------------------------------------
*/

int	VectorSpace::GetVector(double *vector, int position)
{
	if(_pSpace == NULL)
		return(0);

	for(int i = 0; i < _vectorDim; i++)
		vector[i] = _pSpace[position*_vectorDim + i];

	_lastPos = position;

	return(1);
}//end GetVector.

int	VectorSpace::PutVector(double *vector, int position)
{
	if(_pSpace == NULL)
		return(0);

	for(int i = 0; i < _vectorDim; i++)
		_pSpace[position*_vectorDim + i] = vector[i];

	_lastPos = position;

	return(1);
}//end PutVector.

int	VectorSpace::GetRandomVector(double *vector)
{
	if(_pSpace == NULL)
		return(0);

	int position = GetRandomPosition();

	for(int i = 0; i < _vectorDim; i++)
		vector[i] = _pSpace[position*_vectorDim + i];

	_lastPos = position;

	return(1);
}//end GetRandomVector.

int	VectorSpace::GetNonZeroRandomVector(double *vector)
{
	if(_pSpace == NULL)
		return(0);

	int zeroFound = 1;
	int loopCount = 0;
	int i,position;

	while( zeroFound && (loopCount < _length) )
	{

		position = GetRandomPosition();
		for(i = 0; i < _vectorDim; i++)
		{
			vector[i]	= _pSpace[position*_vectorDim + i];
			if(_pSpace[position*_vectorDim + i] > _zeroElement)
				zeroFound = 0;
		}//end for i...

		loopCount++;
	}//end while zeroFound...

	_lastPos = position;

	return(1);
}//end GetNonZeroRandomVector.

int	VectorSpace::VectorIsZero(double *vector)
{
	int i;

	for(i = 0; i < _vectorDim; i++)
	{
		if(vector[i] > _zeroElement) // Not zero.
			return(0);
	}//end for i...

	return(1); 
}//end VectorIsZero.

double VectorSpace::GetVectorEnergy(double *vector)
{
	double energy = 0.0;

	for(int i = 0; i < _vectorDim; i++)
		energy += (vector[i] * vector[i]);

	return(energy);
}//end GetVectorEnergy.

/*
----------------------------------------------------------------------------
	Vector space protected methods.
----------------------------------------------------------------------------
*/

int VectorSpace::GetRandomPosition(void)
{
  int position = (int)( 0.5 + ((double)(_length - 1) * GetRandomNumber()) );

	return(position);

}//end GetRandomPosition.

#define VS_IA				16807
#define VS_IM				2147483647
#define VS_AM				(1.0/(double)VS_IM)
#define VS_IQ				127773
#define VS_IR				2836
#define VS_MASK			123459876

double VectorSpace::GetRandomNumber(void)
{
	long int k;
	double ans;

	_irandom ^= VS_MASK;

	k = _irandom/VS_IQ;

	_irandom = ( VS_IA * (_irandom - (k * VS_IQ)) ) - (VS_IR * k);

	if(_irandom < 0)
		_irandom += VS_IM;

	ans = VS_AM * (double)_irandom;

	_irandom ^= VS_MASK;

	return(ans);
}//end GetRandomNumber.


