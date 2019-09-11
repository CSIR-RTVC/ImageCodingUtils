//****************************************************************
// This function sets up a CSAMPLER object with a set of image
// files generated from the Video.exe program.
// FILE NAME: ImgSmp.cpp
// AUTHOR   : K.L. Ferguson
// DATE     : October 1999
//****************************************************************
#include "StdAfx.h"
#include "imgsmp.h"

#define LEVEL_4_PROCESSING 0

#if LEVEL_4_PROCESSING /////////////////////////////////////////////////////////

#include "..\VQtoC\RawDWTTestSetL4.h"

#else /////////// Level 3 Images ///////////////////////////

#include "..\VQtoC\RawDWTTestSetL3.h"

#endif /////////////////////////////////////////////////////////

//static char *ImgFiles[NUMIMAGES] =
//{
//  "c:\\home\\keithf\\images\\D_Andy176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Andy176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Andy176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Andy176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Adi176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Adi176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Adi176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Adi176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Btof176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Btof176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Btof176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Btof176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Dan176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Dan176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Dan176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Dan176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Ftob176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Ftob176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Ftob176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Ftob176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Hujun176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Hujun176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Hujun176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Hujun176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Keith176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Keith176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Keith176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Keith176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Ltor176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Ltor176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Ltor176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Ltor176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Maja176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Maja176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Maja176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Maja176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Rtol176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Rtol176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Rtol176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Rtol176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Sit176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Sit176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Sit176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Sit176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Stos176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Stos176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Stos176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Stos176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Wrk176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Wrk176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Wrk176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Wrk176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Bor176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Bor176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Bor176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Bor176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Dsk176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Dsk176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Dsk176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Dsk176s_4.raw",
//  "c:\\home\\keithf\\images\\D_Std176s_1.raw",
//  "c:\\home\\keithf\\images\\D_Std176s_2.raw",
//  "c:\\home\\keithf\\images\\D_Std176s_3.raw",
//  "c:\\home\\keithf\\images\\D_Std176s_4.raw"
//};

int SetImgSampler(CSAMPLER *Sp,int lev,int inxdim,int inydim,
                  int colcomp,int dirsubband);

