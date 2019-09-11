#include "stdafx.h"

#include "WImgPce.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Public Implementations.

CWImgPce::CWImgPce()
{
  //Image piece is attached to this total image.
  MainI.pY = NULL;
  MainI.pU = NULL;
  MainI.pV = NULL;
  MainI.Lum_X = 0;
  MainI.Lum_Y = 0;
  MainI.Chr_X = 0;
  MainI.Chr_Y = 0;
  LumLevel = 0;
  ChrLevel = 0;
  WvltPos = LOW_X_LOW_Y;
  hImg = NULL;
  PceI.pY = NULL;
  PceI.pU = NULL;
  PceI.pV = NULL;
  PceI.Lum_X = 0;
  PceI.Lum_Y = 0;
  PceI.Chr_X = 0;
  PceI.Chr_Y = 0;
  DirtyFlag = 0;
  ErrorMsg = "No Error";
}//end CWImgPce Constructor.

CWImgPce::~CWImgPce()
{
}//end CWImgPce Destructor.

int CWImgPce::CreateImagePiece(YUVINFO_TYPE *WI,int LumLev,int ChrLev,int Position)
{
  if((LumLev == 0)||(ChrLev == 0))
    return(0);
  //If image piece is still active destroy it 1st.
  DestroyImagePiece();

  MainI = *WI;
  LumLevel = LumLev;
  ChrLevel = ChrLev;
  WvltPos = Position;

  //Determine all Lum and Chr dimensions.
  int x,y,l;
  y = MainI.Lum_Y;
  x = MainI.Lum_X;
  //The level determines the sub image dimensions.
  for(l = 0; l < LumLevel; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...
  PceI.Lum_X = x;
  PceI.Lum_Y = y;
  y = MainI.Chr_Y;
  x = MainI.Chr_X;
  //The level determines the sub image dimensions.
  for(l = 0; l < ChrLevel; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...
  PceI.Chr_X = x;
  PceI.Chr_Y = y;

  //Create the memory space for the piece of image.
  hImg = GlobalAlloc(GMEM_FIXED,
                     ((PceI.Lum_X*PceI.Lum_Y) + 
                      2*(PceI.Chr_X*PceI.Chr_Y))
                                 * sizeof(int));
  if(!hImg)
  {
    ErrorMsg = "Memory unavailable for image piece";
    return(0);
  }//end if !hImg...

  PceI.pY = (int *)GlobalLock(hImg);
  PceI.pU = (int *)(PceI.pY + (PceI.Lum_X*PceI.Lum_Y));
  PceI.pV = (int *)(PceI.pU + (PceI.Chr_X*PceI.Chr_Y));

  //Update image piece.
  UpdateImagePiece();

  return(1);
}//end CreateImagePiece.

void CWImgPce::DestroyImagePiece(void) 
{
  if(hImg)
  {
		GlobalUnlock(hImg);
 		GlobalFree(hImg);
    hImg = NULL;
    PceI.pY = NULL;
    PceI.pU = NULL;
    PceI.pV = NULL;
  }//end if hImg...

  MainI.pY = NULL;
  MainI.pU = NULL;
  MainI.pV = NULL;
  MainI.Lum_X = 0;
  MainI.Lum_Y = 0;
  MainI.Chr_X = 0;
  MainI.Chr_Y = 0;
  LumLevel = 0;
  ChrLevel = 0;
  WvltPos = LOW_X_LOW_Y;
  PceI.Lum_X = 0;
  PceI.Lum_Y = 0;
  PceI.Chr_X = 0;
  PceI.Chr_Y = 0;
  DirtyFlag = 0;
  ErrorMsg = "No Error";
}//end DestroyImagePiece.

//Transfer the relevent part of the YUV image into 
//the image piece.
void CWImgPce::UpdateImagePiece(void)
{
  if(!hImg)
    return;

  int i,j;
  int x,y;

  //Do luminance 1st.
  int *SrcY;
  int *DstY = PceI.pY;
  int X = MainI.Lum_X;
  x = PceI.Lum_X;
  y = PceI.Lum_Y;
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    SrcY = (int *)(MainI.pY);
    break;
  case LOW_X_HIGH_Y:
    SrcY = (int *)(MainI.pY + X*y);
    break;
  case HIGH_X_LOW_Y:
    SrcY = (int *)(MainI.pY + x);
    break;
  case HIGH_X_HIGH_Y:
    SrcY = (int *)(MainI.pY + X*y + x);
    break;
  }//end switch WvltPos...
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstY + x*i + j) = *(SrcY + X*i + j);
    }//end for i & j...

    //Do chrominance.
  int *SrcU;
  int *SrcV;
  X = MainI.Chr_X;
  x = PceI.Chr_X;
  y = PceI.Chr_Y;
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    SrcU = (int *)(MainI.pU);
    SrcV = (int *)(MainI.pV);
    break;
  case LOW_X_HIGH_Y:
    SrcU = (int *)(MainI.pU + X*y);
    SrcV = (int *)(MainI.pV + X*y);
    break;
  case HIGH_X_LOW_Y:
    SrcU = (int *)(MainI.pU + x);
    SrcV = (int *)(MainI.pV + x);
    break;
  case HIGH_X_HIGH_Y:
    SrcU = (int *)(MainI.pU + X*y + x);
    SrcV = (int *)(MainI.pV + X*y + x);
    break;
  }//end switch WvltPos...
  int *DstU = PceI.pU;
  int *DstV = PceI.pV;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstU + x*i + j) = *(SrcU + X*i + j);
      *(DstV + x*i + j) = *(SrcV + X*i + j);
    }//end for i & j...

}//end UpdateImagePiece.

