// Implementation of the CImage class methods.
#include <memory.h>
#include <stdlib.h>
#include "stdafx.h"

#include "CImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// RGB to YUV colour conversion matrix.
#define RtoY_00   (double)(0.299)
#define RtoY_01   (double)(0.587)
#define RtoY_02   (double)(0.114)
#define RtoY_10   (double)(-0.169)
#define RtoY_11   (double)(-0.331)
#define RtoY_12   (double)(0.5)
#define RtoY_20   (double)(0.5)
#define RtoY_21   (double)(-0.419)
#define RtoY_22   (double)(-0.081)

// YUV to RGB colour conversion matrix.
#define YtoR_00   (double)(1.00)
#define YtoR_01   (double)(-0.000927)
#define YtoR_02   (double)(1.401687)
#define YtoR_10   (double)(1.00)
#define YtoR_11   (double)(-0.343695)
#define YtoR_12   (double)(-0.714169)
#define YtoR_20   (double)(1.00)
#define YtoR_21   (double)(1.772160)
#define YtoR_22   (double)(0.00099)

///////////////////////////////////////////////////
// Public Implementations.
///////////////////////////////////////////////////

CImage::CImage()
{
	hbmp = NULL;
	m_PaletteSize = 0;
	bmih = NULL;
	bmic = NULL;
	bmptr = NULL;
	ErrorStr = " ";
	m_ExtraColorType = BMP_INFO_SPECIFIES;
	hY = NULL;
	m_Y = NULL;
	hU = NULL;
	m_U = NULL;
	hV = NULL;
	m_V = NULL;
  BmpFilename = "default.bmp";
}//end CImage Constructor.

CImage::~CImage()
{
  //Don't let the object go without cleaning up.
  DestroyImage();
}//end CImage Destructor.

// Create memory space and copy the bitmap info
// header data.
// Return : 0 = error, 1 = OK.
int CImage::CreateImage(BITMAPINFOHEADER *pbmpih)
{
  if(pbmpih == NULL)
    return(0);

	if( (pbmpih->biCompression != BI_RGB) &&
		  (pbmpih->biCompression != BI_BITFIELDS) )
	  {
		  ErrorStr = "Image is not valid BMP format!";
		  return(0);
	  }//end if biCompression...
	//Determine the image size.
	if(pbmpih->biSizeImage == 0)
	{
		pbmpih->biSizeImage = (DWORD)
			(((((pbmpih->biWidth * pbmpih->biBitCount)
			+ 31)/32) * 4) * pbmpih->biHeight);
	}//end if biSizeImage...
	//Determine the palette colour size.
	if(pbmpih->biCompression == BI_RGB)
	{
		if((pbmpih->biClrUsed == 0)&&(pbmpih->biBitCount<=8))
			m_PaletteSize = (UINT)((1 << pbmpih->biBitCount) * sizeof(RGBQUAD));
		else
			m_PaletteSize = (UINT)(pbmpih->biClrUsed * sizeof(RGBQUAD));
	}
	else //if pbmpih->biCompression == BI_BITFIELDS
	{
		m_PaletteSize = (UINT)(3 * sizeof(DWORD));
	}//end else...

	//Allocate memory for the image.
	hbmp = GlobalAlloc(GMEM_FIXED,
						sizeof(BITMAPINFOHEADER) +
						m_PaletteSize +
		        pbmpih->biSizeImage);
	if(!hbmp)
	  {
  		ErrorStr = "Insufficient memory for image!";
		  return(0);
	  }//end if !hbmp...
	bmih = (BITMAPINFOHEADER *)GlobalLock(hbmp);
  if(bmih == NULL)
    return(0);

	//Load the bitmap info header.
	*(bmih) = *(pbmpih);
	//Determine the pointers that are offset into
	//the memory.
	bmic = (RGBQUAD *)((LPSTR)bmih + (DWORD)sizeof(BITMAPINFOHEADER));
	bmptr = (LPVOID)((LPSTR)bmic + m_PaletteSize);
	m_ExtraColorType = BMP_INFO_SPECIFIES;

	return(1);
}//end CreateImage.

void CImage::DestroyImage(void)
{
	if(hbmp)
	{
		GlobalUnlock(hbmp);
 		GlobalFree(hbmp);
 		hbmp = NULL;
	}//end if hbmp...
	m_PaletteSize = 0;
	bmih = NULL;
	bmic = NULL;
	bmptr = NULL;
	ErrorStr = " ";
  BmpFilename = "default.bmp";

	DestroyYUVMem();
}//end DestroyImage.

