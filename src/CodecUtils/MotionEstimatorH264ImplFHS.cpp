/** @file

MODULE				: MotionEstimatorH264ImplFHS

TAG						: MEH264IFHS

FILE NAME			: MotionEstimatorH264ImplFHS.cpp

DESCRIPTION		: A fast hexagon diamond grid search motion estimator 
                implementation for Recommendation H.264 (03/2005) with both
                absolute error difference and square error measure. Access
                is via an IMotionEstimator interface. There are 2 mode levels
                of execution speed vs. quality. The boundary is extended to
                accomodate the selected motion range.

COPYRIGHT			: (c)CSIR 2007-2019 all rights resevered

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <cmath>
using namespace std;

#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "MotionEstimatorH264ImplFHS.h"

/*
--------------------------------------------------------------------------
  Constants. 
--------------------------------------------------------------------------
*/
/// Calc = ((16[vec dim] * 16[vec dim]) * 2.
#define MEH264IFHS_FULL_MOTION_NOISE_FLOOR				512
#define MEH264IFHS_MOTION_NOISE_FLOOR						  26 //24 //sqrt 512

/// Boundary padding past the motion vector extremes. Required for calculating
/// sub-pixel interpolations. 
#define MEH264IFHS_PADDING													3

/// At each motion vector sub search pattern the range is limited around the 
/// previous stage's centre. 
#define MEH264IFHS_LOCAL_RANGE											16

/// A threshold distortion value below which is considered a very good match.
#define MEH264IFHS_THRESHOLD_MIN                    1000

/// Search range coord offsets for 5x5 pattern search ordered from inner to outer.
#define MEH264IFHS_MOTION_5X5_POS_LENGTH 	24
MEH264IFHS_COORD MEH264IFHS_5x5Pos[MEH264IFHS_MOTION_5X5_POS_LENGTH] =
{
  { -1, 0 },{  1, 0 },{  0,-1 },{ 0,  1 },
  { -1,-1 },{  1,-1 },{ -1, 1 },{ 1,  1 },
  { 0, -2 },{ -2, 0 },{  2, 0 },{ 0,  2 },
  { -1,-2 },{ 1, -2 },{ -2,-1 },{ 2, -1 },
  { -2, 1 },{ 2,  1 },{ -1, 2 },{ 1,  2 },
  { -2,-2 },{ 2, -2 },{ -2, 2 },{ 2,  2 }
};

