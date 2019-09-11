#include "stdafx.h"

#include "WImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Public Implementations.

CWImage::CWImage()
{
  Texture.LumLevel = 0;
  Texture.ChrLevel = 0;
  Texture.hImg = NULL;
  Texture.pY = NULL;
  Texture.pU = NULL;
  Texture.pV = NULL;
  Texture.Lum_X = 0;
  Texture.Lum_Y = 0;
  Texture.Chr_X = 0;
  Texture.Chr_Y = 0;

  SrchRegionsActive = FALSE;
  for(int i = 0; i < MAX_REGIONS; i++)
  {
    hSrchSubBands[i] = NULL;
    SrchRegion[i].Active = 0;
  }//end for i...
}//end CWImage Constructor.

CWImage::~CWImage()
{
  if(SrchRegionsActive)
    DestroyMotionSearchWin();
}//end CWImage Destructor.

int CWImage::CreateTextureImage(int LumLevel,int ChrLevel)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return(0);
		}//end if m_ExtraColorType...

  if((LumLevel == 0)||(ChrLevel == 0))
    return(0);
  //If a texture image is still active destroy it 1st.
  DestroyTextureImage();

  Texture.LumLevel = LumLevel;
  Texture.ChrLevel = ChrLevel;

  //Determine all Lum and Chr dimensions.
  int x,y,l;
  y = GetYHeight();
  x = GetYWidth();
  //The level determines the sub image dimensions.
  for(l = 0; l < LumLevel; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...
  Texture.Lum_X = x;
  Texture.Lum_Y = y;
  y = GetUVHeight();
  x = GetUVWidth();
  //The level determines the sub image dimensions.
  for(l = 0; l < ChrLevel; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...
  Texture.Chr_X = x;
  Texture.Chr_Y = y;

  //Create the memory space for a texture image.
  Texture.hImg = GlobalAlloc(GMEM_FIXED,
                   ((Texture.Lum_X*Texture.Lum_Y) +
                    2*(Texture.Chr_X*Texture.Chr_Y)) * 
                   sizeof(int));
  if(!Texture.hImg)
    return(0);

  Texture.pY = (int *)GlobalLock(Texture.hImg);
  Texture.pU = (int *)(Texture.pY + (Texture.Lum_X*Texture.Lum_Y));
  Texture.pV = (int *)(Texture.pU + (Texture.Chr_X*Texture.Chr_Y));

  //Update texture image.
  UpdateTexture();

  return(1);
}//end CreateTextureImage.

//Base class overloaded function.
void CWImage::DestroyImage(void) 
{
  DestroyTextureImage();

  //Call the base class function.
  CImage::DestroyImage();
}//end DestroyImage.

void CWImage::DestroyTextureImage(void) 
{
  if(Texture.hImg)
  {
		GlobalUnlock(Texture.hImg);
 		GlobalFree(Texture.hImg);
    Texture.hImg = NULL;
    Texture.pY = NULL;
    Texture.pU = NULL;
    Texture.pV = NULL;
  }//end if hImg...

  Texture.LumLevel = 0;
  Texture.ChrLevel = 0;
  Texture.Lum_X = 0;
  Texture.Lum_Y = 0;
  Texture.Chr_X = 0;
  Texture.Chr_Y = 0;

}//end DestroyTextureImage.

//Transfer the texture part of the YUV image into 
//the texture image.
void CWImage::UpdateTexture(void)
{
  if(!Texture.hImg)
    return;

  int i,j;
  int x,y;

  //Do luminance 1st.
  int *SrcY = m_Y;
  int X = GetYWidth();
  x = Texture.Lum_X;
  y = Texture.Lum_Y;
  int *DstY = Texture.pY;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstY + x*i + j) = *(SrcY + X*i + j);
    }//end for i & j...

    //Do chrominance.
  int *SrcU = m_U;
  int *SrcV = m_V;
  X = GetUVWidth();
  x = Texture.Chr_X;
  y = Texture.Chr_Y;
  int *DstU = Texture.pU;
  int *DstV = Texture.pV;

  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      *(DstU + x*i + j) = *(SrcU + X*i + j);
      *(DstV + x*i + j) = *(SrcV + X*i + j);
    }//end for i & j...

}//end UpdateTexture.

//Base class overloaded function.
void CWImage::UpdateBmp(void)
{
  //Transfer the texture image to the YUV image and then
  //call the base class to transfer to the display image.
  if(Texture.hImg)
  {
    int i,j;
    int x,y;
  
    //Do luminance 1st.
    int *DstY = m_Y;
    int *SrcY = Texture.pY;
    int X = GetYWidth();
    x = Texture.Lum_X;
    y = Texture.Lum_Y;
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstY + X*i + j) = *(SrcY + x*i + j);
      }//end for i & j...
  
      //Do chrominance.
    int *DstU = m_U;
    int *DstV = m_V;
    int *SrcU = Texture.pU;
    int *SrcV = Texture.pV;
    X = GetUVWidth();
    x = Texture.Chr_X;
    y = Texture.Chr_Y;
  
    for(i = 0; i < y; i++)
      for(j = 0; j < x; j++)
      {
        *(DstU + X*i + j) = *(SrcU + x*i + j);
        *(DstV + X*i + j) = *(SrcV + x*i + j);
      }//end for i & j...
  }//end if hImg...

  //Call the base class function.
  CImage::UpdateBmp();
}//end UpdateBmp.

// Get the number of possible wavelet levels for this
// image. i.e. the level at which the texture image
// will have an odd dimension.
int CWImage::GetMaxYLevels(void)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return 0;
		}//end if m_ExtraColorType...

  int level = 0;
  int y = GetYHeight();
  int x = GetYWidth();
  while( ((y % 2)==0)&&((x % 2)==0)&&(y > 16)&&(x > 16) )
  {
    x = x/2;
    y = y/2;
    level++;
  }//end while...
	return(level);
}//end GetMaxYLevels.