int CImage::ImageMemIsAlloc(void)
{
  if(hbmp==NULL)
    return(0);
  return(1);
}//end ImageMemIsAlloc.

// Create memory to store Y,U and V colour data. Convert
// from the current colour space to YUV and force
// the displayable bmp to reflect the new image
// in its current bmp colour format.
int CImage::ConvertToYUV(int YUVFormat)
{
	if(m_ExtraColorType != BMP_INFO_SPECIFIES)
    {
      //Invalidate the YUV memory first.
    	DestroyYUVMem();
    }//end if m_ExtraColorType...

	if(bmih->biBitCount == 24)//RGB 24 bit.
	{
		if(!CreateYUVMem(YUVFormat))
		{
			ErrorStr = "YUV memory unavailable!";
			return(0);
		}//end if !CreateYUVMem...
		if( YUVFormat == YUV411 )
		  ConvertRGB24BITtoYUV411();
		else if( YUVFormat == YUV422 )
		  ConvertRGB24BITtoYUV422();
		else //if( YUVFormat == YUV444 )
		  ConvertRGB24BITtoYUV444();
	}//end else if biBitCount...
	else
	{
	  ErrorStr = "Colour conversion not available";
		return(0);
  }//end else...

	m_ExtraColorType = YUVFormat;
	UpdateBmp();
	return(1);
}//end ConvertToYUV.

void CImage::UpdateBmp(void)
{
	if(bmih->biBitCount == 24)
	{
		if(m_ExtraColorType == YUV411)
		{
			ConvertYUV411toRGB24BIT();
		}//end if m_ExtraColorType...
	  else if(m_ExtraColorType == YUV422)
	  {
			ConvertYUV422toRGB24BIT();
  	}//end else if m_ExtraColorType...
	  else if(m_ExtraColorType == YUV444)
	  {
			ConvertYUV444toRGB24BIT();
  	}//end else if m_ExtraColorType...
	}//end if bmih->biBitCount...
}//end UpdateBmp.

int CImage::GetYWidth(void)
{
	return(bmih->biWidth);
}//end GetYWidth.

int CImage::GetYHeight(void)
{
	return(abs(bmih->biHeight));
}//end GetYHeight.

int CImage::GetUVWidth(void)
{
	int YLine,UVLine;
	YLine = bmih->biWidth;
	if( m_ExtraColorType == YUV411 )
    UVLine = YLine/2;
	else if( m_ExtraColorType == YUV422 )
    UVLine = YLine/2;
	else if( m_ExtraColorType == YUV444 )
    UVLine = YLine;
  else
    UVLine = 0;

  return(UVLine);
}//end GetUVWidth.

int CImage::GetUVHeight(void)
{
	int YCol,UVCol;
	YCol = abs(bmih->biHeight);
	if( m_ExtraColorType == YUV411 )
    UVCol = YCol/2;
	else if( m_ExtraColorType == YUV422 )
    UVCol = YCol;
	else if( m_ExtraColorType == YUV444 )
    UVCol = YCol;
  else
    UVCol = 0;

  return(UVCol);
}//end GetUVHeight.

void CImage::SaveBmp(CString & Filename)
{
  BmpFilename = Filename;
  SaveBmp();
}//end SaveBmp.

void CImage::SaveBmp(void)
{
	CFile Fl;
	int CheckFile;

	CheckFile = Fl.Open(BmpFilename,CFile::modeCreate | CFile::modeWrite);
  if(!CheckFile)
    return;

	//Make the file header.
  BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) +
		m_PaletteSize + 
		bmih->biSizeImage; 
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) +
		m_PaletteSize;

  Fl.Write(&bmfh,sizeof(BITMAPFILEHEADER));
  Fl.Write(bmih,sizeof(BITMAPINFOHEADER));
  Fl.Write(bmic,m_PaletteSize);
  Fl.Write(bmptr,bmih->biSizeImage);

  Fl.Close();

}//end SaveBmp.

int CImage::LoadBmp(CString &Filename)
{
  BmpFilename = Filename;
  return(LoadBmp());
}//end LoadBmp.

