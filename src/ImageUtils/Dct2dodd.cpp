//**********************************************************
// TITLE       :DCT 2-D ODD CODEC CLASS FUNCTIONS
// VERSION     :1.0
// FILE        :Dct2dodd.cpp
// DESCRIPTION :A class for implementing an odd 2 dimensional
//              discrete cosine transform on images. This is 
//              a derived class from the abstract class of 
//              type CODEC.
// DATE        :February 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include "dct2dodd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PI (double)(3.14159265359)
#define I_SQRT2 (double)(0.707106781188)

static long int InTest[8] =
  { 
    245, 103, 174, 155,  80, 142, 122, 130
  };

static long int E_DCT[8] =
  { 
    406,  64,  47,  13,  24,  75,  68,  -6
  };

///////////////////////////////////////////////////////////////
// Public Implementations.

CDCTODDCODEC::CDCTODDCODEC()
{
  hCoeff = NULL;
  pRFCoeff = NULL;
  pRICoeff = NULL;
  pCFCoeff = NULL;
  pCICoeff = NULL;
  pWk = NULL;

  CurrParams.Rows = 0;
  CurrParams.Cols = 0;

}//end CDCTODDCODEC Constructor.

CDCTODDCODEC::~CDCTODDCODEC()
{
}//end CDCTODDCODEC Destructor.

int CDCTODDCODEC::OpenCODEC(void *Params)
{
  DCT_PARAMTYPE *P;
  P = (DCT_PARAMTYPE *)Params;

	//Allocate matrix memory for the forward and reverse transform.
  int MSize = 2*((P->Rows * P->Rows) + (P->Cols * P->Cols)) * 
              sizeof(double);
  //A Matrix of doubles, two Rows and cols of doubles and a row
  //and col of integers.
  int WkSize = ((P->Rows * P->Cols) * sizeof(double)) + 
               ((P->Rows + P->Cols) * sizeof(int)) +
               (2 * (P->Rows + P->Cols) * sizeof(double));
	hCoeff = GlobalAlloc(GMEM_FIXED,MSize + WkSize);
	if(!hCoeff)
  {
    error = 2;//COEFFICIENT MEMORY UNAVAILABLE.
	  return(0);
  }//end if !hCoeff...
  //For transforming along the rows.
	pRFCoeff = (double *)GlobalLock(hCoeff);
	pRICoeff = (double *)(pRFCoeff + (P->Cols * P->Cols));
  //For transforming down the cols.
	pCFCoeff = (double *)(pRICoeff + (P->Cols * P->Cols));
	pCICoeff = (double *)(pCFCoeff + (P->Rows * P->Rows));
	pWk = (int *)((double *)(pCICoeff + (P->Rows * P->Rows)));

  //Update the current set parameters.
  CurrParams = *P;

  //Generate the 1-D transformatiom matrices.

  int X = P->Cols;
  int Y = P->Rows;
  int m,k;

  //DCT along rows requires a Cols x Cols matrix.
  double SQRT2_N = sqrt(2.0/(double)X);
  double PI_2N = PI/(double)(2*X);
  //Forward.
  for(k = 0; k < X; k++)
    for(m = 0; m < X; m++)
    {
      if(k==0)
        pRFCoeff[k*X + m] = SQRT2_N * I_SQRT2 * cos((double)((2*m + 1)*k)*PI_2N);
      else
        pRFCoeff[k*X + m] = SQRT2_N * cos((double)((2*m + 1)*k)*PI_2N);
    }//end for k & m...
  //Inverse.
  for(m = 0; m < X; m++)
    for(k = 0; k < X; k++)
    {
      if(k==0)
        pRICoeff[m*X + k] = SQRT2_N * I_SQRT2 * cos((double)((2*m + 1)*k)*PI_2N);
      else
        pRICoeff[m*X + k] = SQRT2_N * cos((double)((2*m + 1)*k)*PI_2N);
    }//end for k & m...

  //DCT down cols requires a Rows x Rows matrix.
  SQRT2_N = sqrt(2.0/(double)Y);
  PI_2N = PI/(double)(2*Y);
  //Forward.
  for(k = 0; k < Y; k++)
    for(m = 0; m < Y; m++)
    {
      if(k==0)
        pCFCoeff[k*Y + m] = SQRT2_N * I_SQRT2 * cos((double)((2*m + 1)*k)*PI_2N);
      else
        pCFCoeff[k*Y + m] = SQRT2_N * cos((double)((2*m + 1)*k)*PI_2N);
    }//end for k & m...
  //Inverse.
  for(m = 0; m < Y; m++)
    for(k = 0; k < Y; k++)
    {
      if(k==0)
        pCICoeff[m*Y + k] = SQRT2_N * I_SQRT2 * cos((double)((2*m + 1)*k)*PI_2N);
      else
        pCICoeff[m*Y + k] = SQRT2_N * cos((double)((2*m + 1)*k)*PI_2N);
    }//end for k & m...

  return(1);
}//end OpenCODEC.

