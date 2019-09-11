/** @file

MODULE						: ImageHandler

TAG								: IH

FILE NAME					: ImageHandler.cpp

DESCRIPTION				: A Microsoft Managed class  that owns and operates on 
										DIB .bmp formatted images. Also provides functions for 
										colour space conversion and the like.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#include "StdAfx.h"
#include <math.h>
#include <memory.h>

#include "ImageHandler.h"

#using <mscorlib.dll>

using namespace System;
using namespace System::IO;

/*
--------------------------------------------------------------------------
	Constants.
--------------------------------------------------------------------------
*/
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

/*
--------------------------------------------------------------------------
	Public Implementations.
--------------------------------------------------------------------------
*/

ImageHandler::ImageHandler()
{
	_paletteSize		= 0;
	_pbmp						= NULL;
	_bmih						= NULL;
	_bmic						= NULL;
	_bmptr					= NULL;
	_errorStr				= S"No error";
	_extraColorType = BMP_INFO_SPECIFIES;
	_Y							= NULL;
	_U							= NULL;
	_V							= NULL;
  _bmpFilename	= S"default.bmp";
}//end ImageHandler Constructor.

ImageHandler::~ImageHandler()
{
  //Don't let the object go without cleaning up.
  DestroyImage();
}//end ImageHandler Destructor.

// Create memory space and copy the bitmap info
// header data.
// Return : 0 = error, 1 = OK.
int ImageHandler::CreateImage(BITMAPINFOHEADER* pbmpih)
{
	if( (pbmpih->biCompression != BI_RGB) &&
		  (pbmpih->biCompression != BI_BITFIELDS) )
	  {
		  _errorStr = S"ImageHandler: Not valid BMP format!";
		  return(0);
	  }//end if biCompression...
	//Determine the image size.
	if(pbmpih->biSizeImage == 0)
	{
		int height = pbmpih->biHeight;
		if(pbmpih->biHeight < 0)
			height = -(pbmpih->biHeight);

		pbmpih->biSizeImage = (unsigned long)
			(((((pbmpih->biWidth * pbmpih->biBitCount)+ 31)/32) * 4) * height);
	}//end if biSizeImage...
	//Determine the palette colour size.
	if(pbmpih->biCompression == BI_RGB)
	{
		if((pbmpih->biClrUsed == 0)&&(pbmpih->biBitCount<=8))
			_paletteSize = (unsigned int)((1 << pbmpih->biBitCount) * sizeof(RGBQUAD));
		else
			_paletteSize = (unsigned int)(pbmpih->biClrUsed * sizeof(RGBQUAD));
	}
	else //if pbmpih->biCompression == BI_BITFIELDS
	{
		_paletteSize = (unsigned int)(3 * sizeof(unsigned long));
	}//end else...

	// Allocate memory for the image.
	_pbmp = new char[sizeof(BITMAPINFOHEADER) + _paletteSize + pbmpih->biSizeImage];

	if(!_pbmp)
	  {
  		_errorStr = S"ImageHandler: Insufficient memory for image!";
		  return(0);
	  }//end if !_pbmp...
	_bmih = (BITMAPINFOHEADER *)(_pbmp);

	//Load the bitmap info header.
	*(_bmih) = *(pbmpih);
	//Determine the pointers that are offset into
	//the memory.
	_bmic			= (RGBQUAD *)((char *)_bmih + (unsigned long)sizeof(BITMAPINFOHEADER));
	_bmptr		= (void *)((char *)_bmic + _paletteSize);
	_extraColorType = BMP_INFO_SPECIFIES;

	return(1);
}//end CreateImage.

void ImageHandler::DestroyImage(void)
{
	if(_pbmp != NULL)
		delete[] _pbmp;
	_pbmp = NULL;

	_paletteSize	= 0;
	_bmih					= NULL;
	_bmic					= NULL;
	_bmptr				= NULL;
	_errorStr			= S"No error";
  _bmpFilename	= S"default.bmp";

	DestroyYUVMem();
}//end DestroyImage.

int ImageHandler::ImageMemIsAlloc(void)
{
  if(_pbmp == NULL)
    return(0);
  return(1);
}//end ImageMemIsAlloc.