int CImage::LoadBmp(void)
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER BitmapHead;
  UINT ReadSize;
	CFile Fl;
	int CheckFile;

  //Make sure image space is cleared.
  CString Temp = BmpFilename;
  DestroyImage();
  BmpFilename = Temp;

	CheckFile = Fl.Open(BmpFilename,CFile::modeRead | CFile::shareDenyWrite);
  if(!CheckFile)
  {
		ErrorStr = "Unable to open Bmp file!";
    return(0);
  }//end if !CheckFile...

	//Read bitmap file header.
	ReadSize = Fl.Read((void *)(&bmfh),sizeof(BITMAPFILEHEADER));
	if(ReadSize != sizeof(BITMAPFILEHEADER))
	{
		ErrorStr = "File is too short!";
    Fl.Close();
		return(0);
	}//end if ReadSize...
	if(bmfh.bfType != 0x4D42)
	{
		ErrorStr = "File is not in BMP file format!";
    Fl.Close();
		return(0);
	}//end if bfType...
	//Read bitmap info header.
	ReadSize = Fl.Read((void *)(&BitmapHead),sizeof(BITMAPINFOHEADER));
	if(ReadSize != sizeof(BITMAPINFOHEADER))
	{
		ErrorStr = "Unable to read from Bmp file!";
    Fl.Close();
		return(0);
	}//end if ReadSize...

  //Construct the new image.
  if(!CreateImage(&BitmapHead))
  {
    Fl.Close();
    return(0);
  }//end if !CreateImage...

  //Read the colour palette.
	ReadSize = Fl.Read((void *)(bmic),m_PaletteSize);
	if(ReadSize != m_PaletteSize)
	{
		ErrorStr = "Unable to read palette from Bmp file!";
    Fl.Close();
		return(0);
	}//end if ReadSize...

  //Read the image bits.
	ReadSize = Fl.Read((void *)bmptr,bmih->biSizeImage);
	if(ReadSize != bmih->biSizeImage)
	{
		ErrorStr = "Unable to read pixel bits from Bmp file!";
    Fl.Close();
		return(0);
	}//end if ReadSize...

  Fl.Close();

  return(1);
}//end LoadBmp.

int CImage::SaveRaw(CString & Filename)
{
  //Save the displayed image as a raw data format.
  //The header is as follows:
  //  1. No. of colour components - int32.
  //  2. 1st colour comp. X size - int 32.
  //  3. 1st colour comp. Y size - int 32.
  //  4. 1st colour comp. data[X rows *Y cols.] - int32.
  //  5. 2nd colour comp. X, Y, data[X*Y].
  //  6. etc. etc. for all colour components.

  //Currently work on YUV image spaces only.
  if(m_ExtraColorType == BMP_INFO_SPECIFIES)
  {
		ErrorStr = "Operation only valid for YUV formats!";
    return(0);
  }//end if !m_ExtraColorType...

  
	CFile Fl;
	int CheckFile;

	CheckFile = Fl.Open(Filename,CFile::modeCreate | CFile::modeWrite);
  if(!CheckFile)
  {
		ErrorStr = "Unable to open Raw file!";
    return(0);
  }//end if !CheckFile...

  // Write out no. of colour components.
  int ColourComponents = 3;
  Fl.Write(&ColourComponents,sizeof(int));

  // Write out luminance component.
  int x = GetYWidth();
  int y = GetYHeight();
  Fl.Write(&x,sizeof(int));
  Fl.Write(&y,sizeof(int));
  Fl.Write(m_Y,x * y * sizeof(int));

  // Write out U and V chrominance component.
  x = GetUVWidth();
  y = GetUVHeight();
  Fl.Write(&x,sizeof(int));
  Fl.Write(&y,sizeof(int));
  Fl.Write(m_U,x * y * sizeof(int));
  Fl.Write(&x,sizeof(int));
  Fl.Write(&y,sizeof(int));
  Fl.Write(m_V,x * y * sizeof(int));

  Fl.Close();
  return(1);
}//end SaveRaw.

///////////////////////////////////////////////////
// Operator overloads.
///////////////////////////////////////////////////