/// Search range coords for centre motion vectors.
#define MEH264IFHS_MOTION_SUB_POS_LENGTH 	8
MEH264IFHS_COORD MEH264IFHS_SubPos[MEH264IFHS_MOTION_SUB_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

#define MEH264IFHS_MOTION_CROSS_POS_LENGTH 	4
MEH264IFHS_COORD MEH264IFHS_CrossPos[MEH264IFHS_MOTION_CROSS_POS_LENGTH] =
{
  { -1,0 },{ 1,0 },{ 0,-1 },{ 0,1 }
};

/// Search range coord offsets for uneven hexagon pattern search ordered from most to least likely.
#define MEH264IFHS_MOTION_HEX_POS_LENGTH 	16
MEH264IFHS_COORD MEH264IFHS_HexPos[MEH264IFHS_MOTION_HEX_POS_LENGTH] =
{
  { -4, 0 },{ 4, 0 },
  { -4, 1 },{ 4, 1 },{ -4, -1 },{ 4, -1 },
  { -4, 2 },{ 4, 2 },{ -4, -2 },{ 4, -2 },
  { -2, 3 },{ 2, 3 },{ -2, -3 },{ 2, -3 },
  {  0, 4 },{ 0, -4 }
};

/// Search range coord offsets for uneven hexagon pattern search ordered from most to least likely.
#define MEH264IFHS_MOTION_EXTHEX_POS_LENGTH 	6
MEH264IFHS_COORD MEH264IFHS_ExtHexPos[MEH264IFHS_MOTION_EXTHEX_POS_LENGTH] =
{
  { -2, 0 },{ -1, -2 },{ 1, -2 },{ 2, 0 },{ 1, 2 },{ -1, 2 }
};

const MEH264IFHS_COORD MotionEstimatorH264ImplFHS::MEH264IFHS_OptimalPath[] = 
{
  {  5,10 },{ 13, 5 },{  2, 1 },{ 13,14 },{  1,14 },{  9, 1 },{  1, 6 },{ 15, 9 },{  8,13 },{ 14, 2 },{  7, 4 },{ 10, 8 },{  4,15 },{  0,10 },{  0, 3 },{  5, 0 },
  { 12,11 },{  4, 7 },{ 12, 0 },{ 10,14 },{  3,12 },{ 10, 5 },{ 15,13 },{  4, 3 },{  7, 8 },{ 14, 7 },{  6,13 },{ 11, 3 },{  2, 9 },{  9,10 },{ 15, 1 },{  7, 2 },
  {  3, 5 },{  8,15 },{  1,12 },{  8, 6 },{  0, 1 },{ 12, 9 },{ 14,15 },{ 15, 4 },{ 11,12 },{  6, 6 },{  7,11 },{  0, 7 },{ 14,11 },{  7, 0 },{  2,15 },{ 12, 7 },
  {  2, 3 },{  4, 9 },{ 12, 2 },{  9, 3 },{ 12,13 },{  3, 0 },{  5,14 },{  5, 4 },{  0,13 },{  8, 9 },{  2,11 },{ 11,15 },{ 10, 1 },{  2, 7 },{ 15, 6 },{  9,12 },
  {  5, 2 },{  5,12 },{  1, 4 },{ 11, 5 },{ 13, 8 },{  9, 7 },{ 13, 3 },{  5, 8 },{ 10,11 },{  6,15 },{ 13,10 },{  8, 4 },{  3,10 },{ 13, 1 },{  1, 8 },{ 14, 0 },
  {  3, 2 },{ 14,12 },{  6, 5 },{  3,13 },{  0,15 },{  7,10 },{ 11, 6 },{  6, 1 },{  4, 6 },{  1, 1 },{ 15,14 },{ 11, 9 },{  7,14 },{  8, 2 },{ 14, 4 },{  4,11 },
  {  1, 5 },{ 10,13 },{  6, 7 },{ 15,10 },{ 11, 1 },{  0, 9 },{  9, 5 },{  4, 1 },{ 13, 6 },{ 12,15 },{  3, 4 },{  6,11 },{  3,14 },{ 10, 3 },{  9, 9 },{  9,14 },
  {  6, 3 },{ 12, 4 },{  0, 5 },{ 14, 8 },{  1,10 },{  1, 2 },{  2, 8 },{  2,12 },{  2, 6 },{  0,12 },{  3, 7 },{  8,11 },{  3, 8 },{  6, 9 },{  9, 0 },{  1, 0 },
  { 15, 3 },{ 13,12 },{ 11, 8 },{  8, 7 },{  5, 5 },{  5,13 },{ 11, 0 },{ 10,10 },{ 15, 7 },{ 11,14 },{  7,12 },{  7, 5 },{ 14,13 },{ 10, 6 },{ 15, 0 },{  9,15 },
  {  7, 1 },{ 12,10 },{  2,14 },{  4, 2 },{ 12, 5 },{ 10, 2 },{  4,13 },{  8, 8 },{ 15,15 },{ 14, 5 },{  4, 0 },{ 14, 9 },{ 13, 2 },{  5, 7 },{  8, 3 },{  0, 0 },
  {  2, 4 },{  7,15 },{ 11,11 },{  0,11 },{  6,10 },{  8, 0 },{  4, 5 },{  3,15 },{ 10, 4 },{ 15,11 },{  8,12 },{ 13,13 },{  0, 4 },{  4,10 },{ 11, 7 },{  5, 3 },
  {  1,13 },{  7, 7 },{ 14, 3 },{  2, 2 },{ 10,12 },{  0, 8 },{ 12, 1 },{  5,15 },{ 13, 7 },{ 12,14 },{  3, 9 },{  9, 6 },{  6, 2 },{  7,13 },{ 13,11 },{  1,15 },
  {  5,11 },{  9, 8 },{ 15, 5 },{  6, 0 },{ 12, 3 },{  0, 6 },{  2,10 },{ 13, 0 },{ 13,15 },{  3, 3 },{  9, 4 },{ 15, 8 },{  5, 9 },{  2, 0 },{ 10, 9 },{ 15, 2 },
  { 15,12 },{  8,14 },{  7, 6 },{  0,14 },{ 14,14 },{  0, 2 },{ 10, 0 },{  4,12 },{  3, 6 },{ 12, 8 },{  6, 4 },{ 10,15 },{  9,11 },{  8, 1 },{  1,11 },{  6, 8 },
  { 11, 4 },{ 11,13 },{  4,14 },{  1, 7 },{ 14,10 },{  3, 1 },{  8,10 },{ 14, 6 },{  6,12 },{ 11, 2 },{  1, 3 },{ 14, 1 },{  5, 6 },{  7, 3 },{  2,13 },{ 13, 9 },
  {  1, 9 },{  4, 4 },{  9, 2 },{ 12,12 },{  6,14 },{  5, 1 },{ 13, 4 },{  2, 5 },{  3,11 },{  9,13 },{ 12, 6 },{  4, 8 },{  8, 5 },{ 11,10 },{  7, 9 },{ 10, 7 }
};

const MEH264IFHS_COORD MotionEstimatorH264ImplFHS::MEH264IFHS_LinearPath[] =
{
  {  0, 0 },{  0, 1 },{  0, 2 },{  0, 3 },{  0, 4 },{  0, 5 },{  0, 6 },{  0, 7 },{  0, 8 },{  0, 9 },{  0,10 },{  0,11 },{  0,12 },{ 0,13 },{ 0,14 },{ 0,15 }, ///< 0-15
  {  1, 0 },{  1, 1 },{  1, 2 },{  1, 3 },{  1, 4 },{  1, 5 },{  1, 6 },{  1, 7 },{  1, 8 },{  1, 9 },{  1,10 },{  1,11 },{  1,12 },{ 1,13 },{ 1,14 },{ 1,15 }, ///< 16-31
  {  2, 0 },{  2, 1 },{  2, 2 },{  2, 3 },{  2, 4 },{  2, 5 },{  2, 6 },{  2, 7 },{  2, 8 },{  2, 9 },{  2,10 },{  2,11 },{  2,12 },{ 2,13 },{ 2,14 },{ 2,15 }, ///< 32-47
  {  3, 0 },{  3, 1 },{  3, 2 },{  3, 3 },{  3, 4 },{  3, 5 },{  3, 6 },{  3, 7 },{  3, 8 },{  3, 9 },{  3,10 },{  3,11 },{  3,12 },{ 3,13 },{ 3,14 },{ 3,15 }, ///< 48-63
  {  4, 0 },{  4, 1 },{  4, 2 },{  4, 3 },{  4, 4 },{  4, 5 },{  4, 6 },{  4, 7 },{  4, 8 },{  4, 9 },{  4,10 },{  4,11 },{  4,12 },{ 4,13 },{ 4,14 },{ 4,15 }, ///< 64-79
  {  5, 0 },{  5, 1 },{  5, 2 },{  5, 3 },{  5, 4 },{  5, 5 },{  5, 6 },{  5, 7 },{  5, 8 },{  5, 9 },{  5,10 },{  5,11 },{ 5,12 },{ 5,13 },{ 5,14 },{ 5,15 }, ///< 80-95
  {  6, 0 },{  6, 1 },{  6, 2 },{  6, 3 },{  6, 4 },{  6, 5 },{  6, 6 },{  6, 7 },{  6, 8 },{  6, 9 },{  6,10 },{  6,11 },{ 6,12 },{ 6,13 },{ 6,14 },{ 6,15 }, ///< 96-111
  {  7, 0 },{  7, 1 },{  7, 2 },{  7, 3 },{  7, 4 },{  7, 5 },{  7, 6 },{  7, 7 },{  7, 8 },{  7, 9 },{  7,10 },{  7,11 },{ 7,12 },{ 7,13 },{ 7,14 },{ 7,15 }, ///< 112-127
  {  8, 0 },{  8, 1 },{  8, 2 },{  8, 3 },{  8, 4 },{  8, 5 },{  8, 6 },{  8, 7 },{  8, 8 },{  8, 9 },{  8,10 },{  8,11 },{ 8,12 },{ 8,13 },{ 8,14 },{ 8,15 }, ///< 128-143
  {  9, 0 },{  9, 1 },{  9, 2 },{  9, 3 },{  9, 4 },{  9, 5 },{  9, 6 },{  9, 7 },{  9, 8 },{  9, 9 },{  9,10 },{  9,11 },{ 9,12 },{ 9,13 },{ 9,14 },{ 9,15 }, ///< 144-159
  { 10, 0 },{ 10, 1 },{ 10, 2 },{ 10, 3 },{ 10, 4 },{ 10, 5 },{ 10, 6 },{ 10, 7 },{ 10, 8 },{ 10, 9 },{ 10,10 },{ 10,11 },{ 10,12 },{ 10,13 },{ 10,14 },{ 10,15 }, ///< 160-175
  { 11, 0 },{ 11, 1 },{ 11, 2 },{ 11, 3 },{ 11, 4 },{ 11, 5 },{ 11, 6 },{ 11, 7 },{ 11, 8 },{ 11, 9 },{ 11,10 },{ 11,11 },{ 11,12 },{ 11,13 },{ 11,14 },{ 11,15 }, ///< 176-191
  { 12, 0 },{ 12, 1 },{ 12, 2 },{ 12, 3 },{ 12, 4 },{ 12, 5 },{ 12, 6 },{ 12, 7 },{ 12, 8 },{ 12, 9 },{ 12,10 },{ 12,11 },{ 12,12 },{ 12,13 },{ 12,14 },{ 12,15 }, ///< 192-207
  { 13, 0 },{ 13, 1 },{ 13, 2 },{ 13, 3 },{ 13, 4 },{ 13, 5 },{ 13, 6 },{ 13, 7 },{ 13, 8 },{ 13, 9 },{ 13,10 },{ 13,11 },{ 13,12 },{ 13,13 },{ 13,14 },{ 13,15 }, ///< 208-223
  { 14, 0 },{ 14, 1 },{ 14, 2 },{ 14, 3 },{ 14, 4 },{ 14, 5 },{ 14, 6 },{ 14, 7 },{ 14, 8 },{ 14, 9 },{ 14,10 },{ 14,11 },{ 14,12 },{ 14,13 },{ 14,14 },{ 14,15 }, ///< 224-239
  { 15, 0 },{ 15, 1 },{ 15, 2 },{ 15, 3 },{ 15, 4 },{ 15, 5 },{ 15, 6 },{ 15, 7 },{ 15, 8 },{ 15, 9 },{ 15,10 },{ 15,11 },{ 15,12 },{ 15,13 },{ 15,14 },{ 15,15 }  ///< 240-255
};

/// Num of items in the Fifos.
#define MEH264IFHS_FIFO_LENGTH 5

const char MotionEstimatorH264ImplFHS::MEH264IFHS_QPelMap[7][7] =
{
  'e', 'f', 'g', 'd', 'e', 'f', 'g',
  'i', 'j', 'k', 'h', 'i', 'j', 'k',
  'p', 'q', 'r', 'n', 'p', 'q', 'r',
  'a', 'b', 'c', '0', 'a', 'b', 'c',
  'e', 'f', 'g', 'd', 'e', 'f', 'g',
  'i', 'j', 'k', 'h', 'i', 'j', 'k',
  'p', 'q', 'r', 'n', 'p', 'q', 'r'
};

/*
--------------------------------------------------------------------------
Macros.
--------------------------------------------------------------------------
*/
#define MEH264IFHS_CLIP255(x)	( (((x) <= 255)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:255 ) )

#define MEH264IFHS_6TAP(minus3, minus2, minus1, plus1, plus2, plus3) ( (minus3) - 5*(minus2) + 20*(minus1) + 20*(plus1) - 5*(plus2) + (plus3) )
#define MEH264IFHS_VERT_6TAP(ptr, x, y)  ( MEH264IFHS_6TAP((int)((ptr)[(y)-2][(x)]), (int)((ptr)[(y)-1][(x)]), (int)((ptr)[(y)][(x)]), (int)((ptr)[(y)+1][(x)]), (int)((ptr)[(y)+2][(x)]), (int)((ptr)[(y)+3][(x)])) )
#define MEH264IFHS_HORIZ_6TAP(ptr, x, y) ( MEH264IFHS_6TAP((int)((ptr)[(y)][(x)-2]), (int)((ptr)[(y)][(x)-1]), (int)((ptr)[(y)][(x)]), (int)((ptr)[(y)][(x)+1]), (int)((ptr)[(y)][(x)+2]), (int)((ptr)[(y)][(x)+3])) )
#define MEH264IFHS_4_HORIZ_6TAP(ptr4, x4, y4) ( MEH264IFHS_6TAP((int)((ptr4)[(y4)][(x4)-10]), (int)((ptr4)[(y4)][(x4)-6]), (int)((ptr4)[(y4)][(x4)-2]), (int)((ptr4)[(y4)][(x4)+2]), (int)((ptr4)[(y4)][(x4)+6]), (int)((ptr4)[(y4)][(x4)+10])) )

/// "j", "b" and "h" do not include clipping.
#define MEH264IFHS_GET_J(ptr, x, y) ((MEH264IFHS_6TAP(MEH264IFHS_VERT_6TAP((ptr),(x)-2,(y)), MEH264IFHS_VERT_6TAP((ptr),(x)-1,(y)), MEH264IFHS_VERT_6TAP((ptr),(x),(y)), MEH264IFHS_VERT_6TAP((ptr),(x)+1,(y)), MEH264IFHS_VERT_6TAP((ptr),(x)+2,(y)), MEH264IFHS_VERT_6TAP((ptr),(x)+3,(y))) + 512) >> 10)
#define MEH264IFHS_GET_B(ptr, x, y) ((MEH264IFHS_HORIZ_6TAP((ptr),(x),(y)) + 16) >> 5)
#define MEH264IFHS_GET_H(ptr, x, y) ((MEH264IFHS_VERT_6TAP((ptr),(x),(y)) + 16) >> 5)

#define MEH264IFHS_GET_S(ptr, x, y) (MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y)+1))) ///< "s" is a "b" in the row below.
#define MEH264IFHS_GET_M(ptr, x, y) (MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x)+1,(y)))) ///< "m" is an "h" in the next column.

#define MEH264IFHS_GET_A(ptr, x, y) ( ((int)(ptr)[(y)][(x)]   + MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_C(ptr, x, y) ( ((int)(ptr)[(y)][(x)+1] + MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_D(ptr, x, y) ( ((int)(ptr)[(y)][(x)]   + MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_N(ptr, x, y) ( ((int)(ptr)[(y)+1][(x)] + MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_F(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y))) + MEH264IFHS_CLIP255(MEH264IFHS_GET_J((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_I(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x),(y))) + MEH264IFHS_CLIP255(MEH264IFHS_GET_J((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_K(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_J((ptr),(x),(y))) + MEH264IFHS_GET_M((ptr),(x),(y)) + 1) >> 1)
#define MEH264IFHS_GET_Q(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_J((ptr),(x),(y))) + MEH264IFHS_GET_S((ptr),(x),(y)) + 1) >> 1)
#define MEH264IFHS_GET_E(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y))) + MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x),(y))) + 1) >> 1)
#define MEH264IFHS_GET_G(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_B((ptr),(x),(y))) + MEH264IFHS_GET_M((ptr),(x),(y)) + 1) >> 1)
#define MEH264IFHS_GET_P(ptr, x, y) ( (MEH264IFHS_CLIP255(MEH264IFHS_GET_H((ptr),(x),(y))) + MEH264IFHS_GET_S((ptr),(x),(y)) + 1) >> 1)
#define MEH264IFHS_GET_R(ptr, x, y) ( (MEH264IFHS_GET_M((ptr),(x),(y)) + MEH264IFHS_GET_S((ptr),(x),(y)) + 1) >> 1)

/// Quarter pel cache mapping into _QuartPelCache[mapping][][]
#define MEH264IFHS_H  0
#define MEH264IFHS_B  1
#define MEH264IFHS_J  2

#define MEH264IFHS_EUCLID_COST(x,y,xref,yref) ( (((x)-(xref))*((x)-(xref)))+(((y)-(yref))*((y)-(yref))) )
#define MEH264IFHS_EUCLID_MAG(x,y,xref,yref) ( (int)std::sqrt((((x)-(xref))*((x)-(xref)))+(((y)-(yref))*((y)-(yref)))) )

//#define MEH264IFHS_COST(d,x,y,xref,yref) ( (int)(((double)(d)/256.0) + ( 1.05*MEH264IFHS_EUCLID_MAG((x),(y),(xref),(yref)) )) )
#define MEH264IFHS_COST(d,x,y,xref,yref) ( ((d) + ( 269*MEH264IFHS_EUCLID_MAG((x),(y),(xref),(yref)) )) >> 8 )  ///< lamda = 1.05

/*
--------------------------------------------------------------------------
  Construction. 
--------------------------------------------------------------------------
*/

MotionEstimatorH264ImplFHS::MotionEstimatorH264ImplFHS(	const void*             pSrc, 
																												const void*             pRef, 
																												int					            imgWidth, 
																												int					            imgHeight,
																												int					            motionRange,
                                                        IMotionVectorPredictor* pMVPred)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= imgWidth;					///< Width of the src and ref images. 
	_imgHeight			= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth	= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight	= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange		= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput					= pSrc;
	_pRef						= pRef;
  _pMVPred        = pMVPred;

}//end constructor.

MotionEstimatorH264ImplFHS::MotionEstimatorH264ImplFHS(	const void*             pSrc, 
																												const void*             pRef, 
																												int					            imgWidth, 
																												int					            imgHeight,
																												int					            motionRange,
                                                        IMotionVectorPredictor* pMVPred,
																												void*				            pDistortionIncluded)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth							= imgWidth;					///< Width of the src and ref images. 
	_imgHeight						= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth				= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight				= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange					= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput								= pSrc;
	_pRef									= pRef;
  _pMVPred              = pMVPred;
	_pDistortionIncluded	= (bool *)pDistortionIncluded;

}//end constructor.