// Create memory to store Y,U and V colour data. Convert
// from the current colour space to YUV and force
// the displayable bmp to reflect the new image
// in its current bmp colour format.
int ImageHandler::ConvertToYUV(int YUVFormat)
{
	if(_extraColorType != BMP_INFO_SPECIFIES)
    {
      //Invalidate the YUV memory first.
    	DestroyYUVMem();
    }//end if _extraColorType...

	if(_bmih->biBitCount == 24)//RGB 24 bit.
	{
		if(!CreateYUVMem(YUVFormat))
		{
			_errorStr = S"ImageHandler: YUV memory unavailable!";
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
	  _errorStr = S"ImageHandler: Colour conversion not available";
		return(0);
  }//end else...

	_extraColorType = YUVFormat;
	UpdateBmp();
	return(1);
}//end ConvertToYUV.

void ImageHandler::UpdateBmp(void)
{
	if(_bmih->biBitCount == 24)
	{
		if(_extraColorType == YUV411)
		{
			ConvertYUV411toRGB24BIT();
		}//end if _extraColorType...
	  else if(_extraColorType == YUV422)
	  {
			ConvertYUV422toRGB24BIT();
  	}//end else if _extraColorType...
	  else if(_extraColorType == YUV444)
	  {
			ConvertYUV444toRGB24BIT();
  	}//end else if _extraColorType...
	}//end if biBitCount...
}//end UpdateBmp.

int ImageHandler::GetYWidth(void)
{
	return(_bmih->biWidth);
}//end GetYWidth.

int ImageHandler::GetYHeight(void)
{
	return(abs(_bmih->biHeight));
}//end GetYHeight.

int ImageHandler::GetUVWidth(void)
{
	int YLine,UVLine;
	YLine = _bmih->biWidth;
	if( _extraColorType == YUV411 )
    UVLine = YLine/2;
	else if( _extraColorType == YUV422 )
    UVLine = YLine/2;
	else if( _extraColorType == YUV444 )
    UVLine = YLine;
  else
    UVLine = 0;

  return(UVLine);
}//end GetUVWidth.

int ImageHandler::GetUVHeight(void)
{
	int YCol,UVCol;
	YCol = abs(_bmih->biHeight);
	if( _extraColorType == YUV411 )
    UVCol = YCol/2;
	else if( _extraColorType == YUV422 )
    UVCol = YCol;
	else if( _extraColorType == YUV444 )
    UVCol = YCol;
  else
    UVCol = 0;

  return(UVCol);
}//end GetUVHeight.

void ImageHandler::SaveBmp(String* filename)
{
  _bmpFilename = filename;
  SaveBmp();
}//end SaveBmp.

void ImageHandler::SaveBmp(void)
{
	// Create file stream.
	BinaryWriter* pBw = NULL;
	try
	{
		// Get the file stream.
		FileStream* pFs = new FileStream(_bmpFilename, FileMode::Create, FileAccess::ReadWrite);
		// Get the binary writer.
		pBw = new BinaryWriter(pFs);
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = pe->ToString();
		return;
	}//end catch...

	//Make the file header.
  BITMAPFILEHEADER bmfh;
	bmfh.bfType				= 0x4D42;
	bmfh.bfReserved1	= 0;
	bmfh.bfReserved2	= 0;
	bmfh.bfSize				= sizeof(BITMAPFILEHEADER) +
											sizeof(BITMAPINFOHEADER) +
											_paletteSize + 
											_bmih->biSizeImage; 
	bmfh.bfOffBits		= sizeof(BITMAPFILEHEADER) +
											sizeof(BITMAPINFOHEADER) +
											_paletteSize;
	// Write to file stream.
	try
	{
		// BITMAPFILEHEADER.
		pBw->Write((unsigned short)bmfh.bfType);
		pBw->Write((unsigned int)bmfh.bfSize);
		pBw->Write((unsigned short)bmfh.bfReserved1);
		pBw->Write((unsigned short)bmfh.bfReserved2);
		pBw->Write((unsigned int)bmfh.bfOffBits);
		// BITMAPINFOHEADER.
		pBw->Write((unsigned int)_bmih->biSize);
		pBw->Write((int)_bmih->biWidth);
		pBw->Write((int)_bmih->biHeight);
		pBw->Write((unsigned short)_bmih->biPlanes);
		pBw->Write((unsigned short)_bmih->biBitCount);
		pBw->Write((unsigned int)_bmih->biCompression);
		pBw->Write((unsigned int)_bmih->biSizeImage);
		pBw->Write((int)_bmih->biXPelsPerMeter);
		pBw->Write((int)_bmih->biYPelsPerMeter);
		pBw->Write((unsigned int)_bmih->biClrUsed);
		pBw->Write((unsigned int)_bmih->biClrImportant);

		// Colour palette.
		unsigned char* pPalette = (unsigned char *)(_bmic);
		int i;
		for(i = 0; i < _paletteSize; i++)
			pBw->Write(*pPalette++);
	
		// Pels.
		unsigned char* pBmp = (unsigned char *)(_bmptr);
		for(i = 0; i < (int)(_bmih->biSizeImage); i++)
			pBw->Write(*pBmp++);

	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = pe->ToString();
		pBw->Close();
		return;
	}//end catch...

	// Close up on the way out.
	pBw->Flush();
	pBw->Close();
}//end SaveBmp.

int ImageHandler::LoadBmp(String* filename)
{
  _bmpFilename = filename;
  return(LoadBmp());
}//end LoadBmp.

int ImageHandler::LoadBmp(void)
{
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER BitmapHead;

  //Make sure image space is cleared.
  String* Temp = _bmpFilename;
  DestroyImage();
  _bmpFilename = Temp;

	if(!File::Exists(_bmpFilename))
	{
		_errorStr = S"ImageHandler: Bmp file does not exist";
		return(0);
	}//end if !Exists...

	// Open.
	BinaryReader* pBr = NULL;
	try
	{
		// Fet the file stream.
		FileStream* pFs = new FileStream(_bmpFilename, FileMode::Open, FileAccess::Read);
		// Get the binary reader.
		pBr = new BinaryReader(pFs);
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = pe->ToString();
		return(0);
	}//end catch...

	//Read bitmap file header.
	//typedef struct tagBITMAPFILEHEADER { 
	//	WORD    bfType;			// UInt16 
	//	DWORD   bfSize;			// UInt32
	//	WORD    bfReserved1;// UInt16 
	//	WORD    bfReserved2;// UInt16 
	//	DWORD   bfOffBits;	// UInt32
	//} BITMAPFILEHEADER;
	try
	{
		bmfh.bfType				= pBr->ReadUInt16();
		bmfh.bfSize				= pBr->ReadUInt32();
		bmfh.bfReserved1	= pBr->ReadUInt16();
		bmfh.bfReserved2	= pBr->ReadUInt16();
		bmfh.bfOffBits		= pBr->ReadUInt32();
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = pe->ToString();
		pBr->Close();
		return(0);
	}//end catch...
	// Is it the right type?
	if(bmfh.bfType != 0x4D42)
	{
		_errorStr = S"ImageHandler: File is not in BMP file format";
		pBr->Close();
		return(0);
	}//end if bfType...

	//Read bitmap info header.
	//typedef struct tagBITMAPINFOHEADER{
	//	DWORD  biSize;					// UInt32
	//	LONG   biWidth;					// Int32
	//	LONG   biHeight;				// Int32
	//	WORD   biPlanes;				// UInt16
	//	WORD   biBitCount;			// UInt16
	//	DWORD  biCompression;		// UInt32
	//	DWORD  biSizeImage;			// UInt32
	//	LONG   biXPelsPerMeter;	// Int32 
	//	LONG   biYPelsPerMeter; // Int32
	//	DWORD  biClrUsed;				// UInt32
	//	DWORD  biClrImportant;	// UInt32
	//} BITMAPINFOHEADER;
	try
	{
		BitmapHead.biSize						= pBr->ReadUInt32();
		BitmapHead.biWidth					= pBr->ReadInt32();
		BitmapHead.biHeight					= pBr->ReadInt32();
		BitmapHead.biPlanes					= pBr->ReadUInt16();
		BitmapHead.biBitCount				= pBr->ReadUInt16();
		BitmapHead.biCompression		= pBr->ReadUInt32();
		BitmapHead.biSizeImage			= pBr->ReadUInt32();
		BitmapHead.biXPelsPerMeter	= pBr->ReadInt32();
		BitmapHead.biYPelsPerMeter	= pBr->ReadInt32();
		BitmapHead.biClrUsed				= pBr->ReadUInt32();
		BitmapHead.biClrImportant		= pBr->ReadUInt32();
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = pe->ToString();
		pBr->Close();
		return(0);
	}//end catch...

  //Construct the new image.
 if(!CreateImage(&BitmapHead))
 {
		pBr->Close();
    return(0);
  }//end if !CreateImage...

  //Read the colour palette.
 unsigned char* pPalette = (unsigned char *)(_bmic);
 int readSize;
 for(readSize = 0; readSize < _paletteSize; readSize++)
 {
	 try
	 {
		 *pPalette++ = pBr->ReadByte();
	 }//end try...
	 catch(System::Exception* pe)
	 {
			_errorStr = pe->ToString();
			pBr->Close();
			return(0);
	 }//end catch...
 }//end for readSize...
//	Check we have what we asked for.
	if(readSize != _paletteSize)
	{
		_errorStr = S"ImageHandler: Unable to read palette from Bmp file";
		pBr->Close();
		return(0);
	}//end if readSize...

  //Read the image bits.
 unsigned char* pBmp = (unsigned char *)(_bmptr);
 for(readSize = 0; readSize < (int)(_bmih->biSizeImage); readSize++)
 {
	 try
	 {
		 *pBmp++ = pBr->ReadByte();
	 }//end try...
	 catch(System::Exception* pe)
	 {
			_errorStr = pe->ToString();
			pBr->Close();
			return(0);
	 }//end catch...
 }//end for readSize...
//	Check we have what we asked for.
	if(readSize != _bmih->biSizeImage)
	{
		_errorStr = S"ImageHandler: Unable to read pixel bits from Bmp file";
		pBr->Close();
		return(0);
	}//end if readSize...

	pBr->Close();

  return(1);
}//end LoadBmp.

int ImageHandler::SaveRaw(String* filename)
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
  if(_extraColorType == BMP_INFO_SPECIFIES)
  {
		_errorStr = S"Operation only valid for YUV formats";
    return(0);
  }//end if !_extraColorType...

	// Create file stream.
	BinaryWriter* pBw = NULL;
	try
	{
		// Get the file stream.
		FileStream* pFs = new FileStream(filename, FileMode::Create, FileAccess::ReadWrite);
		// Get the binary writer.
		pBw = new BinaryWriter(pFs);
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = S"ImageHandler: Unable to open raw file. Error: ";
		_errorStr->Concat(pe->ToString());
		return(0);
	}//end catch...
  
	// Write the data sections.
	try
	{
		// Write out no. of colour components.
		int ColourComponents = 3;
		pBw->Write(ColourComponents);

		// Write out luminance component.
	  int x = GetYWidth();
		pBw->Write(x);
	  int y = GetYHeight();
		pBw->Write(y);
		int  i;
		int* pY = _Y;
		for(i = 0; i < (x * y); i++)
			pBw->Write(*pY++);

		// Write out U and V chrominance component.
	  x = GetUVWidth();
		pBw->Write(x);
	  y = GetUVHeight();
		pBw->Write(y);
		int* pU = _U;
		for(i = 0; i < (x * y); i++)
			pBw->Write(*pU++);
		pBw->Write(x);
		pBw->Write(y);
		int* pV = _U;
		for(i = 0; i < (x * y); i++)
			pBw->Write(*pV++);
	}//end try...
	catch(System::Exception* pe)
	{
		_errorStr = S"ImageHandler: Unable to write data. Error : ";
		_errorStr->Concat(pe->ToString());
		return(0);
	}//end catch...

	pBw->Flush();
	pBw->Close();

  return(1);
}//end SaveRaw.