CImage &CImage::operator=(CImage &I)
{
	LPSTR Src,Dst;
	long int Limit;


  if(!EqualMemFormat(I))
  {
    //Destroy old image.
    DestroyImage();

    //Create new image space.
    if(!CreateImage(I.bmih))
    {
	    MessageBox(NULL,ErrorStr,NULL,MB_OK);
 	    return *this;
    }//end if !CreateImage...

  	//Create extra memory if neccessary.
	  if(I.m_ExtraColorType != BMP_INFO_SPECIFIES)
		{
		  if(!CreateYUVMem(I.m_ExtraColorType))
			{
    		MessageBox(NULL,ErrorStr,NULL,MB_OK);
  	    return *this;
			}//end if !CreateYUVMem...
    }//end if m_ExtraColorType...
  }//end if !EqualMemFormat...

	//Copy palette and image data.
	Src = (LPSTR)(I.bmic);
	Dst = (LPSTR)(bmic);
	Limit = (long int)
		      (I.m_PaletteSize + I.bmih->biSizeImage);
	memcpy(Dst,Src,Limit);
	m_PaletteSize = I.m_PaletteSize;

	//Deal with extra colour types.
	m_ExtraColorType = I.m_ExtraColorType;
	//Copy YUV if neccessary.
	if(m_ExtraColorType != BMP_INFO_SPECIFIES)
		{
      //Copy the YUV data.
	    long int YSize,UVSize;
	    YSize = bmih->biWidth * abs(bmih->biHeight) * sizeof(int);
	    if( m_ExtraColorType == YUV411 )
 	      UVSize = YSize/4;
	    else if( m_ExtraColorType == YUV422 )
 	      UVSize = YSize/2;
	    else //YUV444
		    UVSize = YSize;
      Src = (LPSTR)(I.m_Y);
      Dst = (LPSTR)m_Y;
	    memcpy(Dst,Src,YSize);
      Src = (LPSTR)(I.m_U);
      Dst = (LPSTR)m_U;
	    memcpy(Dst,Src,UVSize);
      Src = (LPSTR)(I.m_V);
      Dst = (LPSTR)m_V;
	    memcpy(Dst,Src,UVSize);
		}//end if m_ExtraColorType...

	return *this;
}//end operator=.

