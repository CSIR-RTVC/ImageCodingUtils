/*
===========================================================================
MODULE          : SampleSet

TAG             : SSET

FILE NAME       : SampleSet.cpp

DESCRIPTIONS    : A class for implementing a random vector 
		              sampler for use in training SOMs.

                  
REVISION HISTORY: 
                : $Log: SampleSet.cpp,v $
                : Revision 1.1  2004/08/18 08:03:55  keithf
                : A sample space utility used for scalar and vector quantisation training
                : applications.
                : Status: Build-18-08-2004
                :

COPYRIGHT       : (c)VICS 2001  all rights resevered - info@videocoding.com
===========================================================================
*/
#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include "SampleSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef double* pdouble;

///////////////////////////////////////////////////////////////
// Public Implementations.

SampleSet::SampleSet()
{
  pVector = NULL;
  Vector	= NULL;

  Vector_dim = 0;
	NumVectors = 0;
	LastVector = 0;

  error = 0;
  Empty = 1;

}//end SampleSet Constructor.

SampleSet::~SampleSet()
{
  //Please close the door behind you.
  if(!Empty)
    Close();
}//end SampleSet Destructor.

SampleSet &SampleSet::operator=(SampleSet &S)
{
  //Clear out any previously defined maps.
  if(!Empty)
    Close();

  // Set up the sample set.
  if( !Open(S.Vector_dim,S.NumVectors) )
  {
    return(*this);
  }//end if !Open...

	LastVector = S.LastVector;

  //Copy the sample space.
	int i;
  int Size = Vector_dim * NumVectors;
  for(i = 0; i < Size; i++)
    pVector[i] = S.pVector[i];
	//...and the addresses.
  for(i = 0; i < NumVectors; i++)
    Vector[i] = S.Vector[i];

  return(*this);
}//end operator=.

int SampleSet::Open(int vec_dim,int num_vectors)
{
  //Clear out any previously defined vectors.
  if(!Empty)
    Close();

	//Set up parameters.
	Vector_dim = vec_dim;
	NumVectors = num_vectors;

  // Set up the memory space.
	pVector = new double[vec_dim * num_vectors];
	if(pVector == NULL)
  {
    error = 1;//SAMPLER MEMORY UNAVAILABLE.
	  return(0);
  }//end if !pVector...
	Vector = new pdouble[num_vectors];
	if(Vector == NULL)
  {
    error = 1;//SAMPLER MEMORY UNAVAILABLE.
	  return(0);
  }//end if !Vector...
	//Load the addresses.
	for(int i = 0; i < num_vectors; i++)
		Vector[i] = &(pVector[i*vec_dim]);

  LastVector	= 0;
  Empty				= 0;

  //Seed the random number generator.
  srand((unsigned)time(NULL));

  return(1);
}//end Open.

int SampleSet::Open(const char* TestSetFilename)
{
	int ret = 1;
  // Open the file.
  FILE *F;
  if((F = fopen(TestSetFilename,"r")) == NULL)
  {
    error = 6; //UNABLE TO OPEN INPUT FILE.
    return(0);
  }//end if F...

  // Load the header.
	int VecDim,NumVec;
  fscanf(F,"%d %d\n",&VecDim,&NumVec);

	//Prepare the object.
	if(!Open(VecDim,NumVec))
	{
		fclose(F);
		return(0);
	}//end if !Open...

	//Load the set.
	int i;
	float fvec;
	int Size = Vector_dim * NumVectors;
	for(i = 0; i < Size; i++)
	{
		fscanf(F,"%f",&fvec);
		pVector[i] = (double)fvec;
	}//end for i...

  fclose(F);
  return(ret);
}//end Open.

void SampleSet::Close(void)
{
  //Free the working memory.
  if(Vector != NULL)
		delete[] Vector;
	Vector = NULL;
  if(pVector != NULL)
		delete[] pVector;
	pVector = NULL;

  Vector_dim	= 0;
	NumVectors	= 0;
	LastVector	= 0;

  error				= 0;
  Empty				= 1;

}//end Close.

#define ZERO (double)(0.00001)
int SampleSet::TakeRandomSample(double *vec)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Randomly locate the index of a vector in the sample set.
  int loc = (int)(((double)(NumVectors-1)) * ((double)rand()/(double)RAND_MAX));

  //Extract the vector at this location.
  for(int x = 0; x < Vector_dim; x++)
		vec[x] = Vector[loc][x];

  LastVector = loc;

  return(1);
}//end TakeBndRandomSample.

int SampleSet::TakeNonZeroRandomSample(double *vec)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  int nonzero_found = 0;
  int count_limit = 1000;//Make 1000 attempts then give up.
  int count = 0;
  double zero_confidence = 0.000001;
  int loc;

  while( (!nonzero_found)&&(count < count_limit) )
  {
		//Randomly locate the index of a vector in the sample set.
		loc = (int)(((double)(NumVectors-1)) * ((double)rand()/(double)RAND_MAX));

		//Extract the vector at this location.
		for(int x = 0; x < Vector_dim; x++)
		{
			double component = Vector[loc][x];
			vec[x] = component;
      //If any component is non-zero then the vector is non-zero.
      if( (component > zero_confidence)||(component < (-zero_confidence)) )
        nonzero_found = 1;
		}//end for x...

		count++;
  }//end while !nonzero_found...

  LastVector = loc;
  if(count == count_limit)
  {
    error = 4; //NON-ZERO VECTOR NOT FOUND.
    return(0);
  }//end if count...

  return(1);
}//end TakeNonZeroRandomSample.

int SampleSet::TakeSample(double *vec,int loc)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Extract the vector at this location.
  for(int x = 0; x < Vector_dim; x++)
		vec[x] = Vector[loc][x];

  LastVector = loc;

  return(1);
}//end TakeSample.

int SampleSet::OverwriteSample(double *vec,int loc)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Overwrite the vector at this location.
  for(int x = 0; x < Vector_dim; x++)
		Vector[loc][x] = vec[x];

  LastVector = loc;

  return(1);
}//end OverwriteSample.

const char *SampleSet::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"SampleSet:NO ERROR",													             //0
		 "SampleSet:SAMPLER MEMORY UNAVAILABLE",                     //1
     "SampleSet:FUNCTION UNIMPLEMENTED",                         //2
     "SampleSet:SAMPLE SPACE DOES NOT EXIST",                    //3
     "SampleSet:NON-ZERO VECTOR NOT FOUND",                      //4
     "SampleSet:PARAMETERS OUT OF BOUNDS",                       //5
     "SampleSet:UNABLE TO OPEN INPUT FILE"};                     //6

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