int CWImage::GetMaxUVLevels(void)
{
  int level = 0;
  int y = GetUVHeight();
  int x = GetUVWidth();
  while( ((y % 2)==0)&&((x % 2)==0)&&(y > 16)&&(x > 16) )
  {
    x = x/2;
    y = y/2;
    level++;
  }//end while...
	return(level);
}//end GetMaxUVLevels.

// Use Parseval's relation to determine the energy at any
// quadrant of a wavelet transformed image.
int CWImage::GetWvltLumEnergy(int level, int WvltPos)
{
  if(level == 0)
    return(0);

  int *Src;
  int Y = GetYHeight();
  int X = GetYWidth();

  //The level determines the sub image dimensions.
  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  //Determine the starting point.
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    if(Texture.hImg)
      UpdateBmp(); //Just in case it is not the latest.
    Src = m_Y;
    break;
  case LOW_X_HIGH_Y:
    Src = (int *)(m_Y + X*y);
    break;
  case HIGH_X_LOW_Y:
    Src = (int *)(m_Y + x);
    break;
  case HIGH_X_HIGH_Y:
    Src = (int *)(m_Y + X*y + x);
    break;
  }//end switch WvltPos...

  //Accumulate the energy of each coefficient.
  double E_Y = 0.0;
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(Src + X*i + j);
      E_Y += ((double)t * (double)t);
    }//end for i & j...
  E_Y = E_Y/((double)(x * y));

  return((int)E_Y);

}//end GetWvltLumEnergy.

int CWImage::GetWvltChrEnergy(int level, int WvltPos)
{
  if(level == 0)
    return(0);

  //Do the U and V colour components.
  int *SrcU,*SrcV;
  int Y = GetUVHeight();
  int X = GetUVWidth();

  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    if(Texture.hImg)
      UpdateBmp(); //Just in case it is not the latest.
    SrcU = m_U;
    SrcV = m_V;
    break;
  case LOW_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y);
    SrcV = (int *)(m_V + X*y);
    break;
  case HIGH_X_LOW_Y:
    SrcU = (int *)(m_U + x);
    SrcV = (int *)(m_V + x);
    break;
  case HIGH_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y + x);
    SrcV = (int *)(m_V + X*y + x);
    break;
  }//end switch WvltPos...

  double E_U = 0.0;
  double E_V = 0.0;
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(SrcU + X*i + j);
      E_U += ((double)t * (double)t);
      t = *(SrcV + X*i + j);
      E_V += ((double)t * (double)t);
    }//end for i & j...
  E_U = E_U/((double)(x * y));
  E_V = E_V/((double)(x * y));

  return((int)E_U + (int)E_V);

}//end GetWvltChrEnergy.

int CWImage::MultWvltLum(int level,int WvltPos,double By)
{
  if(level == 0)
    return(0);

  int *Src;
  int Y = GetYHeight();
  int X = GetYWidth();

  //The level determines the sub image dimensions.
  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  //Determine the starting point.
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    Src = m_Y;
    break;
  case LOW_X_HIGH_Y:
    Src = (int *)(m_Y + X*y);
    break;
  case HIGH_X_LOW_Y:
    Src = (int *)(m_Y + x);
    break;
  case HIGH_X_HIGH_Y:
    Src = (int *)(m_Y + X*y + x);
    break;
  }//end switch WvltPos...

  //Multiply each coefficient.
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(Src + X*i + j);
      *(Src + X*i + j) = (int)(((double)t * By) + 0.5);
    }//end for i & j...

  //After operating on the YUV image, keep the texture
  //image up to date.
  if(Texture.hImg && (WvltPos == LOW_X_LOW_Y))
    UpdateTexture();

  return(1);
}//end MultWvltLum.

int CWImage::MultWvltChr(int level,int WvltPos,double By)
{
  if(level == 0)
    return(0);

  //Do the U and V colour components.
  int *SrcU,*SrcV;
  int Y = GetUVHeight();
  int X = GetUVWidth();

  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    SrcU = m_U;
    SrcV = m_V;
    break;
  case LOW_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y);
    SrcV = (int *)(m_V + X*y);
    break;
  case HIGH_X_LOW_Y:
    SrcU = (int *)(m_U + x);
    SrcV = (int *)(m_V + x);
    break;
  case HIGH_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y + x);
    SrcV = (int *)(m_V + X*y + x);
    break;
  }//end switch WvltPos...

  //Multiply each coefficient.
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(SrcU + X*i + j);
      *(SrcU + X*i + j)= (int)(((double)t * By) + 0.5);
      t = *(SrcV + X*i + j);
      *(SrcV + X*i + j) = (int)(((double)t * By) + 0.5);
    }//end for i & j...

  //After operating on the YUV image, keep the texture
  //image up to date.
  if(Texture.hImg && (WvltPos == LOW_X_LOW_Y))
    UpdateTexture();

  return(1);
}//end MultWvltChr.

int CWImage::ThresholdWvltLum(int level,int WvltPos,int with)
{
  if(level == 0)
    return(0);

  int *Src;
  int Y = GetYHeight();
  int X = GetYWidth();

  //The level determines the sub image dimensions.
  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  //Determine the starting point.
  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    Src = m_Y;
    break;
  case LOW_X_HIGH_Y:
    Src = (int *)(m_Y + X*y);
    break;
  case HIGH_X_LOW_Y:
    Src = (int *)(m_Y + x);
    break;
  case HIGH_X_HIGH_Y:
    Src = (int *)(m_Y + X*y + x);
    break;
  }//end switch WvltPos...

  //Threshold each coefficient.
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(Src + X*i + j);
      if( abs(t) <= with)
        *(Src + X*i + j) = 0;
    }//end for i & j...

  //After operating on the YUV image, keep the texture
  //image up to date.
  if(Texture.hImg && (WvltPos == LOW_X_LOW_Y))
    UpdateTexture();

  return(1);
}//end MultWvltLum.