CImage &CImage::operator-=(CImage &I)
{
	int *Src,*Dst;
	long int i;

  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	ErrorStr = "Images have different formats!";
 	  return *this;
  }//end if !EqualMemFormat...

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return *this;
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = bmih->biWidth * abs(bmih->biHeight);
	if( m_ExtraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( m_ExtraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Do the subtraction.
  Dst = m_Y;
  Src = I.m_Y;
  for(i = 0; i < YSize; i++)
    {
      *Dst = *Dst - *Src;
      Src++;
      Dst++;
    }//end for i...

  Dst = m_U;
  Src = I.m_U;
  for(i = 0; i < UVSize; i++)
    {
      *Dst = *Dst - *Src;
      Src++;
      Dst++;
    }//end for i...

  Dst = m_V;
  Src = I.m_V;
  for(i = 0; i < UVSize; i++)
    {
      *Dst = *Dst - *Src;
      Src++;
      Dst++;
    }//end for i...

	return *this;
}//end operator-=.

CImage &CImage::operator+=(CImage &I)
{
	int *Src,*Dst;
	long int i;

  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	ErrorStr = "Images have different formats!";
 	  return *this;
  }//end if !EqualMemFormat...

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return *this;
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = bmih->biWidth * abs(bmih->biHeight);
	if( m_ExtraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( m_ExtraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Do the addition.
  Dst = m_Y;
  Src = I.m_Y;
  for(i = 0; i < YSize; i++)
    {
      *Dst = *Dst + *Src;
      Src++;
      Dst++;
    }//end for i...

  Dst = m_U;
  Src = I.m_U;
  for(i = 0; i < UVSize; i++)
    {
      *Dst = *Dst + *Src;
      Src++;
      Dst++;
    }//end for i...

  Dst = m_V;
  Src = I.m_V;
  for(i = 0; i < UVSize; i++)
    {
      *Dst = *Dst + *Src;
      Src++;
      Dst++;
    }//end for i...

	return *this;
}//end operator+=.

int CImage::Copy(CImage &FromImg)
{
	if(!EqualMemFormat(FromImg))
  {
   	ErrorStr = "Images have different formats!";
    return(0);
  }//end if !EqualMemFormat...

	//Copy image data.
	LPSTR S = (LPSTR)(FromImg.bmic);
	LPSTR D = (LPSTR)(bmic);
	long int Limit = (long int)(FromImg.bmih->biSizeImage);
	memcpy(D,S,Limit);

	if(m_ExtraColorType != BMP_INFO_SPECIFIES)
	{
  	int *Dst;
	  int *Src;
	  long int i;

    //Determine the data sizes.
		long int YSize,UVSize;
  
		YSize = bmih->biWidth * abs(bmih->biHeight);
		if( m_ExtraColorType == YUV411 )
 	 	  UVSize = YSize/4;
		else if( m_ExtraColorType == YUV422 )
 	 	  UVSize = YSize/2;
		else //YUV444
			UVSize = YSize;
  
    //Copy luminance.
    Dst = m_Y;
    Src = FromImg.m_Y;
    for(i = 0; i < YSize; i++)
      {
        *Dst++ = *Src++;
      }//end for i...
  
    Dst = m_U;
    Src = FromImg.m_U;
    for(i = 0; i < UVSize; i++)
      {
        *Dst++ = *Src++;
      }//end for i...
  
    Dst = m_V;
    Src = FromImg.m_V;
    for(i = 0; i < UVSize; i++)
      {
        *Dst++ = *Src++;
      }//end for i...
	}//end if m_ExtraColorType...

  return(1);
}//end Copy.

int CImage::EqualMemFormat(CImage &Img)
{
  if( bmih == NULL )
    return(0);

  if( (m_ExtraColorType == Img.m_ExtraColorType) &&
      (bmih->biSize == Img.bmih->biSize) &&
      (bmih->biWidth == Img.bmih->biWidth) &&
      (bmih->biHeight == Img.bmih->biHeight) &&
      (bmih->biPlanes == Img.bmih->biPlanes) &&
      (bmih->biBitCount == Img.bmih->biBitCount) &&
      (bmih->biCompression == Img.bmih->biCompression) &&
      (bmih->biSizeImage == Img.bmih->biSizeImage) &&
      (bmih->biXPelsPerMeter == Img.bmih->biXPelsPerMeter) &&
      (bmih->biYPelsPerMeter == Img.bmih->biYPelsPerMeter) &&
      (bmih->biClrUsed == Img.bmih->biClrUsed) &&
      (bmih->biClrImportant == Img.bmih->biClrImportant) &&
      ImageMemIsAlloc() )
    return(1);

  return(0);
}//end EqualMemFormat.

void CImage::absolute(void)
{
	int *Src;
	long int i;

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return;
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = bmih->biWidth * abs(bmih->biHeight);
	if( m_ExtraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( m_ExtraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Calculate abs.
  Src = m_Y;
  for(i = 0; i < YSize; i++)
    {
      *Src = abs(*Src);
      Src++;
    }//end for i...

  Src = m_U;
  for(i = 0; i < UVSize; i++)
    {
      *Src = abs(*Src);
      Src++;
    }//end for i...

  Src = m_V;
  for(i = 0; i < UVSize; i++)
    {
      *Src = abs(*Src);
      Src++;
    }//end for i...

}//end absolute.

//#define PSNR_LUM_ONLY 1
double CImage::PSNR(CImage &NoiseImg)
{
	int *OSrc;
	int *NSrc;
	long int i;
  double psnr;

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return(0.0);
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = bmih->biWidth * abs(bmih->biHeight);
	if( m_ExtraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( m_ExtraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Calculate the total absolute noise.
  double accLumNoise = 0.000000001;
  double accnoise,t;
  accnoise = 0.000000001;
  OSrc = m_Y;
  NSrc = NoiseImg.m_Y;
  for(i = 0; i < YSize; i++)
    {
      t = ((double)(*OSrc) - (double)(*NSrc));
      accnoise += (t * t);
      OSrc++;
      NSrc++;
    }//end for i...
  accLumNoise = accnoise;

  OSrc = m_U;
  NSrc = NoiseImg.m_U;
  for(i = 0; i < UVSize; i++)
    {
      t = ((double)(*OSrc + 128) - (double)(*NSrc + 128));
      accnoise += (t * t);
      OSrc++;
      NSrc++;
    }//end for i...

  OSrc = m_V;
  NSrc = NoiseImg.m_V;
  for(i = 0; i < UVSize; i++)
    {
      t = ((double)(*OSrc + 128) - (double)(*NSrc + 128));
      accnoise += (t * t);
      OSrc++;
      NSrc++;
    }//end for i...

  //Determine the peak signal to noise ratio.
#ifdef PSNR_LUM_ONLY
  double noise = sqrt(accLumNoise/(double)YSize);
#else
  double noise = sqrt(accnoise / ((double)(YSize + (2*UVSize))));
#endif

  psnr = 20.0 * log10(255.0/noise);

  return(psnr);
}//end PSNR.

int CImage::IncreaseLum(int By)
{

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return(0);
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize = bmih->biWidth * abs(bmih->biHeight);
  for(int i = 0; i < YSize; i++)
    m_Y[i] = m_Y[i] + By;

  UpdateBmp();
  return(1);
}//end IncreaseLum.

int CImage::DecreaseLum(int By)
{

	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return(0);
		}//end if m_ExtraColorType...

  //Determine the data sizes.
	long int YSize = bmih->biWidth * abs(bmih->biHeight);
  for(int i = 0; i < YSize; i++)
    m_Y[i] = m_Y[i] - By;

  UpdateBmp();
  return(1);
}//end DecreaseLum.

// Take the pixels to the right of PixPoint and write them
// to the left. Continue until an image boundary is reached.
void CImage::MirrorXLeft(int PixPoint)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return;
		}//end if m_ExtraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(m_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P - j) = *(P + j);
  }//end for i...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(m_ExtraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(m_U + i*X + PixPoint);
    P_V = (int *)(m_V + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
    {
      *(P_U - j) = *(P_U + j);
      *(P_V - j) = *(P_V + j);
    }//end for j...
  }//end for i...

  UpdateBmp();
}//end MirrorXLeft.

// Take the pixels from the left of PixPoint and write them
// to the right. Continue until an image boundary is reached.
void CImage::MirrorXRight(int PixPoint)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return;
		}//end if m_ExtraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(m_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P + j) = *(P - j);
  }//end for i...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(m_ExtraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(m_U + i*X + PixPoint);
    P_V = (int *)(m_V + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
    {
      *(P_U + j) = *(P_U - j);
      *(P_V + j) = *(P_V - j);
    }//end for j...
  }//end for i...

  UpdateBmp();
}//end MirrorXRight.

