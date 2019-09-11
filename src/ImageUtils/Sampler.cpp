//**********************************************************
// TITLE       :RANDOM SAMPLER CLASS FILE
// VERSION     :1.0
// FILE        :Sampler.cpp
// DESCRIPTION :A class for implementing a random vector 
//              sampler for use in training SOMs.
// DATE        :October 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include "sampler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////
// Public Implementations.

CSAMPLER::CSAMPLER()
{
  h_space = NULL;
  space = NULL;

  CurrParam.vec_y_dim = 0;
  CurrParam.vec_x_dim = 0;
  CurrParam.space_y_dim = 0;
  CurrParam.space_x_dim = 0;

  XBlks = 0;
  YBlks = 0;
  LastX = 0;
  LastY = 0;
  error = 0;
  Empty = 1;

}//end CSAMPLER Constructor.

CSAMPLER::~CSAMPLER()
{
  //Please close the door behind you.
  if(!Empty)
    Close();
}//end CSAMPLER Destructor.

void CSAMPLER::SetParameters(SAMPLER_INFO *Params)
{
  CurrParam = *Params;
}//end SetParameters.

SAMPLER_INFO *CSAMPLER::GetParameters(void)
{
  return(&CurrParam);
}//end GetParameters.

CSAMPLER &CSAMPLER::operator=(CSAMPLER &S)
{
  //Clear out any previously defined maps.
  if(!Empty)
    Close();

  SAMPLER_INFO *Parm = S.GetParameters();

  // Set up the sampler.
  if( !Open(Parm) )
  {
    return(*this);
  }//end if !Open...

  LastX = S.LastX;
  LastY = S.LastY;
  Empty = S.Empty;

  //Copy the sample space.
  int Size = (CurrParam.space_y_dim * CurrParam.space_x_dim) *
             (CurrParam.vec_y_dim * CurrParam.vec_x_dim);
  for(int i = 0; i < Size; i++)
    space[i] = S.space[i];

  return(*this);
}//end operator=.

int CSAMPLER::Open(SAMPLER_INFO *Params)
{
  //Clear out any previously defined maps.
  if(!Empty)
    Close();

  CurrParam.vec_y_dim = Params->vec_y_dim;
  CurrParam.vec_x_dim = Params->vec_x_dim;
  CurrParam.space_y_dim = Params->space_y_dim;
  CurrParam.space_x_dim = Params->space_x_dim;

  // Set up the memory space.
  if( !CreateMemOnCurrParams() )
  {
    return(0);
  }//end if !CreateMemOnCurrParams...

  YBlks = CurrParam.space_y_dim/CurrParam.vec_y_dim;
  XBlks = CurrParam.space_x_dim/CurrParam.vec_x_dim;
  LastX = 0;
  LastY = 0;
  Empty = 0;
  //Seed the random number generator.
  srand((unsigned)time(NULL));

  return(1);
}//end Open.

int CSAMPLER::CreateMemOnCurrParams(void)
{
  int MemReq = (CurrParam.space_y_dim * CurrParam.space_x_dim) *
               (CurrParam.vec_y_dim * CurrParam.vec_x_dim);
  int RealMemReq = MemReq * sizeof(double);
  // Alloc memory.
	h_space = GlobalAlloc(GMEM_FIXED,RealMemReq);
	if(!h_space)
  {
    error = 1;//SAMPLER MEMORY UNAVAILABLE.
	  return(0);
  }//end if !h_space...
	space = (double *)GlobalLock(h_space);

  return(1);
}//end CreateMemOnCurrParams.

void CSAMPLER::DestroyMem(void)
{
  if(h_space != NULL)
  {
		GlobalUnlock(h_space);
 		GlobalFree(h_space);
  }//end if h_space...
 	h_space = NULL;
  space = NULL;

  Empty = 1;
}//end DestroyMem.