MotionEstimatorH264ImplFHS::MotionEstimatorH264ImplFHS( const void*             pSrc,
                                                        const void*             pRef,
                                                        int					            imgWidth,
                                                        int					            imgHeight,
                                                        int					            motionRange,
                                                        IMotionVectorPredictor* pMVPred,
                                                        void*				            pDistortionIncluded,
                                                        MacroBlockH264*         pPrevFrmMBlk)
{
  ResetMembers();

  /// Parameters must remain const for the life time of this instantiation.
  _imgWidth = imgWidth;					///< Width of the src and ref images. 
  _imgHeight = imgHeight;				///< Height of the src and ref images.
  _macroBlkWidth = 16;				  ///< Width of the motion block = 16 for H.263.
  _macroBlkHeight = 16;					///< Height of the motion block = 16 for H.263.
  _motionRange = motionRange;		///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
  _pInput = pSrc;
  _pRef = pRef;
  _pMVPred = pMVPred;
  _pDistortionIncluded = (bool *)pDistortionIncluded;
  _pPrevFrmMBlk = pPrevFrmMBlk;
}//end constructor.

void MotionEstimatorH264ImplFHS::ResetMembers(void)
{
	_ready	= 0;	///< Ready to estimate.
	_mode		= 0;	///< Default to quarterPel resolution.

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= 0;					///< Width of the src and ref images. 
	_imgHeight			= 0;					///< Height of the src and ref images.
	_macroBlkWidth	= 16;					///< Width of the motion block = 16 for H.264.
	_macroBlkHeight	= 16;					///< Height of the motion block = 16 for H.264.
	_motionRange		= 64;					///< (4x,4y) range of the motion vectors.
	_pInput					= NULL;
	_pRef						= NULL;

	/// Input mem overlay members.
	_pInOver					= NULL;			///< Input overlay with mb motion block dim.
	/// Ref mem overlay members.
	_pRefOver					= NULL;			///< Ref overlay with whole block dim.
	_pExtRef					= NULL;			///< Extended ref mem created by ExtendBoundary() call.
	_extWidth					= 0;
	_extHeight				= 0;
	_extBoundary			= 0;
	_pExtRefOver			= NULL;			///< Extended ref overlay with motion block dim.
  /// A 1/4 pel refinement cache.
  _pQuartPelBase    = NULL;
  _ppQuartPelBase   = NULL;
  _quartPelCache    = NULL;

	/// Temp working block and its overlay.
	_pMBlk						= NULL;			///< Motion block temp mem.
	_pMBlkOver				= NULL;			///< Motion block overlay of temp mem.

	/// Hold the resulting motion vectors in a byte array.
	_pMotionVectorStruct = NULL;
  /// Attached motion vector predictor on construction.
  _pMVPred             = NULL;

  /// A flag per macroblock to include it in the distortion accumulation.
	_pDistortionIncluded = NULL;

  /// Reference to encoder macroblocks from the previously encoded frame. Used for prediction.
  _pPrevFrmMBlk        = NULL;

  /// Number of locations to test for partial sums along a path.
  _pathLength = 256;

}//end ResetMembers.

MotionEstimatorH264ImplFHS::~MotionEstimatorH264ImplFHS(void)
{
	Destroy();
}//end destructor.

/*
--------------------------------------------------------------------------
  Public IMotionEstimator Interface. 
--------------------------------------------------------------------------
*/

