/*
===========================================================================
MODULE          : VectorSpace

TAG             : VS

FILE NAME       : VectorSpace.h

DESCRIPTIONS    : An object to provide suitable loading and sampling of a 
									vector space.

                  
REVISION HISTORY: 
                : $Log: VectorSpace.h,v $
                : Revision 1.1  2004/08/18 08:02:53  keithf
                : A vector space utility used for vector quantisation training applications.
                : Status: Build-18-08-2004
                :

COPYRIGHT       : (c)VICS 2002  all rights resevered - info@videocoding.com
===========================================================================
*/

#ifndef _VECTORSPACE_H
#define _VECTORSPACE_H

#include <math.h>

class VectorSpace
{
private:
	long int _irandom;	// Random generator id.

protected :

	/** Properties of the space.*/
  double*		_pSpace;				// Vector space pointer.
	int				_length;				// Number of vectors in the vector space.
	int				_vectorDim;			// Dimension of the vectors.
	double		_zeroElement;		// Value to consider as zero.

	/** Lingering status members.*/
  int			_lastPos;
	char*		_lastError;

	/** Local service functions.*/
  int			Create(void);
  void		Destroy(void);
	int			GetRandomPosition(void);
	double	GetRandomNumber(void);

public :
	VectorSpace(void);
	VectorSpace(int length, int vectorDim);
	~VectorSpace(void);

  double*				GetSpacePtr(void)							{ return(_pSpace); }
	int						GetLength(void)								{ return(_length); }
	int						GetVectorDimension(void)			{ return(_vectorDim); }
	double				GetZeroElement(void)					{ return(_zeroElement); }
	void					SetZeroElement(double zero)		{ _zeroElement = zero; }
	int						GetLastSampledPosition(void)	{ return(_lastPos); }
	const char*	 	GetLastError(void)						{ return(_lastError); }

  int						GetVector(double *vector, int position);
  int						PutVector(double *vector, int position);
  int						GetRandomVector(double *vector);
  int						GetNonZeroRandomVector(double *vector);

	int						VectorIsZero(double *vector);
	double				GetVectorEnergy(double *vector);

};//end VectorSpace class

#endif