// Take the pixels from below PixPoint and write them
// above. Note that for bmp images, 'up' is the end of
// the lines. Continue until an image boundary is reached.
void CImage::MirrorYUp(int PixPoint)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return;
		}//end if m_ExtraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(m_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P + i*X) = *(P - i*X);
  }//end for j...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(m_ExtraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(m_U + PixPoint*X + j);
    P_V = (int *)(m_V + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
    {
      *(P_U + i*X) = *(P_U - i*X);
      *(P_V + i*X) = *(P_V - i*X);
    }//end for i...
  }//end for j...

  UpdateBmp();
}//end MirrorYUp.

// Take the pixels from above PixPoint and write them
// below. Note that for bmp images, 'down' is the beginning of
// the lines. Continue until an image boundary is reached.
void CImage::MirrorYDown(int PixPoint)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	  return;
		}//end if m_ExtraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(m_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P - i*X) = *(P + i*X);
  }//end for j...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(m_ExtraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(m_U + PixPoint*X + j);
    P_V = (int *)(m_V + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
    {
      *(P_U - i*X) = *(P_U + i*X);
      *(P_V - i*X) = *(P_V + i*X);
    }//end for i...
  }//end for j...

  UpdateBmp();
}//end MirrorYDown.

void CImage::KillColor(void)
{
	if(m_ExtraColorType == BMP_INFO_SPECIFIES)
	{
    MessageBox(NULL,"Currently only implemented for YUV formats!",NULL,MB_OK);
  	return;
	}//end if m_ExtraColorType...

  int i;
  int X;
  //Do the Chrominance only.
  X = GetUVWidth() * GetUVHeight();
  for(i = 0; i < X; i++)
  {
    *(m_U + i) = 0;
    *(m_V + i) = 0;
  }//end for i...

  UpdateBmp();
}//end KillColor.

///////////////////////////////////////////////////
// Colour conversion functions.
///////////////////////////////////////////////////

