/** @file

MODULE						: AviFileHandler

TAG								: AFH

FILE NAME					: AviFileHandlerNET.h

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
#pragma once

/////// This code is not yet functional and is abandoned.

#include <windows.h>
#include "IExistance.h"
#include "ImageHandler.h"

using namespace System;
using namespace System::Runtime::InteropServices;

[StructLayoutAttribute(LayoutKind::Sequential)]
public __gc class mAVISTREAMINFO
{
  System::UInt32	fccType;
  System::UInt32  fccHandler;
  System::UInt32  dwFlags;        // Contains AVITF_* flags.
  System::UInt32	dwCaps;
  System::UInt16	wPriority;
  System::UInt16	wLanguage;
  System::UInt32  dwScale;
  System::UInt32  dwRate;					// dwRate / dwScale == samples/second.
  System::UInt32  dwStart;
  System::UInt32  dwLength;				// In units above... 
  System::UInt32	dwInitialFrames;
  System::UInt32  dwSuggestedBufferSize;
  System::UInt32  dwQuality;
  System::UInt32  dwSampleSize;
  RECT						rcFrame;
  System::UInt32	dwEditCount;
  System::UInt32	dwFormatChangeCount;
	System::Char		szName[64];
};

/*
---------------------------------------------------------------------------
	Class definitions.
---------------------------------------------------------------------------
*/
public __gc class AviFileHandler : public IExistance
{
	public:
		AviFileHandler(void);
		virtual ~AviFileHandler(void);

		// IExistance interface.
		int			Exists(void)			{ return(IsOpen()); }
		String*	GetErrorStr(void)	{ return(_errorStr); }


		// Open the avi file called filename and create the ImageHandler _frame. The
		// streamInfo parameter is when mode = WRITE.
		int Open(	String*						filename, 
							int								mode, 
							mAVISTREAMINFO*		streamInfo,
							BITMAPINFOHEADER*	streamBmih);

		// Load the next frame in the avi file to the _frame and pass back reference.
		ImageHandler* GetNextImage(void);
		// Store the input frame into the avi file and copy to _frame.
		int	PutNextImage(ImageHandler* img);

		// Get/Set the frame num position in the file.
		int  GetImageNum(void);
		void SetImageNum(int num) { _streamCount = num; }

		// Move back to the 1st frame in the avi file and
		// load it into _frame.
		ImageHandler* Restart(void);

		// Close the avi file and delete the ImageHandler _frame created in Open().
		void Close(void);

		unsigned long GetFramePeriod_ms(void);

		int						IsOpen(void)					{ return(_fileIsOpen); }

		String*				GetFilename(void)			{ return(_filename); }

		void					SetFilename(String* filename) { _filename = filename; }

		unsigned long GetFrameRate(void)		{ return(_frameRate); }

		ImageHandler*	GetFrame(void)				{ return(_frame); }

		void 					SetFrame(ImageHandler* img)	{ _frame->Assign(*img); }

		mAVISTREAMINFO* GetStreamInfo(void)	{ return(_pStreamInfo); }

		void					SetColourMode(int yuvFormat = 0)	{ _yuvFormat = yuvFormat; }

		int						GetColourMode(void)		{ return(_yuvFormat); }

	private:
		int OpenRead(String* filename);

		int OpenWrite(String*						filename, 
									mAVISTREAMINFO*		streamInfo, 
									BITMAPINFOHEADER*	streamBmih);

	public:
		static const int	MAX_STREAMS	= 5;
		static const int	READ				= 0;
		static const int	WRITE				= 1;

	private:
		ImageHandler*		_frame;	// Hold the working image frame.
		String*					_errorStr;
		int							_fileIsOpen;
		int							_mode;
		LONG						_hAviFile;
		String*					_filename;
		PAVIFILE				_pAviFile;
		mAVISTREAMINFO*	_pStreamInfo;
		PAVISTREAM			_pVideoStream;
		int							_streamCount;
		unsigned long		_frameRate;
		int							_yuvFormat;	// Colour formats specified in CImage.

};//end AviFileHandler.


