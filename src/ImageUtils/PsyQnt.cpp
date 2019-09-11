//**********************************************************
// TITLE       :PSYCHOVISUAL DWT QUANTIZER CODEC CLASS 
//              FUNCTIONS
// VERSION     :1.0
// FILE        :PsyQnt.h
// DESCRIPTION :A class for implementing psychovisual 
//              quantization on a wavelet transformed image.
//              This is a derived class from the abstract 
//              class of type CODEC.
// DATE        :February 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include "Psyqnt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PI (double)(3.14159265359)

///////////////////////////////////////////////////////////////
// Public Implementations.

CPSYWQNTCODEC::CPSYWQNTCODEC()
{
  hQnt = NULL;
  pQnt = NULL;
  pIQnt = NULL;

  CurrParams.Rows = 0;
  CurrParams.Cols = 0;
  CurrParams.WaveLevels = 0;
  CurrParams.Quality = 0;
  CurrParams.MonitorDiagDim = 0;
  CurrParams.ViewDist = 0;
  CurrParams.ScreenXPels = 0;
  CurrParams.ScreenYPels = 0;
}//end CPSYWQNTCODEC Constructor.

CPSYWQNTCODEC::~CPSYWQNTCODEC()
{
}//end CPSYWQNTCODEC Destructor.

int CPSYWQNTCODEC::OpenCODEC(void *Params)
{
  QNT_PARAMTYPE *P;
  P = (QNT_PARAMTYPE *)Params;

	//Allocate memory for the forward and inverse quantization.
  //Require 3 values for each wavelet level and 1 for the
  //texture.
  int QSize = 3*P->WaveLevels + 1;
	hQnt = GlobalAlloc(GMEM_FIXED,2 * QSize * sizeof(double));
	if(!hQnt)
  {
    error = 1;//QUANTIZATION MEMORY UNAVAILABLE.
	  return(0);
  }//end if !hQnt...
	pQnt = (double *)GlobalLock(hQnt);
	pIQnt = (double *)(pQnt + QSize);

  //Update the current set parameters.
  CurrParams = *P;
  if(CurrParams.Quality == 0)
    return(1);

  //Generate the quantization tables.

  //First, determine the max spatial freq. possible with
  //the monitor and resolution parameters.
  double Diag = (double)(P->MonitorDiagDim) * 0.0254;
  //Assume square pixels.
  double width = Diag * cos(atan2((double)(P->ScreenXPels),
                                  (double)(P->ScreenYPels)));
  double radians = atan2(width,(2.0*(double)(P->ViewDist))/100.0);
  double degrees = (radians * 180.0) / PI;
  double MaxFreq = (double)(P->ScreenXPels)/(4.0 * degrees);

  //Determine the max and min spatial freq. at each level
  //and map this to a range on the sensitivity curve. Divide
  //the range into 31 quantization steps and select the step
  //corresponding to the quality factor.
  double max,min,t;
  double stepsize,q;
  for(int l = 0; l < P->WaveLevels; l++)
  {
    max = Sensitivity(MaxFreq);
    MaxFreq = MaxFreq/2.0;
    t = Sensitivity(MaxFreq);
    if(t > max)
    {
      min = max;
      max = t;
    }//end if t...
    else
    {
      min = t;
    }//end else...
    stepsize = (max - min)/31.00;
    q = max - (stepsize * (double)(P->Quality - 1));
    //Load quantization value.
    pQnt[l*3] = q;
    //High X High Y have 20% energy avg. 40% worst case.
    pQnt[l*3 + 1] = q / 2.5;
    pQnt[l*3 + 2] = q;
    //Load inverse quantization value.
    pIQnt[l*3] = 1.0/q;
    //High X High Y have 20% energy avg. 40% worst case.
    pIQnt[l*3 + 1] = 2.5 /q;
    pIQnt[l*3 + 2] = 1.0/q;
  }//end for l...
  //Set the texture quantization.
  max = Sensitivity(MaxFreq);
  MaxFreq = MaxFreq/10.0;
  t = Sensitivity(MaxFreq);
  if(t > max)
  {
    min = max;
    max = t;
  }//end if t...
  else
  {
    min = t;
  }//end else...
  stepsize = (max - min)/31.00;
  q = max - (stepsize * (double)(P->Quality - 1));
  //Load quantization value.
  pQnt[l*3] = q;
  pIQnt[l*3] = 1.0/q;

  return(1);
}//end OpenCODEC.

