/** @file

MODULE						: AviFileHandler

TAG								: AFHN

FILE NAME					: AviFileHandlerNET.cpp

DESCRIPTION				: A managed class for implementing access utilities
										for the Video for Windows file functions.
										vfw32.lib must be included in the link process.
										Video for Windows must be initialised once in 
										the app with a call to AVIFileInit() and at exit
										a call to AVIFileExit() must be done.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/

/////// This code is not yet functional and is abandoned.

#include "stdafx.h"
#include	"AviFileHandlerNET.h"

#using <mscorlib.dll>

using namespace System;
using namespace System::Runtime::InteropServices;

/*
--------------------------------------------------------------------------
  External calls. 
--------------------------------------------------------------------------
*/
[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" void AVIFileInit();

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" void AVIFileExit();

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIFileOpenW(PAVIFILE FAR * ppfile, LPCWSTR szFile, UINT uMode, LPCLSID lpHandler);
#define AVIFileOpen AVIFileOpenW

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIFileGetStreamW(PAVIFILE pfile, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam);
#define AVIFileGetStream AVIFileGetStreamW

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIStreamInfoW(PAVISTREAM pavi, mAVISTREAMINFO* psi, LONG lSize);
#define AVIStreamInfo AVIStreamInfoW

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIStreamReadFormat(PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG FAR *lpcbFormat);

#define AVIStreamFormatSize(pavi, lPos, plSize) AVIStreamReadFormat(pavi,lPos,NULL,plSize)

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" LONG AVIStreamStart(PAVISTREAM pavi);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIFileCreateStreamW(PAVIFILE pfile, PAVISTREAM FAR *ppavi, mAVISTREAMINFO* psi);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIStreamSetFormat(PAVISTREAM pavi, LONG lPos,LPVOID lpFormat,LONG cbFormat);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" ULONG AVIStreamRelease(PAVISTREAM pavi);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" ULONG AVIFileRelease(PAVIFILE pfile);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" LONG AVIStreamLength(PAVISTREAM pavi);

#define AVIStreamEnd(pavi) (AVIStreamStart(pavi) + AVIStreamLength(pavi))

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIStreamRead(PAVISTREAM pavi,LONG lStart,LONG lSamples,LPVOID lpBuffer,LONG cbBuffer,LONG FAR * plBytes,LONG FAR * plSamples);

[DllImportAttribute("avifil32.dll", CharSet=CharSet::Auto)]
extern "C" HRESULT AVIStreamWrite(PAVISTREAM pavi,LONG lStart,LONG lSamples,LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags,LONG FAR *plSampWritten,LONG FAR *plBytesWritten);

/*
--------------------------------------------------------------------------
  Class Methods. 
--------------------------------------------------------------------------
*/

AviFileHandler::AviFileHandler(void)
{
	_frame				= NULL;
	_errorStr			= "AviFileHandler: No error";
	_fileIsOpen		= 0;
	_hAviFile			= 1;
	_filename			= S" ";
	_pAviFile			= NULL;
	_pVideoStream = NULL;
	_pStreamInfo	= NULL;
	_streamCount	= 0;
	_frameRate		= 1;
	_yuvFormat		= 0;
}//end constructor.

AviFileHandler::~AviFileHandler(void)
{
	if(IsOpen())
		Close();
}//end destructor.

int AviFileHandler::Open(String*						filename, 
												 int								mode, 
												 mAVISTREAMINFO*		streamInfo,
												 BITMAPINFOHEADER*	streamBmih)
{
	// Remove any existing open files.
	if(IsOpen())
		Close();
	_fileIsOpen	= 0;	// All closed.

  if(!filename->EndsWith(S".avi"))
  {
    _errorStr = S"AviFileHandler: File is not of type .avi";
    return(0);
  }//end if EndsWith...

	_mode = mode;
	if(mode == WRITE)
		return(OpenWrite(filename, streamInfo, streamBmih));
	else	//(mode == READ) Default.
		return(OpenRead(filename));

}//end Open...

int AviFileHandler::OpenRead(String* filename)
{
  PAVISTREAM				pAVIStream[MAX_STREAMS];
  mAVISTREAMINFO*		streamInfo = new mAVISTREAMINFO();
  BITMAPINFOHEADER	streamBitmapHead;
  int								numStream;
  LONG							sfSize;

  _hAviFile = AVIFileOpen(&_pAviFile, filename, OF_SHARE_DENY_WRITE,0L);
  if (_hAviFile != 0)
	{
    _errorStr = S"AviFileHandler: Unable to open AVI file for reading";
    return(0); 
	}//end if _hAviFile...

  // Update the file associated members.
  _filename		= filename;
  _fileIsOpen = 1;
  
  // Search for the video stream in the file.
  numStream			= 0;
  _pVideoStream	= NULL;
  while( (numStream < MAX_STREAMS) &&
  	     (AVIFileGetStream(_pAviFile, &pAVIStream[numStream], 0L, numStream)==AVIERR_OK) )
	{
  	AVIStreamInfo(pAVIStream[numStream], streamInfo, sizeof(streamInfo));
  	if(streamInfo->fccType == streamtypeVIDEO)
  		_pVideoStream = pAVIStream[numStream];
  	numStream++;
  }//end while numStream...

  if(_pVideoStream == NULL)
	{
  	_errorStr = S"AviFileHandler: No video streams in AVI file";
    Close();  // closes the file. 
    return(0); 
	}//end if pVideoStream...

  // Load and check the stream video format.
  AVIStreamFormatSize(_pVideoStream, 0, &sfSize);
  if( sfSize > sizeof(BITMAPINFOHEADER) )
	{
  	_errorStr = S"AviFileHandler: AVI file video format is too large";
    Close(); 
    return(0); 
	}//end if sfSize...
  sfSize = sizeof(BITMAPINFOHEADER);
	
  AVIStreamReadFormat(_pVideoStream, 0, &streamBitmapHead, &sfSize);
  AVIStreamInfo(_pVideoStream, streamInfo, sizeof(*streamInfo));
  // Set member parameters.
	if(_pStreamInfo != NULL)
		_pStreamInfo = new mAVISTREAMINFO();

  _pStreamInfo->fccType = streamInfo->fccType;
  _pStreamInfo->fccHandler = streamInfo->fccHandler;
  _pStreamInfo->dwFlags = streamInfo->dwFlags;
  _pStreamInfo->dwCaps = streamInfo->dwCaps;
  _pStreamInfo->wPriority = streamInfo->wPriority;
  _pStreamInfo->wLanguage = streamInfo->wLanguage;
  _pStreamInfo->dwScale = streamInfo->dwScale;
  _pStreamInfo->dwRate = streamInfo->dwRate;
  _pStreamInfo->dwStart = streamInfo->dwStart;
  _pStreamInfo->dwLength = streamInfo->dwLength;
  _pStreamInfo->dwInitialFrames = streamInfo->dwInitialFrames;
  _pStreamInfo->dwSuggestedBufferSize = streamInfo->dwSuggestedBufferSize;
  _pStreamInfo->dwQuality = streamInfo->dwQuality;
  _pStreamInfo->dwSampleSize = streamInfo->dwSampleSize;
  _pStreamInfo->rcFrame = streamInfo->rcFrame;
  _pStreamInfo->dwEditCount = streamInfo->dwEditCount;
  _pStreamInfo->dwFormatChangeCount = streamInfo->dwFormatChangeCount;
	for(int i = 0; i < 64; i++)
		_pStreamInfo->szName[i] = streamInfo->szName[i];

  //	Create the owned frame image.
	_frame = new ImageHandler();
	if(_frame == NULL)
	{
  	_errorStr = "AviFileHandler: Cannot create frame image object";
    Close(); 
    return(0); 
	}//end if _frame...

  if(!_frame->CreateImage(&streamBitmapHead))
	{
  	_errorStr = _frame->GetErrorStr();
    Close(); 
    return(0); 
	}//end if !CreateImage...
  _streamCount = AVIStreamStart(_pVideoStream);

  // Determine the frame rate of the video sequence.
  _frameRate = (_pStreamInfo->dwRate)/(_pStreamInfo->dwScale);

	return(1);
}//end OpenRead.

int AviFileHandler::OpenWrite(String*						filename, 
															mAVISTREAMINFO*		streamInfo, 
															BITMAPINFOHEADER* streamBmih)
{
	// Check for validity.
	if((streamInfo == NULL)||(streamBmih == NULL))
	{
    _errorStr = S"AviFileHandler: No valid stream/bitmap info for writing";
    return(0); 
	}//end if streamInfo...

  // Attempt to open an output avi file.
  _hAviFile = AVIFileOpen(&_pAviFile, filename,	OF_CREATE | OF_WRITE,0);
  if (_hAviFile != 0)
	{
    _errorStr = S"AviFileHandler: Unable to open AVI file for writing";
    return(0); 
	}//end if _hAviFile...

  // Update the file associated members.
  _filename		= filename;
  _fileIsOpen = 1;
  
	// Create an output video stream from video	stream parameters.
  LONG hr = AVIFileCreateStream(_pAviFile, &_pVideoStream, streamInfo); 
  if (hr != 0)
	{
  	_errorStr = S"AviFileHandler: Unable to create AVI stream";
    Close();  // closes the file. 
    return(0); 
	}//end if hr...
  // Set member parameters.
	if(_pStreamInfo != NULL)
		_pStreamInfo = new mAVISTREAMINFO();

  _pStreamInfo->fccType = streamInfo->fccType;
  _pStreamInfo->fccHandler = streamInfo->fccHandler;
  _pStreamInfo->dwFlags = streamInfo->dwFlags;
  _pStreamInfo->dwCaps = streamInfo->dwCaps;
  _pStreamInfo->wPriority = streamInfo->wPriority;
  _pStreamInfo->wLanguage = streamInfo->wLanguage;
  _pStreamInfo->dwScale = streamInfo->dwScale;
  _pStreamInfo->dwRate = streamInfo->dwRate;
  _pStreamInfo->dwStart = streamInfo->dwStart;
  _pStreamInfo->dwLength = streamInfo->dwLength;
  _pStreamInfo->dwInitialFrames = streamInfo->dwInitialFrames;
  _pStreamInfo->dwSuggestedBufferSize = streamInfo->dwSuggestedBufferSize;
  _pStreamInfo->dwQuality = streamInfo->dwQuality;
  _pStreamInfo->dwSampleSize = streamInfo->dwSampleSize;
  _pStreamInfo->rcFrame = streamInfo->rcFrame;
  _pStreamInfo->dwEditCount = streamInfo->dwEditCount;
  _pStreamInfo->dwFormatChangeCount = streamInfo->dwFormatChangeCount;
	for(int i = 0; i < 64; i++)
		_pStreamInfo->szName[i] = streamInfo->szName[i];

  //	Create the owned frame image.
	_frame = new ImageHandler();
	if(_frame == NULL)
	{
  	_errorStr = S"AviFileHandler: Cannot create frame image object";
    Close(); 
    return(0); 
	}//end if _frame...

  if(!_frame->CreateImage(streamBmih))
	{
  	_errorStr = _frame->GetErrorStr();
    Close(); 
    return(0); 
	}//end if !CreateImage...

	// Set the format of the created stream to the format of the
  // processed image bitmap.
  hr = AVIStreamSetFormat(_pVideoStream, 0, streamBmih, sizeof(BITMAPINFOHEADER)); 
  if (hr != 0)
	{
  	_errorStr = S"AviFileHandler: Unable to format output AVI stream";
    Close(); 
    return(0); 
  }//end if hr...

  // Reset the stream count.
  _streamCount = 0;

	return(1);
}//end OpenWrite...

void AviFileHandler::Close(void)
{
  if(_hAviFile == 0)
	{
		AVIStreamRelease(_pVideoStream);
	  AVIFileRelease(_pAviFile);  // closes the file 
	}//end if _hAviFile...
  _hAviFile = 1;

	if(_pStreamInfo != NULL)
		_pStreamInfo->Dispose();
	_pStreamInfo = NULL;

  _fileIsOpen		= 0;
  _pVideoStream = NULL;
  _streamCount	= 0;
	_errorStr			= S"AviFileHandler: No error";
	_filename			= S" ";
	_pAviFile			= NULL;
	_frameRate		= 1;
	_yuvFormat		= 0;
	_mode					= READ;

	if(_frame != NULL)
	{
		_frame->DestroyImage();
		delete _frame;
	}//end if _frame...
	_frame = NULL;
}//end Close.

ImageHandler* AviFileHandler::GetNextImage(void)
{
	// Read only method.
	if((!_fileIsOpen)||(_mode != READ))
    return(NULL);
	// ...or at the end.
  if(_streamCount > AVIStreamEnd(_pVideoStream))
    return(NULL);
  
  // Read the image data from the avi video stream.
  if( AVIStreamRead(_pVideoStream, _streamCount, 1,
  	                _frame->bmptr, _frame->bmih->biSizeImage,
  									NULL,NULL)
      != 0 )
    {
    }//end if AVIStreamRead... 

  _streamCount++;

  // Set the colour mode.
	if(_yuvFormat)
	{
		if(!_frame->ConvertToYUV(_yuvFormat))
		{
  		_errorStr = _frame->GetErrorStr();
		}//end if !ConvertToYUV...
	}//end if _yuvFormat...

	// Pass back the reference.
  return(_frame);
}//end GetNextImage.
 
int AviFileHandler::PutNextImage(ImageHandler* img)
{
	// Write only method.
	if((!_fileIsOpen)||(_mode != WRITE))
    return(0);

	// Copy to internal frame.
	SetFrame(img);

  // Write the image data to the output avi video stream.
  if( AVIStreamWrite(_pVideoStream, _streamCount, 1, 
																		_frame->bmptr, 
																		_frame->bmih->biSizeImage,
                     					      AVIIF_KEYFRAME, NULL, NULL)
      != 0 )
    {
    }//end if AVIStreamRead... 

  _streamCount++;

  return(1);
}//end PutNextImage.

int AviFileHandler::GetImageNum(void)
{
  // The current image is one behind the Stream count.
  if(_fileIsOpen)
    return(_streamCount - 1);
  return(0);
}//end GetImageNum.
 
ImageHandler* AviFileHandler::Restart(void)
{
  if(_fileIsOpen)
  {
    _streamCount = AVIStreamStart(_pVideoStream);
  	// Load the first image in read mode.
  	return(GetNextImage());
  }//end if _fileIsOpen...

	return(NULL);
}//end Restart.

unsigned long AviFileHandler::GetFramePeriod_ms(void)
{
  if(_fileIsOpen)
    return(1000/_frameRate);
  else
    return(0);
}//end GetFramePeriod_ms.






