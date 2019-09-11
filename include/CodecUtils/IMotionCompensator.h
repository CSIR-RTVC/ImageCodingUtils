/** @file

MODULE						: IMotionCompensator

TAG								: IMC

FILE NAME					: IMotionCompensator.h

DESCRIPTION				: An interface to motion compensator implementations. 
										VectorList objects are compensated to a YUV420
										image ref.

REVISION HISTORY	:	25/09/2006 (KF): A single vector mode was added to
										the interface to allow one vector at a time to be 
										compensated. This requires an overload for the
										Compensate() method and a new method PrepareForSingleVectorMode()
										for dumping the ref into temp before compensating.

										29/09/2006 (KF): Removed the reference to VectorList objects
										to allow any structured object in the interface. The
										implementations will now define the list structure.

COPYRIGHT			: (c)CSIR 2007-2018 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
								modification, are permitted provided that the following conditions 
								are met:

								* Redistributions of source code must retain the above copyright notice, 
								this list of conditions and the following disclaimer.
								* Redistributions in binary form must reproduce the above copyright notice, 
								this list of conditions and the following disclaimer in the documentation 
								and/or other materials provided with the distribution.
								* Neither the name of the CSIR nor the names of its contributors may be used 
								to endorse or promote products derived from this software without specific 
								prior written permission.

								THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
								"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
								LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
								A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
								CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
								EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
								PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
								PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
								LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
								NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
								SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#ifndef _IMOTIONCOMPENSATOR_H
#define _IMOTIONCOMPENSATOR_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IMotionCompensator
{
	public:
		virtual ~IMotionCompensator() {}
		
		/** Create any private memory required.
		This is primarily to encourage a two stage construction process where
		mem is not alloc in the implementation constructor. The ref points to
		contiguous mem of Lum[imgHeight*imgWidth] followed by ChrU and then ChrV
		at half the image dimensions for YUV420 planar representation.
		@return	:	1 = success, 0 = failure.
		*/
		virtual int Create(void* ref, int imgWidth,				int imgHeight, 
																	int macroBlkWidth,	int macroBlkHeight) = 0;

		/** Destroy any private mem allocations.
		Mem is dealloc without changing the size parameters.
		@return	:	none.
		*/
		virtual void Destroy(void) = 0;

		/** Reset any previous frame assumptions.
		But do not change any	mem variables or delete anything.
		@return	: none.
		*/
		virtual void Reset(void) = 0;

		/** Motion compensate to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create().
		@param pMotionList	: The list of motion vectors.
		@return							: None.
		*/
		virtual void Compensate(void* pMotionList) = 0;

		/** Motion compensate a single vector to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create(). PrepareForSingleVectorMode()
		should be called once per compensation session before this method.
		@param tlx	: Top left x coord of block.
		@param tly	: Top left y coord of block.
		@param mvx	: X coord of the motion vector.
		@param mvy	: Y coord of the motion vector.
		@return			: None.
		*/
		virtual void Compensate(int tlx, int tly, int mvx, int mvy) = 0;
		
		/** Prepare the ref for single motion vector compensation mode.
		Should be used to copy the ref into a temp location from which to
		do the compensation to the ref. Prevents interference and double
		compensation. Use the Invalidate() command to force compensation
    on a vector by vector basis.
		@return : none.
		*/
		virtual void PrepareForSingleVectorMode(void) = 0;
    virtual void Invalidate(void) {}

};//end IMotionEstimator.


#endif
