// Definition of a CWImage class that is a type of CImage
// object. It adds functionality to images that have been
// processed as a wavelet transform.
#ifndef _WIMAGE_H
#define _WIMAGE_H

#include <stdlib.h>
#include "CImage.h"
#include "WImgPce.h"

///////////////////////////////////////////////////////////////
// Wavelet Image Constants and Structures.
///////////////////////////////////////////////////////////////

// Definition of Wavelet Positions.
#define LOW_X_LOW_Y      0
#define LOW_X_HIGH_Y     1
#define HIGH_X_LOW_Y     2
#define HIGH_X_HIGH_Y    3

#define MAX_LEVELS 6

typedef struct tagTEXTURE_TYPE
{
  int LumLevel;
  int ChrLevel;
  HGLOBAL hImg;
  int *pY;
  int *pU;
  int *pV;
  int Lum_X;
  int Lum_Y;
  int Chr_X;
  int Chr_Y;
} TEXTURE_TYPE;

///////////////////////////////////////////////////////////////
// Wavelet Image Motion Estimation Constants and Structures.
///////////////////////////////////////////////////////////////
// These should be defined for the Wavelet image to be used as
// the reference image or searched from image.

// This structure defines the requirements for the overall motion
// estimation parameters. The starting level, block dim and vector
// range determines all higher resolution sub-bands. This is used
// to set up the individual SEARCH_SUBBAND_STRUCTURE's.

typedef struct tagSEARCH_SETTING_STRUCTURE
{
  int blk_x_dim; //Image block dimensions.
  int blk_y_dim;
  int blk_x_rng; //Motion vector range.
  int blk_y_rng;
} SEARCH_SETTING_STRUCTURE;

typedef struct tagMOTION_STRUCTURE
{
  int StartLevel;
  BOOL TextureEstimation; //Do texture estimation.
  BOOL ColourEstimation; //Include colour or not.
  int colour_x_sub;
  int colour_y_sub;
  int start_blk_x_dim; //Image block dimensions.
  int start_blk_y_dim;
  int Start_Vec_X_rng; //Motion vector range.
  int Start_Vec_Y_rng;
  BOOL IndependentEstimation;
  SEARCH_SETTING_STRUCTURE SetBlk[MAX_LEVELS];
  int HalfPixel;
} MOTION_STRUCTURE;

// Define sub-band parameters including extended boundaries.
typedef struct tagSEARCH_SUBBAND_STRUCTURE
{
  int Active;
  int SubBand; //Sub-Band parameters.
  int Level;
  int Colour;
  int Sb_X;
  int Sb_Y;
  int *pSrc;  //Position in total image.
  int Ext_Sb_X; //Extended sub-band parameters.
  int Ext_Sb_Y;
  int extra_x;
  int extra_y;
  int *pExtSrc;
  int blk_x_dim; //Image block dimensions.
  int blk_y_dim;
  int Vec_X_rng; //Motion vector range.
  int Vec_Y_rng;
} SEARCH_SUBBAND_STRUCTURE;

//(Sub-bands * levels * colour components).
// Referenced: SrchRegion[lev-1][colour][subB];
// Referenced: SrchRegion[(lev-1)*(COLOURS*SUBBANDS) + colour*COLOURS + subB];
#define MAX_REGIONS (4 * 6 * 3)

// Define vector vlc structure.
typedef struct tagVECTOR_VLC_BITS
{
  int numbits;
  int codebits;
} VECTOR_VLC_BITS;

//////////////////////////////////////////////////////////////////
// Wavelet Image Class definition.
//////////////////////////////////////////////////////////////////
class CWImage : public CImage
{
// Attributes
protected:
  MOTION_STRUCTURE motion_param;
  HGLOBAL hSrchSubBands[MAX_REGIONS]; //For extended sub-band mem.

public:
  TEXTURE_TYPE Texture;
  SEARCH_SUBBAND_STRUCTURE SrchRegion[MAX_REGIONS];
  BOOL SrchRegionsActive;

// Operations
public:
  int GetMaxYLevels(void);
  int GetMaxUVLevels(void);
	int GetWvltLumEnergy(int level,int WvltPos);
	int GetWvltChrEnergy(int level,int WvltPos);
	int MultWvltLum(int level,int WvltPos,double By);
	int MultWvltChr(int level,int WvltPos,double By);
	int ThresholdWvltLum(int level,int WvltPos,int with);
	int ThresholdWvltChr(int level,int WvltPos,int with);
	int *GetWvltPtr(int level,int WvltPos,int WvltCol);
	int GetWvltXPos(int level,int WvltPos,int WvltCol);
	int GetWvltYPos(int level,int WvltPos,int WvltCol);
	int GetWvltWidth(int level,int WvltCol);
	int GetWvltHeight(int level,int WvltCol);

// Implementation
public:
	CWImage();
	~CWImage();
  int CreateTextureImage(int LumLevel,int ChrLevel);
  void DestroyImage(void); //Overloaded function.
  void DestroyTextureImage(void);
  void UpdateTexture(void);
  void UpdateBmp(void);

  int PrepareMotionSearchWin(MOTION_STRUCTURE *Params);
  void DestroyMotionSearchWin(void);
  int UpdateMotionSearchWin(void);
  int GetRegion(int Level,int SubBand,int Colour) {return(((((Level-1) * 3) + Colour) * 4) + SubBand);}
  int EstimateMotionVec(int *Blk,int Region,int region_x,int region_y,int *x,int *y);
  int CompensateMotionVec(int *Blk,int Region,int region_x,int region_y,int x,int y);
  int HalfPixEstimateMotionVec(int *Blk,int Region,int region_x,int region_y,int *x,int *y);
  int HalfPixCompensateMotionVec(int *Blk,int Region,int region_x,int region_y,int x,int y);
  int GetNumVectorVLCBits(int x,int y);

protected:

};//CWImage class.

#endif