/*
--------------------------------------------------------------------------
	Simulated operator overloads.
--------------------------------------------------------------------------
*/

//ImageHandler &ImageHandler::operator=(ImageHandler &I)
int ImageHandler::Assign(ImageHandler& I)
{
	char*	pSrc;
	char*	pDst;
	int		limit;

  if(!EqualMemFormat(I))
  {
    //Destroy old image.
    DestroyImage();

    //Create new image space.
    if(!CreateImage(I._bmih))
			return(0);

  	//Create extra memory if neccessary.
	  if(I._extraColorType != BMP_INFO_SPECIFIES)
		{
		  if(!CreateYUVMem(I._extraColorType))
				return(0);
    }//end if m_ExtraColorType...
  }//end if !EqualMemFormat...

	//Copy palette and image data.
	pSrc = (char *)(I._bmic);
	pDst = (char *)(_bmic);
	limit = (int)(I._paletteSize + I._bmih->biSizeImage);
	memcpy(pDst, pSrc, limit);
	_paletteSize = I._paletteSize;

	//Deal with extra colour types.
	_extraColorType = I._extraColorType;
	//Copy YUV if neccessary.
	if(_extraColorType != BMP_INFO_SPECIFIES)
	{
    //Copy the YUV data.
	  int YSize,UVSize;
	  YSize = _bmih->biWidth * abs(_bmih->biHeight) * sizeof(int);
	  if( _extraColorType == YUV411 )
 	    UVSize = YSize/4;
	  else if( _extraColorType == YUV422 )
 	    UVSize = YSize/2;
	  else //YUV444
	   UVSize = YSize;
    pSrc = (char *)(I._Y);
    pDst = (char *)_Y;
	  memcpy(pDst, pSrc, YSize);
    pSrc = (char *)(I._U);
    pDst = (char *)_U;
	  memcpy(pDst, pSrc, UVSize);
    pSrc = (char *)(I._V);
    pDst = (char *)_V;
	  memcpy(pDst, pSrc, UVSize);
	}//end if _extraColorType...

	return(1);
}//end Assign.