int CWImage::ThresholdWvltChr(int level,int WvltPos,int with)
{
  if(level == 0)
    return(0);

  //Do the U and V colour components.
  int *SrcU,*SrcV;
  int Y = GetUVHeight();
  int X = GetUVWidth();

  int x = X;
  int y = Y;
  int l;
  for(l = 0; l < level; l++)
  {
    y = y/2;
    x = x/2;
  }//end for l...

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    SrcU = m_U;
    SrcV = m_V;
    break;
  case LOW_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y);
    SrcV = (int *)(m_V + X*y);
    break;
  case HIGH_X_LOW_Y:
    SrcU = (int *)(m_U + x);
    SrcV = (int *)(m_V + x);
    break;
  case HIGH_X_HIGH_Y:
    SrcU = (int *)(m_U + X*y + x);
    SrcV = (int *)(m_V + X*y + x);
    break;
  }//end switch WvltPos...

  //Threshold each coefficient.
  int i,j,t;
  for(i = 0; i < y; i++)
    for(j = 0; j < x; j++)
    {
      t = *(SrcU + X*i + j);
      if( abs(t) <= with)
        *(SrcU + X*i + j)= 0;
      t = *(SrcV + X*i + j);
      if( abs(t) <= with)
        *(SrcV + X*i + j) = 0;
    }//end for i & j...

  //After operating on the YUV image, keep the texture
  //image up to date.
  if(Texture.hImg && (WvltPos == LOW_X_LOW_Y))
    UpdateTexture();

  return(1);
}//end MultWvltChr.

int *CWImage::GetWvltPtr(int level,int WvltPos,int WvltCol)
{
  if(level == 0)
    return(NULL);

  int *P;
  int *Base;
  int X;
  if(WvltCol == 0)
  {
    Base = m_Y;
    X = GetYWidth();
  }//end if WvltCol...
  else if(WvltCol == 1)
  {
    Base = m_U;
    X = GetUVWidth();
  }//end else if WvltCol...
  else
  {
    Base = m_V;
    X = GetUVWidth();
  }//end else...

  int x = GetWvltXPos(level,WvltPos,WvltCol);
  int y = GetWvltYPos(level,WvltPos,WvltCol);

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
    P = Base;
    break;
  case LOW_X_HIGH_Y:
    P = (int *)(Base + X*y);
    break;
  case HIGH_X_LOW_Y:
    P = (int *)(Base + x);
    break;
  case HIGH_X_HIGH_Y:
    P = (int *)(Base + X*y + x);
    break;
  }//end switch WvltPos...

  return(P);
}//end GetWvltPtr.

int CWImage::GetWvltXPos(int level,int WvltPos,int WvltCol)
{
  if(level == 0)
    return(0);

  int x;

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
  case LOW_X_HIGH_Y:
    x = 0;
    break;
  case HIGH_X_LOW_Y:
  case HIGH_X_HIGH_Y:
    x = GetWvltWidth(level,WvltCol);
    break;
  }//end switch WvltPos...

  return(x);
}//end GetWvltXPos.

int CWImage::GetWvltYPos(int level,int WvltPos,int WvltCol)
{
  if(level == 0)
    return(0);

  int y;

  switch(WvltPos)
  {
  case LOW_X_LOW_Y:
  case HIGH_X_LOW_Y:
    y = 0;
    break;
  case LOW_X_HIGH_Y:
  case HIGH_X_HIGH_Y:
    y = GetWvltHeight(level,WvltCol);
    break;
  }//end switch WvltPos...

  return(y);
}//end GetWvltYPos.

int CWImage::GetWvltWidth(int level,int WvltCol)
{
  if(level == 0)
    return(0);

  int x;
  if(WvltCol == 0)
    x = GetYWidth();
  else
    x = GetUVWidth();

  for(int l = 0; l < level; l++)
  {
    x = x/2;
  }//end for l...

  return(x);
}//end GetWvltWidth.

int CWImage::GetWvltHeight(int level,int WvltCol)
{
  if(level == 0)
    return(0);

  int y;
  if(WvltCol == 0)
    y = GetYHeight();
  else
    y = GetUVHeight();

  for(int l = 0; l < level; l++)
  {
    y = y/2;
  }//end for l...

  return(y);
}//end GetWvltHeight.

//////////////////////////////////////////////////////////////////
// Wavelet domain motion estimation and compensation functions.
//////////////////////////////////////////////////////////////////

//typedef struct tagSEARCH_SETTING_STRUCTURE
//{
//  int blk_x_dim; //Image block dimensions.
//  int blk_y_dim;
//  int blk_x_rng; //Motion vector range.
//  int blk_y_rng;
//} SEARCH_SETTING_STRUCTURE;

//typedef struct tagMOTION_STRUCTURE
//{
//  int StartLevel;
//  BOOL TextureEstimation;
//  BOOL ColourEstimation; //Include colour or not.
//  int colour_x_sub;
//  int colour_y_sub;
//  int start_blk_x_dim; //Image block dimensions.
//  int start_blk_y_dim;
//  int Start_Vec_X_rng; //Motion vector range.
//  int Start_Vec_Y_rng;
//  BOOL IndependentEstimation;
//  SEARCH_SETTING_STRUCTURE SetBlk[MAX_LEVELS];
//} MOTION_STRUCTURE;

// Define sub-band parameters including extended boundaries.
//typedef struct tagSEARCH_SUBBAND_STRUCTURE
//{
//  int Active;
//  int SubBand; //Sub-Band parameters.
//  int Level;
//  int Colour;
//  int Sb_X;
//  int Sb_Y;
//  int *pSrc;
//  int Ext_Sb_X; //Extended sub-band parameters.
//  int Ext_Sb_Y;
//  int extra_x;
//  int extra_y;
//  int *pExtSrc;
//  int blk_x_dim; //Image block dimensions.
//  int blk_y_dim;
//  int Vec_X_rng; //Motion vector range.
//  int Vec_Y_rng;
//} SEARCH_SUBBAND_STRUCTURE;