#define ZERO (double)(0.00001)
int CSAMPLER::RemoveZerosFromSpace(void)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  int x_loc,y_loc;
  int x_blk,y_blk;

  // Alloc memory.
 	HGLOBAL h_tvec = NULL;
  int dim = CurrParam.vec_y_dim * CurrParam.vec_x_dim;
	h_tvec = GlobalAlloc(GMEM_FIXED,dim);
	if(!h_tvec)
  {
    error = 1;//SAMPLER MEMORY UNAVAILABLE.
	  return(0);
  }//end if !h_tvec...
	double *tvec = (double *)GlobalLock(h_tvec);

  //Replace every zero vector with a randomly sampled 
  //non-zero vector from the space.
  for(y_blk = 0; y_blk < YBlks; y_blk++)
  {
    for(x_blk = 0; x_blk < XBlks; x_blk++)
    {
      GetVectorLocation(x_blk,y_blk,&x_loc,&y_loc);
      TakeSample(tvec,x_loc,y_loc);
      //Zero test.
      BOOL IsZero = TRUE;
      for(int i = 0; i < dim; i++)
      {
        if((tvec[i] > ZERO)||(tvec[i] < -(ZERO)))
          IsZero = FALSE;
      }//end for i...
      if(IsZero)
      {
        TakeNonZeroRandomSample(tvec);
        OverwriteSample(tvec,x_loc,y_loc);
      }//end if IsZero...
    }//end for x_blk...
  }//end for y_blk...

  if(h_tvec != NULL)
  {
		GlobalUnlock(h_tvec);
 		GlobalFree(h_tvec);
  }//end if h_tvec...
 	h_tvec = NULL;
  tvec = NULL;

  return(1);
}//end RemoveZerosFromSpace.

int CSAMPLER::TakeUnbndRandomSample(double *vec)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  int y_loc;
  int x_loc;

  //Randomly locate the x,y coord. of a vector in the space.
  double y_lim = CurrParam.space_y_dim - CurrParam.vec_y_dim;
  double x_lim = CurrParam.space_x_dim - CurrParam.vec_x_dim;
  y_loc = (int)(y_lim * ((double)rand()/(double)RAND_MAX));
  x_loc = (int)(x_lim * ((double)rand()/(double)RAND_MAX));
  //Extract the vector at this location.
  for(int y = 0; y < CurrParam.vec_y_dim; y++)
    for(int x = 0; x < CurrParam.vec_x_dim; x++)
    {
      vec[y*CurrParam.vec_x_dim + x] = 
                 space[(y+y_loc)*CurrParam.space_x_dim + (x_loc+x)];
    }//end for x & y...
  LastX = x_loc;
  LastY = y_loc;
  return(1);
}//end TakeUnbndRandomSample.

int CSAMPLER::TakeBndRandomSample(double *vec)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  int y_loc;
  int x_loc;

  //Randomly locate the x,y coord. of a vector in the input space
  //that is a multiple of vector blocks.
  int y_blk = (int)(((double)(YBlks-1)) * ((double)rand()/(double)RAND_MAX));
  int x_blk = (int)(((double)(XBlks-1)) * ((double)rand()/(double)RAND_MAX));
  y_loc = y_blk * CurrParam.vec_y_dim;
  x_loc = x_blk * CurrParam.vec_x_dim;
  //Extract the vector at this location.
  for(int y = 0; y < CurrParam.vec_y_dim; y++)
    for(int x = 0; x < CurrParam.vec_x_dim; x++)
    {
      vec[y*CurrParam.vec_x_dim + x] = 
                 space[(y+y_loc)*CurrParam.space_x_dim + (x_loc+x)];
    }//end for x & y...
  LastX = x_loc;
  LastY = y_loc;
  return(1);
}//end TakeBndRandomSample.

