//**********************************************************
// TITLE       :VECTOR QUANTISER CODEC CLASS FILE
// VERSION     :1.0
// FILE        :VQcodec.cpp
// DESCRIPTION :A class for implementing a vector quantiser.
// DATE        :August 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include "vqcodec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////
// Public Implementations.

CVQCODEC::CVQCODEC()
{
  map_w_int = NULL;

  VLCTableExists = 0;
  IntegerBitPrecision = 8;
}//end CVQCODEC Constructor.

CVQCODEC::~CVQCODEC()
{
  //Please close the door behind you.
  if(!Empty)
    CloseCODEC();
}//end CVQCODEC Destructor.

int CVQCODEC::OpenCODEC(void)
{
  //Clear out any previously defined maps.
  if(!Empty)
    CloseCODEC();

  // Set up the memory space.
  if( !CreateIntMemOnCurrParams() )
  {
    return(0);
  }//end if !CreateIntMemOnCurrParams...

  return(CSOM::Open());
}//end OpenCODEC.

int CVQCODEC::OpenCODEC(const char *VQFilename)
{
  // Check for old VQ.
  if(!Empty)
    CloseCODEC();

  // Call base class.
  if(!CSOM2D::Open(VQFilename))
  {
    CloseCODEC();
    return(0);
  }//end if !Open...

  // Set up the integer map memory space.
  if( !CreateIntMemOnCurrParams() )
  {
    CloseCODEC();
    return(0);
  }//end if !CreateIntMemOnCurrParams...
  UpdateIntMap();

  return(1);
}//end OpenCODEC.

int CVQCODEC::CreateIntMemOnCurrParams(void)
{
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;

	map_w_int = NULL;
	map_w_int = new int[MapReq];
	if(map_w_int == NULL)
  {
    error = 1;//VECTOR QUANTISATION MEMORY UNAVAILABLE.
    CloseCODEC();
	  return(0);
  }//end if map_w_int...

  return(1);
}//end CreateIntMemOnCurrParams.

void CVQCODEC::DestroyIntMem(void)
{
	//Delete if it exists.
	if(map_w_int)
		delete[] map_w_int;

  map_w_int = NULL;

}//end DestroyIntMem.

int CVQCODEC::AssocciateVLC(const char *VLCFilename)
{
  if(Empty)
  {
    error = 4; //VECTOR QUANTISATION MAP DOES NOT EXIST.
    return(0);
  }//end if Empty...

  // Open the VLC file.
  FILE *F;
  if((F = fopen(VLCFilename,"r")) == NULL)
  {
    error = 11; //VLC FILE DOES NOT EXIST.
    return(0);
  }//end if F...

  // Check for old VLC.
  if(VLCTableExists)
    DestroyVLCMem();

  // Load the parameters.
  int TableSize;
  fscanf(F,"%d\n",&TableSize);

  if( TableSize != (map_y_dim * map_x_dim) )
  {
    fclose(F);
    error = 12; //VLC TABLE DOES NOT MATCH VQ PARAMETERS.
    return(0);
  }//end if TableSize...

  // Set up the memory space.
	h_vlc = GlobalAlloc(GMEM_FIXED,TableSize * sizeof(VLC_BITS));
	if(!h_vlc)
  {
    fclose(F);
    error = 15;//VLC TABLE MEMORY UNAVAILABLE.
	  return(0);
  }//end if !h_vlc...
	vlc = (VLC_BITS *)GlobalLock(h_vlc);

  // Load the table from the file.
  // File format per line = [index number_of_bits code_bits].
  int i;
  int index, number_of_bits, code_bits;
  int Num_read = 0;
  int IsEnd;
  for( i = 0; i < TableSize; i++)
  {
    Num_read += fscanf(F,"%d %d %d\n",&index,&number_of_bits,&code_bits);
    IsEnd = feof(F);
    if(index >= TableSize)
    {
      error = 15;//VLC INDEX OUT OF RANGE.
      fclose(F);
      DestroyVLCMem();
	    return(0);
    }//end if index...
    vlc[index].numbits = number_of_bits;
    vlc[index].codebits = code_bits;
  }//end for i...
  if(Num_read != (TableSize*3))
  {
    error = 14;//VLC TABLE NOT READ.
    fclose(F);
    DestroyVLCMem();
	  return(0);
  }//end if Num_read...
  VLCTableExists = 1;

  fclose(F);
  return(1);
}//end AssocciateVLC...

