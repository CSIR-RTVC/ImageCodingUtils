/** @file

MODULE						: ImageObjectHandler

TAG								: IO

FILE NAME					: ImageObjectHandler.cpp

DESCRIPTION				: Implementation of the InmageObjectHandler class to provide funcionality 
                    for an object to be placed on a screen and image enhancements to be made. 
                    These include: Chromakeying,Positioning, Scaling and Rotating the Image on a screen.

REVISION HISTORY	:	

COPYRIGHT					: @vsentongo@csir.co,za 

RESTRICTIONS			: 
===========================================================================
*/

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "ImageObjectHandler.h"
#include <math.h>

using namespace std;

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define PI 3.14159265
#define PI 3.1416
/*
---------------------------------------------------------------------------
	Public class methods.
---------------------------------------------------------------------------
*/
ImageObjectHandler::ImageObjectHandler()
{
  m_nBitsPerPixel = 24; //BITS_PER_PIXEL_RGB24
	_ready = 0; ///< Start with mem not ready as it is not yet declared.

	/// Position that this img obj will paint itself to on the canvas.
	_xPos = 0;
	_yPos = 0;

	/// An image that is the scaled version of the object.
	_pDisplayImg = NULL;
	_pDisplayImg = new ImageHandlerV2();

  _pImgScaler = NULL;
	_pImgScaler = new PicScalerRGB24Impl();
	if( (_pDisplayImg != NULL)&&(_pImgScaler != NULL) )
		_ready = 1;

	_displyWidth	= 0;
	_displyHeight	= 0;

  /// Angle that this img obj will paint itself at on the canvas.
  _angle = 0;

	/// Transparency of this obj on the canvas.

}//end DuckObjecHandler Constructor.

ImageObjectHandler::~ImageObjectHandler()
{
 	if(_pDisplayImg != NULL)
		delete _pDisplayImg;
	_pDisplayImg = NULL;

	if(_pImgScaler != NULL)
		delete _pImgScaler;
	_pImgScaler = NULL;

	_ready = 0;

}//end ImageHandler Destructor.

int ImageObjectHandler::SetDisplaySize(void)
{       
  BITMAPINFOHEADER lclbmih;   //create new bitmapinfo header
  
  lclbmih.biHeight        = _displyHeight;
  lclbmih.biWidth         = _displyWidth;
  lclbmih.biSize          = sizeof(BITMAPINFOHEADER);
  lclbmih.biPlanes        = 1;
  lclbmih.biBitCount      = 24;                                // 24-bit
  lclbmih.biCompression   = BI_RGB;                            // no compression
  lclbmih.biSizeImage     = _displyHeight * _displyWidth *3 ;  // width * height * (RGB bytes)
  lclbmih.biXPelsPerMeter = 0;
  lclbmih.biYPelsPerMeter = 0;
  lclbmih.biClrUsed       = 0;
  lclbmih.biClrImportant  = 0;

        
  return (_pDisplayImg->CreateImage(&lclbmih));
         
}

