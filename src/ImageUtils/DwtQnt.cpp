//**********************************************************
// TITLE       :DWT QUANTIZER CODEC CLASS FUNCTIONS
// VERSION     :1.0
// FILE        :DwtQnt.cpp
// DESCRIPTION :A class for implementing psychovisual 
//              quantization, linear quantization and 
//              thresholding on a wavelet transformed image.
// DATE        :February 1999
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include "dwtqnt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PI (double)(3.14159265359)

double ThresholdShape[MAX_ORIENT][MAX_QNT_LEVELS] =
{
  { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0 }
};

// Defined for the 9_7_ wavelet filter.
double BasisAmplitude[MAX_ORIENT][MAX_QNT_LEVELS] =
{
  { 0.621710, 0.345374, 0.180040, 0.091401, 0.0459435, 0.0230128 },
  { 0.672341, 0.413174, 0.227267, 0.117925, 0.0597584, 0.0300184 },
  { 0.672341, 0.413174, 0.227267, 0.117925, 0.0597584, 0.0300184 },
  { 0.727095, 0.494284, 0.286881, 0.152145, 0.0777274, 0.0391565 }
};

// Define the model parameters.
#define G_OFFSET 3

double ModelParameters[3][7] =
{
  //  a      k      f0  g(LxLy)g(LxHy)g(HxLy)g(HxHy)
  { 0.495, 0.466, 0.401, 1.501, 1.000, 1.000, 0.534 }, // Y.
  { 1.633, 0.353, 0.209, 1.520, 1.000, 1.000, 0.502 }, // U.
  { 0.944, 0.521, 0.404, 1.868, 1.000, 1.000, 0.516 }  // V.
};

///////////////////////////////////////////////////////////////
// Public Implementations.

CDWTQNTCODEC::CDWTQNTCODEC()
{
  CurrParams.Rows = 0;
  CurrParams.Cols = 0;
  CurrParams.WaveLevels = 0;
  CurrParams.Colour = Qnt_Lum;
  CurrParams.PsychoQuality = 0;
  CurrParams.Threshold = 0;
  CurrParams.MonitorDiagDim = 0;
  CurrParams.ViewDist = 0;
  CurrParams.ScreenXPels = 0;
  CurrParams.ScreenYPels = 0;
  CurrParams.ChrToLumXRatio = 1; //YUV411 = 2, YUV422 = 2, YUV444 = 1. 
  CurrParams.ChrToLumYRatio = 1; //YUV411 = 2, YUV422 = 1, YUV444 = 1.
}//end CDWTQNTCODEC Constructor.

CDWTQNTCODEC::~CDWTQNTCODEC()
{
}//end CDWTQNTCODEC Destructor.

int CDWTQNTCODEC::Open(void *Params)
{
  DWT_QNT_STRUCTURE *P;
  P = (DWT_QNT_STRUCTURE *)Params;

  //Update the current set parameters.
  CurrParams = *P;
  if(CurrParams.WaveLevels > MAX_QNT_LEVELS)
    CurrParams.WaveLevels = MAX_QNT_LEVELS;

  //Generate the quantization tables.

  //First, determine the max spatial freq. possible with
  //the monitor and resolution parameters for each colour component.
  //This calc. is simplified by normalising to the x dim. only.
  double Diag = (double)(P->MonitorDiagDim) * 0.0254;
  //Assume square pixels.
  double width = Diag * cos(atan2((double)(P->ScreenXPels),
                                  (double)(P->ScreenYPels)));
  double radians = atan2(width,(2.0*(double)(P->ViewDist))/100.0);
  double degrees = (radians * 180.0) / PI;
  double MaxFreq = (double)(P->ScreenXPels)/(4.0 * degrees * (double)(P->ChrToLumXRatio));

  //Determine the max and min spatial freq. at each level
  //and map this to a range on the sensitivity curve. Divide
  //the range into 31 quantization steps and select the step
  //corresponding to the quality factor.
  double fmax,fmin;
  double q;
  for(int l = 0; l < P->WaveLevels; l++) //each level.
  {
    fmax = MaxFreq;
    MaxFreq = MaxFreq/2.0;
    fmin = MaxFreq;
    for(int orient = 0; orient < 4; orient++) //each orientation.
    {
      if(orient == Qnt_LxLy)
      {
        fmax = fmin;
        fmin = 1.0;
      }//end if orient...

      if(P->PsychoQuality > 0)
        q = Sensitivity(fmax,l,orient,P->PsychoQuality);
      else
        q = 1.0;

      //Load the quantization matrix with multiplication factors.
      Qnt[orient][l] = 1.0/q;
      IQnt[orient][l] = q;

      //Load the thresholding matrix.
      Thresh[orient][l] = (int)(ThresholdShape[orient][l] * (double)(P->Threshold));

    }//end for orient...
  }//end for l...

  return(1);
}//end Open.