int MotionEstimatorH264ImplFHS::Create(void)
{
	/// Clean out old mem.
	Destroy();

	/// --------------- Configure input overlays --------------------------------
	/// Put an overlay on the input image with the block size set to the mb vector 
	/// dim. This is used to access input vectors.
	_pInOver = new OverlayMem2Dv2((void *)_pInput,_imgWidth,_imgHeight,_macroBlkWidth,_macroBlkHeight);
	if(_pInOver == NULL)
	{
		Destroy();
		return(0);
	}//end _pInOver...

	/// --------------- Configure ref overlays --------------------------------
	/// Overlay the whole reference. The reference will have an extended 
	/// boundary for motion estimation and must therefore create its own mem.
	_pRefOver = new OverlayMem2Dv2((void *)_pRef, _imgWidth, _imgHeight, _imgWidth, _imgHeight);
	if(_pRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pRefOver...

	/// Create the new extended boundary ref into _pExtRef. The boundary is extended by
	/// the max dimension of the macroblock plus some padding to cater for quarter pel
  /// searches on the edges of the boundary. The mem is allocated in the method call.
	_extBoundary = _macroBlkWidth + MEH264IFHS_PADDING;
	if(_macroBlkHeight > _macroBlkWidth)
		_extBoundary = _macroBlkHeight + MEH264IFHS_PADDING;
	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRef, 
																				_imgWidth,						
																				_imgHeight, 
																				_extBoundary,	///< Extend left and right by...
																				_extBoundary,	///< Extend top and bottom by...
																				(void **)(&_pExtRef)) )	///< Created in the method and returned.
  {
		Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extWidth	 = _imgWidth + (2 * _extBoundary);
	_extHeight = _imgHeight + (2 * _extBoundary);

	/// Place an overlay on the extended boundary ref with block size set to the mb motion 
  /// vec dim.
	_pExtRefOver = new OverlayExtMem2Dv2(	_pExtRef,				///< Src description created in the ExtendBoundary() call. 
																				_extWidth, 
																				_extHeight,
																				_macroBlkWidth,	///< Block size description.
																				_macroBlkHeight,
																				_extBoundary,		///< Boundary size for both left and right.
																				_extBoundary  );
	if(_pExtRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pExtRefOver...

	/// --------------- Configure temp overlays --------------------------------
	/// Alloc some temp mem and overlay it to use for half/quarter pel motion 
  /// estimation and compensation. The block size is the same as the mem size.
	_pMBlk = new short[_macroBlkWidth * _macroBlkHeight];
	_pMBlkOver = new OverlayMem2Dv2(_pMBlk, _macroBlkWidth, _macroBlkHeight, 
																					_macroBlkWidth, _macroBlkHeight);
	if( (_pMBlk == NULL)||(_pMBlkOver == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pMBlk...

	/// --------------- Configure result ---------------------------------------
	/// The structure container for the motion vectors.
	_pMotionVectorStruct = new VectorStructList(VectorStructList::SIMPLE2D);
	if(_pMotionVectorStruct != NULL)
	{
		/// How many motion vectors will there be at the block dim.
		int numVecs = (_imgWidth/_macroBlkWidth) * (_imgHeight/_macroBlkHeight);
		if(!_pMotionVectorStruct->SetLength(numVecs))
		{
			Destroy();
			return(0);
		}//end _pMotionVectorStruct...
	}//end if _pMotionVectorStruct...
	else
  {
		Destroy();
	  return(0);
  }//end if else...

	/// --------------- Refinement Window ---------------------------------------
	/// Prepare a 1/4 pel search cache for motion estimation refinement. The 1/4 
  /// pel cache must use contiguous mem.
  _pQuartPelBase  = new int[3 * 18 * 18];
  _ppQuartPelBase = new int*[3 * 18];
  _quartPelCache  = new int**[3];
  if ((_pQuartPelBase == NULL) || (_ppQuartPelBase == NULL) || (_quartPelCache == NULL))
  {
    Destroy();
    return(0);
  }//end if !_pQuartPelBase...

  /// Load the address arrays.
  for (int i = 0; i < (3 * 18); i++)
    _ppQuartPelBase[i] = &(_pQuartPelBase[i * 18]); ///< 18 row addresses for each of the 3 caches.
  for (int i = 0; i < 3; i++)
    _quartPelCache[i] = &(_ppQuartPelBase[i * 18]); ///< 3 cache addresses for each 18th row.

  /// --------------- Measurements -------------------------------------------
#ifdef MEH264IFHS_TAKE_MEASUREMENTS
  _mtLen = 15000;
  _mtPos = 0;
  _mt.Create(9, _mtLen);
  _mt.SetTitle("Partial Path Accumulated Distortion");

  _mt.SetHeading(0, "16");
  _mt.SetHeading(1, "32");
  _mt.SetHeading(2, "48");
  _mt.SetHeading(3, "64");
  _mt.SetHeading(4, "80");
  _mt.SetHeading(5, "96");
  _mt.SetHeading(6, "112");
  _mt.SetHeading(7, "128");
  _mt.SetHeading(8, "Full");

  for (int j = 0; j < 9; j++)
    _mt.SetDataType(j, MeasurementTable::INT);
#endif

	_ready = 1;
	return(1);
}//end Create.

/** Motion estimate the source within the reference.
Do the estimation with the block sizes and image sizes defined in the implementation. 
The returned type holds the vectors. This is a telescopic cross search algorithm with 
extended boundaries with a choice of absolute difference or squared difference criteria. 
@param avgDistortion  : Return the motion compensated distortion.
@return				        : The list of motion vectors.
*/
void* MotionEstimatorH264ImplFHS::Estimate(long* avgDistortion)
{
  int		i,j,m,n;
	int		included = 0;
	long	totalDifference = 0;

	/// Set the motion vector struct storage structure.
	int		maxLength	= _pMotionVectorStruct->GetLength();
	int		vecPos		= 0;

	/// Write the ref and fill its extended boundary. The centre part of
	/// _pExtRefOver is copied from _pRefOver before filling the boundary.
	_pExtRefOver->SetOrigin(0, 0);
	_pExtRefOver->SetOverlayDim(_imgWidth, _imgHeight);
	_pExtRefOver->Write(*_pRefOver);	///< _pRefOver dimensions are always set to the whole image.
	_pExtRefOver->FillBoundaryProxy();
	_pExtRefOver->SetOverlayDim(_macroBlkWidth, _macroBlkHeight);

  /// _motionRange is in 1/4 pel units and must be converted to full pel units.
  int mRng = _motionRange / 4;  

  /// Gather the motion vector absolute differnce/square error data and choose the vector.
	/// m,n step level 0 vec dim = _macroBlkHeight, _macroBlkWidth.
  for(m = 0; m < _imgHeight; m += _macroBlkHeight)
    for (n = 0; n < _imgWidth; n += _macroBlkWidth)
    {
      int mx  = 0;	int my  = 0;  ///< Full pel grid.
      int hmx = 0;	int hmy = 0;  ///< 1/2 pel on 1/4 pel grid.
      int qmx = 0;	int qmy = 0;  ///< 1/4 pel grid.
      int rmx = 0;	int rmy = 0;  ///< Refinement motion vector centre.
      int mvx = 0;  int mvy = 0;  ///< Final 1/4 resolution mv.

      /// Depending on which img boundary we are on will limit the full search range.
      int xlRng, xrRng, yuRng, ydRng;
      /// Set the postiion of the input mb to work with.
      _pInOver->SetOrigin(n, m);

      ///--------------------------- Full pel reference point ---------------------------------------------------
      /// The predicted vector difference between the input and ref blocks is the most likely candidate 
      /// and is therefore the best initial starting point. The predicted distortion is referenced for
      /// early termination factors.
      int predX, predY;
      int predD = 0;
      _pMVPred->Get16x16Prediction(NULL, vecPos, &predX, &predY, &predD);
      int predXQuart = predX % 4;
      int predYQuart = predY % 4;
      int predX0 = predX / 4;  ///< Nearest full pel pred motion vector.
      int predY0 = predY / 4;
      int orgPredX0 = predX0;
      int orgPredY0 = predY0;

      /// Truncate the predicted mv to be within the extended bounds of the frame.
      if ((predX0 + n) > _imgWidth)
        predX0 = _macroBlkWidth;
      if ((predX0 + n) < -_macroBlkWidth)
        predX0 = -_macroBlkWidth;
      if ((predY0 + m) > _imgHeight)
        predY0 = _macroBlkHeight;
      if ((predY0 + m) < -_macroBlkHeight)
        predY0 = -_macroBlkHeight;
      /// If the full pel pred mv was truncated then reset the 1/4 pel offsets as they are not valid.
      if ((orgPredX0 != predX0) || (orgPredY0 != predY0)) { predXQuart = 0; predYQuart = 0; }
      int reconstructPredX = (predX0 * 4) + predXQuart;
      int reconstructPredY = (predY0 * 4) + predYQuart;

      /// From the mb [0, 0] position determine the full pel search range permitted for the mv search points.
      GetMotionRange(n, m, 0, 0, &xlRng, &xrRng, &yuRng, &ydRng, mRng);

      /// --------------- Initial predicted 1/4 pel mv ------------------------------------------------------
      /// Search on the predicted 1/4 pel mv closest to the predicted full pel mv point. 
      _pExtRefOver->SetOrigin(n + predX0, m + predY0);    ///< (predX, predY)

      /// Only do 1/4 pel pred mv if required to.
      int predVecDiff;
      if (predXQuart || predYQuart)
      {
        /// Read the quarter grid pels into temp.
        QuarterRead(_pMBlkOver, _pExtRefOver, predXQuart, predYQuart);
//        _pExtRefOver->QuarterRead(*_pMBlkOver, predXQuart, predYQuart);
        /// Absolute/square diff comparison method.
#ifdef USE_ABSOLUTE_DIFFERENCE
        predVecDiff = _pInOver->Tad16x16(*_pMBlkOver);
#else
        predVecDiff = _pInOver->Tsd16x16(*_pMBlkOver);
#endif
      }//end if predXQuart...
      else
      {
#ifdef USE_ABSOLUTE_DIFFERENCE
        predVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
        predVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif
      }//end else...

      /// Default the best mv to the nearest predicted full pel mv but the cost is from the pred 1/4 pel mv.
      int minDiff = predVecDiff; mx = predX0; my = predY0;
      int minCost = predVecDiff / 256;

      int  priorMinDiff = 0; 
      if(predVecDiff < MEH264IFHS_THRESHOLD_MIN)  ///< Early exit test.
      { mvx = reconstructPredX; mvy = reconstructPredY; goto MEH264IFHS_ALL_DONE; }

      /// --------------- Zero 1/4 pel mv ------------------------------------------------------
      /// Search on the zero full pel mv point that is also the zero 1/4 pel mv if the pred mv is 
      /// not also the zero mv.
      if (reconstructPredX || reconstructPredY)
      {
        _pExtRefOver->SetOrigin(n, m);                     ///< (0, 0)
#ifdef USE_ABSOLUTE_DIFFERENCE
        //int zeroVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
        //int zeroVecDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
        //int zeroVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
        //int zeroVecDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
        //int zeroVecDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, minDiff);
#endif

        int zeroVecDiff = Td16x16OptimalPathLessThan(_pInOver->Get2DSrcPtr(), _pInOver->GetOriginX(), _pInOver->GetOriginY(),
                                                     _pExtRefOver->Get2DSrcPtr(), _pExtRefOver->GetOriginX(), _pExtRefOver->GetOriginY(), 
                                                     minDiff);
        /// Select the best starting point full pel motion vector.
        if (zeroVecDiff < minDiff)
        {
          int zeroCost = MEH264IFHS_COST(zeroVecDiff, 0, 0, predX0, predY0);
          if (zeroCost < minCost) { minDiff = zeroVecDiff; minCost = zeroCost; mx = 0; my = 0; }
          if (zeroVecDiff < MEH264IFHS_THRESHOLD_MIN) { mvx = 0; mvy = 0; goto MEH264IFHS_ALL_DONE; } ///< Early exit test.
        }//end if zeroVecDiff...
      }//end if reconstructPredX...

      /// --------------- Previous frame 1/4 pel mv ---------------------------------------------
/*
      /// Search on the aligned mb mv in the previous frame if it is not zero or equal to the pred mv. 
      /// Ignore prev frame intra encoded mbs. 
      if (!_pPrevFrmMBlk[vecPos]._intraFlag)
      {
        int prevX = _pPrevFrmMBlk[vecPos]._mvX[0];
        int prevY = _pPrevFrmMBlk[vecPos]._mvY[0];
        if((prevX || prevY) && ((prevX != reconstructPredX) || (prevY != reconstructPredY)))
        {
          int prevX0      = prevX / 4; ///< Convert from 1/4 pel res to full pel res.
          int prevY0      = prevY / 4;
          int prevXQuart = prevX % 4; ///< 1/4 offset from full pel.
          int prevYQuart = prevY % 4;

          _pExtRefOver->SetOrigin(n + prevX0, m + prevY0);    ///< (prevX, prevY)

          /// Only do 1/4 pel if necessary.
          int prevVecDiff;
          if (prevXQuart || prevYQuart)
          {
            /// Read the quarter grid pels into temp.
            QuarterRead(_pMBlkOver, _pExtRefOver, prevXQuart, prevYQuart);
            /// Absolute/square diff comparison method.
#ifdef USE_ABSOLUTE_DIFFERENCE
            prevVecDiff = _pInOver->Tad16x16LessThan(*_pMBlkOver, minDiff);
#else
            //prevVecDiff = _pInOver->Tsd16x16(*_pMBlkOver);
            prevVecDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pMBlkOver, minDiff);
#endif
          }//end if prevXQuart...
          else
          {
#ifdef USE_ABSOLUTE_DIFFERENCE
            prevVecDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
            //prevVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
            prevVecDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, minDiff);
#endif
          }//end else...

          /// Check if this is a better full pel starting point. The mv is set to full pel but the distortion is 1/4 pel.
          int prevCost = MEH264IFHS_COST(prevVecDiff, prevX0, prevY0, predX0, predY0);
          if (prevCost < minCost) { minDiff = prevVecDiff; minCost = prevCost; mx = prevX0; my = prevY0; }
          if (prevVecDiff < MEH264IFHS_THRESHOLD_MIN) { mvx = prevX; mvy = prevY; goto MEH264IFHS_ALL_DONE; } ///< Early exit test.
        }//end if prevX...

      }//end if !_intraFlag...
*/
    ///------------ 1st predicted mv Early Termination exit test to full pel local refinement searchs -----------
    /// Absolute thresholding used. In addition, if the predicted mv is the best initial mv and if its distortion
    /// is within 20% of the predicted distortion (i.e. the prediction is accurate) then assume the pred mv is
    /// the most likley mv and jump to the local refinement.

    if(minDiff < MEH264IFHS_THRESHOLD_MIN)  goto MEH264IFHS_EXTENDED_DIAMOND_SEARCH;
    else if ((minDiff < 4000) || ((minDiff == predVecDiff) && (minDiff < (predD * 12 / 10)) && (minDiff >(predD * 8 / 10))))
      goto MEH264IFHS_EXTENDED_HEX_SEARCH; /// Go to hexigon & diamond search

    ///--------------------------- Full pel uneven multi-hexagon grid search -------------------------------------
    /// Search for an improvement on the initial search mv with a scaled 16-point hexagon pattern.
    rmx = 0; rmy = 0;
    priorMinDiff = minDiff;

    for (int w = 1; w <= 4; w++) ///< Uneven hexagon multiplier from the centre. Limit the offset range to +/-16.
    {
      /// Next 16-point hexagon range.
      for (int x = 0; x < MEH264IFHS_MOTION_HEX_POS_LENGTH; x++)
      {
        i = w * MEH264IFHS_HexPos[x].y;
        j = w * MEH264IFHS_HexPos[x].x;
        /// Check that this offset is within the range of the frame boundaries.
        if ( ((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng) )
        {
          /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
          _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
          /// The distortion returned is a prediction of true distortion for the blk (patial path early return).
#ifdef USE_ABSOLUTE_DIFFERENCE
          //int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
          ///int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
          //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, minDiff);
          //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, minDiff);
#ifdef MEH264IFHS_TAKE_MEASUREMENTS
          if (((m == 144) && (n == 176))||((m == 176) && (n == 176)))  /// Central MB only.
          {
            if (_mtPos < _mtLen)
            {
              _mt.WriteItem(0, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 16));
              _mt.WriteItem(1, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 32));
              _mt.WriteItem(2, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 48));
              _mt.WriteItem(3, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 64));
              _mt.WriteItem(4, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 80));
              _mt.WriteItem(5, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 96));
              _mt.WriteItem(6, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 112));
              _mt.WriteItem(7, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 128));
              _mt.WriteItem(8, _mtPos, _pInOver->Tsd16x16PartialPath(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, 256));
              _mtPos++;
            }//end if _mtPos
          }//end if m...
#endif

#endif
          int blkDiff = Td16x16OptimalPathLessThan(_pInOver->Get2DSrcPtr(), _pInOver->GetOriginX(), _pInOver->GetOriginY(),
                                                   _pExtRefOver->Get2DSrcPtr(), _pExtRefOver->GetOriginX(), _pExtRefOver->GetOriginY(),
                                                   minDiff);

          if (blkDiff <= minDiff)  ///< Better partial candidate mv offset.
          {
            int lclCost = MEH264IFHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
            if (lclCost < minCost)
            { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if lclCost...
          }//end if blkDiff...
        }//if i...

      }//end for x...

      /// 4th early termination is tested after each scaled 16-point hexagon pattern.

      /// None.

    }//end for w...

    /// If there was no early exit then update centre of the best mv from the completed 5x5 and uneven hexagon searches.
    mx += rmx; my += rmy;

    ///--------------------------- Full pel local refinement small hexagon search -------------------------------------
    MEH264IFHS_EXTENDED_HEX_SEARCH:

    rmx = 0; rmy = 0;
    for (int x = 0; x < MEH264IFHS_MOTION_EXTHEX_POS_LENGTH; x++)
    {
      i = MEH264IFHS_ExtHexPos[x].y;
      j = MEH264IFHS_ExtHexPos[x].x;
      /// Check that this offset is within the range of the frame boundaries.
      if (((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng))
      {
        /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
        _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
        /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef USE_ABSOLUTE_DIFFERENCE
        int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
        //int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
        //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, minDiff); ///< max path len = 80
        //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, minDiff); ///< max path len = 80
#endif
        int blkDiff = Td16x16OptimalPathLessThan(_pInOver->Get2DSrcPtr(), _pInOver->GetOriginX(), _pInOver->GetOriginY(),
                                                 _pExtRefOver->Get2DSrcPtr(), _pExtRefOver->GetOriginX(), _pExtRefOver->GetOriginY(),
                                                 minDiff);

        if (blkDiff <= minDiff)  ///< Better partial candidate mv offset.
        {
          int lclCost = MEH264IFHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
          if (lclCost < minCost)
          { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if blkDiff...
        }//end if blkDiff...
      }//if i...
    }//end for x...
    /// Readjust the best centre mv.
    mx += rmx; my += rmy;

    ///--------------------------- Full pel local refinement small diamond search -------------------------------------
    MEH264IFHS_EXTENDED_DIAMOND_SEARCH:

    rmx = 0; rmy = 0;
    for (int x = 0; x < MEH264IFHS_MOTION_CROSS_POS_LENGTH; x++)
    {
      i = MEH264IFHS_CrossPos[x].y;
      j = MEH264IFHS_CrossPos[x].x;
      /// Check that this offset is within the range of the frame boundaries.
      if (((i + my) >= yuRng) && ((i + my) <= ydRng) && ((j + mx) >= xlRng) && ((j + mx) <= xrRng))
      {
        /// Set the block to the [j,i] offset mv from the [mx,my] mv around the [n,m] reference frame point.
        _pExtRefOver->SetOrigin(n + mx + j, m + my + i);
        /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (patial path early return).
#ifdef USE_ABSOLUTE_DIFFERENCE
        int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
        //int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
        //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, (void *)MEH264IFHS_OptimalPath, minDiff); ///< max path len = 80
        //int blkDiff = _pInOver->Tsd16x16OptimalPathLessThan(*_pExtRefOver, minDiff); ///< max path len = 80
#endif
        int blkDiff = Td16x16OptimalPathLessThan(_pInOver->Get2DSrcPtr(), _pInOver->GetOriginX(), _pInOver->GetOriginY(),
                                                 _pExtRefOver->Get2DSrcPtr(), _pExtRefOver->GetOriginX(), _pExtRefOver->GetOriginY(),
                                                 minDiff);

        if (blkDiff <= minDiff)  ///< Better partial candidate mv offset.
        {
          int lclCost = MEH264IFHS_COST(blkDiff, mx + j, my + i, predX0, predY0);
          if (lclCost < minCost)
          { minDiff = blkDiff; minCost = lclCost; rmx = j; rmy = i; }//end if blkDiff...
        }//end if blkDiff...

      }//if i...
    }//end for x...
    /// Readjust the best centre mv.
    mx += rmx; my += rmy;

		///----------------------- Quarter pel refined search ----------------------------------------
    /// Search around the min diff full pel motion vector on a 1/4 pel grid firstly on the
		/// 1/2 pel positions and then refine the winner on the 1/4 pel positions. 

    _pExtRefOver->SetOrigin(n + mx, m + my);
    mvx = mx * 4;	///< Convert to 1/4 pel units.
		mvy = my * 4;

    /// In-line estimation
    if (_mode == 0) ///< 1/4 pel estimation.
    {

      qmx = 0, qmy = 0;
      int newMin = QuarterPelEstimate(_pInOver, _pExtRefOver, minDiff, &qmx, &qmy);
      if (newMin < minDiff)
      {
        minDiff = newMin;
        mvx += qmx;
        mvy += qmy;
      }//end if newMin...
    }//end if 1/4 pel...
    else if (_mode == 1)  ///< 1/2 pel estimation.
    {
      hmx = 0, hmy = 0;
      int newMin = HalfPelEstimate(_pInOver, _pExtRefOver, minDiff, &hmx, &hmy);
      if (newMin < minDiff)
      {
        minDiff = newMin;
        mvx += 2 * hmx;
        mvy += 2 * hmy;
      }//end if newMin...
    }//end else if 1/2 pel...

		///----------------------- Quarter pel pred vector ----------------------------
    /// Test the searched best 1/4 pel mv cost against the pred 1/4 pel mv cost. Note that
    /// the weighting of the euclidian distance is 4 times greater (from 1/4 pel multiplier
    /// effect).
    if ((MEH264IFHS_COST(predVecDiff, reconstructPredX, reconstructPredY, predX0, predY0)) <= (MEH264IFHS_COST(minDiff, mvx, mvy, predX0, predY0)))
    {
      minDiff = predVecDiff;
      mvx = reconstructPredX;
      mvy = reconstructPredY;
    }//end if predVecDiff...

    /// This is a bail out point where no further searching is required but the distortion and mv coords must be set.
    MEH264IFHS_ALL_DONE:

    /// Check for inclusion in the distortion calculation.
    if (_pDistortionIncluded != NULL)
    {
      if (_pDistortionIncluded[vecPos])
      {
        included++;
        totalDifference += minDiff;
      }//end if _pDistortionIncluded...
    }//end if _pDistortionIncluded...

		/// Load the selected vector coord.
		if(vecPos < maxLength)
		{
			_pMotionVectorStruct->SetSimpleElement(vecPos, 0, mvx);
			_pMotionVectorStruct->SetSimpleElement(vecPos, 1, mvy);
      /// Set macroblock vector for future predictions.
      _pMVPred->Set16x16MotionVector(vecPos, mvx, mvy, minDiff);
			vecPos++;
		}//end if vecPos...

  }//end for m & n...

	/// In this context avg distortion is actually avg difference.
//	*avgDistortion = totalDifference/maxLength;
	if(included)	///< Prevent divide by zero error.
		*avgDistortion = totalDifference/included;
	else
		*avgDistortion = 0;
	return((void *)_pMotionVectorStruct);

}//end Estimate.

