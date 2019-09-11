/** @file

MODULE				: IMotionEstimator

TAG						: IME

FILE NAME			: IMotionEstimator.h

DESCRIPTION		: An interface to motion estimator implementations. Results
								are generated as a VectorList object.

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
#ifndef _IMOTIONESTIMATOR_H
#define _IMOTIONESTIMATOR_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IMotionEstimator
{
	public:
		virtual ~IMotionEstimator() {}
		
		/** Create any private memory required.
		This is primarily to encourage a two stage construction process where
		mem is not alloc in the implementation constructor.
		@return	:	1 = success, 0 = failure.
		*/
		virtual int Create(void) = 0;

		/** Reset any previous frame assumptions.
		But do not change any	mem variables or delete anything.
		@return	: none.
		*/
		virtual void Reset(void) = 0;

		/** Ready to estimate.
		Was the implementation correctly constructed and ready to estimate.
		@return	: 1 = yes, 0 = no.
		*/
		virtual int Ready(void) = 0;

		/** Set decision modes.
		These modes are defined by the implementation and are typically 
		compromises between speed and accuracy.
		@param mode	:	Mode to set. default (0 = auto).
		*/
		virtual void	SetMode(int mode) = 0;
		virtual int		GetMode(void) = 0;

		/** Motion estimate the source within the reference.
		Do the estimation with the block sizes and image sizes defined in
		the implementation. The returned type holds the vectors.
		@param pSrc		: Input image to estimate.
		@param pRef		: Ref to estimate with.
		@return				: The list of motion vectors.
		*/
		virtual void* Estimate(	const void* pSrc, 
														const void* pRef,
														long* avgDistortion) = 0;
		virtual void* Estimate(long* avgDistortion) = 0;
    virtual void* Estimate(long* avgDistortion, void* param) { return(Estimate(avgDistortion)); }

};//end IMotionEstimator.


#endif