//*************************************************************
//Read in each group of images and construct long images of 
//concatenated DWT sub-bands. i.e. group the sub-bands. The 
//images are in a raw format generated from the Video.exe program.
int SetImgSampler(CSAMPLER *Sp,int lev,int inxdim,int inydim,
                   int colcomp,int dirsubband)
{
  char ErrStr[80];
  HGLOBAL h_In_img;
  int *In_img;
  h_In_img = NULL;
  In_img = NULL;

  UINT ReadBytes;
  CFile Fl;
	int CheckFile;

  SAMPLER_INFO Y_Space_param;
  SAMPLER_INFO UV_Space_param;
  double *pIn_Space;
  int imgs,col_comp,Y_x,Y_y,UV_x,UV_y;
  int Y_Level_x, Y_Level_y;
  int Y_Total_Level_x;
  int UV_Level_x, UV_Level_y;
  int UV_Total_Level_x;
  int x,y;

  //Get the image details from the 1st image.
	CheckFile = Fl.Open(ImgFiles[0],CFile::modeRead);
  if(!CheckFile)
  {
    printf("File %s does not exist.",ImgFiles[0]);
    return(0);
  }//end if !CheckFile...
  //Get the number of colour components in the file.
  ReadBytes = Fl.Read(&col_comp,sizeof(int));
  //Get the lum component dimensions.
  ReadBytes = Fl.Read(&Y_x,sizeof(int));
  ReadBytes = Fl.Read(&Y_y,sizeof(int));

  //Alloc buffer memory for one file input image. There will
  //always be enough space for the colour components because
  //they are equal or less than that of Lum component.
  h_In_img = GlobalAlloc(GMEM_FIXED,Y_x * Y_y * sizeof(int));
  if(h_In_img == NULL)
  {
    Fl.Close();
    printf("Insufficient memory for image buffer!");
    return(0);
  }//end if h_In_img...
  In_img = (int *)GlobalLock(h_In_img);
  //Dummy read all the Lum values.
  ReadBytes = Fl.Read(In_img,(Y_x * Y_y * sizeof(int)));

  //Get the chr component dimensions. (Assume U dim = V dim).
  ReadBytes = Fl.Read(&UV_x,sizeof(int));
  ReadBytes = Fl.Read(&UV_y,sizeof(int));
  Fl.Close();

  //Determine the operational level dimensions.
  Y_Level_x = Y_x/2;
  Y_Level_y = Y_y/2;
  UV_Level_x = UV_x/2;
  UV_Level_y = UV_y/2;
  for(int l = 1; l < lev; l++)
  {
    Y_Level_x = Y_Level_x/2;
    Y_Level_y = Y_Level_y/2;
    UV_Level_x = UV_Level_x/2;
    UV_Level_y = UV_Level_y/2;
  }//end for l...
  Y_Total_Level_x = NUMIMAGES * Y_Level_x;
  UV_Total_Level_x = NUMIMAGES * UV_Level_x;

  //Create sampling spaces for each sub-band direction long image.
  Y_Space_param.vec_y_dim = inydim;
  Y_Space_param.vec_x_dim = inxdim;
  Y_Space_param.space_y_dim = Y_Level_y;
  Y_Space_param.space_x_dim = Y_Total_Level_x;
  UV_Space_param.vec_y_dim = inydim;
  UV_Space_param.vec_x_dim = inxdim;
  UV_Space_param.space_y_dim = UV_Level_y;
  UV_Space_param.space_x_dim = UV_Total_Level_x;

  int Successful;
  if(colcomp == 0) //Luminance.
  {
    Successful = Sp->Open(&Y_Space_param);
  }//end if colcomp...
  else //Chrominance.
  {
    Successful = Sp->Open(&UV_Space_param);
  }//end else...
  if(!Successful)
  {
    //Clean up memory first.
	  GlobalUnlock(h_In_img);
 	  GlobalFree(h_In_img);
    printf(Sp->GetErrorStr(Sp->Error(),ErrStr));
    return(0);
  }//end if !Successful...
  pIn_Space = Sp->GetSpacePtr();

  //Load 'em up.
  for(imgs = 0; imgs < NUMIMAGES; imgs++)
  {
    //Get each image from each group.
    printf("Loading image %s\n",ImgFiles[imgs]);
		CheckFile = Fl.Open(ImgFiles[imgs],CFile::modeRead);
    if(!CheckFile)
    {
      //Clean up memory.
  	  GlobalUnlock(h_In_img);
 	    GlobalFree(h_In_img);
      printf("File %s does not exist.",ImgFiles[imgs]);
      return(0);
    }//end if !CheckFile...
    Fl.Read(&col_comp,sizeof(int));

    //Load Lum parameters and its image.
    Fl.Read(&Y_x,sizeof(int));
    Fl.Read(&Y_y,sizeof(int));
    //Load into image buffer.
    ReadBytes = Fl.Read(In_img,(Y_x * Y_y * sizeof(int)));
    if(ReadBytes != (Y_x * Y_y * sizeof(int)))
    {
  	  GlobalUnlock(h_In_img);
 	    GlobalFree(h_In_img);
      Fl.Close();
      printf("%d falures to read from file %s.\n",
             (Y_x*Y_y*sizeof(int))-ReadBytes,
             ImgFiles[imgs]);
      return(0);
    }//end if ReadFails...

    //If we are dealing with Lum then copy sub-bands into place.
    if(colcomp == 0) //0=Lum.
    {
      //Select sub-band.
      if(dirsubband == 0) //0=LxHy.
      {
        for(y = 0; y < Y_Level_y; y++)
          for(x = 0; x < Y_Level_x; x++)
            pIn_Space[y*Y_Total_Level_x + imgs*Y_Level_x + x] = ((double)In_img[(y+Y_Level_y)*Y_x + x])/255.0;
      }//end if dirsubband...
      else if(dirsubband == 1) //1=HxHy.
      {
        for(y = 0; y < Y_Level_y; y++)
          for(x = 0; x < Y_Level_x; x++)
            pIn_Space[y*Y_Total_Level_x + imgs*Y_Level_x + x] = ((double)In_img[(y+Y_Level_y)*Y_x + (x+Y_Level_x)])/255.0;
      }//end else if dirsubband...
      else if(dirsubband == 2) //2=HxLy.
      {
        for(y = 0; y < Y_Level_y; y++)
          for(x = 0; x < Y_Level_x; x++)
            pIn_Space[y*Y_Total_Level_x + imgs*Y_Level_x + x] = ((double)In_img[y*Y_x + (x+Y_Level_x)])/255.0;
      }//end else if...
      else //if(dirsubband == 3) //3=LxLy.
      {
        for(y = 0; y < Y_Level_y; y++)
          for(x = 0; x < Y_Level_x; x++)
            pIn_Space[y*Y_Total_Level_x + imgs*Y_Level_x + x] = ((double)In_img[y*Y_x + x])/255.0;
      }//end else...
    }//end if colcomp...

    //Load U Chr parameters and its image.
    Fl.Read(&UV_x,sizeof(int));
    Fl.Read(&UV_y,sizeof(int));
    //Load into image buffer.
    ReadBytes = Fl.Read(In_img,(UV_x * UV_y * sizeof(int)));
    if(ReadBytes != (UV_x * UV_y * sizeof(int)))
    {
  	  GlobalUnlock(h_In_img);
 	    GlobalFree(h_In_img);
      Fl.Close();
      printf("%d falures to read from file %s.\n",
             (UV_x*UV_y*sizeof(int))-ReadBytes,
             ImgFiles[imgs]);
      return(0);
    }//end if ReadFails...

    //If we are dealing with U Chr then copy sub-bands into place.
    if(colcomp == 1) //1=UChr.
    {
      //Select sub-band.
      if(dirsubband == 0) //0=LxHy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[(y+UV_Level_y)*UV_x + x])/255.0;
      }//end if dirsubband...
      else if(dirsubband == 1) //1=HxHy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[(y+UV_Level_y)*UV_x + (x+UV_Level_x)])/255.0;
      }//end else if dirsubband...
      else if(dirsubband == 2) //2=HxLy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[y*UV_x + (x+UV_Level_x)])/255.0;
      }//end else if...
      else //if(dirsubband == 3) //3=LxLy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[y*UV_x + x])/255.0;
      }//end else...
    }//end if colcomp...

    //Load V Chr parameters and its image.
    Fl.Read(&UV_x,sizeof(int));
    Fl.Read(&UV_y,sizeof(int));
    //Load into image buffer.
    ReadBytes = Fl.Read(In_img,(UV_x * UV_y * sizeof(int)));
    if(ReadBytes != (UV_x * UV_y * sizeof(int)))
    {
  	  GlobalUnlock(h_In_img);
 	    GlobalFree(h_In_img);
      Fl.Close();
      printf("%d falures to read from file %s.\n",
             (UV_x*UV_y*sizeof(int))-ReadBytes,
             ImgFiles[imgs]);
      return(0);
    }//end if ReadFails...

    //If we are dealing with V Chr then copy sub-bands into place.
    if(colcomp == 2) //2=VChr.
    {
      //Select sub-band.
      if(dirsubband == 0) //0=LxHy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[(y+UV_Level_y)*UV_x + x])/255.0;
      }//end if dirsubband...
      else if(dirsubband == 1) //1=HxHy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[(y+UV_Level_y)*UV_x + (x+UV_Level_x)])/255.0;
      }//end else if dirsubband...
      else if(dirsubband == 2) //2=HxLy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[y*UV_x + (x+UV_Level_x)])/255.0;
      }//end else if...
      else //if(dirsubband == 3) //3=LxLy.
      {
        for(y = 0; y < UV_Level_y; y++)
          for(x = 0; x < UV_Level_x; x++)
            pIn_Space[y*UV_Total_Level_x + imgs*UV_Level_x + x] = ((double)In_img[y*UV_x + x])/255.0;
      }//end else...
    }//end if colcomp...

    Fl.Close();
  }//end for imgs...

  //Clean up memory not required.
	GlobalUnlock(h_In_img);
 	GlobalFree(h_In_img);
  return(1);
}//end SetImgSampler.