//(Sub-bands * levels * colour components).
// Referenced: SrchRegion[lev-1][colour][subB];
// Referenced: SrchRegion[(lev-1)*(COLOURS*SUBBANDS) + colour*COLOURS + subB];
//#define MAX_REGIONS (4 * 6 * 3)
//
//// Define vector vlc structure.
//typedef struct tagVECTOR_VLC_BITS
//{
//  int numbits;
//  int codebits;
//} VECTOR_VLC_BITS;

//  MOTION_STRUCTURE motion_param;
//  HGLOBAL hSrchSubBands[MAX_REGIONS]; //For extended sub-band mem.
//  SEARCH_SUBBAND_STRUCTURE SrchRegion[MAX_REGIONS];
//  BOOL SrchRegionsActive;
#define MAX_VLC_BITS 18
//The table excludes the sign bit. 
static VECTOR_VLC_BITS MotVecBits[MAX_VLC_BITS] =
{
  { 1, 1}, //0.      0.
  { 2, 1}, //+1 -1.  +0.5 -0.5
  { 3, 1}, //+2 -2.  +1   -1
  { 4, 1}, //+3 -3.  +1.5 -1.5
  { 6, 3}, //+4 -4.  +2   -2
  { 7, 5}, //+5 -5.  +2.5 -2.5
  { 7, 4}, //+6 -6.  +3   -3
  { 7, 3}, //+7 -7.  +3.5 -3.5
  { 9,11}, //+8 -8.  +4   -4
  { 9,10}, //+9 -9.  +4.5 -4.5
  { 9, 9}, //+10-10. +5   -5
  {10,17}, //+11-11. +5.5 -5.5
  {10,16}, //+12-12. +6   -6
  {10,15}, //+13-13. +6.5 -6.5
  {10,14}, //+14-14. +7   -7
  {10,13}, //+15-15. +7.5 -7.5
  {10,12}, //+16-16. +8   -8
  {10,11}  //+17-17. +8.5 -8.5
};