/*
--------------------------------------------------------------------------
  Private methods. 
--------------------------------------------------------------------------
*/

void MotionEstimatorH264ImplFHS::Destroy(void)
{
	_ready = 0;

#ifdef MEH264IFHS_TAKE_MEASUREMENTS
  if(_mtPos > 0)
    _mt.Save("C:/Google Drive/PC/Excel/MotionEvaluation/experiment.csv", ",", 1);
#endif

  if (_quartPelCache != NULL)
    delete _quartPelCache;
  _quartPelCache = NULL;
  if (_ppQuartPelBase != NULL)
    delete _ppQuartPelBase;
  _ppQuartPelBase = NULL;
  if (_pQuartPelBase != NULL)
    delete _pQuartPelBase;
  _pQuartPelBase = NULL;

	if(_pInOver != NULL)
		delete _pInOver;
	_pInOver = NULL;

	if(_pRefOver != NULL)
		delete _pRefOver;
	_pRefOver	= NULL;

	if(_pExtRef != NULL)
		delete[] _pExtRef;
	_pExtRef = NULL;

	if(_pExtRefOver != NULL)
		delete _pExtRefOver;
	_pExtRefOver = NULL;

	if(_pMBlk != NULL)
		delete[] _pMBlk;
	_pMBlk = NULL;

	if(_pMBlkOver != NULL)
		delete _pMBlkOver;
	_pMBlkOver = NULL;

	if(_pMotionVectorStruct != NULL)
		delete _pMotionVectorStruct;
	_pMotionVectorStruct = NULL;

}//end Destroy.

/** Test a single full pel motion vector position with offsets.
In-line code refactoring method.
*/
//minDiff = TestForBetterCandidateMotionVec(mx, my, j, i, predX0Rnd, predY0Rnd, *rmx, *rmy, minDiff);
int MotionEstimatorH264ImplFHS::TestForBetterCandidateMotionVec(int currx, int curry, int testx, int testy, int basemvx, int basemvy, int* offx, int* offy, int CurrMin)
{
  int d = CurrMin;
  int rmx = *offx;
  int rmy = *offy;

  /// If the distortion returned is NOT less than minDiff then it is not a true distortion for the blk (early return).
#ifdef USE_ABSOLUTE_DIFFERENCE
  int blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, d);
#else
  int blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, d);
#endif
  if (blkDiff <= d)  ///< Better candidate mv offset.
  {
    if ( (MEH264IFHS_COST(blkDiff, currx + testx, curry + testy, basemvx, basemvy)) < (MEH264IFHS_COST(d, currx + rmx, curry + rmy, basemvx, basemvy))  )
    { d = blkDiff; rmx = testx; rmy = testy; }//end if blkDiff...
  }//end if blkDiff...

  *offx = rmx; *offy = rmy;
  return(d);
}//end TestForBetterCandidateMotionVec.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The returned values are offset limits from (x,y) image coordinate.
@param x				: X coord of block.
@param y				: Y coord of block.
@param xlr			: Returned allowed left range offset from x.
@param xrr			: Returned allowed right range offset from x.
@param yur			: Returned allowed up range offset from y.
@param ydr			: Returned allowed down range offset from y.
@param range		: Desired range of motion.
@return					: none.
*/
void MotionEstimatorH264ImplFHS::GetMotionRange( int  x,			int  y, 
																								  int* xlr,		int* xrr, 
																									int* yur,		int* ydr,
																									int	 range)
{
	int boundary	= _extBoundary - MEH264IFHS_PADDING;
	int	width			= _imgWidth;
	int	height		= _imgHeight;

	if( (x - range) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -range;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + range) < width )	///< Rest of block extends into the bounday region.
		*xrr = range;
	else
		*xrr = width - x;

	if( (y - range) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -range;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + range) < height )	///< Rest of block extends into the bounday region.
		*ydr = range;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The range is further checked to ensure that the motion vector is within 
its defined max range. The returned values are offset limits from the (xpos+xoff,ypos+yoff) 
image coordinates.
@param xpos			: X coord of block in the image.
@param ypos			: Y coord of block in the image.
@param xoff			: Current offset (vector) from xpos.
@param yoff			: Current offset (vector) from ypos.
@param xlr			: Returned allowed left range offset from xpos+xoff.
@param xrr			: Returned allowed right range offset from xpos+xoff.
@param yur			: Returned allowed up range offset from ypos+yoff.
@param ydr			: Returned allowed down range offset from ypos+yoff.
@param range		: Desired range of motion.
@return					: none.
*/
void MotionEstimatorH264ImplFHS::GetMotionRange(	int  xpos,	int  ypos,
																									int	 xoff,	int  yoff,
																									int* xlr,		int* xrr, 
																									int* yur,		int* ydr,
																									int	 range)
{
	int x = xpos + xoff;
	int y = ypos + yoff;

	int xLRange, xRRange, yURange, yDRange;

	int boundary	= _extBoundary - MEH264IFHS_PADDING;
	int	width			= _imgWidth;
	int	height		= _imgHeight;
	int	vecRange	= _motionRange/4;	///< Convert 1/4 pel range to full pel units.

	/// Limit the range of the motion vector.
	if( (xoff - range) > -vecRange )
		xLRange = range;
	else
		xLRange = (vecRange-1) + xoff;
	if( (xoff + range) < vecRange )
		xRRange = range;
	else
		xRRange = (vecRange-1) - xoff;
	if( (yoff - range) > -vecRange )
		yURange = range;
	else
		yURange = (vecRange-1) + yoff;
	if( (yoff + range) < vecRange )
		yDRange = range;
	else
		yDRange = (vecRange-1) - yoff;

	if( (x - xLRange) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -xLRange;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + xRRange) < width )	///< Rest of block extends into the bounday region.
		*xrr = xRRange;
	else
		*xrr = width - x;

	if( (y - yURange) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -yURange;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + yDRange) < height )	///< Rest of block extends into the bounday region.
		*ydr = yDRange;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** A half pel 16x16 block motion estimation refinement around a full pel result. 