//ImageHandler &ImageHandler::operator-=(ImageHandler &I)
int ImageHandler::Subtract(ImageHandler &I)
{
	int*	pSrc;
	int*	pDst;
	int		i;

  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	_errorStr = S"Images have different formats!";
 	  return(0);
  }//end if !EqualMemFormat...

	if(_extraColorType == BMP_INFO_SPECIFIES)
		{
    	_errorStr = S"Currently only implemented for YUV formats!";
  	  return(0);
		}//end if _extraColorType...

  //Determine the data sizes.
	int YSize,UVSize;

	YSize = _bmih->biWidth * abs(_bmih->biHeight);
	if( _extraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( _extraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Do the subtraction.
  pDst = _Y;
  pSrc = I._Y;
  for(i = 0; i < YSize; i++)
  {
    *pDst = *pDst - *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

  pDst = _U;
  pSrc = I._U;
  for(i = 0; i < UVSize; i++)
  {
    *pDst = *pDst - *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

  pDst = _V;
  pSrc = I._V;
  for(i = 0; i < UVSize; i++)
  {
    *pDst = *pDst - *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

	return(1);
}//end Subtract.

//ImageHandler &ImageHandler::operator+=(ImageHandler &I)
int ImageHandler::Add(ImageHandler &I)
{
	int*	pSrc;
	int*	pDst;
	int		i;

  // The images must be of the same type and size.
	if(!EqualMemFormat(I))
  {
   	_errorStr = S"Images have different formats!";
 	  return(0);
  }//end if !EqualMemFormat...

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return(0);
	}//end if _extraColorType...

  //Determine the data sizes.
	int YSize,UVSize;

	YSize = _bmih->biWidth * abs(_bmih->biHeight);
	if( _extraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( _extraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Do the addition.
  pDst = _Y;
  pSrc = I._Y;
  for(i = 0; i < YSize; i++)
  {
    *pDst = *pDst + *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

  pDst = _U;
  pSrc = I._U;
  for(i = 0; i < UVSize; i++)
  {
    *pDst = *pDst + *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

  pDst = _V;
  pSrc = I._V;
  for(i = 0; i < UVSize; i++)
  {
    *pDst = *pDst + *pSrc;
    pSrc++;
    pDst++;
  }//end for i...

	return(1);
}//end Add.

int ImageHandler::Copy(ImageHandler& FromImg)
{
	if(!EqualMemFormat(FromImg))
  {
   	_errorStr = "Images have different formats!";
    return(0);
  }//end if !EqualMemFormat...

	//Copy image data.
	char* pS		= (char *)(FromImg._bmic);
	char* pD		= (char *)(_bmic);
	int		limit = (int)(FromImg._bmih->biSizeImage);
	memcpy(pD, pS, limit);

	if(_extraColorType != BMP_INFO_SPECIFIES)
	{
  	int*	pDst;
	  int*	pSrc;
	  int		i;

    //Determine the data sizes.
		int YSize,UVSize;
  
		YSize = _bmih->biWidth * abs(_bmih->biHeight);
		if( _extraColorType == YUV411 )
 	 	  UVSize = YSize/4;
		else if( _extraColorType == YUV422 )
 	 	  UVSize = YSize/2;
		else //YUV444
			UVSize = YSize;
  
    //Copy luminance.
    pDst = _Y;
    pSrc = FromImg._Y;
    for(i = 0; i < YSize; i++)
    {
      *pDst++ = *pSrc++;
    }//end for i...
  
    pDst = _U;
    pSrc = FromImg._U;
    for(i = 0; i < UVSize; i++)
    {
      *pDst++ = *pSrc++;
    }//end for i...
  
    pDst = _V;
    pSrc = FromImg._V;
    for(i = 0; i < UVSize; i++)
    {
      *pDst++ = *pSrc++;
    }//end for i...
	}//end if _extraColorType...

  return(1);
}//end Copy.

int ImageHandler::EqualMemFormat(ImageHandler& Img)
{
  if( _bmih == NULL )
    return(0);

  if( (_extraColorType				== Img._extraColorType) &&
      (_bmih->biSize					== Img._bmih->biSize) &&
      (_bmih->biWidth					== Img._bmih->biWidth) &&
      (_bmih->biHeight				== Img._bmih->biHeight) &&
      (_bmih->biPlanes				== Img._bmih->biPlanes) &&
      (_bmih->biBitCount			== Img._bmih->biBitCount) &&
      (_bmih->biCompression		== Img._bmih->biCompression) &&
      (_bmih->biSizeImage			== Img._bmih->biSizeImage) &&
      (_bmih->biXPelsPerMeter == Img._bmih->biXPelsPerMeter) &&
      (_bmih->biYPelsPerMeter == Img._bmih->biYPelsPerMeter) &&
      (_bmih->biClrUsed				== Img._bmih->biClrUsed) &&
      (_bmih->biClrImportant	== Img._bmih->biClrImportant) &&
      ImageMemIsAlloc() )
    return(1);

  return(0);
}//end EqualMemFormat.

void ImageHandler::absolute(void)
{
	int*	pSrc;
	int		i;

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
  	_errorStr = S"Currently only implemented for YUV formats!";
    return;
	}//end if _extraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = _bmih->biWidth * abs(_bmih->biHeight);
	if( _extraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( _extraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Calculate abs.
  pSrc = _Y;
  for(i = 0; i < YSize; i++)
  {
    *pSrc = abs(*pSrc);
    pSrc++;
  }//end for i...

  pSrc = _U;
  for(i = 0; i < UVSize; i++)
  {
    *pSrc = abs(*pSrc);
    pSrc++;
  }//end for i...

  pSrc = _V;
  for(i = 0; i < UVSize; i++)
  {
    *pSrc = abs(*pSrc);
    pSrc++;
  }//end for i...

}//end absolute.

double ImageHandler::PSNR(ImageHandler& NoiseImg)
{
	int*		pOSrc;
	int*		pNSrc;
	int			i;
  double	psnr;

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return(0.0);
	}//end if _extraColorType...

  //Determine the data sizes.
	long int YSize,UVSize;

	YSize = _bmih->biWidth * abs(_bmih->biHeight);
	if( _extraColorType == YUV411 )
 	  UVSize = YSize/4;
	else if( _extraColorType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

  //Calculate the total absolute noise.
  double accnoise,t;
  accnoise = 0.0000001;
  pOSrc = _Y;
  pNSrc = NoiseImg._Y;
  for(i = 0; i < YSize; i++)
  {
    t = ((double)(*pOSrc) - (double)(*pNSrc));
    accnoise += (t * t);
    pOSrc++;
    pNSrc++;
  }//end for i...

  pOSrc = _U;
  pNSrc = NoiseImg._U;
  for(i = 0; i < UVSize; i++)
  {
    t = ((double)(*pOSrc + 128) - (double)(*pNSrc + 128));
    accnoise += (t * t);
    pOSrc++;
    pNSrc++;
  }//end for i...

  pOSrc = _V;
  pNSrc = NoiseImg._V;
  for(i = 0; i < UVSize; i++)
  {
    t = ((double)(*pOSrc + 128) - (double)(*pNSrc + 128));
    accnoise += (t * t);
    pOSrc++;
    pNSrc++;
  }//end for i...

  //Determine the peak signal to noise ratio.
  double noise = sqrt(accnoise / ((double)(YSize + (2*UVSize))));

  psnr = 20.0 * log10(255.0/noise);

  return(psnr);
}//end PSNR.

int ImageHandler::IncreaseLum(int By)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return(0);
	}//end if _extraColorType...

  //Determine the data sizes.
	long int YSize = _bmih->biWidth * abs(_bmih->biHeight);
  for(int i = 0; i < YSize; i++)
    _Y[i] = _Y[i] + By;

  UpdateBmp();
  return(1);
}//end IncreaseLum.

int ImageHandler::DecreaseLum(int By)
{

	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return(0);
	}//end if _extraColorType...

  //Determine the data sizes.
	long int YSize = _bmih->biWidth * abs(_bmih->biHeight);
  for(int i = 0; i < YSize; i++)
    _Y[i] = _Y[i] - By;

  UpdateBmp();
  return(1);
}//end DecreaseLum.

// Take the pixels to the right of PixPoint and write them
// to the left. Continue until an image boundary is reached.
void ImageHandler::MirrorXLeft(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P - j) = *(P + j);
  }//end for i...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(_extraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(_U + i*X + PixPoint);
    P_V = (int *)(_V + i*X + PixPoint);
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
void ImageHandler::MirrorXRight(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(i = 0; i < Y; i++)
  {
    P = (int *)(_Y + i*X + PixPoint);
    for(j = 0; (j <= PixPoint)&&((PixPoint+j)<X); j++)
      *(P + j) = *(P - j);
  }//end for i...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(_extraColorType != YUV444)
    PixPoint = PixPoint/2;
  for(i = 0; i < Y; i++)
  {
    P_U = (int *)(_U + i*X + PixPoint);
    P_V = (int *)(_V + i*X + PixPoint);
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
void ImageHandler::MirrorYUp(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr  = S"Currently only implemented for YUV formats!";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P + i*X) = *(P - i*X);
  }//end for j...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(_extraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(_U + PixPoint*X + j);
    P_V = (int *)(_V + PixPoint*X + j);
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
void ImageHandler::MirrorYDown(int PixPoint)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
   	_errorStr = S"Currently only implemented for YUV formats!";
    return;
	}//end if _extraColorType...

  int i,j;
  int X,Y;
  //Do the Luminance.
  X = GetYWidth();
  Y = GetYHeight();
  int *P;
  for(j = 0; j < X; j++)
  {
    P = (int *)(_Y + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
      *(P - i*X) = *(P + i*X);
  }//end for j...

  //Do the Chrominance.
  X = GetUVWidth();
  Y = GetUVHeight();
  int *P_U;
  int *P_V;
  //NOTE: No allowance for odd width images.
  if(_extraColorType == YUV411)
    PixPoint = PixPoint/2;
  for(j = 0; j < X; j++)
  {
    P_U = (int *)(_U + PixPoint*X + j);
    P_V = (int *)(_V + PixPoint*X + j);
    for(i = 0; (i <= PixPoint)&&((PixPoint+i)<Y); i++)
    {
      *(P_U - i*X) = *(P_U + i*X);
      *(P_V - i*X) = *(P_V + i*X);
    }//end for i...
  }//end for j...

  UpdateBmp();
}//end MirrorYDown.

void ImageHandler::KillColor(void)
{
	if(_extraColorType == BMP_INFO_SPECIFIES)
	{
    _errorStr = S"Currently only implemented for YUV formats!";
  	return;
	}//end if _extraColorType...

  int i;
  int X;
  //Do the Chrominance only.
  X = GetUVWidth() * GetUVHeight();
  for(i = 0; i < X; i++)
  {
    *(_U + i) = 0;
    *(_V + i) = 0;
  }//end for i...

  UpdateBmp();
}//end KillColor.

/*
--------------------------------------------------------------------------
	Colour conversion functions.
--------------------------------------------------------------------------
*/
int ImageHandler::CreateYUVMem(int YUVType)
{
	int YSize,UVSize;

	YSize = _bmih->biWidth * abs(_bmih->biHeight) * sizeof(int);
	if( YUVType == YUV411 )
 	  UVSize = YSize/4;
	else if( YUVType == YUV422 )
 	  UVSize = YSize/2;
	else //YUV444
		UVSize = YSize;

	//Allocate memory for the extra image components.
	_Y = new int[YSize];
	//hY = GlobalAlloc(GMEM_FIXED,YSize);
	if(!_Y)
  {
	  return(0);
  }//end if !_Y...
	//m_Y = (int *)GlobalLock(hY);

	_U = new int[UVSize];
	//hU = GlobalAlloc(GMEM_FIXED,UVSize);
	if(!_U)
  {
		//GlobalUnlock(hY);
 		//GlobalFree(hY);
 		//hY = NULL;
		delete[] _Y;
  	_Y = NULL;
	  return(0);
  }//end if !_U...
	//m_U = (int *)GlobalLock(hU);

	_V = new int[UVSize];
	//hV = GlobalAlloc(GMEM_FIXED,UVSize);
	if(!_V)
  {
		//GlobalUnlock(hY);
 		//GlobalFree(hY);
 		//hY = NULL;
		delete[] _Y;
  	_Y = NULL;
		//GlobalUnlock(hU);
 		//GlobalFree(hU);
 		//hU = NULL;
		delete[] _U;
  	_U = NULL;
	  return(0);
  }//end if !_V...
	//m_V = (int *)GlobalLock(hV);

	return(1);
}//end CreateYUVMem.

void ImageHandler::DestroyYUVMem(void)
{
	if(_Y != NULL)
	{
		//GlobalUnlock(hY);
 		//GlobalFree(hY);
 		//hY = NULL;
		delete[] _Y;
  	_Y = NULL;
	}//end if hY...
	if(_U != NULL)
	{
		//GlobalUnlock(hU);
 		//GlobalFree(hU);
 		//hU = NULL;
		delete[] _U;
  	_U = NULL;
	}//end if _U...
	if(_V != NULL)
	{
		//GlobalUnlock(hV);
 		//GlobalFree(hV);
 		//hV = NULL;
		delete[] _V;
  	_V = NULL;
	}//end if _V...
	_extraColorType = BMP_INFO_SPECIFIES;
}//end DestroyYUVMem.

void ImageHandler::ConvertRGB24BITtoYUV411(void)
	{
	  // Y1, Y2, Y3 & Y4 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = _bmih->biWidth;
		S = (unsigned char *)_bmptr;
		//4 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Top left pix.
			b = (int)( *(S + (yb*xpix + xb)*6) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			_Y[(yb*xpix + xb)*2] = y;

			//Top right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + 1] = y;

			//Bottom left pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 2) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + xpix] = y;

			//Bottom right pix.
			b = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb)*6 + xpix*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[(yb*xpix + xb)*2 + xpix + 1] = y;

			u = accu/4;
			v = accv/4;
			_U[yb*(xpix/2) + xb] = u;
			_V[yb*(xpix/2) + xb] = v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV411.

void ImageHandler::ConvertYUV411toRGB24BIT(void)
{
	  // R, G & B have range 0..255,
		unsigned char *D;
		register int xb,yb,XB,YB;
		int xpix,r,g,b;
		int y,u,v;

		xpix = _bmih->biWidth;
		D = (unsigned char *)_bmptr;
		//4 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight >> 1;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			u = _U[yb*(xpix/2) + xb];
			v = _V[yb*(xpix/2) + xb];

			//Top left pix.
			y = _Y[(yb*xpix + xb)*2];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 2) = (unsigned char)r;

			//Top right pix.
			y = _Y[(yb*xpix + xb)*2 + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + 5) = (unsigned char)r;

			//Bottom left pix.
			y = _Y[(yb*xpix + xb)*2 + xpix];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 1) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 2) = (unsigned char)r;

			//Bottom right pix.
			y = _Y[(yb*xpix + xb)*2 + xpix + 1];
      YUVtoRGB(y,u,v,r,g,b);
			*(D + (yb*xpix + xb)*6 + xpix*3 + 3) = (unsigned char)b;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 4) = (unsigned char)g;
			*(D + (yb*xpix + xb)*6 + xpix*3 + 5) = (unsigned char)r;
		}//end for xb & yb...

}//end ConvertYUV411toRGB24BIT.