int ImageObjectHandler::PaintOnto(ImageHandlerV2 * canvas)
{
  if(canvas == NULL)
    {
      _errorStr = "ImageObjectHandler: Canvas does not exist";
        return(0);
    }
  else if(!canvas->ImageMemIsAlloc())
  {
    _errorStr = "ImageObjectHandler: Canvas is empty";
    return(0);
  }
  if(!ImageMemIsAlloc())
    {
      _errorStr = "ImageObjectHandler: Object bitmap does not exist";
        return(0);
    }
  if(_displyWidth <0)  ///check for negative width magnitude values
    {
       _errorStr = "ImageObjectHandler:Cannot display a negative width dimensional image";
       return(0);
    } 
  if(_displyHeight < 0) ///check for negative height magnitude values
    _displyHeight = abs(_displyHeight);
  
  int ret = 1;
  if(!_pDisplayImg->ImageMemIsAlloc())
   ret = SetDisplaySize();  ///Set correct bitmap memory size defined by (_displyWidth, _displyHeight)
  else
  { 
    if((_displyWidth != _pDisplayImg->_bmih->biWidth) || (_displyHeight != _pDisplayImg->_bmih->biHeight))
       ret = SetDisplaySize();
  }
  if(!ret)
  {
     _errorStr = "ImageObjectHandler:Unable to create Display Image";
     return(0);
  }

  //***************** Scaling the image object********************************************************

  if (_pImgScaler)
  {
    //Call scaling conversion code
    _pImgScaler->SetInDimensions(_bmih->biWidth,abs(_bmih->biHeight)); //configuring scaler
		_pImgScaler->SetOutDimensions(_pDisplayImg->_bmih->biWidth,abs(_pDisplayImg->_bmih->biHeight));//configuring scale
    _pImgScaler->Scale((void*)_pDisplayImg->_bmptr, (void*)_bmptr); //scaling video
 	}
  
 //***************** Rotating the image object**********************************************************

  double radians     = (2*PI*_angle)/360;              //changes degrees of angle of rotation to radians

  unsigned char * pDisplay = (unsigned char*)(_pDisplayImg->_bmptr); ///pointer to first byte of image to be painted
  unsigned char * pCanvas  = (unsigned char*)(canvas->_bmptr);       ///pointer to first byte of canvas to be painted on

  int canvas_height = canvas->_bmih->biHeight ;        // dimensions of canvas
  int canvas_width  = canvas->_bmih->biWidth ;

  int height_offset = _yPos;		                       //position of image on canvas
  int width_offset  = _xPos; 

  int canvasCentrX = width_offset*2;                   //new centre of rotation
  int canvasCentrY = height_offset*2;  

  int centrX = _displyWidth/2;                         //adjusted centre of rotation
  int centrY = _displyHeight/2;
 
  if (canvas->_bmih->biBitCount != 24)                               //checks if image is RGB 24-bit
     return (0);
                
  for (int row=0; row<canvas_height; ++row)
   { 
     for (int col=0; col<canvas_width; ++col)
     { 
       int canvasPosx = (width_offset+col);
       int canvasPosy = (height_offset+row);
  
       if ((canvasPosx > 0) && (canvasPosx < canvas_width) && (canvasPosy > 0) && (canvasPosy < canvas_height)) ///keeps object within image boundaries
          { 
            int newx = col - canvasCentrX;
            int newy = canvasCentrY - row;

            double hypotenuse = ((newx*newx)+(newy*newy)); 
            double radius = sqrt(hypotenuse);

            double PolarAngle = 0.0;

            if (newx == 0)                             //checking for quadrant in which new points lie
            { 
              if (newy == 0)
                {
                 pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 0]= pDisplay[((centrY*_displyWidth)+ centrX)*3 + 0]; 
                 pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 1]= pDisplay[((centrY*_displyWidth)+ centrX)*3 + 1];
                 pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 2]= pDisplay[((centrY*_displyWidth)+ centrX)*3 + 2];
                 continue;
                }
              else if (newy<0)
              {
                PolarAngle = 1.5 * PI;
              }
              else 
              {
                PolarAngle = 0.5 * PI;
              }
            }
            else
            {
              PolarAngle = atan2((double)newy,(double)newx);
            }

            PolarAngle -= radians;
            
            double newxRotatedPos =  radius* cos(PolarAngle);
            double newyRotatedPos =  radius* sin(PolarAngle);

            double xRotatedPos = newxRotatedPos + (double)centrX;
            double yRotatedPos = (double)centrY - newyRotatedPos;
                    
            // convert polar to Cartesian
            int floorX   = (int)(floor(xRotatedPos));
            int floorY   = (int)(floor(yRotatedPos));
            int ceilX = (int)(ceil(xRotatedPos));
            int ceilY = (int)(ceil(yRotatedPos));

            // check bounds
           if (!(floorX < 0 || ceilX < 0 || floorX >= _displyWidth || ceilX >= _displyWidth || floorY < 0 || ceilY < 0 || floorY >= _displyHeight || ceilY >= _displyHeight))
           {
             //continue;

              double DeltaX = xRotatedPos - (double)floorX;
              double DeltaY = yRotatedPos - (double)floorY;

              int clrTopLeft     = ((floorY*_displyWidth)+floorX)*3;
              int clrTopRight    = ((floorY*_displyWidth)+ceilX)*3;
              int clrBottomLeft  = ((ceilY*_displyWidth)+floorX)*3;
              int clrBottomRight = ((ceilY*_displyWidth)+ceilX)*3;

             // linearly interpolate horizontally between top neighbours
              double topBlue  = (1 - DeltaX) * pDisplay[clrTopLeft+0] + DeltaX * pDisplay[clrTopRight+0];
              double topGreen = (1 - DeltaX) * pDisplay[clrTopLeft+1] + DeltaX * pDisplay[clrTopRight+1];
              double topRed   = (1 - DeltaX) * pDisplay[clrTopLeft+2] + DeltaX * pDisplay[clrTopRight+2];
             
             // linearly interpolate horizontally between bottom neighbours
              double bottomBlue   = (1 - DeltaX) *  pDisplay[clrBottomLeft+0] + DeltaX *  pDisplay[clrBottomRight+0];
              double bottomGreen  = (1 - DeltaX) *  pDisplay[clrBottomLeft+1] + DeltaX *  pDisplay[clrBottomRight+1];
              double bottomRed    = (1 - DeltaX) *  pDisplay[clrBottomLeft+2] + DeltaX *  pDisplay[clrBottomRight+2];

             // linearly interpolate vertically between top and bottom interpolated results
              int newBlue  = int(floor((1 - DeltaY) * topBlue + DeltaY * bottomBlue) + 0.5);          
              int newGreen = int(floor((1 - DeltaY) * topGreen + DeltaY * bottomGreen) + 0.5);
              int newRed   = int(floor((1 - DeltaY) * topRed + DeltaY * bottomRed) + 0.5);
              
              // make sure colour values are valid
              if (newRed   < 0)   newRed   = 0;
              if (newRed   > 255) newRed   = 255;
              if (newGreen < 0)   newGreen = 0;
              if (newGreen > 255) newGreen = 255;
              if (newBlue  < 0)   newBlue  = 0;
              if (newBlue  > 255) newBlue  = 255;

//*********** Painting the image on the canvas ************************************************************
 
              if(!((newBlue == 0xFF) && (newGreen == 0x00) && (newRed == 0x00))) //chromakeying the image
              {
                pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 0]= newBlue; 
                pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 1]= newGreen;
                pCanvas[((canvasPosy*canvas_width)+canvasPosx)*3 + 2]= newRed;               
               }
           }
         }
        }
       }
    return(1);         
}