int CDCTODDCODEC::CODE(void *Img)
{
  int *I;
  I = (int *)Img;
  int x = CurrParams.Cols;
  int y = CurrParams.Rows;

  double *M = (double *)((int *)(pWk + x + y));
  double *V1 = (double *)(M + (x * y));
  double *V2 = (double *)(V1 + (x + y));
  int i,j;

  for(i = 0; i < y; i++) //Along the rows.
  {
    for(j = 0; j < x; j++) //Extract the row.
      V1[j] = (double)(I[i*x + j]);
    MatMult(x,x,pRFCoeff,V1,(double *)(M + i*x));
  }//end for i...

  for(j = 0; j < x; j++) //Down the cols.
  {
    for(i = 0; i < y; i++) //Extract the col.
      V1[i] = M[i*x + j];
    MatMult(y,y,pCFCoeff,V1,V2);
    for(i = 0; i < y; i++) //Normalize and Place the col.
      I[i*x + j] = (int)(V2[i] + 0.5);
  }//end for j...

  return(1);
}//end CODE.

int CDCTODDCODEC::DECODE(void *Coeff)
{
  int *C;
  C = (int *)Coeff;
  int x = CurrParams.Cols;
  int y = CurrParams.Rows;

  double *M = (double *)((int *)(pWk + x + y));
  double *V1 = (double *)(M + (x * y));
  double *V2 = (double *)(V1 + (x + y));
  int i,j;

  for(j = 0; j < x; j++) //Down the cols.
  {
    for(i = 0; i < y; i++) //Normalize and Extract the col.
      V1[i] = (double)(C[i*x + j]);
    MatMult(y,y,pCICoeff,V1,V2);
    for(i = 0; i < y; i++) //Place the col.
      M[i*x + j] = V2[i];
  }//end for j...

  for(i = 0; i < y; i++) //Along the rows.
  {
    MatMult(x,x,pRICoeff,(double *)(M + i*x),V1);
    for(j = 0; j < x; j++) //Place the row.
      C[i*x + j] = (int)(V1[j] + 0.5);
  }//end for i...

  return(1);
}//end DECODE.

void CDCTODDCODEC::CloseCODEC(void)
{
  //Free the working memory.
	if(hCoeff)
	{
		GlobalUnlock(hCoeff);
 		GlobalFree(hCoeff);
 		hCoeff = NULL;
    pRFCoeff = NULL;
    pRICoeff = NULL;
    pCFCoeff = NULL;
    pCICoeff = NULL;
    pWk = NULL;
	}//end if hWk...

  CurrParams.Rows = 0;
  CurrParams.Cols = 0;

}//end CloseCODEC.

const char *CDCTODDCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													//0
		 "COEFFICIENT MEMORY UNAVAILABLE", 		//1
     "ONLY YUV IMAGES MAY BE DECODED"};		//2

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

////////////////////////////////////////////////////////
// Protected utility functions.
////////////////////////////////////////////////////////

void CDCTODDCODEC::MatMult(int x,int y,double *Mat,
                           double *Vin,double *Vout)
{
  int i,j;
  double acc;

  for(i = 0; i < y; i++)
  {
    acc = 0.0;
    for(j = 0; j < x; j++)
    {
      acc += Mat[i*x + j] * Vin[j];
    }//end for j...
    Vout[i] = acc;
  }//end for i...

}//end MatMult.