void ImageHandler::ConvertRGB24BITtoYUV422(void)
	{
	  // Y1 & Y2 have range 0..255,
	  // U & V have range -128..127.
		unsigned char *S;
		register int xb,yb,XB,YB;
		int xpix,y,u,v;
    int accu,accv;
		int r,g,b;

		xpix = _bmih->biWidth;
		S = (unsigned char *)_bmptr;
		//2 pix per Block.
		XB = _bmih->biWidth >> 1;
		YB = _bmih->biHeight;

		for(yb = 0; yb < YB; yb++)
  	 for(xb = 0; xb < XB; xb++)
		{
			//Left pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 1) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 2) );
      RGBtoYUV(r,g,b,y,accu,accv);
			_Y[yb*xpix + xb*2] = y;

			//Right pix.
			b = (int)( *(S + (yb*xpix + xb*2)*3 + 3) );
			g = (int)( *(S + (yb*xpix + xb*2)*3 + 4) );
			r = (int)( *(S + (yb*xpix + xb*2)*3 + 5) );
      RGBtoYUV(r,g,b,y,u,v);
      accu += u;
      accv += v;
			_Y[yb*xpix + xb*2 + 1] = y;

			u = accu/2;
			v = accv/2;
			_U[yb*(xpix/2) + xb] = u;
			_V[yb*(xpix/2) + xb] = v;
 		}//end for xb & yb...

	}//end ConvertRGB24BITtoYUV422.

