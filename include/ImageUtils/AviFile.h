//**********************************************************
// TITLE       :AVI FILE UTILITY CLASS HEADER FILE
// VERSION     :1.0
// FILE        :AviFile.h
// DESCRIPTION :A class for implementing a access utilities
//              for the Video for Windows file functions.
//              vfw32.lib must be included in the link process. 
// DATE        :February 2001
// AUTHOR      :K.L.Ferguson
//**********************************************************
#ifndef _AVIFILE_H
#define _AVIFILE_H

#include <vfw.h>
#include "CImage.h"

///////////////////////////////////////////////////////////////////
#define MAX_STREAMS  5

///////////////////////////////////////////////////////////////////
class CAVIFILE
{
// Attributes
protected:
	char *ErrorStr;
  unsigned long FrameRate;

  //Define AVI file details.
  int IsOpen;
  LONG hFile;
  CString sFile;
	PAVIFILE pFile;
  AVISTREAMINFO StreamInfo;
	PAVISTREAM pVideoStream;
	int StreamCount;

public:
  //Frame is available to parent.
  CImage Frame;

// Implementation
public:
	CAVIFILE();
	~CAVIFILE();

  char *GetErrorStr(void) { return(ErrorStr); }
  // Open the avi file called Filename and create the CImage Frame.
  // Write the 1st frame in the avi file to Frame.
  int Open(CString &Filename);
  // Load the next frame in the avi file to Frame.
  int GetNextImage(void);
  int GetImageNum(void);
  // Move back to the 1st frame in the avi file and
  // load it into Frame.
  void Restart(void);
  // Close the avi file and delete the CImage Frame mem.
  void Close(void);
  int FileIsOpen(void) { return(IsOpen); }
  unsigned long int GetFramePeriod_ms(void);

protected:
  void ResetMembers(void);

};//CAVIFILE class.


#endif