void CVQCODEC::DestroyVLCMem(void)
{
  if(h_vlc != NULL)
  {
		GlobalUnlock(h_vlc);
 		GlobalFree(h_vlc);
  }//end if h_vlc...
 	h_vlc = NULL;
  vlc = NULL;
  VLCTableExists = 0;
}//end DestroyVLCMem.

int CVQCODEC::CODE(double *vec,int *q_index)
{
  return(CSOM2D::GetWinner(vec,q_index));
}//end CODE.

int CVQCODEC::CODE(int *vec,int *q_index)
{
  if(Empty)
  {
    error = 4; //VECTOR QUANTISATION MAP DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Do a full search.
  int MapVectors = map_y_dim * map_x_dim;
  int InputDim = vec_y_dim * vec_x_dim;
  int i,j;
  double Eucled,Min_Eucled,diff;
  Min_Eucled = 100000000.0;
  int k = 0; //Winning vector.
  for(i = 0; i < MapVectors; i++)
  {
    //Eucledean dist.
    Eucled = 0.0;
    for(j = 0; j < InputDim; j++)
    {
      diff = *(vec + j) - *(map_w_int + i*InputDim + j);
      Eucled = Eucled + (double)(diff * diff);
    }//end for j...
    Eucled = sqrt(Eucled);
    if( Eucled < Min_Eucled )
    {
      Min_Eucled = Eucled;
      k = i;
    }//end if Eucled...
  }//end for i...
  win_distortion = Min_Eucled;

  last_win_index = k;
  *q_index = k;
  return(1);
}//end CODE.

unsigned long int CVQCODEC::NumOfCodedBits(int q_index)
{
  unsigned long int BitLen;
  int MaxIndex = map_y_dim * map_x_dim;

  BitLen = 0;
  if(!VLCTableExists)
  {
    BitLen = 8;
  }//end if !VLCTableExists...
  else
  {
    int i = (int)abs(q_index);
    if(i < MaxIndex)
      BitLen = vlc[i].numbits;
  }//end else...

  return(BitLen);
}//end NumOfCodedBits.

int CVQCODEC::DECODE(int q_index,double *vec)
{
  return(CSOM2D::GetVector(q_index,vec));
}//end DECODE.

int CVQCODEC::DECODE(int q_index,int *vec)
{
  if(Empty)
  {
    error = 4; //VECTOR QUANTISATION MAP DOES NOT EXIST.
    return(0);
  }//end if Empty...

  //Copy the vector to the output parameter.
  int MapVectors = map_y_dim * map_x_dim;
  if( q_index >= MapVectors )
  {
    error = 5; //VECTOR QUANTISATION MAP INDEX OUT OF RANGE.
    return(0);
  }//end if q_index...

  int InputDim = vec_y_dim * vec_x_dim;
  int i;
  for(i = 0; i < InputDim; i++)
  {
    *(vec + i) = *(map_w_int + q_index*InputDim + i);
  }//end for i...

  return(1);
}//end DECODE.

void CVQCODEC::CloseCODEC(void)
{
  //Free working memory.
  DestroyIntMem();
  //Free the VLC memory.
  if(VLCTableExists)
    DestroyVLCMem();
  VLCTableExists = 0;

  //Call the base class.
  CSOM2D::Close();
}//end CloseCODEC.

const char *CVQCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													             //0
		 "VECTOR QUANTISATION MEMORY UNAVAILABLE",         //1
     "FUNCTION UNIMPLEMENTED",                         //2
     "VECTOR QUANTISATION FILE DOES NOT EXIST",        //3
     "VECTOR QUANTISATION MAP DOES NOT EXIST",         //4
     "VECTOR QUANTISATION MAP INDEX OUT OF RANGE",     //5
     "VECTOR QUANTISATION MAP NOT READ",               //6
     "CANNOT OPEN VECTOR QUANTISATION FILE",           //7
     "CANNOT OPEN PMF FILE",                           //8
     "EXCEEDED MAX ALLOWABLE SEARCH REGIONS",          //9
     "SEARCH REGIONS HAVE NOT BEEN PREPARED",          //10
     "VLC FILE DOES NOT EXIST",                        //11
     "VLC TABLE DOES NOT MATCH VQ PARAMETERS",         //12
     "VLC TABLE MEMORY UNAVAILABLE",                   //13
     "VLC TABLE NOT READ",                             //14
     "VLC INDEX OUT OF RANGE"};                        //15

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

