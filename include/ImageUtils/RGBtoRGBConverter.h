/** @file

MODULE						: RGBtoRGBConverter

TAG								: RGBRGBC

FILE NAME					: RGBtoRGBConverter.h

DESCRIPTION				: Colour convertions to between differing RGB colour spaces.
										This class is the base class defining the minimum interface	
										and properties for all derived classes. The conversion method
										is pure virtual and must be implemented by derived classes.

REVISION HISTORY	:

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _RGBTORGBCONVERTER_H
#define _RGBTORGBCONVERTER_H
/*
===========================================================================
  Class definition.
===========================================================================
*/
class RGBtoRGBConverter
{
public:
	// Construction and destruction.
	RGBtoRGBConverter(void) {_width = 0; _height = 0; }
	RGBtoRGBConverter(int width, int height) {_width = width; _height = height;}
	virtual ~RGBtoRGBConverter(void) {}

	// Interface.
	virtual void Convert(void* pRgbIn, void* pRgbOut) = 0;

	// Member interface.
	int	GetWidth(void)		{ return(_width); }
	int	GetHeight(void)		{ return(_height); }

	void	SetDimensions(int width, int height)	{_width = width; _height = height;}

protected:
	// Members.
	int	_width;
	int	_height;

};//end RGBtoRGBConverter.

#endif	// _RGBTORGBCONVERTER_H