int CDWTQNTCODEC::Code(void *Img)
{
  int *I;
  I = (int *)Img;
  int X = CurrParams.Cols;
  int Y = CurrParams.Rows;
  int *Src;
  double Q;
  int T;
  int i,j,k,t;

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
    //Set the quantization value and threshold.
    Q = Qnt[Qnt_LxHy][l];
    T = Thresh[Qnt_LxHy][l];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
        {
          k = (int)(((double)t * Q) - 0.5);
          if(k >= (-T))
            k = 0;
        }//end if t...
        else
        {
          k = (int)(((double)t * Q) + 0.5);
          if(k <= T)
            k = 0;
        }//end else...
        *(Src + X*i + j) = k;
      }//end for i & j...

    //Determine the High X High Y starting point.
    Src = (int *)(I + X*y + x);
    //Set the quantization value.
    Q = Qnt[Qnt_HxHy][l];
    T = Thresh[Qnt_HxHy][l];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
        {
          k = (int)(((double)t * Q) - 0.5);
          if(k >= (-T))
            k = 0;
        }//end if t...
        else
        {
          k = (int)(((double)t * Q) + 0.5);
          if(k <= T)
            k = 0;
        }//end else...
        *(Src + X*i + j) = k;
      }//end for i & j...

    //Determine the High X Low Y starting point.
    Src = (int *)(I + x);
    //Set the quantization value.
    Q = Qnt[Qnt_HxLy][l];
    T = Thresh[Qnt_HxLy][l];
    //Do the quantization.
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        t = *(Src + X*i + j);
        if(t < 0)
        {
          k = (int)(((double)t * Q) - 0.5);
          if(k >= (-T))
            k = 0;
        }//end if t...
        else
        {
          k = (int)(((double)t * Q) + 0.5);
          if(k <= T)
            k = 0;
        }//end else...
        *(Src + X*i + j) = k;
      }//end for i & j...
  }//end for l...

  //Determine the texture starting point.
  Src = I;
  //Set the quantization value.
  Q = Qnt[Qnt_LxLy][CurrParams.WaveLevels-1];
  T = Thresh[Qnt_LxLy][CurrParams.WaveLevels-1];
  //Do the quantization.
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      //Do not quantize the 1st value.
      if( (i!=0)||(j!=0) )
      {
        t = *(Src + X*i + j);
        if(t < 0)
        {
          k = (int)(((double)t * Q) - 0.5);
          if(k >= (-T))
            k = 0;
        }//end if t...
        else
        {
          k = (int)(((double)t * Q) + 0.5);
          if(k <= T)
            k = 0;
        }//end else...
        *(Src + X*i + j) = k;
      }//end if i...
    }//end for i & j...

  return(1);
}//end Code.

int CDWTQNTCODEC::Decode(void *Img)
{
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
    Q = IQnt[Qnt_LxHy][l];
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
    Q = IQnt[Qnt_HxHy][l];
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
    Q = IQnt[Qnt_HxLy][l];
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
  Q = IQnt[Qnt_LxLy][CurrParams.WaveLevels-1];
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
}//end Decode.

void CDWTQNTCODEC::Close(void)
{
  CurrParams.Rows = 0;
  CurrParams.Cols = 0;
  CurrParams.WaveLevels = 0;
  CurrParams.Colour = Qnt_Lum;
  CurrParams.PsychoQuality = 0;
  CurrParams.Threshold = 0;
  CurrParams.MonitorDiagDim = 0;
  CurrParams.ViewDist = 0;
  CurrParams.ScreenXPels = 0;
  CurrParams.ScreenYPels = 0;
  CurrParams.ChrToLumXRatio = 1; //YUV411 = 2, YUV422 = 2, YUV444 = 1. 
  CurrParams.ChrToLumYRatio = 1; //YUV411 = 2, YUV422 = 1, YUV444 = 1.
}//end Close.

const char *CDWTQNTCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
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

double CDWTQNTCODEC::Sensitivity(double freq, int level, int Orient, int Scale)
{
  double x;

  x = (ModelParameters[CurrParams.Colour][2] * 
       ModelParameters[CurrParams.Colour][G_OFFSET+Orient])/freq;
  x = log10(x);
  x = ModelParameters[CurrParams.Colour][1] * pow(x,2.0);
  x = ModelParameters[CurrParams.Colour][0] * pow(10.0,x);
  // Apply the linear quantization that is in 1/4ths from 1.0.
  double lin_q = 1.0 + ((double)(Scale)/4.0);
  x = x * lin_q;
  x = (2.0 * x)*BasisAmplitude[Orient][level];
  //The range for colour is -128...127 so half is required.
  if(CurrParams.Colour != Qnt_Lum)
    x = x/2.0;
  if(x < 1.0)
    x = 1.0;
  return(x);
}//end Sensitivity.