Search the 8 surrounding "h", "b" and "j" positions around the reference overlay
position marked by its origin. The origin is set to the full pel result prior to
entering this method. The half pel values are predicted from a linear filter defined
in the H.264 recommendation. 
@param in		: Overlay of the input image.
@param ref	: Overlay of the reference image with origin at full pel mv.
@param x		: Return the half pel x coord offset.
@param y		: Return the half pel y coord offset.
@return			: Distortion at the half pel mv.
*/
int MotionEstimatorH264ImplFHS::HalfPelEstimate(OverlayMem2Dv2* in, OverlayMem2Dv2* ref, int min, int* x, int* y)
{
  int r, c;
  int lclMin = min;
  int hx = 0; int hy = 0;
  int dminus1minus1 = 0; int dplus1minus1 = 0; int dminus1plus1 = 0; int dplus1plus1 = 0;

  short** lclRef  = ref->Get2DSrcPtr();
  int			refX    = ref->GetOriginX();
  int			refY    = ref->GetOriginY();
  short** lclIn   = in->Get2DSrcPtr();
  int			inX     = in->GetOriginX();
  int			inY     = in->GetOriginY();

  ///< "h" for (0,-1) and (0,1) half pel mv. Intersperse the distortion calc for the 2 half pel mv.
  int dminus1 = 0; int dplus1 = 0;  ///< Distortion accumulators for mv (0,-1) and (0,1), respectively.
  for(r = -1; r < 16; r++)  ///< r and c are offsets in the ref.
    for(c = 0; c < 16; c++)
    {
      int h = MEH264IFHS_CLIP255(MEH264IFHS_GET_H(lclRef, refX + c, refY + r));
      _quartPelCache[MEH264IFHS_H][r + 1][c + 1] = h;
      if ( (dminus1 <= lclMin) && (r <= 14) && (r >= -1) )  ///< (0,-1) mv with early stop. Cache rows -1 to 14, all cols 0 to 15.
        dminus1 += DISTORTION((int)lclIn[inY + r + 1][inX + c], h);
      if ( (dplus1 <= lclMin) && (r <= 15) && (r >= 0) )  ///< (0,1) mv with early stop. Cache rows 0 to 15, all cols 0 to 15.
        dplus1 += DISTORTION((int)lclIn[inY + r][inX + c], h);
    }//end for r & c...

  /// Was there a new mv winner in the "h" positions?
  if (dminus1 < lclMin) { lclMin = dminus1; hx = 0; hy = -1; }
  if (dplus1 < lclMin)  { lclMin = dplus1; hx = 0; hy = 1; }
  if (lclMin < MEH264IFHS_THRESHOLD_MIN) goto MEH264IFHS_HPE_ALL_DONE;

  /// The 'h' cache is incomplete and requires column (x,y) = (-1, -1...15) and for 'm' (16, -1...15)
  for (r = -1; r < 16; r++)  ///< r is offset in the ref.
  {
    _quartPelCache[MEH264IFHS_H][r + 1][0] = MEH264IFHS_CLIP255(MEH264IFHS_GET_H(lclRef, refX - 1, refY + r));  ///< 'h'
    _quartPelCache[MEH264IFHS_H][r + 1][17] = MEH264IFHS_CLIP255(MEH264IFHS_GET_H(lclRef, refX + 16, refY + r));  ///< 'm'
  }//end for r...

  /// Repeat the proceedure for "b" on the (-1,0) and (1,0) half pel mv.
  dminus1 = 0; dplus1 = 0;
  for (r = 0; r < 16; r++)
    for (c = -1; c < 16; c++)
    {
      int b = MEH264IFHS_CLIP255(MEH264IFHS_GET_B(lclRef, refX + c, refY + r));
      _quartPelCache[MEH264IFHS_B][r + 1][c + 1] = b;
      if ( (dminus1 <= lclMin) && (c <= 14) )  ///< (-1,0) mv with early stop. Ref offset cols -1 to 14, all rows.
        dminus1 += DISTORTION((int)lclIn[inY + r][inX + c + 1], b);
      if ( (dplus1 <= lclMin) && (c >= 0) )  ///< (1,0) mv with early stop.Ref offset cols 0 to 15, all rows.
        dplus1 += DISTORTION((int)lclIn[inY + r][inX + c], b);
    }//end for r & c...

  /// Was there a new mv winner in the "b" positions?
  if (dminus1 < lclMin) { lclMin = dminus1; hx = -1; hy = 0; }
  if (dplus1 < lclMin) { lclMin = dplus1; hx = 1; hy = 0; }
  if (lclMin < MEH264IFHS_THRESHOLD_MIN) goto MEH264IFHS_HPE_ALL_DONE;

  /// The 'b' cache is incomplete and requires row (x,y) = (-1...15, -1) and for 's' (-1...15, 16).
  for (c = -1; c < 16; c++)
  {
    _quartPelCache[MEH264IFHS_B][0][c + 1] = MEH264IFHS_CLIP255(MEH264IFHS_GET_B(lclRef, refX + c, refY - 1));  ///< 'b'
    _quartPelCache[MEH264IFHS_B][17][c + 1] = MEH264IFHS_CLIP255(MEH264IFHS_GET_B(lclRef, refX + c, refY + 16));  ///< 's'
  }//end for c...

  /// Repeat the proceedure for "j" on the (-1,-1), (1,-1), (-1,1) and (1,1) half pel mv.
  for (r = -1; r < 16; r++) 
    for (c = -1; c < 16; c++)
    {
      int j = MEH264IFHS_CLIP255(MEH264IFHS_GET_J(lclRef, refX + c, refY + r));
      _quartPelCache[MEH264IFHS_J][r + 1][c + 1] = j;
      if ( (dminus1minus1 <= lclMin) && (r <= 14) && (r >= -1) && (c >= -1) && (c <= 14) )  ///< (-1,-1) mv with early stop. Cache rows -1 to 14, cols -1 to 14.
        dminus1minus1 += DISTORTION((int)lclIn[inY + r + 1][inX + c + 1], j);
      if ((dplus1minus1 <= lclMin) && (r <= 14) && (r >= -1) && (c >= 0) && (c <= 15))  ///< (1,-1) mv with early stop. Cache rows -1 to 14, cols 0 to 15.
        dplus1minus1 += DISTORTION((int)lclIn[inY + r + 1][inX + c], j);
      if ((dminus1plus1 <= lclMin) && (r <= 15) && (r >= 0) && (c >= -1) && (c <= 14))  ///< (-1,1) mv with early stop. Cache rows 0 to 15, cols -1 to 14.
        dminus1plus1 += DISTORTION((int)lclIn[inY + r][inX + c + 1], j);
      if ((dplus1plus1 <= lclMin) && (r <= 15) && (r >= 0) && (c >= 0) && (c <= 15))  ///< (1,1) mv with early stop. Cache rows 0 to 15, cols 0 to 15.
        dplus1plus1 += DISTORTION((int)lclIn[inY + r][inX + c], j);
    }//end for r & c...

  /// Was there a new mv winner in the "j" positions?
  if (dminus1minus1 < lclMin) { lclMin = dminus1minus1; hx = -1; hy = -1; }
  if (dplus1minus1  < lclMin) { lclMin = dplus1minus1;  hx = 1;  hy = -1; }
  if (dminus1plus1  < lclMin) { lclMin = dminus1plus1;  hx = -1; hy = 1; }
  if (dplus1plus1   < lclMin) { lclMin = dplus1plus1;   hx = 1;  hy = 1; }

  MEH264IFHS_HPE_ALL_DONE:

  *x = hx; *y = hy;
  return(lclMin);
}//end HalfPelEstimate.

 /** A quarter pel 16x16 block motion estimation refinement around a full pel result.
 First, search the 8 surrounding "h", "b" and "j" positions around the reference overlay
 position marked by its origin. Then refine to a quarter pel around the winning half
 pel location. The origin is set to the full pel result prior to entering this method. 
 The half and quater pel values are predicted from a linear filter defined in the 
 H.264 recommendation.
 @param in	: Overlay of the input image.
 @param ref	: Overlay of the reference image with origin at full pel mv.
 @param x		: Return the quarter pel x coord offset.
 @param y		: Retyrn the quarter pel y coord offset.
 @return		: Distortion at the quarter pel mv.
 */
int MotionEstimatorH264ImplFHS::QuarterPelEstimate(OverlayMem2Dv2* in, OverlayMem2Dv2* ref, int min, int* x, int* y)
{
//  int r, c;
  int lclMin = min;
  int hx = 0; int hy = 0;
  int qx = 0; int qy = 0;
  short** lclRef;
  int refX;
  int refY;
  short** lclIn;
  int inX;
  int inY;

  /// 1/2 pel estimate.
  int halfMin = HalfPelEstimate(in, ref, min, &hx, &hy);
  if (halfMin < min) {  lclMin = halfMin; if(halfMin < MEH264IFHS_THRESHOLD_MIN) goto MEH264IFHS_QPE_ALL_DONE; }

  lclRef = ref->Get2DSrcPtr();
  refX = ref->GetOriginX();
  refY = ref->GetOriginY();
  lclIn = in->Get2DSrcPtr();
  inX = in->GetOriginX();
  inY = in->GetOriginY();

  /// ------------------- 1/4 Pel Search ---------------------------------------
  /// Search around the winning 1/2 pel at the 8 surrounding 1/4 locations.
  for (int sub = 0; sub < MEH264IFHS_MOTION_SUB_POS_LENGTH; sub++)
  {
    int qOffX = (2 * hx) + MEH264IFHS_SubPos[sub].x;
    int qOffY = (2 * hy) + MEH264IFHS_SubPos[sub].y;
    int lclModX = 0;  if (qOffX < 0) lclModX = - 1;
    int lclModY = 0;  if (qOffY < 0) lclModY = - 1;
    int refOffX = refX + lclModX;
    int refOffY = refY + lclModY;

    int extraX1 = 0; int extraX2 = 0;
    int extraY1 = 0; int extraY2 = 0;

    int qMin = 0;
    const char selection = MEH264IFHS_QPelMap[qOffY + 3][qOffX + 3];

    switch (selection)
    {
    case 'c':
      refOffX++;
    case 'a':
    {
      int cy2 = lclModY + 1; int cx2 = lclModX + 1;
      for (int r = 0; (r < 16) && (qMin <= qMin); r++)
      {
        int iy = inY + r; int ry = refOffY + r; int cy = cy2 + r;
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c] + _quartPelCache[MEH264IFHS_B][cy][cx2 + c] + 1) >> 1));
      }//end for r...
    }
    break;
    case 'n':
      refOffY++;
    case 'd':
    {
      int cy2 = lclModY + 1; int cx2 = lclModX + 1;
      for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
      {
        int iy = inY + r; int ry = refOffY + r; int cy = cy2 + r;
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c] + _quartPelCache[MEH264IFHS_H][cy][cx2 + c] + 1) >> 1));
      }//end for r...
    }
    break;
    case 'g':
      extraX2++;
    case 'e':
    {
      int cy12 = lclModY + 1; int cx1 = lclModX + 1; int cx2 = lclModX + 1 + extraX2;
      for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
      {
        int iy = inY + r; int cy = cy12 + r; 
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_B][cy][cx1 + c] + _quartPelCache[MEH264IFHS_H][cy][cx2 + c] + 1) >> 1));
      }//end for r...
    }
    break;
    case 'q':
      /// An 's' is a 'b' in the next row.
      extraY1++;
    case 'f':
    {
      int cy12 = lclModY + 1; int cy112 = lclModY + 1 + extraY1; int cx = lclModX + 1;
      for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
      {
        int iy = inY + r; int cy1 = cy112 + r; int cy2 = cy12 + r;
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_B][cy1][cx + c] + _quartPelCache[MEH264IFHS_J][cy2][cx + c] + 1) >> 1));
      }//end for r...
    }
    break;
    case 'k':
      /// An 'm' is an 'h' in the next col.
      extraX1++;
    case 'i':
    {
      int cy12 = lclModY + 1; int cx2 = lclModX + 1; int cx1 = lclModX + 1 + extraX1;
      for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
      {
        int iy = inY + r; int cy = cy12 + r; 
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_H][cy][cx1 + c] + _quartPelCache[MEH264IFHS_J][cy][cx2 + c] + 1) >> 1));
      }//end for r...
    }
    break;
    case 'r':
      /// An 'm' is an 'h' in the next col.
      extraX1++;
    case 'p':
    {
      /// An 's' is a 'b' in the next row.
      int cy112 = lclModY + 1; int cy212 = lclModY + 2;
      int cx2 = lclModX + 1; int cx1 = lclModX + 1 + extraX1;
      for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
      {
        int iy = inY + r; int cy1 = cy112 + r; int cy2 = cy212 + r;
        for (int c = 0; c < 16; c++)
          qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_H][cy1][cx1 + c] + _quartPelCache[MEH264IFHS_B][cy2][cx2 + c] + 1) >> 1));
      }//end for r...
    }
    break;
    }//end switch selection...

