//*********************************************************************
// TITLE       :VIDEO CODEC ABSTRACT CLASS
// VERSION     :1.0
// FILE        :VideoCodec.h
// DESCRIPTION :An abstract base class for inline video codec type objects.
// DATE        :May 2000
// AUTHOR      :K.L.Ferguson and C.M. Domoney
//**********************************************************************
#ifndef _VIDEOCODEC_H
#define _VIDEOCODEC_H

///////////////////////////////////////////////////////////////////
// Define a structure to hold the video codec operational parameters.
typedef struct tagVIDEOCODEC_PARAMETERS
{
  //Required operating parameters.
  int Q;
  int Thresh;
  //Description and location of the source/destination image.
  void *pPel;
  int Pel_X;
  int Pel_Y;

  //Compressed data absolute limit in bits.
  unsigned long int MAX_BIT_SIZE;

  //Motion Estimation and compensation parameters.
  int Motion; //Motion? 0 = None, 1 = 1 pix, 2 = 1/2 pix.
} VIDEOCODEC_PARAMETERS;

// C.M. Domoney : Added 
///////////////////////////////////////////////////////////////////
// Define a structure to describe the video codec
typedef struct tagVIDEOCODEC_INFO
{
	// The unique 16bit ID code identifying the codec
	unsigned short idCode;

	// The full name of the codec
	const char *longDescription;
		
	// The short name of the codec - keep less than 12 characters
	const char *shortDescription;

	// The mime type of the codec
	const char *mimeType;

	// The version string in the form "VC123" where :
	// "V" stands for VICS
	// "C" is the codec type - "C" is DCT based, "W" is wavelet based
  // "1" is coding loop type
  // "23" is version number
	const char *name;
}	VIDEOCODEC_INFO;

///////////////////////////////////////////////////////////////////
// Class definition.
class CVIDEOCODEC
{
protected :
  int CodecIsOpen;
	char *ErrorStr;
  VIDEOCODEC_PARAMETERS Parameters;
	VIDEOCODEC_INFO CodecInfo;
  unsigned long int BitStreamSize;

public :
	CVIDEOCODEC(void) {CodecIsOpen = 0; ErrorStr = "No Error"; BitStreamSize = 0; CodecInfo.idCode = 0;}
  CVIDEOCODEC(const CVIDEOCODEC &SrcCodec) { } //Copy constructor.
  bool operator==(const CVIDEOCODEC &src) { return(this == &src); }

  virtual VIDEOCODEC_PARAMETERS *GetParameters(void) { return(&Parameters); }
  virtual int SetParameters(VIDEOCODEC_PARAMETERS *Params) { return(1); }
  virtual void *GetCodecSpecificParameters(void) { return(NULL); }
  virtual int SetCodecSpecificParameters(void *Params) { return(1); }
  
  // C.M. Domoney : Added
  virtual VIDEOCODEC_INFO *GetCodecInfo(void) { return(&CodecInfo); }
  
  virtual int Ready(void) { return(CodecIsOpen); }
  
  virtual char *GetErrorStr(void) { return(ErrorStr); }
  virtual unsigned long int GetCompressedBitLength(void) { return(BitStreamSize); }
  virtual void CorrectPartitionError(int partition) { }
  virtual void Restart(void) { }
  virtual void DynamicSetQuality(int Q) { } 
  virtual void DynamicSetThreshold(int Thresh) { Parameters.Thresh = Thresh;} 
  virtual void DynamicSetMotionType(int Motion) { } 

	virtual int Open(VIDEOCODEC_PARAMETERS *Params) { return(1);}
	virtual int Open(void) { return(Open(&Parameters));}
	
  virtual int Close(void) { return(1); }
	virtual int Code(void *pI,void *pCmp,unsigned long int FrameBitLimit) = 0;
	virtual int Decode(void *pCmp,unsigned long int FrameBitSize,void *pI) = 0;

};//end CVIDEOCODEC class

#endif
