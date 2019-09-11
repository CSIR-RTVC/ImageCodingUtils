/** @file

MODULE						: AviFileHandlerUsingCImage

TAG								: AFH

FILE NAME					: AviFileHandlerUsingCImage.h

DESCRIPTION				: A class for implementing access utilities
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
#ifndef _AVIFILEHANDLERUSINGCIMAGE_H
#define _AVIFILEHANDLERUSINGCIMAGE_H

#include <vfw.h>

#include "CImage.h"

/*
---------------------------------------------------------------------------
	Class definitions.
---------------------------------------------------------------------------
*/
class AviFileHandler
{
	public:
		AviFileHandler(void);
		~AviFileHandler(void);

		// Open the avi file called filename and create the CImage _frame. The
		// streamInfo parameter is when mode = WRITE.
		int Open(	CString&					filename, 
							int								mode				= READ, 
							AVISTREAMINFO*		streamInfo	= NULL,
							BITMAPINFOHEADER*	streamBmih	= NULL);

		// Load the next frame in the avi file to the _frame and pass back reference.
		CImage* GetNextImage(void);
		// Store the input frame into the avi file and copy to _frame.
		int	PutNextImage(CImage* img);

		// Get/Set the frame num position in the file.
		int  GetImageNum(void);
		void SetImageNum(int num) { _streamCount = num; }

		// Move back to the 1st frame in the avi file and
		// load it into _frame.
		CImage* Restart(void);

		// Close the avi file and delete the CImage _frame created in Open().
		void Close(void);

		unsigned long GetFramePeriod_ms(void);

		int						IsOpen(void)					{ return(_fileIsOpen); }

		char*					GetErrorStr(void)			{ return(_errorStr); }

		CString&			GetFilename(void)			{ return(_filename); }

		void					SetFilename(CString& filename) { _filename = filename; }

		unsigned long GetFrameRate(void)		{ return(_frameRate); }

		CImage*				GetFrame(void)				{ return(_frame); }

		void 					SetFrame(CImage* img)	{ *_frame = *img; }

		AVISTREAMINFO* GetStreamInfo(void)	{ return(&_streamInfo); }

		void					SetColourMode(int yuvFormat = 0)	{ _yuvFormat = yuvFormat; }

		int						GetColourMode(void)		{ return(_yuvFormat); }

	private:
		int OpenRead(CString& filename);

		int OpenWrite(CString&					filename, 
									AVISTREAMINFO*		streamInfo, 
									BITMAPINFOHEADER*	streamBmih);

	public:
		static const int	MAX_STREAMS;
		static const int	READ;
		static const int	WRITE;

	private:
		CImage*				_frame;	// Hold the working image frame.
		char*					_errorStr;
		int						_fileIsOpen;
		int						_mode;
		LONG					_hAviFile;
		CString				_filename;
		PAVIFILE			_pAviFile;
		AVISTREAMINFO _streamInfo;
		PAVISTREAM		_pVideoStream;
		int						_streamCount;
		unsigned long	_frameRate;
		int						_yuvFormat;	// Colour formats specified in CImage.

};//end AviFileHandler.


#endif // _AVIFILEHANDLERUSINGCIMAGE_H