/// ----- No assimilation -------------------------------
/*
    switch(selection)
    {
    case 'a':
      {
        for (int r = 0; (r < 16) && (qMin <= qMin); r++)
        {
          int iy = inY + r; int ry = refOffY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c] + _quartPelCache[MEH264IFHS_B][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'c':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int ry = refOffY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c + 1] + _quartPelCache[MEH264IFHS_B][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'd':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int ry = refOffY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c] + _quartPelCache[MEH264IFHS_H][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'e':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_B][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_H][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'f':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_B][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_J][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'g':
      {
        /// An 'm' is an 'h' in the next col.
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_B][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_H][cy][lclModX + c + 2] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'i':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_H][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_J][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'k':
      {
        /// An 'm' is an 'h' in the next col.
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_J][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_H][cy][lclModX + c + 2] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'n':
      {
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int ry = refOffY + r + 1; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], (((int)lclRef[ry][refOffX + c] + _quartPelCache[MEH264IFHS_H][cy][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'p':
      {
        /// An 's' is a 'b' in the next row.
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_H][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_B][cy + 1][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'q':
      {
        /// An 's' is a 'b' in the next row.
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_J][cy][lclModX + c + 1] + _quartPelCache[MEH264IFHS_B][cy+1][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    case 'r':
      {
        /// Am 'm' is an 'h' in the next col and an 's' is a 'b' in the next row.
        for (int r = 0; (r < 16) && (qMin <= lclMin); r++)
        {
          int iy = inY + r; int cy = lclModY + r + 1;
          for (int c = 0; c < 16; c++)
            qMin += DISTORTION((int)lclIn[iy][inX + c], ((_quartPelCache[MEH264IFHS_H][cy][lclModX + c + 2] + _quartPelCache[MEH264IFHS_B][cy + 1][lclModX + c + 1] + 1) >> 1));
        }//end for r...
      }
      break;
    }//end switch selection...
*/
    if (qMin < lclMin) { lclMin = qMin; qx = qOffX; qy = qOffY; if (qMin < MEH264IFHS_THRESHOLD_MIN) goto MEH264IFHS_QPE_ALL_DONE; }
  }//end for sub...

  /// Termination point for early exit.
  MEH264IFHS_QPE_ALL_DONE:

  *x = qx; *y = qy;
  return(lclMin);
}//end QuarterPelEstimate.

/** Read a quarter pel 16x16 block from a reference into a destination.
The reference input origin must be set to the central full pel. The 1/4 pel offsets
are (x,y) = (-3...3,-3...3) around the full pel.
@param dst	  : Overlay of the destination block.
@param ref	  : Overlay of the reference image with origin at full pel.
@param qoffx	: Quarter pel x coord offset.
@param qoffy	: Quarter pel y coord offset.
@return		    : none.
*/
void MotionEstimatorH264ImplFHS::QuarterRead(OverlayMem2Dv2* dst, OverlayMem2Dv2* ref, int qoffx, int qoffy)
{
  short** lclRef  = ref->Get2DSrcPtr();
  int			refX    = ref->GetOriginX();
  int			refY    = ref->GetOriginY();
  short** lclDst  = dst->Get2DSrcPtr();
  int			dstX    = dst->GetOriginX();
  int			dstY    = dst->GetOriginY();

  int lclModX = 0;  if (qoffx < 0) lclModX = -1;
  int lclModY = 0;  if (qoffy < 0) lclModY = -1;
  int refOffX = refX + lclModX;
  int refOffY = refY + lclModY;

  const char selection = MEH264IFHS_QPelMap[qoffy + 3][qoffx + 3];

  switch(selection)
  {
  case 'a':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_A(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'b':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_CLIP255(MEH264IFHS_GET_B(lclRef, refOffX + c, ry)));
    }//end for r...
  }
  break;
  case 'c':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_C(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'd':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_D(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'e':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_E(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'f':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_F(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'g':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_G(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'h':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_CLIP255(MEH264IFHS_GET_H(lclRef, refOffX + c, ry)));
    }//end for r...
  }
  break;
  case 'i':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_I(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'j':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_CLIP255(MEH264IFHS_GET_J(lclRef, refOffX + c, ry)));
    }//end for r...
  }
  break;
  case 'k':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_K(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'n':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_N(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'p':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_P(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'q':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_Q(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  case 'r':
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = (short)(MEH264IFHS_GET_R(lclRef, refOffX + c, ry));
    }//end for r...
  }
  break;
  default:  ///case '0': Straight copy.
  {
    for (int r = 0; r < 16; r++)
    {
      int dy = dstY + r; int ry = refOffY + r;
      for (int c = 0; c < 16; c++)
        lclDst[dy][dstX + c] = lclRef[ry][refOffX + c];
    }//end for r...
  }
  break;
  }//end switch selection...

}//end QuarterRead.