int CWImage::PrepareMotionSearchWin(MOTION_STRUCTURE *Params)
{
  //Clear out old search windows.
  if(SrchRegionsActive)
    DestroyMotionSearchWin();

  // The input MOTION_STRUCTURE is used to create the required
  // levels and mem areas to set up the SEARCH_SUBBAND_STRUCTUREs.
  motion_param = *Params;

  int SUBBANDS = 4;
  int COLOURS = 3;
  int lev,col,subb;
  int region;
  BOOL Do;
  BOOL ColourSubSampled = (motion_param.colour_x_sub > 1)||(motion_param.colour_y_sub > 1);
  int ColourStartLevel;
  if(ColourSubSampled)
    ColourStartLevel = motion_param.StartLevel - 1;
  else
    ColourStartLevel = motion_param.StartLevel;

  if(!motion_param.IndependentEstimation)
  {
////////////////////// Dependent estimation /////////////////////
    //The process is seperated into Lum and Chr parts.
  
    //Lum block, sub-band and range parameters.
    col = 0;
    int curr_blk_x_dim = motion_param.start_blk_x_dim; 
    int curr_blk_y_dim = motion_param.start_blk_y_dim;
    int Curr_Vec_X_rng = motion_param.Start_Vec_X_rng; 
    int Curr_Vec_Y_rng = motion_param.Start_Vec_Y_rng;
    int Curr_extra_x = motion_param.Start_Vec_X_rng + 1; 
    int Curr_extra_y = motion_param.Start_Vec_Y_rng + 1;
  
    for(lev = motion_param.StartLevel; lev > 0; lev--)
    {
      for(subb = 0; subb < SUBBANDS; subb++)
      {
        //Do we need to make a region?
        // LowXLowY subband only if at start level.
        Do = (subb != LOW_X_LOW_Y) || (lev == motion_param.StartLevel);
        if( Do )
        {
          region = GetRegion(lev,subb,col);
          //Sub-Band parameters.
          SrchRegion[region].SubBand = subb;
          SrchRegion[region].Level = lev;
          SrchRegion[region].Colour = col;
          //Position in total image.
          SrchRegion[region].Sb_X = GetWvltWidth(lev,col); 
          SrchRegion[region].Sb_Y = GetWvltHeight(lev,col);
          SrchRegion[region].pSrc = GetWvltPtr(lev,subb,col);
  
          //Extended sub-band parameters.
          SrchRegion[region].extra_x = Curr_extra_x;
          SrchRegion[region].extra_y = Curr_extra_y;
          SrchRegion[region].Ext_Sb_X = GetWvltWidth(lev,col) + (2*Curr_extra_x); 
          SrchRegion[region].Ext_Sb_Y = GetWvltHeight(lev,col) + (2*Curr_extra_y);
          //Alloc extended sub-band mem space.
          hSrchSubBands[region] = GlobalAlloc(GMEM_FIXED,
                  (SrchRegion[region].Ext_Sb_X * SrchRegion[region].Ext_Sb_Y) *
                   sizeof(int));
          if(!hSrchSubBands[region])
          {
            DestroyMotionSearchWin();
            return(0);
          }//end if !hSrchSubBands...
          SrchRegion[region].pExtSrc = (int *)GlobalLock(hSrchSubBands[region]);
          SrchRegion[region].Active = 1;
  
          //Image block dimensions for this region.
          SrchRegion[region].blk_x_dim = curr_blk_x_dim;
          SrchRegion[region].blk_y_dim = curr_blk_y_dim;
          //Motion vector range for this region.
          SrchRegion[region].Vec_X_rng = Curr_Vec_X_rng;
          SrchRegion[region].Vec_Y_rng = Curr_Vec_Y_rng;
        }//end if Do...
      }//end for subb...
      //Update the current parameters before each change of level.
      curr_blk_x_dim = curr_blk_x_dim * 2;
      curr_blk_y_dim = curr_blk_y_dim * 2;
      Curr_Vec_X_rng = Curr_Vec_X_rng/2;
      Curr_Vec_Y_rng = Curr_Vec_Y_rng/2;
      Curr_extra_x = (2*Curr_extra_x) + Curr_Vec_X_rng;
      Curr_extra_y = (2*Curr_extra_y) + Curr_Vec_Y_rng;
    }//end for lev...
  
    //Chr block, sub-band and range parameters.
  
    curr_blk_x_dim = (2*motion_param.start_blk_x_dim)/motion_param.colour_x_sub; 
    curr_blk_y_dim = (2*motion_param.start_blk_y_dim)/motion_param.colour_y_sub;
    Curr_Vec_X_rng = (motion_param.Start_Vec_X_rng * motion_param.colour_x_sub)/2; 
    Curr_Vec_Y_rng = (motion_param.Start_Vec_Y_rng * motion_param.colour_y_sub)/2;
    Curr_extra_x = Curr_Vec_X_rng + 1; 
    Curr_extra_y = Curr_Vec_Y_rng + 1;
  
    for(lev = ColourStartLevel; lev > 0; lev--)
    {
      for(col = 1; col < COLOURS; col++)
      {
        for(subb = 0; subb < SUBBANDS; subb++)
        {
          //Do we need to make a region?
          // LowXLowY subband only if at start level.
          Do = (subb != LOW_X_LOW_Y) || (lev == ColourStartLevel);
          if( Do )
          {
            region = GetRegion(lev,subb,col);
            //Sub-Band parameters.
            SrchRegion[region].SubBand = subb;
            SrchRegion[region].Level = lev;
            SrchRegion[region].Colour = col;
            //Position in total image.
            SrchRegion[region].Sb_X = GetWvltWidth(lev,col); 
            SrchRegion[region].Sb_Y = GetWvltHeight(lev,col);
            SrchRegion[region].pSrc = GetWvltPtr(lev,subb,col);
        
            //Extended sub-band parameters.
            SrchRegion[region].extra_x = Curr_extra_x;
            SrchRegion[region].extra_y = Curr_extra_y;
            SrchRegion[region].Ext_Sb_X = GetWvltWidth(lev,col) + (2*Curr_extra_x); 
            SrchRegion[region].Ext_Sb_Y = GetWvltHeight(lev,col) + (2*Curr_extra_y);
            //Alloc extended sub-band mem space.
            hSrchSubBands[region] = GlobalAlloc(GMEM_FIXED,
                    (SrchRegion[region].Ext_Sb_X * SrchRegion[region].Ext_Sb_Y) *
                     sizeof(int));
            if(!hSrchSubBands[region])
            {
              DestroyMotionSearchWin();
              return(0);
            }//end if !hSrchSubBands...
            SrchRegion[region].pExtSrc = (int *)GlobalLock(hSrchSubBands[region]);
            SrchRegion[region].Active = 1;
        
            //Image block dimensions for this region.
            SrchRegion[region].blk_x_dim = curr_blk_x_dim;
            SrchRegion[region].blk_y_dim = curr_blk_y_dim;
            //Motion vector range for this region.
            SrchRegion[region].Vec_X_rng = Curr_Vec_X_rng;
            SrchRegion[region].Vec_Y_rng = Curr_Vec_Y_rng;
          }//end if Do...
        }//end for subb...
      }//end for colour...
      //Update the current parameters at each change of level.
      curr_blk_x_dim = curr_blk_x_dim * 2;
      curr_blk_y_dim = curr_blk_y_dim * 2;
      Curr_Vec_X_rng = Curr_Vec_X_rng/2;
      Curr_Vec_Y_rng = Curr_Vec_Y_rng/2;
      Curr_extra_x = (2*Curr_extra_x) + Curr_Vec_X_rng;
      Curr_extra_y = (2*Curr_extra_y) + Curr_Vec_Y_rng;
    }//end for lev...
  }//end if !IndependentEstimation...
  else
  {
////////////////////// Independent estimation /////////////////////
    //The process is seperated into Lum and Chr parts.
  
    //Lum block, sub-band and range parameters.
    col = 0;
    for(lev = motion_param.StartLevel; lev > 0; lev--)
    {
      for(subb = 0; subb < SUBBANDS; subb++)
      {
        //Do we need to make a region?
        // LowXLowY subband only if at start level.
        Do = (subb != LOW_X_LOW_Y) || (lev == motion_param.StartLevel);
        if( Do )
        {
          region = GetRegion(lev,subb,col);
          //Sub-Band parameters.
          SrchRegion[region].SubBand = subb;
          SrchRegion[region].Level = lev;
          SrchRegion[region].Colour = col;
          //Position in total image.
          SrchRegion[region].Sb_X = GetWvltWidth(lev,col); 
          SrchRegion[region].Sb_Y = GetWvltHeight(lev,col);
          SrchRegion[region].pSrc = GetWvltPtr(lev,subb,col);
  
          //Image block dimensions for this region.
          SrchRegion[region].blk_x_dim = motion_param.SetBlk[lev-1].blk_x_dim;
          SrchRegion[region].blk_y_dim = motion_param.SetBlk[lev-1].blk_y_dim;
          //Motion vector range for this region.
          SrchRegion[region].Vec_X_rng = motion_param.SetBlk[lev-1].blk_x_rng;
          SrchRegion[region].Vec_Y_rng = motion_param.SetBlk[lev-1].blk_y_rng;

          //Extended sub-band parameters.
          //The extra 1 is security for overflow of small odd sub-bands.
          SrchRegion[region].extra_x = motion_param.SetBlk[lev-1].blk_x_rng + 1;
          SrchRegion[region].extra_y = motion_param.SetBlk[lev-1].blk_y_rng + 1;
          SrchRegion[region].Ext_Sb_X = GetWvltWidth(lev,col) + (2*SrchRegion[region].extra_x); 
          SrchRegion[region].Ext_Sb_Y = GetWvltHeight(lev,col) + (2*SrchRegion[region].extra_y);
          //Alloc extended sub-band mem space.
          hSrchSubBands[region] = GlobalAlloc(GMEM_FIXED,
                  (SrchRegion[region].Ext_Sb_X * SrchRegion[region].Ext_Sb_Y) *
                   sizeof(int));
          if(!hSrchSubBands[region])
          {
            DestroyMotionSearchWin();
            return(0);
          }//end if !hSrchSubBands...
          SrchRegion[region].pExtSrc = (int *)GlobalLock(hSrchSubBands[region]);
          SrchRegion[region].Active = 1;
  
        }//end if Do...
      }//end for subb...
    }//end for lev...
  
    //Chr block, sub-band and range parameters.
  
    for(lev = ColourStartLevel; lev > 0; lev--)
    {
      for(col = 1; col < COLOURS; col++)
      {
        for(subb = 0; subb < SUBBANDS; subb++)
        {
          //Do we need to make a region?
          // LowXLowY subband only if at start level.
          Do = (subb != LOW_X_LOW_Y) || (lev == ColourStartLevel);
          if( Do )
          {
            region = GetRegion(lev,subb,col);
            //Sub-Band parameters.
            SrchRegion[region].SubBand = subb;
            SrchRegion[region].Level = lev;
            SrchRegion[region].Colour = col;
            //Position in total image.
            SrchRegion[region].Sb_X = GetWvltWidth(lev,col); 
            SrchRegion[region].Sb_Y = GetWvltHeight(lev,col);
            SrchRegion[region].pSrc = GetWvltPtr(lev,subb,col);
        
            //Image block dimensions for this region.
            //Modify for colour sub-sampling.
            SrchRegion[region].blk_x_dim = motion_param.SetBlk[(lev-1)+(motion_param.colour_x_sub/2)].blk_x_dim;
            SrchRegion[region].blk_y_dim = motion_param.SetBlk[(lev-1)+(motion_param.colour_y_sub/2)].blk_y_dim;
            //Motion vector range for this region.
            SrchRegion[region].Vec_X_rng = motion_param.SetBlk[(lev-1)+(motion_param.colour_x_sub/2)].blk_x_rng;
            SrchRegion[region].Vec_Y_rng = motion_param.SetBlk[(lev-1)+(motion_param.colour_y_sub/2)].blk_y_rng;

            //Extended sub-band parameters.
            //The extra 1 is security for overflow of small odd sub-bands.
            SrchRegion[region].extra_x = SrchRegion[region].Vec_X_rng + 1;
            SrchRegion[region].extra_y = SrchRegion[region].Vec_Y_rng + 1;
            SrchRegion[region].Ext_Sb_X = GetWvltWidth(lev,col) + (2*SrchRegion[region].extra_x); 
            SrchRegion[region].Ext_Sb_Y = GetWvltHeight(lev,col) + (2*SrchRegion[region].extra_y);
            //Alloc extended sub-band mem space.
            hSrchSubBands[region] = GlobalAlloc(GMEM_FIXED,
                    (SrchRegion[region].Ext_Sb_X * SrchRegion[region].Ext_Sb_Y) *
                     sizeof(int));
            if(!hSrchSubBands[region])
            {
              DestroyMotionSearchWin();
              return(0);
            }//end if !hSrchSubBands...
            SrchRegion[region].pExtSrc = (int *)GlobalLock(hSrchSubBands[region]);
            SrchRegion[region].Active = 1;
          }//end if Do...
        }//end for subb...
      }//end for colour...
    }//end for lev...
  }//end else...

  SrchRegionsActive = TRUE;
  //Now fill the extention windows with image data.
  UpdateMotionSearchWin();
  return(1);
}//end PrepareMotionSearchWin.