void CWImgPce::UpdateYUVImage(void)
{
  // Transfer the image piece to the relevent part 
  // of the YUV image only if it has been modified.
  if(!DirtyFlag)
    return;

  if(!hImg)
    return;

  int i,j;
  int x,y;

  //Do luminance 1st.
  int *DstY;
  int *SrcY = PceI.pY;
  int X = MainI.Lum_X;
  x = PceI.Lum_X;
  y = PceI.Lum_Y;
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    DstY = (int *)(MainI.pY);
    break;
  case LOW_X_HIGH_Y:
    DstY = (int *)(MainI.pY + X*y);
    break;
  case HIGH_X_LOW_Y:
    DstY = (int *)(MainI.pY + x);
    break;
  case HIGH_X_HIGH_Y:
    DstY = (int *)(MainI.pY + X*y + x);
    break;
  }//end switch WvltPos...
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstY + X*i + j) = *(SrcY + x*i + j);
    }//end for i & j...

    //Do chrominance.
  int *DstU;
  int *DstV;
  int *SrcU = PceI.pU;
  int *SrcV = PceI.pV;
  X = MainI.Chr_X;
  x = PceI.Chr_X;
  y = PceI.Chr_Y;
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    DstU = (int *)(MainI.pU);
    DstV = (int *)(MainI.pV);
    break;
  case LOW_X_HIGH_Y:
    DstU = (int *)(MainI.pU + X*y);
    DstV = (int *)(MainI.pV + X*y);
    break;
  case HIGH_X_LOW_Y:
    DstU = (int *)(MainI.pU + x);
    DstV = (int *)(MainI.pV + x);
    break;
  case HIGH_X_HIGH_Y:
    DstU = (int *)(MainI.pU + X*y + x);
    DstV = (int *)(MainI.pV + X*y + x);
    break;
  }//end switch WvltPos...
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstU + X*i + j) = *(SrcU + x*i + j);
      *(DstV + X*i + j) = *(SrcV + x*i + j);
    }//end for i & j...
  DirtyFlag = 0;
}//end UpdateYUVImage.