/** Total distortion between two 16x16 blocks along an optimal path.
Localisation of the OverlayMem2Dv2 method of the same name in the hope that it improves
the speed of execution.
@param in	  : 2D ptr of the input image.
@param inx  : X position in the input image.
@param iny  : Y position in the input image.
@param ref  : 2D ptr of the reference image.
@param refx : X position in the reference image.
@param refy : Y position in the reference image.
@param min	: Current minimum difference to improve on.
@return		  : New expected minimum difference.
*/
int MotionEstimatorH264ImplFHS::Td16x16OptimalPathLessThan(short** in, int inx, int iny, short** ref, int refx, int refy, int min)
{
  //------------------- Unrolled loop version -----------------------------------
  int Dp1 = 0; int Dp2 = 0;
  //int Dp    = 0;  ///< Accumulated partial partial distortion.
  int predD = 0;  ///< Linear 'y=x' prediction of path distortion.
  //  int upperThresh2p, lowerThresh2p;

  /// Process 16 path locations at a time before an early exit check. Also early exit on less than 2% change in prediction.
  /// 0
  Dp1 += DISTORTION((int)in[iny + 10][inx + 5], (int)ref[refy + 10][refx + 5]);   ///< {  5,10 }
  Dp2 += DISTORTION((int)in[iny + 5][inx + 13], (int)ref[refy + 5][refx + 13]);   ///< { 13, 5 }
  Dp1 += DISTORTION((int)in[iny + 1][inx + 2], (int)ref[refy + 1][refx + 2]);     ///< {  2, 1 }
  Dp2 += DISTORTION((int)in[iny + 14][inx + 13], (int)ref[refy + 14][refx + 13]); ///< { 13,14 }
  Dp1 += DISTORTION((int)in[iny + 14][inx + 1], (int)ref[refy + 14][refx + 1]);   ///< {  1,14 }
  Dp2 += DISTORTION((int)in[iny + 1][inx + 9], (int)ref[refy + 1][refx + 9]);     ///< {  9, 1 }
  Dp1 += DISTORTION((int)in[iny + 6][inx + 1], (int)ref[refy + 6][refx + 1]);     ///< {  1, 6 }
  Dp2 += DISTORTION((int)in[iny + 9][inx + 15], (int)ref[refy + 9][refx + 15]);   ///< { 15, 9 }
  /// 8
  Dp1 += DISTORTION((int)in[iny + 13][inx + 8], (int)ref[refy + 13][refx + 8]);   ///< {  8,13 }
  Dp2 += DISTORTION((int)in[iny + 2][inx + 14], (int)ref[refy + 2][refx + 14]);   ///< { 14, 2 }
  Dp1 += DISTORTION((int)in[iny + 4][inx + 7], (int)ref[refy + 4][refx + 7]);     ///< {  7, 4 }
  Dp2 += DISTORTION((int)in[iny + 8][inx + 10], (int)ref[refy + 8][refx + 10]);   ///< { 10, 8 }
  Dp1 += DISTORTION((int)in[iny + 15][inx + 4], (int)ref[refy + 15][refx + 4]);   ///< {  4,15 }
  Dp2 += DISTORTION((int)in[iny + 10][inx + 0], (int)ref[refy + 10][refx + 0]);   ///< {  0,10 }
  Dp1 += DISTORTION((int)in[iny + 3][inx + 0], (int)ref[refy + 3][refx + 0]);     ///< {  0, 3 }
  Dp2 += DISTORTION((int)in[iny + 0][inx + 5], (int)ref[refy + 0][refx + 5]);     ///< {  5, 0 }
  //  predD = (Dp1+Dp2) * 16;  if (predD > min) return(predD);
  /// 16
  Dp1 += DISTORTION((int)in[iny + 11][inx + 12], (int)ref[refy + 11][refx + 12]); ///< { 12,11 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 4], (int)ref[refy + 7][refx + 4]);     ///< {  4, 7 }
  Dp1 += DISTORTION((int)in[iny + 0][inx + 12], (int)ref[refy + 0][refx + 12]);   ///< { 12, 0 }
  Dp2 += DISTORTION((int)in[iny + 14][inx + 10], (int)ref[refy + 14][refx + 10]); ///< { 10,14 }
  Dp1 += DISTORTION((int)in[iny + 12][inx + 3], (int)ref[refy + 12][refx + 3]);   ///< {  3,12 }
  Dp2 += DISTORTION((int)in[iny + 5][inx + 10], (int)ref[refy + 5][refx + 10]);   ///< { 10, 5 }
  Dp1 += DISTORTION((int)in[iny + 13][inx + 15], (int)ref[refy + 13][refx + 15]); ///< { 15,13 }
  Dp2 += DISTORTION((int)in[iny + 3][inx + 4], (int)ref[refy + 3][refx + 4]);     ///< {  4, 3 }
  //  predD = (Dp1+Dp2) * 256/24;  if (predD > min) return(predD);
  /// 24
  Dp1 += DISTORTION((int)in[iny + 8][inx + 7], (int)ref[refy + 8][refx + 7]);     ///< {  7, 8 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 14], (int)ref[refy + 7][refx + 14]);   ///< { 14, 7 }
  Dp1 += DISTORTION((int)in[iny + 13][inx + 6], (int)ref[refy + 13][refx + 6]);   ///< {  6,13 }
  Dp2 += DISTORTION((int)in[iny + 3][inx + 11], (int)ref[refy + 3][refx + 11]);   ///< { 11, 3 }
  Dp1 += DISTORTION((int)in[iny + 9][inx + 2], (int)ref[refy + 9][refx + 2]);     ///< {  2, 9 }
  Dp2 += DISTORTION((int)in[iny + 10][inx + 9], (int)ref[refy + 10][refx + 9]);   ///< {  9,10 }
  Dp1 += DISTORTION((int)in[iny + 1][inx + 15], (int)ref[refy + 1][refx + 15]);   ///< { 15, 1 }
  Dp2 += DISTORTION((int)in[iny + 2][inx + 7], (int)ref[refy + 2][refx + 7]);     ///< {  7, 2 }
  predD = (Dp1+Dp2) * 8;  if (predD > min) return(predD);
  //  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2%
  /// 32
  Dp1 += DISTORTION((int)in[iny + 5][inx + 3], (int)ref[refy + 5][refx + 3]);     ///< {  3, 5 }
  Dp2 += DISTORTION((int)in[iny + 15][inx + 8], (int)ref[refy + 15][refx + 8]);   ///< {  8,15 }
  Dp1 += DISTORTION((int)in[iny + 12][inx + 1], (int)ref[refy + 12][refx + 1]);   ///< {  1,12 }
  Dp2 += DISTORTION((int)in[iny + 6][inx + 8], (int)ref[refy + 6][refx + 8]);     ///< {  8, 6 }
  Dp1 += DISTORTION((int)in[iny + 1][inx + 0], (int)ref[refy + 1][refx + 0]);     ///< {  0, 1 }
  Dp2 += DISTORTION((int)in[iny + 9][inx + 12], (int)ref[refy + 9][refx + 12]);   ///< { 12, 9 }
  Dp1 += DISTORTION((int)in[iny + 15][inx + 14], (int)ref[refy + 15][refx + 14]); ///< { 14,15 }
  Dp2 += DISTORTION((int)in[iny + 4][inx + 15], (int)ref[refy + 4][refx + 15]);   ///< { 15, 4 }
  predD = (Dp1+Dp2) * 256 / 40;  if (predD > min) return(predD);
  /// 40
  Dp1 += DISTORTION((int)in[iny + 12][inx + 11], (int)ref[refy + 12][refx + 11]); ///< { 11,12 }
  Dp2 += DISTORTION((int)in[iny + 6][inx + 6], (int)ref[refy + 6][refx + 6]);     ///< {  6, 6 }
  Dp1 += DISTORTION((int)in[iny + 11][inx + 7], (int)ref[refy + 11][refx + 7]);   ///< {  7,11 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 0], (int)ref[refy + 7][refx + 0]);     ///< {  0, 7 }
  Dp1 += DISTORTION((int)in[iny + 11][inx + 14], (int)ref[refy + 11][refx + 14]); ///< { 14,11 }
  Dp2 += DISTORTION((int)in[iny + 0][inx + 7], (int)ref[refy + 0][refx + 7]);     ///< {  7, 0 }
  Dp1 += DISTORTION((int)in[iny + 15][inx + 2], (int)ref[refy + 15][refx + 2]);   ///< {  2,15 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 12], (int)ref[refy + 7][refx + 12]);   ///< { 12, 7 }
  //  predD = Dp * 256/48;  if ((predD > min)||( (predD < upperThresh2p)&&(predD > lowerThresh2p) )) return(predD);
  //  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2% change
  predD = (Dp1+Dp2) * 256 / 48;  if (predD > min) return(predD);
  /// 48
  Dp1 += DISTORTION((int)in[iny + 3][inx + 2], (int)ref[refy + 3][refx + 2]);     ///< {  2, 3 }
  Dp2 += DISTORTION((int)in[iny + 9][inx + 4], (int)ref[refy + 9][refx + 4]);     ///< {  4, 9 }
  Dp1 += DISTORTION((int)in[iny + 2][inx + 12], (int)ref[refy + 2][refx + 12]);   ///< { 12, 2 }
  Dp2 += DISTORTION((int)in[iny + 3][inx + 9], (int)ref[refy + 3][refx + 9]);     ///< {  9, 3 }
  Dp1 += DISTORTION((int)in[iny + 13][inx + 12], (int)ref[refy + 13][refx + 12]); ///< { 12,13 }
  Dp2 += DISTORTION((int)in[iny + 0][inx + 3], (int)ref[refy + 0][refx + 3]);     ///< {  3, 0 }
  Dp1 += DISTORTION((int)in[iny + 14][inx + 5], (int)ref[refy + 14][refx + 5]);   ///< {  5,14 }
  Dp2 += DISTORTION((int)in[iny + 4][inx + 5], (int)ref[refy + 4][refx + 5]);     ///< {  5, 4 }
  predD = (Dp1+Dp2) * 256 / 56;  if (predD > min) return(predD);
  /// 56
  Dp1 += DISTORTION((int)in[iny + 13][inx + 0], (int)ref[refy + 13][refx + 0]);   ///< {  0,13 }
  Dp2 += DISTORTION((int)in[iny + 9][inx + 8], (int)ref[refy + 9][refx + 8]);     ///< {  8, 9 }
  Dp1 += DISTORTION((int)in[iny + 11][inx + 2], (int)ref[refy + 11][refx + 2]);   ///< {  2,11 }
  Dp2 += DISTORTION((int)in[iny + 15][inx + 11], (int)ref[refy + 15][refx + 11]); ///< { 11,15 }
  Dp1 += DISTORTION((int)in[iny + 1][inx + 10], (int)ref[refy + 1][refx + 10]);   ///< { 10, 1 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 2], (int)ref[refy + 7][refx + 2]);     ///< {  2, 7 }
  Dp1 += DISTORTION((int)in[iny + 6][inx + 15], (int)ref[refy + 6][refx + 15]);   ///< { 15, 6 }
  Dp2 += DISTORTION((int)in[iny + 12][inx + 9], (int)ref[refy + 12][refx + 9]);   ///< {  9,12 }
  //  predD = Dp * 4;  if ((predD > min) || ((predD < upperThresh2p) && (predD > lowerThresh2p))) return(predD);
  //  upperThresh2p = predD + (predD / 50); lowerThresh2p = predD - (predD / 50); ///< +/- 2% change
  predD = (Dp1+Dp2) * 4;  if (predD > min) return(predD);
  /// 64
  Dp1 += DISTORTION((int)in[iny + 2][inx + 5], (int)ref[refy + 2][refx + 5]);     ///< {  5, 2 }
  Dp2 += DISTORTION((int)in[iny + 12][inx + 5], (int)ref[refy + 12][refx + 5]);   ///< {  5,12 }
  Dp1 += DISTORTION((int)in[iny + 4][inx + 1], (int)ref[refy + 4][refx + 1]);     ///< {  1, 4 }
  Dp2 += DISTORTION((int)in[iny + 5][inx + 11], (int)ref[refy + 5][refx + 11]);   ///< { 11, 5 }
  Dp1 += DISTORTION((int)in[iny + 8][inx + 13], (int)ref[refy + 8][refx + 13]);   ///< { 13, 8 }
  Dp2 += DISTORTION((int)in[iny + 7][inx + 9], (int)ref[refy + 7][refx + 9]);     ///< {  9, 7 }
  Dp1 += DISTORTION((int)in[iny + 3][inx + 13], (int)ref[refy + 3][refx + 13]);   ///< { 13, 3 }
  Dp2 += DISTORTION((int)in[iny + 8][inx + 5], (int)ref[refy + 8][refx + 5]);     ///< {  5, 8 }
  predD = (Dp1+Dp2) * 256 / 72;  if (predD > min) return(predD);
  /// 72
  Dp1 += DISTORTION((int)in[iny + 11][inx + 10], (int)ref[refy + 11][refx + 10]); ///< { 10,11 }
  Dp2 += DISTORTION((int)in[iny + 15][inx + 6], (int)ref[refy + 15][refx + 6]);   ///< {  6,15 }
  Dp1 += DISTORTION((int)in[iny + 10][inx + 13], (int)ref[refy + 10][refx + 13]); ///< { 13,10 }
  Dp2 += DISTORTION((int)in[iny + 4][inx + 8], (int)ref[refy + 4][refx + 8]);     ///< {  8, 4 }
  Dp1 += DISTORTION((int)in[iny + 10][inx + 3], (int)ref[refy + 10][refx + 3]);   ///< {  3,10 }
  Dp2 += DISTORTION((int)in[iny + 1][inx + 13], (int)ref[refy + 1][refx + 13]);   ///< { 13, 1 }
  Dp1 += DISTORTION((int)in[iny + 8][inx + 1], (int)ref[refy + 8][refx + 1]);     ///< {  1, 8 }
  Dp2 += DISTORTION((int)in[iny + 0][inx + 14], (int)ref[refy + 0][refx + 14]);   ///< { 14, 0 }
  predD = (Dp1+Dp2) * 256 / 80;  //if (predD > min) return(predD);
  /// 80
  return(predD);

  //------------------- Looped version ------------------------------------------
/*
  int Dp      = 0;  ///< Accumulated partial distortion.
  int predD   = 0;  ///< Linear 'y=x' prediction of path distortion.
  int pathPos = 0;  ///< Position along the optimal path.
  const MEH264IFHS_COORD* path = MEH264IFHS_OptimalPath;

  /// The path length is implicit and is a max of 256. 
  for (int b = 0; (b < 5)&&(predD <= min); b++)   ///< 5 batches of 16 positions = 80 path locations.
  {
    for (int p = 0; p < 4; p++) ///< 4 at a time.
    {
      int pX = (int)path[pathPos].x; int pY = (int)path[pathPos].y; pathPos++;
      Dp += DISTORTION((int)in[iny + pY][inx + pX], (int)ref[refy + pY][refx + pX]);

      int pX2 = (int)path[pathPos].x; int pY2 = (int)path[pathPos].y; pathPos++;
      Dp += DISTORTION((int)in[iny + pY2][inx + pX2], (int)ref[refy + pY2][refx + pX2]);

      pX = (int)path[pathPos].x; pY = (int)path[pathPos].y; pathPos++;
      Dp += DISTORTION((int)in[iny + pY][inx + pX], (int)ref[refy + pY][refx + pX]);

      pX2 = (int)path[pathPos].x; pY2 = (int)path[pathPos].y; pathPos++;
      Dp += DISTORTION((int)in[iny + pY2][inx + pX2], (int)ref[refy + pY2][refx + pX2]);
    }//end for p...

    /// Predict the final result for the full 256 path locations.
    predD = (Dp << 8) / pathPos;
  }//end for b...

  return(predD);
*/
}//end Td16x16OptimalPathLessThan. 



