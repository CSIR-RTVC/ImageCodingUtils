//*********************************************************************
// TITLE       :CODEC ABSTRACT CLASS
// VERSION     :2.0
// FILE        :codec.h
// DESCRIPTION :An abstract class for inline CODEC type objects.
// DATE        :December 1997
// AUTHOR      :K.L.Ferguson
//**********************************************************************
#ifndef _CODEC_H
#define _CODEC_H

class CODEC
{
protected :
	int error;

public :
	CODEC(void) {error = 0;}

	virtual void SetParameters(void *Params) { }
	virtual int OpenCODEC(void *Params) { return(1);}
	virtual void CloseCODEC(void) { }
	virtual int CODE(void *Data) = 0;
	virtual int DECODE(void *Data) = 0;
	virtual int Error(void) {int reterr = error;error=0;return(reterr);}
	virtual const char *GetErrorStr(int ErrorNum, char *ErrStr) = 0;
};//end CODEC class

#endif