void CWImage::DestroyMotionSearchWin(void)
{
  for(int i = 0; i < MAX_REGIONS; i++)
  {
    if(hSrchSubBands[i] != NULL)
    {
		  GlobalUnlock(hSrchSubBands[i]);
 		  GlobalFree(hSrchSubBands[i]);
      hSrchSubBands[i] = NULL;
      SrchRegion[i].Active = 0;
    }//end if hSrchSubBands...
  }//end for i...
  SrchRegionsActive = FALSE;
}//end DestroyMotionSearchWin.

int CWImage::UpdateMotionSearchWin(void)
{
  //Are any regions active.
  if(!SrchRegionsActive)
    return(0);

  int i,j;
  int *S;
  int *D;
  for(int r = 0; r < MAX_REGIONS; r++)
  {
    if(SrchRegion[r].Active == 1)
    {
      int Img_X;
      if(SrchRegion[r].Colour == 0)
        Img_X = GetYWidth();
      else
        Img_X = GetUVWidth();
      int X = SrchRegion[r].Ext_Sb_X;
      int Y = SrchRegion[r].Ext_Sb_Y;

      //Get the central image part 1st.
      S = SrchRegion[r].pSrc;
      D = (int *)(SrchRegion[r].pExtSrc + X*SrchRegion[r].extra_y + SrchRegion[r].extra_x);
      for(i = 0; i < SrchRegion[r].Sb_Y; i++)
        for(j = 0; j < SrchRegion[r].Sb_X; j++)
        {
          *(D + i*X + j) = *(S + i*Img_X + j);
        }//end for i & j...

      //Get the top left extention. A repeat of the corner pixel.
      S = SrchRegion[r].pSrc;
      D = SrchRegion[r].pExtSrc;
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *S;
        }//end for i & j...

      //Get the top right extention. A repeat of the corner pixel.
      S = (int *)(SrchRegion[r].pSrc + SrchRegion[r].Sb_X - 1);
      D = (int *)(SrchRegion[r].pExtSrc + X - SrchRegion[r].extra_x);
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *S;
        }//end for i & j...

      //Get the bottom left extention. A repeat of the corner pixel.
      S = (int *)(SrchRegion[r].pSrc + Img_X*(SrchRegion[r].Sb_Y - 1));
      D = (int *)(SrchRegion[r].pExtSrc + X*(Y - SrchRegion[r].extra_y));
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *S;
        }//end for i & j...

      //Get the bottom right extention. A repeat of the corner pixel.
      S = (int *)(SrchRegion[r].pSrc + Img_X*SrchRegion[r].Sb_Y - 1);
      D = (int *)(SrchRegion[r].pExtSrc + X*(Y - SrchRegion[r].extra_y) + (X - SrchRegion[r].extra_x));
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *S;
        }//end for i & j...

      //Get the top extention. A repeat of the top pixels.
      S = SrchRegion[r].pSrc;
      D = (int *)(SrchRegion[r].pExtSrc + SrchRegion[r].extra_x);
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].Sb_X; j++)
        {
          *(D + i*X + j) = *(S + j);
        }//end for i & j...

      //Get the bottom extention. A repeat of the bottom pixels.
      S = (int *)(SrchRegion[r].pSrc + Img_X*(SrchRegion[r].Sb_Y - 1));
      D = (int *)(SrchRegion[r].pExtSrc + X*(Y - SrchRegion[r].extra_y) + SrchRegion[r].extra_x);
      for(i = 0; i < SrchRegion[r].extra_y; i++)
        for(j = 0; j < SrchRegion[r].Sb_X; j++)
        {
          *(D + i*X + j) = *(S + j);
        }//end for i & j...

      //Get the left extention. A repeat of the left pixels.
      S = SrchRegion[r].pSrc;
      D = (int *)(SrchRegion[r].pExtSrc + X*SrchRegion[r].extra_y);
      for(i = 0; i < SrchRegion[r].Sb_Y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *(S + i*Img_X);
        }//end for i & j...

      //Get the right extention. A repeat of the right pixels.
      S = (int *)(SrchRegion[r].pSrc + SrchRegion[r].Sb_X - 1);
      D = (int *)(SrchRegion[r].pExtSrc + X*SrchRegion[r].extra_y + (X - SrchRegion[r].extra_x));
      for(i = 0; i < SrchRegion[r].Sb_Y; i++)
        for(j = 0; j < SrchRegion[r].extra_x; j++)
        {
          *(D + i*X + j) = *(S + i*Img_X);
        }//end for i & j...

    }//end if Active...
  }//end for r...
  return(1);
}//end UpdateMotionSearchWin.