void CVQCODEC::SetIntegerBitPrecision(int Bpp)
{
  if(Bpp > 8)
    IntegerBitPrecision = 8;
  else if (Bpp < 1)
    IntegerBitPrecision = 1;
  else
    IntegerBitPrecision = Bpp;
}//end SetIntegerBitPrecision.

////////////////////////////////////////////////////////////////
// Utility functions.
////////////////////////////////////////////////////////////////

// Update the integer map from the real map.
// Range change from 0.0 - 1.0 to 0 - 255.
void CVQCODEC::UpdateIntMap(void)
{
  if(Empty)
    return;

  double multiplier = (double)((1 << IntegerBitPrecision) - 1);
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;
  for(int i = 0; i < MapReq; i++)
  {
    if(map[i] < 0.0)
      map_w_int[i] = (int)(map[i]*multiplier - 0.5);
    else
      map_w_int[i] = (int)(map[i]*multiplier + 0.5);
  }//end for i...
}//end UpdateIntMap.

// Update the integer map from the real map.
// Range change from -1.0..+1.0 to -128..+128 (Mid-rise).
void CVQCODEC::UpdateIntMap2(void)
{
  if(Empty)
    return;

  double multiplier = (double)(1 << (IntegerBitPrecision - 1));
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;
  for(int i = 0; i < MapReq; i++)
  {
    if(map[i] < 0.0)
      map_w_int[i] = (int)(map[i]*multiplier - 0.5);
    else
      map_w_int[i] = (int)(map[i]*multiplier + 0.5);
  }//end for i...
}//end UpdateIntMap2.

// Update the real map from the integer map.
// Range change from 0 - 255 to 0.0 - 1.0.
void CVQCODEC::UpdateRealMap(void)
{
  if(Empty)
    return;

  double divisor = (double)((1 << IntegerBitPrecision) - 1);
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;
  for(int i = 0; i < MapReq; i++)
  {
    map[i] = (double)(map_w_int[i]) / divisor;
  }//end for i...
}//end UpdateRealMap.

// Update the real map from the integer map.
// Range change from -128..+128 to -1.0..+1.0 (Mid-rise).
void CVQCODEC::UpdateRealMap2(void)
{
  if(Empty)
    return;

  double divisor = (double)((1 << IntegerBitPrecision) - 1);
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;
  for(int i = 0; i < MapReq; i++)
  {
    map[i] = (double)(map_w_int[i]) / divisor;
  }//end for i...
}//end UpdateRealMap2.

int CVQCODEC::SaveRealMap(const char *VQFilename)
{
  return(CSOM2D::SaveMap(VQFilename));
}//end SaveRealMap.

int CVQCODEC::SaveIntMap(const char *VQFilename)
{
  if(Empty)
  {
    error = 4; //VECTOR QUANTISATION MAP DOES NOT EXIST.
    return(0);
  }//end if Empty...

  // Open the VQ file.
  FILE *F;
  if((F = fopen(VQFilename,"w")) == NULL)
  {
    error = 7; //CANNOT OPEN VECTOR QUANTISATION FILE.
    return(0);
  }//end if F...
  // File is OK, so save the name.
  strcpy(Filename,VQFilename);

  //Store the input vector dimension and the SOM map dimension.
  fprintf(F,"%d %d %d %d\n",vec_y_dim,vec_x_dim,map_y_dim,map_x_dim);
  int MapReq = map_y_dim * map_x_dim * vec_y_dim * vec_x_dim;
  for(int i = 0; i < MapReq; i++)
    fprintf(F,"%d\n",map_w_int[i]);
  fclose(F);
  return(1);
}//end SaveIntMap.