void ImageHandler::ConvertYUV422toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix,r,b,g;
	int y,u,v;

	xpix = _bmih->biWidth;
	D = (unsigned char *)_bmptr;
	//2 pix per Block.
	XB = _bmih->biWidth >> 1;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = _U[yb*(xpix/2) + xb];
		v = _V[yb*(xpix/2) + xb];

		//Left pix.
		y = _Y[yb*xpix + xb*2];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 2) = (unsigned char)r;

		//Right pix.
		y = _Y[yb*xpix + xb*2 + 1];
    YUVtoRGB(y,u,v,r,g,b);
		*(D + (yb*xpix + xb*2)*3 + 3) = (unsigned char)b;
		*(D + (yb*xpix + xb*2)*3 + 4) = (unsigned char)g;
		*(D + (yb*xpix + xb*2)*3 + 5) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV422toRGB24BIT.

void ImageHandler::ConvertRGB24BITtoYUV444(void)
{
  // Y has range 0..255,
  // U & V have range -128..127.
	unsigned char *S;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = _bmih->biWidth;
	S = (unsigned char *)_bmptr;
	//1 pix per Block.
	XB = _bmih->biWidth;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
	 for(xb = 0; xb < XB; xb++)
	{
		b = (int)( *(S + (yb*xpix + xb)*3) );
		g = (int)( *(S + (yb*xpix + xb)*3 + 1) );
		r = (int)( *(S + (yb*xpix + xb)*3 + 2) );

    RGBtoYUV(r,g,b,y,u,v);

		_Y[yb*xpix + xb] = y;
		_U[yb*xpix + xb] = u;
		_V[yb*xpix + xb] = v;
	}//end for xb & yb...

}//end ConvertRGB24BITtoYUV444.