//The coord region_x,region_y is in sub-band coord. not ext. sub-band
//or image coord.
int CWImage::EstimateMotionVec(int *Blk,int Region,int region_x,int region_y,int *x,int *y)
{
  //Is the region active?
  if(!SrchRegionsActive || (SrchRegion[Region].Active == 0))
    return(0);

  int Sx,Sy,Lx,Ly;
  long int min_dist = 10000000;
  *x = 0;
  *y = 0;
  int X = SrchRegion[Region].Ext_Sb_X;
  int BlkX = SrchRegion[Region].blk_x_dim;
  //Convert region coords into extended sub-band coords.
  int *P = (int *)(SrchRegion[Region].pExtSrc + X*(SrchRegion[Region].extra_y + region_y) + (SrchRegion[Region].extra_x + region_x));
  for(Sy = -(SrchRegion[Region].Vec_Y_rng); Sy < SrchRegion[Region].Vec_Y_rng; Sy++)
    for(Sx = -(SrchRegion[Region].Vec_X_rng); Sx < SrchRegion[Region].Vec_X_rng; Sx++)
    {
      long int acc = 0;
      //Local blk top left ptr is offset from search region top left ptr.
      int *L = (int *)(P + X*Sy + Sx);
      for(Ly = 0; Ly < SrchRegion[Region].blk_y_dim; Ly++)
        for(Lx = 0; Lx < SrchRegion[Region].blk_x_dim; Lx++)
        {
          long int v = Blk[BlkX*Ly + Lx] - L[X*Ly + Lx];
          acc = acc + (v*v);
        }//end for Ly & Lx...
      if(acc < min_dist)
      {
        min_dist = acc;
        *x = Sx;
        *y = Sy;
      }//end if acc...
    }//end for Sy & Sx...

  return(1);
}//end EstimateMotionVec.

int CWImage::CompensateMotionVec(int *Blk,int Region,int region_x,int region_y,int x,int y)
{
  //Is the region active?
  if(!SrchRegionsActive || (SrchRegion[Region].Active == 0))
    return(0);

  int Lx,Ly;
  int X = SrchRegion[Region].Ext_Sb_X;
  int BlkX = SrchRegion[Region].blk_x_dim;
  //Convert region coords into extended sub-band coords.
  int *P = (int *)(SrchRegion[Region].pExtSrc + X*(SrchRegion[Region].extra_y + region_y) + (SrchRegion[Region].extra_x + region_x));
  //Bounds check.
  if((SrchRegion[Region].extra_y + region_y + y + SrchRegion[Region].blk_y_dim) > SrchRegion[Region].Ext_Sb_Y)
    return(0);
  if((SrchRegion[Region].extra_y + region_y + y) < 0)
    return(0);
  if((SrchRegion[Region].extra_x + region_x + x + SrchRegion[Region].blk_x_dim) > SrchRegion[Region].Ext_Sb_X)
    return(0);
  if((SrchRegion[Region].extra_x + region_x + x) < 0)
    return(0);
  //Local blk top left ptr is offset from search region top left ptr.
  int *L = (int *)(P + X*y + x);
  for(Ly = 0; Ly < SrchRegion[Region].blk_y_dim; Ly++)
    for(Lx = 0; Lx < SrchRegion[Region].blk_x_dim; Lx++)
    {
      Blk[BlkX*Ly + Lx] = L[X*Ly + Lx];
    }//end for Ly & Lx...

  return(1);
}//end CompensateMotionVec.

