/** @file

MODULE						: ImageDefs

TAG								: ID

FILE NAME					: ImageDefs.h

DESCRIPTION				: Describe the image definitions in a Managed C++ structure.
										This is used to interface to old unmanaged libraries.
REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#pragma once

namespace ImageDefs
{
using namespace System::Runtime::InteropServices;
//using namespace System;

#ifndef BI_RGB
	#define BI_RGB				 0
#endif
#ifndef BI_BITFIELDS
	#define BI_BITFIELDS	 3
#endif

[StructLayoutAttribute(LayoutKind::Sequential)]
public __gc class mBITMAPINFOHEADER
{
	System::UInt32		biSize; 
	System::Int32			biWidth; 
  System::Int32			biHeight; 
  System::UInt16		biPlanes; 
  System::UInt16		biBitCount; 
  System::UInt32		biCompression; 
  System::UInt32		biSizeImage; 
  System::Int32			biXPelsPerMeter; 
  System::Int32			biYPelsPerMeter; 
  System::UInt32		biClrUsed; 
  System::UInt32		biClrImportant; 
};

[StructLayoutAttribute(LayoutKind::Sequential)]
public __gc class mRGBQUAD 
{
public:
	System::Byte    rgbBlue; 
  System::Byte    rgbGreen; 
  System::Byte    rgbRed; 
  System::Byte    rgbReserved; 
};

}//end namespace ImageDefs