void ImageHandler::ConvertYUV444toRGB24BIT(void)
{
	// R, G & B have range 0..255,
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
	int r,g,b,y,u,v;

	xpix = _bmih->biWidth;
	D = (unsigned char *)_bmptr;
	//1 pix per Block.
	XB = _bmih->biWidth;
	YB = _bmih->biHeight;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
		u = _U[yb*xpix + xb];
		v = _V[yb*xpix + xb];
		y = _Y[yb*xpix + xb];

    YUVtoRGB(y,u,v,r,g,b);

		*(D + (yb*xpix + xb)*3) = (unsigned char)b;
		*(D + (yb*xpix + xb)*3 + 1) = (unsigned char)g;
		*(D + (yb*xpix + xb)*3 + 2) = (unsigned char)r;
	}//end for xb & yb...

}//end ConvertYUV444toRGB24BIT.

// Matrix conversion including rounding and bounds
// checking.
void ImageHandler::RGBtoYUV(	double R,double G,double B,
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

//void ImageHandler::RGBtoYUV(int R,int G,int B,
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

void ImageHandler::RGBtoYUV(int R,int G,int B,
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

//void ImageHandler::YUVtoRGB(int Y,int U,int V,
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

void ImageHandler::YUVtoRGB(int Y,int U,int V,
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


