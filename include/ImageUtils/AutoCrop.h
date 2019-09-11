/** @file

MODULE						: AutoCrop

TAG								: AC

FILE NAME					: AutoCrop.h

DESCRIPTION				: A class to automatically find the dark boundaries of an
										image.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2004  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#if !defined(AUTOCROP__H)
#define AUTOCROP__H

#include "VicsDefs/VicsDefs.h"

class AutoCrop  
{
public:
	AutoCrop();
	virtual ~AutoCrop();

	VICS_INT	ProcessImage(BITMAPINFOHEADER *pBmih);
	void			Reset();

	// Memeber access interface.
	VICS_INT	GetLeftMargin()		{ return(_marginLeft); }
	VICS_INT	GetRightMargin()	{ return(_marginRight); }
	VICS_INT	GetTopMargin()		{ return(_marginTop); }
	VICS_INT	GetBottomMargin()	{ return(_marginBottom); }

	void			SetMultiple(VICS_INT x) { _multiple = x; }
	VICS_INT	GetMultiple(void) { return(_multiple); }

	// Local methods.
	VICS_INT	Create(VICS_INT w,VICS_INT h);
	void			Destroy(void);
	VICS_INT	Find1stEdge(VICS_PINT32 x,VICS_INT len);
	VICS_INT	FindLastEdge(VICS_PINT32 x,VICS_INT len);

protected:
	VICS_INT	_marginLeft;
	VICS_INT	_marginRight;
	VICS_INT	_marginTop;
	VICS_INT	_marginBottom;

	VICS_INT	_multiple;

	// Local members.
	VICS_INT			_width;						// Currently held image dimensions.
	VICS_INT			_height;
	VICS_PINT32		_pLeftRightAcc;		// Length of 1 row = no. of cols.
	VICS_PINT32		_pTopBottomAcc;		// Length of 1 col = no. of rows.
};

#endif // !defined(AUTOCROP__H)