int CSAMPLER::TakeNonZeroRandomSample(double *vec)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  int nonzero_found = 0;
  int count_limit = 1000;//Make 1000 attempts then give up.
  int count = 0;
  double zero_confidence = 0.0001;
  int y_loc;
  int x_loc;

  while( (!nonzero_found)&&(count < count_limit) )
  {
  //Randomly locate the x,y coord. of a vector in the input space
  //that is a multiple of vector blocks.
  int y_blk = (int)(((double)(YBlks-1)) * ((double)rand()/(double)RAND_MAX));
  int x_blk = (int)(((double)(XBlks-1)) * ((double)rand()/(double)RAND_MAX));
  y_loc = y_blk * CurrParam.vec_y_dim;
  x_loc = x_blk * CurrParam.vec_x_dim;
  //Extract the vector at this location.
  for(int y = 0; y < CurrParam.vec_y_dim; y++)
    for(int x = 0; x < CurrParam.vec_x_dim; x++)
    {
      double pixel = space[(y+y_loc)*CurrParam.space_x_dim + (x_loc+x)];
      vec[y*CurrParam.vec_x_dim + x] = pixel;
      //If any pixel is non-zero then the vector is non-zero.
      if( (pixel > zero_confidence)||(pixel < (-zero_confidence)) )
        nonzero_found = 1;
    }//end for x & y...
  count++;
  }//end while !nonzero_found...

  LastX = x_loc;
  LastY = y_loc;
  if(count == count_limit)
  {
    error = 4; //NON-ZERO VECTOR NOT FOUND.
    return(0);
  }//end if count...

  return(1);
}//end TakeNonZeroRandomSample.

int CSAMPLER::TakeSample(double *vec,int x_loc,int y_loc)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Extract the vector at this location.
  for(int y = 0; y < CurrParam.vec_y_dim; y++)
    for(int x = 0; x < CurrParam.vec_x_dim; x++)
    {
      vec[y*CurrParam.vec_x_dim + x] = 
                 space[(y+y_loc)*CurrParam.space_x_dim + (x_loc+x)];
    }//end for x & y...
  LastX = x_loc;
  LastY = y_loc;
  return(1);
}//end TakeSample.

int CSAMPLER::OverwriteSample(double *vec,int x_loc,int y_loc)
{
  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Overwrite the vector at this location.
  for(int y = 0; y < CurrParam.vec_y_dim; y++)
    for(int x = 0; x < CurrParam.vec_x_dim; x++)
    {
      space[(y+y_loc)*CurrParam.space_x_dim + (x_loc+x)] = 
                                    vec[y*CurrParam.vec_x_dim + x];
    }//end for x & y...
  LastX = x_loc;
  LastY = y_loc;
  return(1);
}//end OverwriteSample.

double *CSAMPLER::GetVectorLocation(int XBlkNum,int YBlkNum,int *x_loc,int *y_loc)
{
  double *pS = NULL;

  if(Empty)
  {
    error = 3; //SAMPLE SPACE DOES NOT EXIST.
    return(pS);
  }//end if Empty...

  if((XBlkNum >= XBlks)||(YBlkNum >= YBlks))
  {
    error = 5; //PARAMETERS OUT OF BOUNDS.
    return(pS);
  }//end if XBlkNum...

  //Locate position.
  *y_loc = YBlkNum * CurrParam.vec_y_dim;
  *x_loc = XBlkNum * CurrParam.vec_x_dim;
  pS = (double *)(space + (*y_loc * CurrParam.space_x_dim) + (*x_loc));
  return(pS);
}//end GetVectorLocation.

void CSAMPLER::Close(void)
{
  //Free the working memory.
  DestroyMem();

  CurrParam.vec_y_dim = 0;
  CurrParam.vec_x_dim = 0;
  CurrParam.space_y_dim = 0;
  CurrParam.space_x_dim = 0;

  LastX = 0;
  LastY = 0;
}//end Close.

const char *CSAMPLER::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													             //0
		 "SAMPLER MEMORY UNAVAILABLE",                     //1
     "FUNCTION UNIMPLEMENTED",                         //2
     "SAMPLE SPACE DOES NOT EXIST",                    //3
     "NON-ZERO VECTOR NOT FOUND",                      //4
     "PARAMETERS OUT OF BOUNDS"};                      //5

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