//Transfer a specific colour component of the relevent 
//part of the YUV image into the image piece.
void CWImgPce::UpdateImagePiece(int Colour)
{
  if(!hImg)
    return;

  int i,j;
  int x,y,X;

  if(Colour == COLOUR_LUM)
  {
    //Do luminance 1st.
    int *SrcY;
    int *DstY = PceI.pY;
    X = MainI.Lum_X;
    x = PceI.Lum_X;
    y = PceI.Lum_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      SrcY = (int *)(MainI.pY);
      break;
    case LOW_X_HIGH_Y:
      SrcY = (int *)(MainI.pY + X*y);
      break;
    case HIGH_X_LOW_Y:
      SrcY = (int *)(MainI.pY + x);
      break;
    case HIGH_X_HIGH_Y:
      SrcY = (int *)(MainI.pY + X*y + x);
      break;
    }//end switch WvltPos...
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstY + x*i + j) = *(SrcY + X*i + j);
      }//end for i & j...
  }//end if Colour...
  else if(Colour == COLOUR_U_CHR)
  {
    //Do U chrominance.
    int *SrcU;
    X = MainI.Chr_X;
    x = PceI.Chr_X;
    y = PceI.Chr_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      SrcU = (int *)(MainI.pU);
      break;
    case LOW_X_HIGH_Y:
      SrcU = (int *)(MainI.pU + X*y);
      break;
    case HIGH_X_LOW_Y:
      SrcU = (int *)(MainI.pU + x);
      break;
    case HIGH_X_HIGH_Y:
      SrcU = (int *)(MainI.pU + X*y + x);
      break;
    }//end switch WvltPos...
    int *DstU = PceI.pU;
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstU + x*i + j) = *(SrcU + X*i + j);
      }//end for i & j...
  }//end else if Colour...
  else //if(Colour == COLOUR_V_CHR)
  {
    //Do V chrominance.
    int *SrcV;
    X = MainI.Chr_X;
    x = PceI.Chr_X;
    y = PceI.Chr_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      SrcV = (int *)(MainI.pV);
      break;
    case LOW_X_HIGH_Y:
      SrcV = (int *)(MainI.pV + X*y);
      break;
    case HIGH_X_LOW_Y:
      SrcV = (int *)(MainI.pV + x);
      break;
    case HIGH_X_HIGH_Y:
      SrcV = (int *)(MainI.pV + X*y + x);
      break;
    }//end switch WvltPos...
    int *DstV = PceI.pV;
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstV + x*i + j) = *(SrcV + X*i + j);
      }//end for i & j...
  }//end else if Colour...

}//end UpdateImagePiece.

void CWImgPce::UpdateYUVImage(int Colour)
{
  // Transfer a specific colour component of the image 
  // piece to the relevent part of the YUV image only 
  // if it has been modified.
  if(!DirtyFlag)
    return;

  if(!hImg)
    return;

  int i,j;
  int x,y,X;

  if(Colour == COLOUR_LUM)
  {
    //Do luminance 1st.
    int *DstY;
    int *SrcY = PceI.pY;
    X = MainI.Lum_X;
    x = PceI.Lum_X;
    y = PceI.Lum_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      DstY = (int *)(MainI.pY);
      break;
    case LOW_X_HIGH_Y:
      DstY = (int *)(MainI.pY + X*y);
      break;
    case HIGH_X_LOW_Y:
      DstY = (int *)(MainI.pY + x);
      break;
    case HIGH_X_HIGH_Y:
      DstY = (int *)(MainI.pY + X*y + x);
      break;
    }//end switch WvltPos...
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstY + X*i + j) = *(SrcY + x*i + j);
      }//end for i & j...
  }//end if Colour...
  else if(Colour == COLOUR_U_CHR)
  {
    //Do U chrominance.
    int *DstU;
    int *SrcU = PceI.pU;
    X = MainI.Chr_X;
    x = PceI.Chr_X;
    y = PceI.Chr_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      DstU = (int *)(MainI.pU);
      break;
    case LOW_X_HIGH_Y:
      DstU = (int *)(MainI.pU + X*y);
      break;
    case HIGH_X_LOW_Y:
      DstU = (int *)(MainI.pU + x);
      break;
    case HIGH_X_HIGH_Y:
      DstU = (int *)(MainI.pU + X*y + x);
      break;
    }//end switch WvltPos...
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstU + X*i + j) = *(SrcU + x*i + j);
      }//end for i & j...
  }//end else if Colour...
  else //if(Colour == COLOUR_V_CHR)
  {
    //Do V chrominance.
    int *DstV;
    int *SrcV = PceI.pV;
    X = MainI.Chr_X;
    x = PceI.Chr_X;
    y = PceI.Chr_Y;
    switch(WvltPos)
    {
    case LOW_X_LOW_Y:
      DstV = (int *)(MainI.pV);
      break;
    case LOW_X_HIGH_Y:
      DstV = (int *)(MainI.pV + X*y);
      break;
    case HIGH_X_LOW_Y:
      DstV = (int *)(MainI.pV + x);
      break;
    case HIGH_X_HIGH_Y:
      DstV = (int *)(MainI.pV + X*y + x);
      break;
    }//end switch WvltPos...
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstV + X*i + j) = *(SrcV + x*i + j);
      }//end for i & j...
  }//end else...

  DirtyFlag = 0;
}//end UpdateYUVImage.