int CPSYWQNTCODEC::CODE(void *Img)
{
  if(CurrParams.Quality == 0)
    return(1);

  int *I;
  I = (int *)Img;
  int X = CurrParams.Cols;
  int Y = CurrParams.Rows;
  int *Src;
  double Q;
  int i,j,t;

  //The level determines the sub image dimensions.
  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < CurrParams.WaveLevels; l++)
  {
    y = y/2;
    x = x/2;

    //Determine the Low X High Y starting point.
    Src = (int *)(I + X*y);
    //Set the quantization value.
    Q = pQnt[l*3];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...

    //Determine the High X High Y starting point.
    Src = (int *)(I + X*y + x);
    //Set the quantization value.
    Q = pQnt[l*3 + 1];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...

    //Determine the High X Low Y starting point.
    Src = (int *)(I + x);
    //Set the quantization value.
    Q = pQnt[l*3 + 2];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...
  }//end for l...

  //Do not quantize the texture.
  //Determine the texture starting point.
  Src = I;
  //Set the quantization value.
  Q = pQnt[l*3];
  //Do the quantization.
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      //Do not quantize the 1st value.
      if( (i!=0)||(j!=0) )
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end if i...
    }//end for i & j...

  return(1);
}//end CODE.

int CPSYWQNTCODEC::DECODE(void *Img)
{
  if(CurrParams.Quality == 0)
    return(1);

  int *I;
  I = (int *)Img;
  int X = CurrParams.Cols;
  int Y = CurrParams.Rows;
  int *Src;
  double Q;
  int i,j,t;

  //The level determines the sub image dimensions.
  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < CurrParams.WaveLevels; l++)
  {
    y = y/2;
    x = x/2;

    //Determine the Low X High Y starting point.
    Src = (int *)(I + X*y);
    //Set the quantization value.
    Q = pIQnt[l*3];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...

    //Determine the High X High Y starting point.
    Src = (int *)(I + X*y + x);
    //Set the quantization value.
    Q = pIQnt[l*3 + 1];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...

    //Determine the High X Low Y starting point.
    Src = (int *)(I + x);
    //Set the quantization value.
    Q = pIQnt[l*3 + 2];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end for i & j...
  }//end for l...

  //Do not inverse quantize the texture.
  //Determine the texture starting point.
  Src = I;
  //Set the quantization value.
  Q = pIQnt[l*3];
  //Do the quantization.
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      //Do not quantize the 1st value.
      if( (i!=0)||(j!=0) )
      {
        t = *(Src + X*i + j);
        if(t < 0)
          *(Src + X*i + j) = (int)(((double)t * Q) - 0.5);
        else
          *(Src + X*i + j) = (int)(((double)t * Q) + 0.5);
      }//end if i...
    }//end for i & j...

  return(1);
}//end DECODE.

void CPSYWQNTCODEC::CloseCODEC(void)
{
  //Free the working memory.
	if(hQnt)
	{
		GlobalUnlock(hQnt);
 		GlobalFree(hQnt);
 		hQnt = NULL;
    pQnt = NULL;
    pIQnt = NULL;
	}//end if hQnt...

  CurrParams.Rows = 0;
  CurrParams.Cols = 0;
  CurrParams.WaveLevels = 0;
  CurrParams.Quality = 0;
  CurrParams.MonitorDiagDim = 0;
  CurrParams.ViewDist = 0;
  CurrParams.ScreenXPels = 0;
  CurrParams.ScreenYPels = 0;
}//end CloseCODEC.

const char *CPSYWQNTCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													//0
     "QUANTIZATION MEMORY UNAVAILABLE"}; 	//1

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

////////////////////////////////////////////////////////
// Protected utility functions.
////////////////////////////////////////////////////////

double CPSYWQNTCODEC::Sensitivity(double SpatialFreq)
{
  double Result;
  Result = (0.31 + 0.69*SpatialFreq)*exp(-0.29*SpatialFreq);

  return(Result);
}//end Sensitivity.