int CImage::CreateYUVMem(int YUVType)
{
	long int YSize,UVSize;

	YSize = bmih->biWidth * abs(bmih->biHeight) * sizeof(int);
	if( YUVType == YUV411 )
 	  UVSize = YSize/4;
	else if( YUVType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

	//Allocate memory for the extra image components.
	hY = GlobalAlloc(GMEM_FIXED,YSize);
	if(!hY)
  {
	  return(0);
  }//end if !hY...
	m_Y = (int *)GlobalLock(hY);
	hU = GlobalAlloc(GMEM_FIXED,UVSize);
	if(!hU)
  {
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
  	m_Y = NULL;
	  return(0);
  }//end if !hU...
	m_U = (int *)GlobalLock(hU);
	hV = GlobalAlloc(GMEM_FIXED,UVSize);
	if(!hV)
  {
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
  	m_Y = NULL;
		GlobalUnlock(hU);
 		GlobalFree(hU);
 		hU = NULL;
  	m_U = NULL;
	  return(0);
  }//end if !hV...
	m_V = (int *)GlobalLock(hV);

	return(1);
}//end CreateYUVMem.

void CImage::DestroyYUVMem(void)
{
	if(hY)
	{
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
  	m_Y = NULL;
	}//end if hY...
	if(hU)
	{
		GlobalUnlock(hU);
 		GlobalFree(hU);
 		hU = NULL;
  	m_U = NULL;
	}//end if hU...
	if(hV)
	{
		GlobalUnlock(hV);
 		GlobalFree(hV);
 		hV = NULL;
  	m_V = NULL;
	}//end if hV...
	m_ExtraColorType = BMP_INFO_SPECIFIES;
}//end DestroyYUVMem.

void CImage::ConvertRGB24BITtoYUV411(void)
	{
	  // Y1, Y2, Y3 & Y4 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = bmih->biWidth;
		S = (unsigned char *)bmptr;
		//4 pix per Block.
		XB = bmih->biWidth >> 1;
		YB = bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Top left pix.
			b = (int)( *(S + (yb*xpix + xb)*6) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			m_Y[(yb*xpix + xb)*2] = y;

			//Top right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			m_Y[(yb*xpix + xb)*2 + 1] = y;

			//Bottom left pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 2) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			m_Y[(yb*xpix + xb)*2 + xpix] = y;

			//Bottom right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			m_Y[(yb*xpix + xb)*2 + xpix + 1] = y;

			u = accu/4;
			v = accv/4;
			m_U[yb*(xpix/2) + xb] = u;
			m_V[yb*(xpix/2) + xb] = v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV411.

void CImage::ConvertYUV411toRGB24BIT(void)
{
	  // R, G & B have range 0..255,
		unsigned char *D;
		register int xb,yb,XB,YB;
		int xpix,r,g,b;
		int y,u,v;

		xpix = bmih->biWidth;
		D = (unsigned char *)bmptr;
		//4 pix per Block.
		XB = bmih->biWidth >> 1;
		YB = bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			u = m_U[yb*(xpix/2) + xb];
			v = m_V[yb*(xpix/2) + xb];

			//Top left pix.
			y = m_Y[(yb*xpix + xb)*2];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 2) = (unsigned char)r;

			//Top right pix.
			y = m_Y[(yb*xpix + xb)*2 + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 5) = (unsigned char)r;

			//Bottom left pix.
			y = m_Y[(yb*xpix + xb)*2 + xpix];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 2) = (unsigned char)r;

			//Bottom right pix.
			y = m_Y[(yb*xpix + xb)*2 + xpix + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 5) = (unsigned char)r;
		}//end for xb & yb...

}//end ConvertYUV411toRGB24BIT.

void CImage::ConvertRGB24BITtoYUV422(void)
	{
	  // Y1 & Y2 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = bmih->biWidth;
		S = (unsigned char *)bmptr;
		//2 pix per Block.
		XB = bmih->biWidth >> 1;
		YB = bmih->biHeight;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Left pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			m_Y[yb*xpix + xb*2] = y;

			//Right pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			m_Y[yb*xpix + xb*2 + 1] = y;

			u = accu/2;
			v = accv/2;
			m_U[yb*(xpix/2) + xb] = u;
			m_V[yb*(xpix/2) + xb] = v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV422.

void CImage::ConvertYUV422toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix,r,b,g;
	int y,u,v;

	xpix = bmih->biWidth;
	D = (unsigned char *)bmptr;
	//2 pix per Block.
	XB = bmih->biWidth >> 1;
	YB = bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = m_U[yb*(xpix/2) + xb];
		v = m_V[yb*(xpix/2) + xb];

		//Left pix.
		y = m_Y[yb*xpix + xb*2];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 2) = (unsigned char)r;

		//Right pix.
		y = m_Y[yb*xpix + xb*2 + 1];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3 + 3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 4) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 5) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV422toRGB24BIT.

void CImage::ConvertRGB24BITtoYUV444(void)
{
  // Y has range 0..255,
  // U & V have range -128..127.
	unsigned char *S;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = bmih->biWidth;
	S = (unsigned char *)bmptr;
	//1 pix per Block.
	XB = bmih->biWidth;
	YB = bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
	 for(xb = 0; xb < XB; xb++)
	{
		b = (int)( *(S + (yb*xpix + xb)*3) );
		g = (int)( *(S + (yb*xpix + xb)*3 + 1) );
		r = (int)( *(S + (yb*xpix + xb)*3 + 2) );

    RGBtoYUV(r,g,b,y,u,v);

		m_Y[yb*xpix + xb] = y;
		m_U[yb*xpix + xb] = u;
		m_V[yb*xpix + xb] = v;
	}//end for xb & yb...

}//end ConvertRGB24BITtoYUV444.

void CImage::ConvertYUV444toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = bmih->biWidth;
	D = (unsigned char *)bmptr;
	//1 pix per Block.
	XB = bmih->biWidth;
	YB = bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = m_U[yb*xpix + xb];
		v = m_V[yb*xpix + xb];
		y = m_Y[yb*xpix + xb];

    YUVtoRGB(y,u,v,r,g,b);

		*(D + (yb*xpix + xb)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb)*3 + 2) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV444toRGB24BIT.

