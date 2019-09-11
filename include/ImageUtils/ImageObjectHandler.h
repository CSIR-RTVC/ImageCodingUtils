/** @file

MODULE						: ImageObjectHandler

TAG								: IO

FILE NAME					: ImageObjectHandler.h

DESCRIPTION				: Definition of class to provide funcionality for an object to be placed on an 
                    screen and image modifications to be made.

REVISION HISTORY	:	

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <string>
#include "ImageHandlerV2.h"
#include "PicScalerRGB24Impl.h"

using std::string;

#ifndef _IMAGEOBJECTHANDLER_H
#define _IMAGEOBJECTHANDLER_H

#pragma once

// Forward declarations
class PicScalerBase;

class ImageObjectHandler : public ImageHandlerV2
{

  // Implementation
public:
	ImageObjectHandler();

	virtual ~ImageObjectHandler();

/// Public interface
public:
	/// Paint this obj onto the canvas specified in the method parameter. Return 1/0 = success/failure.
  virtual int PaintOnto(ImageHandlerV2* _pDisplayImg);

	void	SetPosition(int x, int y) { _xPos = x; _yPos = y; }
  void	SetAngle(int degree) { _angle = degree;}

	int		GetXPos(void)							{ return(_xPos); }
	int		GetYPos(void)							{ return(_yPos); }
  int		GetAngle(void)							{ return(_angle); }

	ImageHandlerV2* GetDisplayImg(void)											{ return(_pDisplayImg); }
	void						SetDisplayDim(int width, int height)		{ _displyWidth = width; _displyHeight = height;}
	int							GetDisplayWidth(void)										{ return(_displyWidth); }
	int							GetDisplayHeight(void)									{ return(_displyHeight); }

/// Private methods.
protected:
	/// Convert the obj size to the dimensions defined by _displyWidth and _displyHeight. Return 1/0 = success/failure.
 virtual int SetDisplaySize(void);
 //virtual int RotateImage(ImageHandlerV2* _pDisplayImg);

 int Ready(void) { return(_ready); } /// Test that the constructor declared mem successfully.


/// Class properties.
protected:
	/// Position that this img obj will paint itself to on the canvas.
	int		_xPos;
	int		_yPos;

  /// Angle that this img obj will paint itself at on the canvas.
  int   _angle; 
 
	/// An image that is the scaled version of the object.
	ImageHandlerV2*			_pDisplayImg;
  //ImageHandlerV2*			_pDiagonal;
  int									_displyWidth;
	int									_displyHeight;
  int                 m_nBitsPerPixel;
	PicScalerRGB24Impl*	_pImgScaler;

	int	_ready; /// Memory declaration is successful

	/// Transparency of this obj on the canvas.


};//ImageObjectHandler class. 


#endif	//_IMAGEOBJECTHANDLER_H
