/*
===========================================================================
MODULE          : SampleSet

TAG             : SSET

FILE NAME       : SampleSet.h

DESCRIPTIONS    : A class for implementing a random vector 
		              sampler for use in training SOMs.

                  
REVISION HISTORY: 
                : $Log: SampleSet.h,v $
                : Revision 1.1  2004/08/18 08:03:55  keithf
                : A sample space utility used for scalar and vector quantisation training
                : applications.
                : Status: Build-18-08-2004
                :

COPYRIGHT       : (c)VICS 2001  all rights resevered - info@videocoding.com
===========================================================================
*/
#ifndef _SAMPLESET_H
#define _SAMPLESET_H

class SampleSet
{
protected :
	// Sample set vector parameters.
	int				Vector_dim;
	int				NumVectors;
  double*		pVector;
  double**	Vector;

  int				LastVector;	//Index of the previous vector selected.

  int				Empty;
  int				error;

protected :
  int		OverwriteSample(double *vec,int loc);
  int		CreateMemOnCurrParams(void);
  void	DestroyMem(void);

public :
	SampleSet(void);
	~SampleSet(void);

	SampleSet		&operator=(SampleSet &S);
	int					Open(int vec_dim,int num_vectors);
	int					Open(const char* TestSetFilename);
	void				Close(void);

  double			*GetSetPtr(void) {return(pVector);}
  double			**GetSet(void) {return(Vector);}

  int					TakeRandomSample(double *vec);
  int					TakeNonZeroRandomSample(double *vec);
  int					TakeSample(double *vec,int loc);

  int					GetVectorDim(void) {return(Vector_dim);}
  int					GetTotalNumVectors(void) {return(NumVectors);}
  int					GetLastVectorIndex(void) {return(LastVector);}

	int					Error(void) {int reterr = error;error = 0;return(reterr);}
	const char	*GetErrorStr(int ErrorNum, char *ErrStr);

};//end SampleSet class

#endif