// Use Parseval's relation to determine the energy
// of a wavelet transformed image.
int CWImgPce::GetLumPieceEnergy(void)
{
  if(LumLevel == 0)
    return(0);

  //Accumulate the energy of each coefficient.
  double E_Y = 0.0;
  int i,j,t;
  for(i = 0; i < PceI.Lum_Y; i++)
    for(j = 0; j < PceI.Lum_X; j++)
    {
      t = *(PceI.pY + PceI.Lum_X*i + j);
      E_Y += ((double)t * (double)t);
    }//end for i & j...
  E_Y = E_Y/((double)(PceI.Lum_X * PceI.Lum_Y));

  return((int)E_Y);

}//end GetLumPieceEnergy.

int CWImgPce::GetChrPieceEnergy(void)
{
  if(ChrLevel == 0)
    return(0);

  //Do the U and V colour components.
  double E_U = 0.0;
  double E_V = 0.0;
  int i,j,t;
  for(i = 0; i < PceI.Chr_Y; i++)
    for(j = 0; j < PceI.Chr_X; j++)
    {
      t = *(PceI.pU + PceI.Chr_X*i + j);
      E_U += ((double)t * (double)t);
      t = *(PceI.pV + PceI.Chr_X*i + j);
      E_V += ((double)t * (double)t);
    }//end for i & j...
  E_U = E_U/((double)(PceI.Chr_X * PceI.Chr_Y));
  E_V = E_V/((double)(PceI.Chr_X * PceI.Chr_Y));

  return((int)E_U + (int)E_V);

}//end GetChrPieceEnergy.

int CWImgPce::MultLumPiece(double By)
{
  if(LumLevel == 0)
    return(0);

  //Multiply each coefficient.
  int i,j,t;
  for(i = 0; i < PceI.Lum_Y; i++)
    for(j = 0; j < PceI.Lum_X; j++)
    {
      t = *(PceI.pY + PceI.Lum_X*i + j);
      *(PceI.pY + PceI.Lum_X*i + j) = (int)(((double)t * By) + 0.5);
    }//end for i & j...

  //After operating on the image piece, mark the
  //piece as dirty.
  DirtyFlag = 1;

  return(1);
}//end MultLumPiece.

int CWImgPce::MultChrPiece(double By)
{
  if(ChrLevel == 0)
    return(0);

  //Multiply each coefficient.
  int i,j,t;
  for(i = 0; i < PceI.Chr_Y; i++)
    for(j = 0; j < PceI.Chr_X; j++)
    {
      t = *(PceI.pU + PceI.Chr_X*i + j);
      *(PceI.pU + PceI.Chr_X*i + j)= (int)(((double)t * By) + 0.5);
      t = *(PceI.pV + PceI.Chr_X*i + j);
      *(PceI.pV + PceI.Chr_X*i + j) = (int)(((double)t * By) + 0.5);
    }//end for i & j...

  //After operating on the image piece, mark the
  //piece as dirty.
  DirtyFlag = 1;

  return(1);
}//end MultChrPiece.

void CWImgPce::GetLumPieceHisto(int *H,int Rng)
{
}//end GetLumPieceHisto.

void CWImgPce::GetChrPieceHisto(int *H,int Rng)
{
}//end GetChrPieceHisto.