//////////////////////////////////////////////////////
// CONVERSION ROUTINES.
// Note: They are not class functions.
//////////////////////////////////////////////////////

// Matrix conversion including rounding and bounds
// checking.
void RGBtoYUV(double R,double G,double B,
              double &Y,double &U,double &V)
{
	Y = RtoY_00*R + RtoY_01*G + RtoY_02*B;
  if(Y < 0.0) 
    Y = 0.0;
  else if(Y > 255.0)
    Y = 255.0;
  else
    Y += 0.5;

	U = RtoY_10*R + RtoY_11*G + RtoY_12*B;
  if(U < 0.0) 
    U -= 0.5;
  else 
    U += 0.5;
  if(U > 127.0)
    U = 127.0;
  else if(U < -128.0)
    U = -128.0;

	V = RtoY_20*R + RtoY_21*G + RtoY_22*B;
  if(V < 0.0) 
    V -= 0.5;
  else 
    V += 0.5;
  if(V > 127.0)
    V = 127.0;
  else if(V < -128.0)
    V = -128.0;

}//end RGBtoYUV.

//void RGBtoYUV(int R,int G,int B,
//              int &Y,int &U,int &V)
//{
//  double r = (double)R;
//  double g = (double)G;
//  double b = (double)B;
//	Y = (int)(RtoY_00*r + RtoY_01*g + RtoY_02*b + 0.5);
//	double u = RtoY_10*r + RtoY_11*g + RtoY_12*b;
//	double v = RtoY_20*r + RtoY_21*g + RtoY_22*b;
//
//  if(Y < 0) 
//    Y = 0;
//  else if(Y > 255)
//    Y = 255;
//
//  if(u < 0.0) 
//    u -= 0.5;
//  else 
//    u += 0.5;
//  U = (int)u;
//  if(U > 127)
//    U = 127;
//  else if(U < -128)
//    U = -128;
//
//  if(v < 0.0) 
//    v -= 0.5;
//  else 
//    v += 0.5;
//  V = (int)v;
//  if(V > 127)
//    V = 127;
//  else if(V < -128)
//    V = -128;
//
//}//end RGBtoYUV.

void RGBtoYUV(int R,int G,int B,
              int &Y,int &U,int &V)
{
  double r = (double)R;
  double g = (double)G;
  double b = (double)B;
	Y = (int)(0.299*r + 0.587*g + 0.114*b + 0.5);
	double u = 0.436*b - 0.147*r - 0.289*g;
	double v = 0.615*r - 0.515*g - 0.100*b;

  if(Y < 0) 
    Y = 0;
  else if(Y > 255)
    Y = 255;

  if(u < 0.0) 
    u -= 0.5;
  else 
    u += 0.5;
  U = (int)u;
  if(U > 127)
    U = 127;
  else if(U < -128)
    U = -128;

  if(v < 0.0) 
    v -= 0.5;
  else 
    v += 0.5;
  V = (int)v;
  if(V > 127)
    V = 127;
  else if(V < -128)
    V = -128;

}//end RGBtoYUV.

//void YUVtoRGB(int Y,int U,int V,
//              int &R,int &G,int &B)
//{
//  double y = (double)Y;
//  double u = (double)U;
//  double v = (double)V;
//	R = (int)(y + YtoR_01*u + YtoR_02*v + 0.5);
//	G = (int)(y + YtoR_11*u + YtoR_12*v + 0.5);
//	B = (int)(y + YtoR_21*u + YtoR_22*v + 0.5);
//	if( R > 255 )
//	  R = 255;
//	else if( R < 0 )
//	  R = 0;
//	if( G > 255 )
//	  G = 255;
//	else if( G < 0 )
//	  G = 0;
//	if( B > 255 )
//	  B = 255;
//	else if( B < 0 )
//	  B = 0;
//}//end YUVtoRGB.

void YUVtoRGB(int Y,int U,int V,
              int &R,int &G,int &B)
{
  double y = (double)Y;
  double u = (double)U;
  double v = (double)V;
	R = (int)(y + 1.140*v + 0.5);
	G = (int)(y - 0.394*u - 0.581*v + 0.5);
	B = (int)(y + 2.032*u + 0.5);
	if( R > 255 )
	  R = 255;
	else if( R < 0 )
	  R = 0;
	if( G > 255 )
	  G = 255;
	else if( G < 0 )
	  G = 0;
	if( B > 255 )
	  B = 255;
	else if( B < 0 )
	  B = 0;
}//end YUVtoRGB.


