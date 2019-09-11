//**********************************************************
// TITLE       :AVI FILE UTILITY CLASS DEFINITION FILE
// VERSION     :1.0
// FILE        :AviFile.cpp
// DESCRIPTION :A class for implementing a access utilities
//              for the Video for Windows file functions.
//              vfw32.lib must be included in the link process. 
// DATE        :February 2001
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include <string.h>
#include "AviFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*//////////////////////////////////////////////////////////*/
/*/ Construction and destruction.                           */
/*//////////////////////////////////////////////////////////*/

CAVIFILE::CAVIFILE()
{
  ResetMembers();
}/*end constructor.*/

CAVIFILE::~CAVIFILE()
{
  if(IsOpen)
    Close();
}/*end constructor.*/

void CAVIFILE::ResetMembers(void)
{
  ErrorStr = "No error";
  FrameRate = 0;

	IsOpen = 0;
	hFile = 1;
	sFile = _T(" ");
	pFile = NULL;
	pVideoStream = NULL;
	StreamCount = 0;
}//end ResetMembers.

/*//////////////////////////////////////////////////////////*/
/*/ Public methods.                                         */
/*//////////////////////////////////////////////////////////*/

int CAVIFILE::Open(CString &Filename)
{
  PAVISTREAM pStream[MAX_STREAMS];
  AVISTREAMINFO localStreamInfo;
  BITMAPINFOHEADER StreamBitmapHead;
  int NumStream;
  LONG SFSize;

  IsOpen = 0;
 
  if(Filename.Find(_T(".avi")) == -1)
  {
    ErrorStr = "CAVIFILE: File is not of type .avi!";
    return(0);
  }//end if Find...

  hFile = AVIFileOpen(&pFile,Filename,OF_SHARE_DENY_WRITE,0L);
  if (hFile != 0)
	{
	  ErrorStr = "CAVIFILE: Unable to open AVI file!";
    return(0); 
	}//end if hFile...
  //Update the assocciated AVI file name.
  sFile = Filename;
  IsOpen = 1;
  
  // Search for the video stream in the file.
  NumStream = 0;
  pVideoStream = NULL;
  while( (NumStream < MAX_STREAMS) &&
  	     (AVIFileGetStream(pFile,&pStream[NumStream],0L,NumStream)==AVIERR_OK) )
	{
  	AVIStreamInfo(pStream[NumStream],&StreamInfo,sizeof(StreamInfo));
  	if(StreamInfo.fccType == streamtypeVIDEO)
  		pVideoStream = pStream[NumStream];
  	NumStream++;
  }//end while NumStream...
  if(pVideoStream == NULL)
	{
  	ErrorStr = "CAVIFILE: No video streams in AVI file!";
    Close();  // closes the file 
    return(0); 
	}//end if pVideoStream...

  // Load and check the stream video format.
  AVIStreamFormatSize(pVideoStream,0,&SFSize);
  if( SFSize > sizeof(BITMAPINFOHEADER) )
	{
  	ErrorStr = "CAVIFILE: AVI file video format is too large!";
    Close();  // closes the file 
    return(0); 
	}//end if SFSize...
  SFSize = sizeof(BITMAPINFOHEADER); 
  AVIStreamReadFormat(pVideoStream,0,&StreamBitmapHead,&SFSize);
  AVIStreamInfo(pVideoStream,&localStreamInfo,sizeof(localStreamInfo));
  //Set member parameters.
  StreamInfo = localStreamInfo;
  //Create the relavent image space.
  if(!Frame.CreateImage(&StreamBitmapHead))
	{
    LPTSTR lpsz = new TCHAR[Frame.ErrorStr.GetLength()+1];
    strcpy(lpsz, Frame.ErrorStr);
  	ErrorStr = lpsz;
    Close();  // closes the file 
  	return(0);
	}//end if !CreateImage...
  StreamCount = AVIStreamStart(pVideoStream);

  //Determine the frame rate of the video sequence.
  FrameRate = (StreamInfo.dwRate)/(StreamInfo.dwScale);

  // Load the first image and put valid data
  // in the DIB bitmap.
  if( !GetNextImage() )
	{
    Close();  // closes the file 
  	ErrorStr = "CAVIFILE: No stream data in AVI file!";
  	return(0);
	}//end if !GetNextImage... 

  return(1);
}//end Open.
 
int CAVIFILE::GetNextImage(void)
{
  if(StreamCount > AVIStreamEnd(pVideoStream))
    return(0);
  
  //Read the image data from the avi video stream.
  if( AVIStreamRead(pVideoStream,StreamCount,1,
  	                Frame.bmptr,Frame.bmih->biSizeImage,
  									NULL,NULL)
      != 0 )
    {
    }//end if AVIStreamRead... 

  StreamCount++;
  return(1);
}//end GetNextImage.
 
int CAVIFILE::GetImageNum(void)
{
  //The current image is one behind the Stream count.
  if(IsOpen)
    return(StreamCount - 1);
  return(0);
}//end GetImageNum.
 
void CAVIFILE::Restart(void)
{
  if(IsOpen)
  {
    StreamCount = AVIStreamStart(pVideoStream);
  	// Load the first image.
  	GetNextImage();
  }//end if IsOpen.

}//end Restart.

void CAVIFILE::Close(void)
{
  if(hFile == 0)
	{
		AVIStreamRelease(pVideoStream);
	  AVIFileRelease(pFile);  // closes the file 
	}//end if hFile...
  hFile = 1;
  IsOpen = 0;
  pVideoStream = NULL;
  StreamCount = 0;
  Frame.DestroyImage();
}//end Close.

unsigned long int CAVIFILE::GetFramePeriod_ms(void)
{
  if(IsOpen)
    return(1000/FrameRate);
  else
    return(0);
}//end GetFramePeriod_ms.

/*//////////////////////////////////////////////////////////*/
/*/ Protected methods.                                      */
/*//////////////////////////////////////////////////////////*/