//The coord region_x,region_y is in sub-band coord. not ext. sub-band
//or image coord.
int CWImage::HalfPixEstimateMotionVec(int *Blk,int Region,int region_x,int region_y,int *x,int *y)
{
  //Is the region active?
  if(!SrchRegionsActive || (SrchRegion[Region].Active == 0))
    return(0);

  int Sx,Sy,Lx,Ly;
  long int min_dist = 10000000;
  *x = 0;
  *y = 0;
  int X = SrchRegion[Region].Ext_Sb_X;
  int BlkX = SrchRegion[Region].blk_x_dim;
  //Convert region coords into extended sub-band coords of (0,0) 
  //coord of search space.
  int *P = (int *)(SrchRegion[Region].pExtSrc + X*(SrchRegion[Region].extra_y + region_y) + (SrchRegion[Region].extra_x + region_x));
  //Vector range values are in pixel values. There are 2x the number
  //of half pixel locations in each dimension.
  for(Sy = -(SrchRegion[Region].Vec_Y_rng * 2); Sy < (SrchRegion[Region].Vec_Y_rng * 2); Sy++)
    for(Sx = -(SrchRegion[Region].Vec_X_rng * 2); Sx < (SrchRegion[Region].Vec_X_rng * 2); Sx++)
    {
      //Sx and Sy are in half pixel dimensions.
      int p_x = Sx/2;
      int p_y = Sy/2;
      int h_x = Sx%2;
      int h_y = Sy%2;
      long int acc = 0;
      //Local blk top left ptr is offset from search region top left ptr.
      int *L = (int *)(P + X*p_y + p_x);
      for(Ly = 0; Ly < SrchRegion[Region].blk_y_dim; Ly++)
        for(Lx = 0; Lx < SrchRegion[Region].blk_x_dim; Lx++)
        {
          int L_pix;
          if( (h_x!=0)&&(h_y!=0) )
            L_pix = (L[X*Ly + Lx] + L[X*(Ly+h_y) + Lx] + L[X*(Ly+h_y) + (Lx+h_x)] + L[X*Ly + (Lx+h_x)] + 2)/4;
          else if( (h_x!=0)&&(h_y==0) )
            L_pix = (L[X*Ly + Lx] + L[X*Ly + (Lx+h_x)] + 1)/2;
          else if( (h_x==0)&&(h_y!=0) )
            L_pix = (L[X*Ly + Lx] + L[X*(Ly+h_y) + Lx] + 1)/2;
          else //if ( (h_x==0)&&(h_y==0) )
            L_pix = L[X*Ly + Lx];

          long int v = Blk[BlkX*Ly + Lx] - L_pix;
          acc = acc + (v*v);
        }//end for Ly & Lx...
      if(acc < min_dist)
      {
        min_dist = acc;
        *x = Sx;
        *y = Sy;
      }//end if acc...
    }//end for Sy & Sx...

  return(1);
}//end HalfPixEstimateMotionVec.

int CWImage::HalfPixCompensateMotionVec(int *Blk,int Region,int region_x,int region_y,int x,int y)
{
  //Is the region active?
  if(!SrchRegionsActive || (SrchRegion[Region].Active == 0))
    return(0);

  int Lx,Ly;
  int X = SrchRegion[Region].Ext_Sb_X;
  int BlkX = SrchRegion[Region].blk_x_dim;
  //Convert region coords into extended sub-band coords.
  int *P = (int *)(SrchRegion[Region].pExtSrc + X*(SrchRegion[Region].extra_y + region_y) + (SrchRegion[Region].extra_x + region_x));
  // x and y are in half pixel coordinates.
  int Sx = x/2;
  int Sy = y/2;
  int h_x = x%2;
  int h_y = y%2;
  //Bounds check.
  if((SrchRegion[Region].extra_y + region_y + (Sy+1) + SrchRegion[Region].blk_y_dim) > SrchRegion[Region].Ext_Sb_Y)
    return(0);
  if((SrchRegion[Region].extra_y + region_y + (Sy-1)) < 0)
    return(0);
  if((SrchRegion[Region].extra_x + region_x + (Sx+1) + SrchRegion[Region].blk_x_dim) > SrchRegion[Region].Ext_Sb_X)
    return(0);
  if((SrchRegion[Region].extra_x + region_x + (Sx-1)) < 0)
    return(0);
  //Local blk top left ptr is offset from search region top left ptr.
  int *L = (int *)(P + X*Sy + Sx);
  for(Ly = 0; Ly < SrchRegion[Region].blk_y_dim; Ly++)
    for(Lx = 0; Lx < SrchRegion[Region].blk_x_dim; Lx++)
    {
      int L_pix;
      if( (h_x!=0)&&(h_y!=0) )
        L_pix = (L[X*Ly + Lx] + L[X*(Ly+h_y) + Lx] + L[X*(Ly+h_y) + (Lx+h_x)] + L[X*Ly + (Lx+h_x)] + 2)/4;
      else if( (h_x!=0)&&(h_y==0) )
        L_pix = (L[X*Ly + Lx] + L[X*Ly + (Lx+h_x)] + 1)/2;
      else if( (h_x==0)&&(h_y!=0) )
        L_pix = (L[X*Ly + Lx] + L[X*(Ly+h_y) + Lx] + 1)/2;
      else //if ( (h_x==0)&&(h_y==0) )
        L_pix = L[X*Ly + Lx];

      Blk[BlkX*Ly + Lx] = L_pix;
    }//end for Ly & Lx...

  return(1);
}//end HalfPixCompensateMotionVec.

int CWImage::GetNumVectorVLCBits(int x,int y)
{
  int abs_x = abs(x);
  int abs_y = abs(y);
  if( (abs_x >= MAX_VLC_BITS)||(abs_y >= MAX_VLC_BITS) )
    return(0);

  int bits = MotVecBits[abs_x].numbits;
  if(x != 0)
    bits++;
  bits = bits + MotVecBits[abs_y].numbits;
  if(y != 0)
    bits++;
  return(bits);
}//end GetNumVectorVLCBits.