// Improve the utility of the integer map by replacing
// repeated weight vectors with the immediate neighbourhood.
void CVQCODEC::ImproveIntMapWithAvg(int passes)
{
  if(Empty)
    return;

  int MapDim = map_y_dim * map_x_dim;
  int InputDim = vec_y_dim * vec_x_dim;

  #define NUM_VECTORS 10
  int i,j,x,y,map_index;
  double *vec[NUM_VECTORS];
  HGLOBAL h_vec[NUM_VECTORS];
  int Failure = 0;
  for(i = 0; i < NUM_VECTORS; i++)
  {
    h_vec[i] = NULL;
  	h_vec[i] = GlobalAlloc(GMEM_FIXED,InputDim*sizeof(double));
    if(h_vec[i] == NULL)
      Failure = 1;
    else
      vec[i] = (double *)GlobalLock(h_vec[i]);
  }//end for i...
  if(Failure)
  {
    for(i = 0; i < NUM_VECTORS; i++)
    {
      if(h_vec[i] != NULL)
      {
		    GlobalUnlock(h_vec[i]);
 		    GlobalFree(h_vec[i]);
      }//end if h_vec...
    }//end for i...
    return;
  }//end if Failure...

  //Repeat the operation passes number of times.
  for(j = 0; j < passes; j++)
  {
    for(int i_row = 0; i_row < map_y_dim; i_row++)
      for(int i_col = 0; i_col < map_x_dim; i_col++)
      {
        //Zero the last temp vector to use as an accum.
        for(i = 0; i < InputDim; i++)
          *(vec[NUM_VECTORS-1] + i) = 0.0;
  
        //Collect the immediate neighbourhood vectors and accum.
        //their per component total;
        for(y = (i_row-1); y < (i_row+1); y++)
          for(x = (i_col-1); x < (i_col+1); x++)
          {
            //Map wrapping.
            int m_row = GetWrappedMapRow(y);
            int m_col = GetWrappedMapCol(x);
            //Map and vector index.
            map_index = m_row*map_x_dim + m_col;
            int vec_index = (y - (i_row-1))*3 + (x - (i_col-1));
            for(i = 0; i < InputDim; i++)
            {
              *(vec[vec_index] + i) = (double)(map_w_int[map_index*InputDim + i]);
              if(vec_index != 4) //Exclude centre vector.
                *(vec[NUM_VECTORS-1] + i) += *(vec[vec_index] + i);
            }//end for i...
          }//end for y & x...
        //Check for any vectors in the neighb. that are equal.
        int IsEqual = 0;
        for(x = 0; x < 9; x++)
        {
          if(x != 4) //Exclude the centre vector.
          {
            double diff = 0.0;
            for(i = 0; i < InputDim; i++)
              diff += (*(vec[4] + i) - *(vec[x] + i))*(*(vec[4] + i) - *(vec[x] + i));
            diff = diff/InputDim;
            if(diff < 0.0000001)
              IsEqual = 1;
          }//end if x...
        }//end for x...
        //If any are equal then replace this centre vector
        //with the average.
        if(IsEqual)
        {
          map_index = i_row*map_x_dim + i_col;
          for(i = 0; i < InputDim; i++)
          {
            double avg_val = *(vec[NUM_VECTORS-1] + i) / 8.0;
            if(avg_val < 0.0)
              map_w_int[map_index*InputDim + i] = (int)(avg_val - 0.5);
            else
              map_w_int[map_index*InputDim + i] = (int)(avg_val + 0.5);
          }//end for i...
        }//end if IsEqual...
      }//end for i_col...
  }//end for j...

  for(i = 0; i < NUM_VECTORS; i++)
  {
    if(h_vec[i] != NULL)
    {
	    GlobalUnlock(h_vec[i]);
 	    GlobalFree(h_vec[i]);
    }//end if h_vec...
  }//end for i...
}//end ImproveIntMapWithAvg.


