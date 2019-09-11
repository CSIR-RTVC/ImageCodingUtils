/*******************************************************************/
/* TITLE       :H263 VIDEO COMPRESSION CODEC CLASS DEFINITION      */
/*              FILE                                               */
/* VERSION     :1.0                                                */
/* FILE        :H263Codec.cpp                                      */
/* DESCRIPTION :A class for implementing an H263 sub-set video     */
/*              compression and decompression codec.               */
/*              A RGB24/YUV411 video is coded into a bit stream    */
/*              and the bit stream is decoded into a RGB24/YUV411  */
/*              image.                                             */
/* DATE        :February 2000                                      */
/* AUTHOR      :K.L.Ferguson                                       */
/*******************************************************************/
#include "stdafx.h"

#include "H263Codec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*/////////////////////////////////////////////////////////*/
/* DCT Constants.                                          */
/*/////////////////////////////////////////////////////////*/
#define C_1 (long int)362 //Conv(0.707)
#define C_2 (long int)196 //Conv(0.383)
#define C_3 (long int)277 //Conv(0.541)
#define C_4 (long int)669 //Conv(1.307)

#define FC_1 (double)(0.707)
#define FC_2 (double)(0.383)
#define FC_3 (double)(0.541)
#define FC_4 (double)(1.307)

/*/////////////////////////////////////////////////////////*/
/* DCT Scaling Matrix Definition                           */
/*/////////////////////////////////////////////////////////*/
static double SDCT[64] =
{ 0.12500, 0.09012, 0.09567, 0.10630, 0.12500, 0.15910, 0.23097, 0.45306,
	0.09012, 0.06497, 0.06898, 0.07664, 0.09012, 0.11470, 0.16652, 0.32664,
	0.09567, 0.06898, 0.07322, 0.08136, 0.09567, 0.12177, 0.17678, 0.34676,
	0.10630, 0.07664, 0.08136, 0.09040, 0.10630, 0.13530, 0.19642, 0.38530,
	0.12500, 0.09012, 0.09567, 0.10630, 0.12500, 0.15910, 0.23097, 0.45306,
	0.15910, 0.11470, 0.12177, 0.13530, 0.15910, 0.20249, 0.29397, 0.57664,
	0.23097, 0.16652, 0.17678, 0.19642, 0.23097, 0.29397, 0.42678, 0.83715,
	0.45306, 0.32664, 0.34676, 0.38530, 0.45306, 0.57664, 0.83715, 1.64213
};

/*///////////////////////////////////////////////////////////////////*/
/* Centrally weighted Dct 8x8 and 16x16 block sequence ordering.     */
/*///////////////////////////////////////////////////////////////////*/

#define DctXBlks_176_8 22
#define DctYBlks_144_8 18
static COORD DctBlk_176x144_8x8[DctXBlks_176_8*DctYBlks_144_8] = 
{
  {10*8, 8*8},{11*8, 8*8},{11*8, 9*8},{10*8, 9*8},{ 9*8, 9*8},{ 9*8, 8*8},{ 9*8, 7*8},{10*8, 7*8},{11*8, 7*8},{12*8, 7*8},{12*8, 8*8},{12*8, 9*8},{12*8,10*8},{11*8,10*8},{10*8,10*8},{ 9*8,10*8},{ 8*8,10*8},{ 8*8, 9*8},{ 8*8, 8*8},{ 8*8, 7*8},{ 8*8, 6*8},{ 9*8, 6*8},
  {10*8, 6*8},{11*8, 6*8},{12*8, 6*8},{13*8, 6*8},{13*8, 7*8},{13*8, 8*8},{13*8, 9*8},{13*8,10*8},{13*8,11*8},{12*8,11*8},{11*8,11*8},{10*8,11*8},{ 9*8,11*8},{ 8*8,11*8},{ 7*8,11*8},{ 7*8,10*8},{ 7*8, 9*8},{ 7*8, 8*8},{ 7*8, 7*8},{ 7*8, 6*8},{ 7*8, 5*8},{ 8*8, 5*8},
  { 9*8, 5*8},{10*8, 5*8},{11*8, 5*8},{12*8, 5*8},{13*8, 5*8},{14*8, 5*8},{14*8, 6*8},{14*8, 7*8},{14*8, 8*8},{14*8, 9*8},{14*8,10*8},{14*8,11*8},{14*8,12*8},{13*8,12*8},{12*8,12*8},{11*8,12*8},{10*8,12*8},{ 9*8,12*8},{ 8*8,12*8},{ 7*8,12*8},{ 6*8,12*8},{ 6*8,11*8},
  { 6*8,10*8},{ 6*8, 9*8},{ 6*8, 8*8},{ 6*8, 7*8},{ 6*8, 6*8},{ 6*8, 5*8},{ 6*8, 4*8},{ 7*8, 4*8},{ 8*8, 4*8},{ 9*8, 4*8},{10*8, 4*8},{11*8, 4*8},{12*8, 4*8},{13*8, 4*8},{14*8, 4*8},{15*8, 4*8},{15*8, 5*8},{15*8, 6*8},{15*8, 7*8},{15*8, 8*8},{15*8, 9*8},{15*8,10*8},
  {15*8,11*8},{15*8,12*8},{15*8,13*8},{14*8,13*8},{13*8,13*8},{12*8,13*8},{11*8,13*8},{10*8,13*8},{ 9*8,13*8},{ 8*8,13*8},{ 7*8,13*8},{ 6*8,13*8},{ 5*8,13*8},{ 5*8,12*8},{ 5*8,11*8},{ 5*8,10*8},{ 5*8, 9*8},{ 5*8, 8*8},{ 5*8, 7*8},{ 5*8, 6*8},{ 5*8, 5*8},{ 5*8, 4*8},
  { 5*8, 3*8},{ 6*8, 3*8},{ 7*8, 3*8},{ 8*8, 3*8},{ 9*8, 3*8},{10*8, 3*8},{11*8, 3*8},{12*8, 3*8},{13*8, 3*8},{14*8, 3*8},{15*8, 3*8},{16*8, 3*8},{16*8, 4*8},{16*8, 5*8},{16*8, 6*8},{16*8, 7*8},{16*8, 8*8},{16*8, 9*8},{16*8,10*8},{16*8,11*8},{16*8,12*8},{16*8,13*8},
  {16*8,14*8},{15*8,14*8},{14*8,14*8},{13*8,14*8},{12*8,14*8},{11*8,14*8},{10*8,14*8},{ 9*8,14*8},{ 8*8,14*8},{ 7*8,14*8},{ 6*8,14*8},{ 5*8,14*8},{ 4*8,14*8},{ 4*8,13*8},{ 4*8,12*8},{ 4*8,11*8},{ 4*8,10*8},{ 4*8, 9*8},{ 4*8, 8*8},{ 4*8, 7*8},{ 4*8, 6*8},{ 4*8, 5*8},
  { 4*8, 4*8},{ 4*8, 3*8},{ 4*8, 2*8},{ 5*8, 2*8},{ 6*8, 2*8},{ 7*8, 2*8},{ 8*8, 2*8},{ 9*8, 2*8},{10*8, 2*8},{11*8, 2*8},{12*8, 2*8},{13*8, 2*8},{14*8, 2*8},{15*8, 2*8},{16*8, 2*8},{17*8, 2*8},{17*8, 3*8},{17*8, 4*8},{17*8, 5*8},{17*8, 6*8},{17*8, 7*8},{17*8, 8*8},
  {17*8, 9*8},{17*8,10*8},{17*8,11*8},{17*8,12*8},{17*8,13*8},{17*8,14*8},{17*8,15*8},{16*8,15*8},{15*8,15*8},{14*8,15*8},{13*8,15*8},{12*8,15*8},{11*8,15*8},{10*8,15*8},{ 9*8,15*8},{ 8*8,15*8},{ 7*8,15*8},{ 6*8,15*8},{ 5*8,15*8},{ 4*8,15*8},{ 3*8,15*8},{ 3*8,14*8},
  { 3*8,13*8},{ 3*8,12*8},{ 3*8,11*8},{ 3*8,10*8},{ 3*8, 9*8},{ 3*8, 8*8},{ 3*8, 7*8},{ 3*8, 6*8},{ 3*8, 5*8},{ 3*8, 4*8},{ 3*8, 3*8},{ 3*8, 2*8},{ 3*8, 1*8},{ 4*8, 1*8},{ 5*8, 1*8},{ 6*8, 1*8},{ 7*8, 1*8},{ 8*8, 1*8},{ 9*8, 1*8},{10*8, 1*8},{11*8, 1*8},{12*8, 1*8},
  {13*8, 1*8},{14*8, 1*8},{15*8, 1*8},{16*8, 1*8},{17*8, 1*8},{18*8, 1*8},{18*8, 2*8},{18*8, 3*8},{18*8, 4*8},{18*8, 5*8},{18*8, 6*8},{18*8, 7*8},{18*8, 8*8},{18*8, 9*8},{18*8,10*8},{18*8,11*8},{18*8,12*8},{18*8,13*8},{18*8,14*8},{18*8,15*8},{18*8,16*8},{17*8,16*8},
  {16*8,16*8},{15*8,16*8},{14*8,16*8},{13*8,16*8},{12*8,16*8},{11*8,16*8},{10*8,16*8},{ 9*8,16*8},{ 8*8,16*8},{ 7*8,16*8},{ 6*8,16*8},{ 5*8,16*8},{ 4*8,16*8},{ 3*8,16*8},{ 2*8,16*8},{ 2*8,15*8},{ 2*8,14*8},{ 2*8,13*8},{ 2*8,12*8},{ 2*8,11*8},{ 2*8,10*8},{ 2*8, 9*8},
  { 2*8, 8*8},{ 2*8, 7*8},{ 2*8, 6*8},{ 2*8, 5*8},{ 2*8, 4*8},{ 2*8, 3*8},{ 2*8, 2*8},{ 2*8, 1*8},{ 2*8, 0*8},{ 3*8, 0*8},{ 4*8, 0*8},{ 5*8, 0*8},{ 6*8, 0*8},{ 7*8, 0*8},{ 8*8, 0*8},{ 9*8, 0*8},{10*8, 0*8},{11*8, 0*8},{12*8, 0*8},{13*8, 0*8},{14*8, 0*8},{15*8, 0*8},
  {16*8, 0*8},{17*8, 0*8},{18*8, 0*8},{19*8, 0*8},{19*8, 1*8},{19*8, 2*8},{19*8, 3*8},{19*8, 4*8},{19*8, 5*8},{19*8, 6*8},{19*8, 7*8},{19*8, 8*8},{19*8, 9*8},{19*8,10*8},{19*8,11*8},{19*8,12*8},{19*8,13*8},{19*8,14*8},{19*8,15*8},{19*8,16*8},{19*8,17*8},{18*8,17*8},
  {17*8,17*8},{16*8,17*8},{15*8,17*8},{14*8,17*8},{13*8,17*8},{12*8,17*8},{11*8,17*8},{10*8,17*8},{ 9*8,17*8},{ 8*8,17*8},{ 7*8,17*8},{ 6*8,17*8},{ 5*8,17*8},{ 4*8,17*8},{ 3*8,17*8},{ 2*8,17*8},{ 1*8,17*8},{ 1*8,16*8},{ 1*8,15*8},{ 1*8,14*8},{ 1*8,13*8},{ 1*8,12*8},
  { 1*8,11*8},{ 1*8,10*8},{ 1*8, 9*8},{ 1*8, 8*8},{ 1*8, 7*8},{ 1*8, 6*8},{ 1*8, 5*8},{ 1*8, 4*8},{ 1*8, 3*8},{ 1*8, 2*8},{ 1*8, 1*8},{ 1*8, 0*8},{20*8, 0*8},{20*8, 1*8},{20*8, 2*8},{20*8, 3*8},{20*8, 4*8},{20*8, 5*8},{20*8, 6*8},{20*8, 7*8},{20*8, 8*8},{20*8, 9*8},
  {20*8,10*8},{20*8,11*8},{20*8,12*8},{20*8,13*8},{20*8,14*8},{20*8,15*8},{20*8,16*8},{20*8,17*8},{ 0*8,17*8},{ 0*8,16*8},{ 0*8,15*8},{ 0*8,14*8},{ 0*8,13*8},{ 0*8,12*8},{ 0*8,11*8},{ 0*8,10*8},{ 0*8, 9*8},{ 0*8, 8*8},{ 0*8, 7*8},{ 0*8, 6*8},{ 0*8, 5*8},{ 0*8, 4*8},
  { 0*8, 3*8},{ 0*8, 2*8},{ 0*8, 1*8},{ 0*8, 0*8},{21*8, 0*8},{21*8, 1*8},{21*8, 2*8},{21*8, 3*8},{21*8, 4*8},{21*8, 5*8},{21*8, 6*8},{21*8, 7*8},{21*8, 8*8},{21*8, 9*8},{21*8,10*8},{21*8,11*8},{21*8,12*8},{21*8,13*8},{21*8,14*8},{21*8,15*8},{21*8,16*8},{21*8,17*8}
};

/* Motion vector boundary type.*/
/* 0 = inner, 1 = top, 2 = bottom, 3 = left, 4 = right, */
/* 5 = top left corner, 6 = top right corner, */
/* 7 = bottom left corner, 8 = bottom right corner. */
static int BoundryType_176x144_8x8[DctXBlks_176_8*DctYBlks_144_8] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,2,7,3,3,3,3,3,3,3,3,3,3,3,3,3,
  3,3,3,5,6,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,8
};

#define ColourXBlks_88_4 22
#define ColourYBlks_72_4 18
static COORD ColourBlk_88x72_4x4[ColourXBlks_88_4*ColourYBlks_72_4] = 
{
  {10*4, 8*4},{11*4, 8*4},{11*4, 9*4},{10*4, 9*4},{ 9*4, 9*4},{ 9*4, 8*4},{ 9*4, 7*4},{10*4, 7*4},{11*4, 7*4},{12*4, 7*4},{12*4, 8*4},{12*4, 9*4},{12*4,10*4},{11*4,10*4},{10*4,10*4},{ 9*4,10*4},{ 8*4,10*4},{ 8*4, 9*4},{ 8*4, 8*4},{ 8*4, 7*4},{ 8*4, 6*4},{ 9*4, 6*4},
  {10*4, 6*4},{11*4, 6*4},{12*4, 6*4},{13*4, 6*4},{13*4, 7*4},{13*4, 8*4},{13*4, 9*4},{13*4,10*4},{13*4,11*4},{12*4,11*4},{11*4,11*4},{10*4,11*4},{ 9*4,11*4},{ 8*4,11*4},{ 7*4,11*4},{ 7*4,10*4},{ 7*4, 9*4},{ 7*4, 8*4},{ 7*4, 7*4},{ 7*4, 6*4},{ 7*4, 5*4},{ 8*4, 5*4},
  { 9*4, 5*4},{10*4, 5*4},{11*4, 5*4},{12*4, 5*4},{13*4, 5*4},{14*4, 5*4},{14*4, 6*4},{14*4, 7*4},{14*4, 8*4},{14*4, 9*4},{14*4,10*4},{14*4,11*4},{14*4,12*4},{13*4,12*4},{12*4,12*4},{11*4,12*4},{10*4,12*4},{ 9*4,12*4},{ 8*4,12*4},{ 7*4,12*4},{ 6*4,12*4},{ 6*4,11*4},
  { 6*4,10*4},{ 6*4, 9*4},{ 6*4, 8*4},{ 6*4, 7*4},{ 6*4, 6*4},{ 6*4, 5*4},{ 6*4, 4*4},{ 7*4, 4*4},{ 8*4, 4*4},{ 9*4, 4*4},{10*4, 4*4},{11*4, 4*4},{12*4, 4*4},{13*4, 4*4},{14*4, 4*4},{15*4, 4*4},{15*4, 5*4},{15*4, 6*4},{15*4, 7*4},{15*4, 8*4},{15*4, 9*4},{15*4,10*4},
  {15*4,11*4},{15*4,12*4},{15*4,13*4},{14*4,13*4},{13*4,13*4},{12*4,13*4},{11*4,13*4},{10*4,13*4},{ 9*4,13*4},{ 8*4,13*4},{ 7*4,13*4},{ 6*4,13*4},{ 5*4,13*4},{ 5*4,12*4},{ 5*4,11*4},{ 5*4,10*4},{ 5*4, 9*4},{ 5*4, 8*4},{ 5*4, 7*4},{ 5*4, 6*4},{ 5*4, 5*4},{ 5*4, 4*4},
  { 5*4, 3*4},{ 6*4, 3*4},{ 7*4, 3*4},{ 8*4, 3*4},{ 9*4, 3*4},{10*4, 3*4},{11*4, 3*4},{12*4, 3*4},{13*4, 3*4},{14*4, 3*4},{15*4, 3*4},{16*4, 3*4},{16*4, 4*4},{16*4, 5*4},{16*4, 6*4},{16*4, 7*4},{16*4, 8*4},{16*4, 9*4},{16*4,10*4},{16*4,11*4},{16*4,12*4},{16*4,13*4},
  {16*4,14*4},{15*4,14*4},{14*4,14*4},{13*4,14*4},{12*4,14*4},{11*4,14*4},{10*4,14*4},{ 9*4,14*4},{ 8*4,14*4},{ 7*4,14*4},{ 6*4,14*4},{ 5*4,14*4},{ 4*4,14*4},{ 4*4,13*4},{ 4*4,12*4},{ 4*4,11*4},{ 4*4,10*4},{ 4*4, 9*4},{ 4*4, 8*4},{ 4*4, 7*4},{ 4*4, 6*4},{ 4*4, 5*4},
  { 4*4, 4*4},{ 4*4, 3*4},{ 4*4, 2*4},{ 5*4, 2*4},{ 6*4, 2*4},{ 7*4, 2*4},{ 8*4, 2*4},{ 9*4, 2*4},{10*4, 2*4},{11*4, 2*4},{12*4, 2*4},{13*4, 2*4},{14*4, 2*4},{15*4, 2*4},{16*4, 2*4},{17*4, 2*4},{17*4, 3*4},{17*4, 4*4},{17*4, 5*4},{17*4, 6*4},{17*4, 7*4},{17*4, 8*4},
  {17*4, 9*4},{17*4,10*4},{17*4,11*4},{17*4,12*4},{17*4,13*4},{17*4,14*4},{17*4,15*4},{16*4,15*4},{15*4,15*4},{14*4,15*4},{13*4,15*4},{12*4,15*4},{11*4,15*4},{10*4,15*4},{ 9*4,15*4},{ 8*4,15*4},{ 7*4,15*4},{ 6*4,15*4},{ 5*4,15*4},{ 4*4,15*4},{ 3*4,15*4},{ 3*4,14*4},
  { 3*4,13*4},{ 3*4,12*4},{ 3*4,11*4},{ 3*4,10*4},{ 3*4, 9*4},{ 3*4, 8*4},{ 3*4, 7*4},{ 3*4, 6*4},{ 3*4, 5*4},{ 3*4, 4*4},{ 3*4, 3*4},{ 3*4, 2*4},{ 3*4, 1*4},{ 4*4, 1*4},{ 5*4, 1*4},{ 6*4, 1*4},{ 7*4, 1*4},{ 8*4, 1*4},{ 9*4, 1*4},{10*4, 1*4},{11*4, 1*4},{12*4, 1*4},
  {13*4, 1*4},{14*4, 1*4},{15*4, 1*4},{16*4, 1*4},{17*4, 1*4},{18*4, 1*4},{18*4, 2*4},{18*4, 3*4},{18*4, 4*4},{18*4, 5*4},{18*4, 6*4},{18*4, 7*4},{18*4, 8*4},{18*4, 9*4},{18*4,10*4},{18*4,11*4},{18*4,12*4},{18*4,13*4},{18*4,14*4},{18*4,15*4},{18*4,16*4},{17*4,16*4},
  {16*4,16*4},{15*4,16*4},{14*4,16*4},{13*4,16*4},{12*4,16*4},{11*4,16*4},{10*4,16*4},{ 9*4,16*4},{ 8*4,16*4},{ 7*4,16*4},{ 6*4,16*4},{ 5*4,16*4},{ 4*4,16*4},{ 3*4,16*4},{ 2*4,16*4},{ 2*4,15*4},{ 2*4,14*4},{ 2*4,13*4},{ 2*4,12*4},{ 2*4,11*4},{ 2*4,10*4},{ 2*4, 9*4},
  { 2*4, 8*4},{ 2*4, 7*4},{ 2*4, 6*4},{ 2*4, 5*4},{ 2*4, 4*4},{ 2*4, 3*4},{ 2*4, 2*4},{ 2*4, 1*4},{ 2*4, 0*4},{ 3*4, 0*4},{ 4*4, 0*4},{ 5*4, 0*4},{ 6*4, 0*4},{ 7*4, 0*4},{ 8*4, 0*4},{ 9*4, 0*4},{10*4, 0*4},{11*4, 0*4},{12*4, 0*4},{13*4, 0*4},{14*4, 0*4},{15*4, 0*4},
  {16*4, 0*4},{17*4, 0*4},{18*4, 0*4},{19*4, 0*4},{19*4, 1*4},{19*4, 2*4},{19*4, 3*4},{19*4, 4*4},{19*4, 5*4},{19*4, 6*4},{19*4, 7*4},{19*4, 8*4},{19*4, 9*4},{19*4,10*4},{19*4,11*4},{19*4,12*4},{19*4,13*4},{19*4,14*4},{19*4,15*4},{19*4,16*4},{19*4,17*4},{18*4,17*4},
  {17*4,17*4},{16*4,17*4},{15*4,17*4},{14*4,17*4},{13*4,17*4},{12*4,17*4},{11*4,17*4},{10*4,17*4},{ 9*4,17*4},{ 8*4,17*4},{ 7*4,17*4},{ 6*4,17*4},{ 5*4,17*4},{ 4*4,17*4},{ 3*4,17*4},{ 2*4,17*4},{ 1*4,17*4},{ 1*4,16*4},{ 1*4,15*4},{ 1*4,14*4},{ 1*4,13*4},{ 1*4,12*4},
  { 1*4,11*4},{ 1*4,10*4},{ 1*4, 9*4},{ 1*4, 8*4},{ 1*4, 7*4},{ 1*4, 6*4},{ 1*4, 5*4},{ 1*4, 4*4},{ 1*4, 3*4},{ 1*4, 2*4},{ 1*4, 1*4},{ 1*4, 0*4},{20*4, 0*4},{20*4, 1*4},{20*4, 2*4},{20*4, 3*4},{20*4, 4*4},{20*4, 5*4},{20*4, 6*4},{20*4, 7*4},{20*4, 8*4},{20*4, 9*4},
  {20*4,10*4},{20*4,11*4},{20*4,12*4},{20*4,13*4},{20*4,14*4},{20*4,15*4},{20*4,16*4},{20*4,17*4},{ 0*4,17*4},{ 0*4,16*4},{ 0*4,15*4},{ 0*4,14*4},{ 0*4,13*4},{ 0*4,12*4},{ 0*4,11*4},{ 0*4,10*4},{ 0*4, 9*4},{ 0*4, 8*4},{ 0*4, 7*4},{ 0*4, 6*4},{ 0*4, 5*4},{ 0*4, 4*4},
  { 0*4, 3*4},{ 0*4, 2*4},{ 0*4, 1*4},{ 0*4, 0*4},{21*4, 0*4},{21*4, 1*4},{21*4, 2*4},{21*4, 3*4},{21*4, 4*4},{21*4, 5*4},{21*4, 6*4},{21*4, 7*4},{21*4, 8*4},{21*4, 9*4},{21*4,10*4},{21*4,11*4},{21*4,12*4},{21*4,13*4},{21*4,14*4},{21*4,15*4},{21*4,16*4},{21*4,17*4}
};

#define DctXBlks_88_8 11
#define DctYBlks_72_8 9
static COORD DctBlk_88x72_8x8[DctXBlks_88_8*DctYBlks_72_8] = 
{
  { 5*8,4*8},{ 5*8,5*8},{ 4*8,5*8},{ 4*8,4*8},{ 4*8,3*8},{ 5*8,3*8},{ 6*8,3*8},{ 6*8,4*8},{ 6*8,5*8},
  { 6*8,6*8},{ 5*8,6*8},{ 4*8,6*8},{ 3*8,6*8},{ 3*8,5*8},{ 3*8,4*8},{ 3*8,3*8},{ 3*8,2*8},{ 4*8,2*8},
  { 5*8,2*8},{ 6*8,2*8},{ 7*8,2*8},{ 7*8,3*8},{ 7*8,4*8},{ 7*8,5*8},{ 7*8,6*8},{ 7*8,7*8},{ 6*8,7*8},
  { 5*8,7*8},{ 4*8,7*8},{ 3*8,7*8},{ 2*8,7*8},{ 2*8,6*8},{ 2*8,5*8},{ 2*8,4*8},{ 2*8,3*8},{ 2*8,2*8},
  { 2*8,1*8},{ 3*8,1*8},{ 4*8,1*8},{ 5*8,1*8},{ 6*8,1*8},{ 7*8,1*8},{ 8*8,1*8},{ 8*8,2*8},{ 8*8,3*8},
  { 8*8,4*8},{ 8*8,5*8},{ 8*8,6*8},{ 8*8,7*8},{ 8*8,8*8},{ 7*8,8*8},{ 6*8,8*8},{ 5*8,8*8},{ 4*8,8*8},
  { 3*8,8*8},{ 2*8,8*8},{ 1*8,8*8},{ 1*8,7*8},{ 1*8,6*8},{ 1*8,5*8},{ 1*8,4*8},{ 1*8,3*8},{ 1*8,2*8},
  { 1*8,1*8},{ 1*8,0*8},{ 2*8,0*8},{ 3*8,0*8},{ 4*8,0*8},{ 5*8,0*8},{ 6*8,0*8},{ 7*8,0*8},{ 8*8,0*8},
  { 9*8,0*8},{ 9*8,1*8},{ 9*8,2*8},{ 9*8,3*8},{ 9*8,4*8},{ 9*8,5*8},{ 9*8,6*8},{ 9*8,7*8},{ 9*8,8*8},
  { 0*8,8*8},{ 0*8,7*8},{ 0*8,6*8},{ 0*8,5*8},{ 0*8,4*8},{ 0*8,3*8},{ 0*8,2*8},{ 0*8,1*8},{ 0*8,0*8},
  {10*8,0*8},{10*8,1*8},{10*8,2*8},{10*8,3*8},{10*8,4*8},{10*8,5*8},{10*8,6*8},{10*8,7*8},{10*8,8*8}
};

#define MotionXBlks_176_16 11
#define MotionYBlks_144_16 9
static COORD MotionBlk_176x144_16x16[MotionXBlks_176_16*MotionYBlks_144_16] = 
{
  { 5*16,4*16},{ 5*16,5*16},{ 4*16,5*16},{ 4*16,4*16},{ 4*16,3*16},{ 5*16,3*16},{ 6*16,3*16},{ 6*16,4*16},{ 6*16,5*16},
  { 6*16,6*16},{ 5*16,6*16},{ 4*16,6*16},{ 3*16,6*16},{ 3*16,5*16},{ 3*16,4*16},{ 3*16,3*16},{ 3*16,2*16},{ 4*16,2*16},
  { 5*16,2*16},{ 6*16,2*16},{ 7*16,2*16},{ 7*16,3*16},{ 7*16,4*16},{ 7*16,5*16},{ 7*16,6*16},{ 7*16,7*16},{ 6*16,7*16},
  { 5*16,7*16},{ 4*16,7*16},{ 3*16,7*16},{ 2*16,7*16},{ 2*16,6*16},{ 2*16,5*16},{ 2*16,4*16},{ 2*16,3*16},{ 2*16,2*16},
  { 2*16,1*16},{ 3*16,1*16},{ 4*16,1*16},{ 5*16,1*16},{ 6*16,1*16},{ 7*16,1*16},{ 8*16,1*16},{ 8*16,2*16},{ 8*16,3*16},
  { 8*16,4*16},{ 8*16,5*16},{ 8*16,6*16},{ 8*16,7*16},{ 8*16,8*16},{ 7*16,8*16},{ 6*16,8*16},{ 5*16,8*16},{ 4*16,8*16},
  { 3*16,8*16},{ 2*16,8*16},{ 1*16,8*16},{ 1*16,7*16},{ 1*16,6*16},{ 1*16,5*16},{ 1*16,4*16},{ 1*16,3*16},{ 1*16,2*16},
  { 1*16,1*16},{ 1*16,0*16},{ 2*16,0*16},{ 3*16,0*16},{ 4*16,0*16},{ 5*16,0*16},{ 6*16,0*16},{ 7*16,0*16},{ 8*16,0*16},
  { 9*16,0*16},{ 9*16,1*16},{ 9*16,2*16},{ 9*16,3*16},{ 9*16,4*16},{ 9*16,5*16},{ 9*16,6*16},{ 9*16,7*16},{ 9*16,8*16},
  { 0*16,8*16},{ 0*16,7*16},{ 0*16,6*16},{ 0*16,5*16},{ 0*16,4*16},{ 0*16,3*16},{ 0*16,2*16},{ 0*16,1*16},{ 0*16,0*16},
  {10*16,0*16},{10*16,1*16},{10*16,2*16},{10*16,3*16},{10*16,4*16},{10*16,5*16},{10*16,6*16},{10*16,7*16},{10*16,8*16}
};

/* 0 = inner, 1 = top, 2 = bottom, 3 = left, 4 = right, */
/* 5 = top left corner, 6 = top right corner, */
/* 7 = bottom left corner, 8 = bottom right corner. */
static int BoundryType_176x144_16x16[MotionXBlks_176_16*MotionYBlks_144_16] = 
{
  0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,
  0,0,0,0,2,2,2,2,2,
  2,2,2,0,0,0,0,0,0,
  0,1,1,1,1,1,1,1,1,
  1,0,0,0,0,0,0,0,2,
  7,3,3,3,3,3,3,3,5,
  6,4,4,4,4,4,4,4,8
};

#define MOTION_BLK_SIZE 16

/****************************************************************/
/*  8x8 Zig-zag Matrix Definition. */
/****************************************************************/
static COORD ZZ_Order[] = 
{
  {0,0},{1,0},{0,1},{0,2},{1,1},{2,0},{3,0},{2,1},
  {1,2},{0,3},{0,4},{1,3},{2,2},{3,1},{4,0},{5,0},
  {4,1},{3,2},{2,3},{1,4},{0,5},{0,6},{1,5},{2,4},
  {3,3},{4,2},{5,1},{6,0},{7,0},{6,1},{5,2},{4,3},
  {3,4},{2,5},{1,6},{0,7},{1,7},{2,6},{3,5},{4,4},
  {5,3},{6,2},{7,1},{7,2},{6,3},{5,4},{4,5},{3,6},
  {2,7},{3,7},{4,6},{5,5},{6,4},{7,3},{7,4},{6,5},
  {5,6},{4,7},{5,7},{6,6},{7,5},{7,6},{6,7},{7,7}
};

/****************************************************************/
/* VLC definitions and tables for coding the motion vector value.*/
/****************************************************************/
#define CH263CODEC_EOV_INDEX 18
#define CH263CODEC_EOV_BITS 11 /*Includes sign bit.*/
#define CH263CODEC_EOV_CODE 0x00000140

#define CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE 19
/* The table excludes the sign bit. */
static H263_VLC_TYPE CH263CODEC_MotionVlcTable[CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE] =
{ /* <- LSB unique.   */
  { 0, 0, 0, 1, 0x00000001},   //0.      0.           */
  { 0, 0, 0, 2, 0x00000002},   //+1 -1.  +0.5 -0.5    */
  { 0, 0, 0, 3, 0x00000004},   //+2 -2.  +1   -1      */
  { 0, 0, 0, 4, 0x00000008},   //+3 -3.  +1.5 -1.5    */
  { 0, 0, 0, 6, 0x00000030},   //+4 -4.  +2   -2      */
  { 0, 0, 0, 7, 0x00000050},   //+5 -5.  +2.5 -2.5    */
  { 0, 0, 0, 7, 0x00000010},   //+6 -6.  +3   -3      */
  { 0, 0, 0, 7, 0x00000060},   //+7 -7.  +3.5 -3.5    */
  { 0, 0, 0, 9, 0x000001A0},   //+8 -8.  +4   -4      */
  { 0, 0, 0, 9, 0x000000A0},   //+9 -9.  +4.5 -4.5    */
  { 0, 0, 0, 9, 0x00000120},   //+10-10. +5   -5      */
  { 0, 0, 0,10, 0x00000220},   //+11-11. +5.5 -5.5    */
  { 0, 0, 0,10, 0x00000020},   //+12-12. +6   -6      */
  { 0, 0, 0,10, 0x000003C0},   //+13-13. +6.5 -6.5    */
  { 0, 0, 0,10, 0x000001C0},   //+14-14. +7   -7      */
  { 0, 0, 0,10, 0x000002C0},   //+15-15. +7.5 -7.5    */
  { 0, 0, 0,10, 0x000000C0},   //+16-16. +8   -8      */
  { 0, 0, 0,10, 0x00000340},   //+17-17. +8.5 -8.5    */
  { 0, 0, 0,10, 0x00000140}    // End of Vectors Marker */
};

#define CH263CODEC_ESC_BITS 7
#define CH263CODEC_ESC_CODE 0x00000060
#define CH263CODEC_ESC_RUN_BITS 9
#define CH263CODEC_ESC_RUN_MASK 0x000001FF
#define CH263CODEC_ESC_LEVEL_BITS 8
#define CH263CODEC_ESC_LEVEL_MASK 0x000000FF

#define CH263CODEC_EOP_INDEX 101
#define CH263CODEC_EOP_BITS 12
#define CH263CODEC_EOP_CODE 0x000007A0

#define CH263CODEC_EOI_INDEX 102
#define CH263CODEC_EOI_BITS 12
#define CH263CODEC_EOI_CODE 0x00000FA0

#define CH263CODEC_COEFF_VLC_TABLE_SIZE  103
/* The table excludes the sign bit. */
static H263_VLC_TYPE CH263CODEC_CoeffVlcTable[CH263CODEC_COEFF_VLC_TABLE_SIZE] =
{ /*  last,   run, level,numbits,bits <- LSB unique.   */
  {      0,     0,     1,      2, 0x00000001},
  {      0,     1,     1,      3, 0x00000003},
  {      0,     0,     2,      4, 0x0000000F},
  {      0,     2,     1,      4, 0x00000007},
  {      1,     0,     1,      4, 0x0000000E},
  {      0,     3,     1,      5, 0x00000016},
  {      0,     4,     1,      5, 0x00000006},
  {      0,     5,     1,      5, 0x0000001A},
  {      0,     0,     3,      6, 0x0000002A},
  {      0,     1,     2,      6, 0x0000000A},
  {      0,     6,     1,      6, 0x00000032},
  {      0,     7,     1,      6, 0x00000012},
  {      0,     8,     1,      6, 0x00000022},
  {      0,     9,     1,      6, 0x00000002},
  {      1,     1,     1,      6, 0x0000003C},
  {      1,     2,     1,      6, 0x0000001C},
  {      1,     3,     1,      6, 0x0000002C},
  {      1,     4,     1,      6, 0x0000000C},
  {      0,     0,     4,      7, 0x00000074},
  {      0,    10,     1,      7, 0x00000034},
  {      0,    11,     1,      7, 0x00000054},
  {      0,    12,     1,      7, 0x00000014},
  {      1,     5,     1,      7, 0x00000064},
  {      1,     6,     1,      7, 0x00000024},
  {      1,     7,     1,      7, 0x00000044},
  {      1,     8,     1,      7, 0x00000004},
  {      2,     0,     0,      7, 0x00000060}, /*ESCAPE*/
  {      0,     0,     5,      8, 0x000000F8},
  {      0,     1,     3,      8, 0x00000078},
  {      0,     2,     2,      8, 0x000000B8},
  {      0,    13,     1,      8, 0x00000038},
  {      0,    14,     1,      8, 0x000000D8},
  {      1,     9,     1,      8, 0x00000058},
  {      1,    10,     1,      8, 0x00000098},
  {      1,    11,     1,      8, 0x00000018},
  {      1,    12,     1,      8, 0x000000E8},
  {      1,    13,     1,      8, 0x00000068},
  {      1,    14,     1,      8, 0x000000A8},
  {      1,    15,     1,      8, 0x00000028},
  {      1,    16,     1,      8, 0x000000C8},
  {      0,     0,     6,      9, 0x00000148},
  {      0,     0,     7,      9, 0x00000048},
  {      0,     3,     2,      9, 0x00000188},
  {      0,     4,     2,      9, 0x00000088},
  {      0,    15,     1,      9, 0x00000108},
  {      0,    16,     1,      9, 0x00000008},
  {      0,    17,     1,      9, 0x000001F0},
  {      0,    18,     1,      9, 0x000000F0},
  {      0,    19,     1,      9, 0x00000170},
  {      0,    20,     1,      9, 0x00000070},
  {      0,    21,     1,      9, 0x000001B0},
  {      0,    22,     1,      9, 0x000000B0},
  {      1,     0,     2,      9, 0x00000130},
  {      1,    17,     1,      9, 0x00000030},
  {      1,    18,     1,      9, 0x000001D0},
  {      1,    19,     1,      9, 0x000000D0},
  {      1,    20,     1,      9, 0x00000150},
  {      1,    21,     1,      9, 0x00000050},
  {      1,    22,     1,      9, 0x00000190},
  {      1,    23,     1,      9, 0x00000090},
  {      1,    24,     1,      9, 0x00000110},
  {      0,     0,     8,     10, 0x00000210},
  {      0,     0,     9,     10, 0x00000010},
  {      0,     1,     4,     10, 0x000003C0},
  {      0,     2,     3,     10, 0x000001C0},
  {      0,     3,     3,     10, 0x000002C0},
  {      0,     5,     2,     10, 0x000000C0},
  {      0,     6,     2,     10, 0x00000340},
  {      0,     7,     2,     10, 0x00000140},
  {      0,     8,     2,     10, 0x00000240},
  {      0,     9,     2,     10, 0x00000040},
  {      1,    25,     1,     10, 0x00000380},
  {      1,    26,     1,     10, 0x00000180},
  {      1,    27,     1,     10, 0x00000280},
  {      1,    28,     1,     10, 0x00000080},
  {      0,     0,    10,     11, 0x00000700},
  {      0,     0,    11,     11, 0x00000300},
  {      0,     0,    12,     11, 0x00000020},
  {      0,     1,     5,     11, 0x00000420},
  {      0,    23,     1,     11, 0x00000220},
  {      0,    24,     1,     11, 0x00000620},
  {      1,     0,     3,     11, 0x00000500},
  {      1,     1,     2,     11, 0x00000100},
  {      1,    29,     1,     11, 0x00000120},
  {      1,    30,     1,     11, 0x00000520},
  {      1,    31,     1,     11, 0x00000320},
  {      1,    32,     1,     11, 0x00000720},
  {      0,     1,     6,     12, 0x000000A0},
  {      0,     2,     4,     12, 0x000008A0},
  {      0,     4,     3,     12, 0x000004A0},
  {      0,     5,     3,     12, 0x00000CA0},
  {      0,     6,     3,     12, 0x000002A0},
  {      0,    10,     2,     12, 0x00000AA0},
  {      0,    25,     1,     12, 0x000006A0},
  {      0,    26,     1,     12, 0x00000EA0},
  {      1,    33,     1,     12, 0x000001A0},
  {      1,    34,     1,     12, 0x000009A0},
  {      1,    35,     1,     12, 0x000005A0},
  {      1,    36,     1,     12, 0x00000DA0},
  {      1,    37,     1,     12, 0x000003A0},
  {      1,    38,     1,     12, 0x00000BA0},
  {      3,     0,     0,     12, 0x000007A0},  /* EOP Marker.*/
  {      4,     0,     0,     12, 0x00000FA0}   /* EOI Marker.*/
};

/*//////////////////////////////////////////////////////////*/
/*/ Construction and destruction.                           */
/*//////////////////////////////////////////////////////////*/

CH263CODEC::CH263CODEC()
{
  hY = NULL;
  hU = NULL;
  hV = NULL;
  ErrorStr = "No Erorr";
  BitStream = NULL;
  BitStreamSize = 0;
  CodecIsOpen = 0;
  FrameCounter = 0;

  PS.Q = 1;
  PS.Thresh = 0;
  PS.Bpp = 8;
  PS.pRGB = NULL;
  PS.RGB_X = 0;
  PS.RGB_Y = 0;
  PS.MAX_BITSTREAM_SIZE = 0;
  PS.Motion = 0;

  /*Work image.*/
  Y = NULL; 
  Y_X = 0;
  Y_Y = 0;
  U = NULL;
  V = NULL;
  UV_X = 0;
  UV_Y = 0;
  /*Reference image.*/
  hrY = NULL;
  hrU = NULL;
  hrV = NULL;
  rY = NULL;
  rU = NULL;
  rV = NULL;
  /*Compensated reference image.*/
  hcrY = NULL;
  hcrU = NULL;
  hcrV = NULL;
  crY = NULL;
  crU = NULL;
  crV = NULL;

  /*Motion vector structure.*/
  pMVD = NULL;
  hMVD = NULL;
  /*Partition structures.*/
  for(int i = 0; i < PARTITIONS; i++)
  {
    Partition[i].pCodes = NULL;
    Partition[i].hCodes = NULL;
  }/*end for i...*/

  /* 8x8 blocks.*/
  Y_XBlk = 0;
  Y_YBlk = 0;
  UV_XBlk = 0;
  UV_YBlk = 0;

}/*end constructor.*/

CH263CODEC::~CH263CODEC()
{
  /*If open then close first before exiting.*/
  if(CodecIsOpen)
    Close();
}/*end destructor.*/

/*//////////////////////////////////////////////////////////*/
/*/ Public Implementation.                                  */
/*//////////////////////////////////////////////////////////*/

int CH263CODEC::Open(CH263CODEC_STRUCT *Params)
{
  int i;

  /*If already open then close first before continuing.*/
  if(CodecIsOpen)
    Close();

  /*Update to the new parameters.*/
  PS = *Params;

  /*Create a YUV411 memory space.*/
	hY = GlobalAlloc(GMEM_FIXED,(PS.RGB_X * PS.RGB_Y * sizeof(pel)));
	if(!hY)
  {
    ErrorStr = "Luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hY...*/
	Y = (pel *)GlobalLock(hY);
  Y_X = PS.RGB_X;
  Y_Y = PS.RGB_Y;
  UV_X = PS.RGB_X/2;
  UV_Y = PS.RGB_Y/2;
	hU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hU)
  {
    ErrorStr = "U Chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hU...*/
	U = (pel *)GlobalLock(hU);
	hV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hV)
  {
    ErrorStr = "V Chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hV...*/
	V = (pel *)GlobalLock(hV);

  /*Create a reference YUV411 memory space.*/
	hrY = GlobalAlloc(GMEM_FIXED,(Y_X * Y_Y * sizeof(pel)));
	if(!hrY)
  {
    ErrorStr = "Reference luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrY...*/
	rY = (pel *)GlobalLock(hrY);
	hrU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hrU)
  {
    ErrorStr = "Reference U chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrU...*/
	rU = (pel *)GlobalLock(hrU);
	hrV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hrV)
  {
    ErrorStr = "Reference V chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hrV...*/
	rV = (pel *)GlobalLock(hrV);

  /*Create a compensated reference YUV411 memory space.*/
	hcrY = GlobalAlloc(GMEM_FIXED,(Y_X * Y_Y * sizeof(pel)));
	if(!hcrY)
  {
    ErrorStr = "Compensated reference luminance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrY...*/
	crY = (pel *)GlobalLock(hcrY);
	hcrU = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hcrU)
  {
    ErrorStr = "Compensated reference U chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrU...*/
	crU = (pel *)GlobalLock(hcrU);
	hcrV = GlobalAlloc(GMEM_FIXED,(UV_X * UV_Y * sizeof(pel)));
	if(!hcrV)
  {
    ErrorStr = "Compensated reference V chrominance memory unavailable!";
    Close();
	  return(0);
  }/*end if !hcrV...*/
	crV = (pel *)GlobalLock(hcrV);

  /*Zero the reference and the compensated reference image.*/
  int size = Y_X * Y_Y;
  for(i = 0; i < size; i++)
  {
    rY[i] = 0;
    crY[i] = 0;
  }/*end for size...*/
  size = UV_X * UV_Y;
  for(i = 0; i < size; i++)
  {
    rU[i] = 0;
    rV[i] = 0;
    crU[i] = 0;
    crV[i] = 0;
  }/*end for i...*/

  /*Construct the quantization tables.*/
	for(i = 0; i < 64; i++)
  {
    double q;
    q = 1.0 + ((double)(PS.Q)/4.0);
    if( q < 1.0 )
      q = 1.0;
		if((i == 0)/*&&(q > 8.0)*/) /*Max quant for DC term.*/
		{
			QTable[i] = Conv(SDCT[i]/((double)8.0));
			IQTable[i] = Conv(SDCT[i]*((double)8.0));
			ITable[i] = Conv(SDCT[i]);
			FQTable[i] = SDCT[i]/((double)8.0);
			FIQTable[i] = SDCT[i]*((double)8.0);
		}//end if i...
		else
		{
			QTable[i] = Conv(SDCT[i]/q);
			IQTable[i] = Conv(SDCT[i]*q);
			ITable[i] = Conv(SDCT[i]);
			FQTable[i] = SDCT[i]/q;
			FIQTable[i] = SDCT[i]*q;
		}//end else...
  }/*end for i...*/

  /* 8x8 blocks assuming div by 8.*/
  Y_XBlk = Y_X >> 3;
  Y_YBlk = Y_Y >> 3;
  UV_XBlk = UV_X >> 3;
  UV_YBlk = UV_Y >> 3;
  /* Create motion vector structure mem. Only require luminance.*/
  size = Y_XBlk * Y_YBlk * sizeof(MOTIONVECTOR_STRUCT);
	hMVD = GlobalAlloc(GMEM_FIXED,size);
	if(!hMVD)
  {
    ErrorStr = "Motion vector memory unavailable!";
    Close();
	  return(0);
  }/*end if !hMVD...*/
	pMVD = (MOTIONVECTOR_STRUCT *)GlobalLock(hMVD);

  /*Create 8x8 block structure mem. */
  if(!CreatePartitions())
  {
    Close();
	  return(0);
  }/*end if !CreatePartitions...*/

  ErrorStr = "No Erorr";
  CodecIsOpen = 1;
  FrameCounter = 0;
  return(1);
}/*end Open.*/

int CH263CODEC::Code(void *pI,void *pCmp,unsigned int FrameBitLimit)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/
  if(FrameBitLimit > PS.MAX_BITSTREAM_SIZE)
  {
    ErrorStr = "Frame bits exceed memory capacity!";
    return(0);
  }/*end if FrameBitLimit...*/

  /*Image to compress must be 24 bit RGB.*/
  PS.pRGB = pI;
  BitStream = (unsigned int *)pCmp;

  RGB24toYUV411(PS.pRGB, PS.RGB_X, PS.RGB_Y, Y, U, V);

  int RunOutOfBits,BitPos;
  unsigned int BitLimit;
  int i,x,y,col,p;
  int PartitionSize;
  int Bits;
  int EndMarker;
  int PartitionCnt = 0;
  BitStreamSize = 0;
  RunOutOfBits = 0;
  BitPos = 0;

  if(PS.Motion)
  {
    if(FrameCounter > 1)
    {
      /*Allow some slack*/
      BitLimit = FrameBitLimit - (CH263CODEC_EOV_BITS+CH263CODEC_EOI_BITS);
      
      MotionEstimationAndCompensation(Y,U,V);
  
      /*Code the motion vectors as the first partition.*/
      int BitsX,BitsY;
      int CodeX,CodeY;
      COORD *pYBlk;
      COORD *pUVBlk;
      int COL_MOTION_BLK_SIZE;
  
      if(MOTION_BLK_SIZE == 8)
      {
        PartitionSize = DctXBlks_176_8*DctYBlks_144_8;
        pYBlk = DctBlk_176x144_8x8;
        pUVBlk = ColourBlk_88x72_4x4;
      }/*end if MOTION_BLK_SIZE...*/
      else /*if(MOTION_BLK_SIZE == 16) */
      {
        PartitionSize = MotionXBlks_176_16*MotionYBlks_144_16;
        pYBlk = MotionBlk_176x144_16x16;
        pUVBlk = DctBlk_88x72_8x8;
      }/*end else...*/
      COL_MOTION_BLK_SIZE = MOTION_BLK_SIZE/2;
  
      /*Only Lum vectors exist.*/
      for(p = 0; (p < PartitionSize)&&(!RunOutOfBits); p++)
      {
        int mvx = pMVD[p].x;
        int mvy = pMVD[p].y;
        if((mvx >= (CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE-1))||(mvy >= (CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE-1)))
        {
          ErrorStr = "Motion vector out of vlc table range!";
          return(0);
        }/*end if mvx...*/
        BitsX = GetMotionVecBits(mvx,&CodeX);
        BitsY = GetMotionVecBits(mvy,&CodeY);
        if( !(BitsX && BitsY) )
          return(0);
        Bits = BitsX + BitsY;
        if((BitStreamSize + Bits) < BitLimit)
        {
          /*Add the coded bits to the bit stream.*/
          BitStream = PutBits(BitStream,&BitPos,BitsX,CodeX);
          BitStream = PutBits(BitStream,&BitPos,BitsY,CodeY);
          BitStreamSize += Bits;
        }/*end if BitStreamSize...*/
        else
        {
          RunOutOfBits = 1;
          /*NOTE: Jump the queue only, no EOI marker.*/
        }/*end else...*/
      }/*end for p...*/
      /*Clear the effect of the remaining motion vectors from the */
      /*compensated reference, if unable to complete coding. */
      if(RunOutOfBits)
      {
        for(; p < PartitionSize; p++)
        {
          i = Y_X*pYBlk[p].Y + pYBlk[p].X;
          pel *crTL = crY + i;
          pel *rTL = rY + i;
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            int k = Y_X*y;
            pel *locCR = (pel *)(crTL + k);
            pel *locR = (pel *)(rTL + k);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = *(locR++);
            }/*end for x...*/
          }/*end for y...*/
  
          i = UV_X*pUVBlk[p].Y + pUVBlk[p].X;
          pel *crTL_U = crU + i;
          pel *rTL_U = rU + i;
          pel *crTL_V = crV + i;
          pel *rTL_V = rV + i;
          for(y = 0; y < COL_MOTION_BLK_SIZE; y++)
          {
            int k = UV_X*y;
            pel *locCR_U = (pel *)(crTL_U + k);
            pel *locR_U = (pel *)(rTL_U + k);
            pel *locCR_V = (pel *)(crTL_V + k);
            pel *locR_V = (pel *)(rTL_V + k);
            for(x = 0; x < COL_MOTION_BLK_SIZE; x++)
            {
              *(locCR_U++) = *(locR_U++);
              *(locCR_V++) = *(locR_V++);
            }/*end for x...*/
          }/*end for y...*/
  
        }/*end for p...*/
      }/*end if RunOutOfBits...*/

    /*The motion vectors make up the 1st partition.*/
    /*Add the end of partition coded bits to the bit stream.*/
    BitStream = PutBits(BitStream,&BitPos,CH263CODEC_EOV_BITS,CH263CODEC_EOV_CODE);
    BitStreamSize += CH263CODEC_EOV_BITS;

    /*Mark the partition bit position.*/
    PartitionBitPos[PartitionCnt] = BitStreamSize;
    PartitionCnt++;

    }/*end if FrameCounter...*/
    else
    {
      int size = Y_X * Y_Y;
      for(i = 0; i < size; i++)
        crY[i] = rY[i];
      size = UV_X * UV_Y;
      for(i = 0; i < size; i++)
      {
        crU[i] = rU[i];
        crV[i] = rV[i];
      }/*end for i...*/
    }/*end else...*/

  }/*end if Motion...*/

  /*Subtract the reference from the input image.*/
  pel *rSrc;
  pel *rDst;
  pel *Img;
  int TotalPels;

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Img = Y;
      if(PS.Motion)
        rSrc = crY;
      else
        rSrc = rY;
      TotalPels = Y_X * Y_Y;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
      {
        Img = U;
        if(PS.Motion)
          rSrc = crU;
        else
          rSrc = rU;
      }/*end if col...*/
      else /* col == Col_V */
      {
        Img = V;
        if(PS.Motion)
          rSrc = crV;
        else
          rSrc = rV;
      }/*end if col...*/
      TotalPels = UV_X * UV_Y;
    }/*end else...*/

    for(i = TotalPels; i > 0 ; i--)
      *(Img++) -= (*rSrc++);

  }/*end for col...*/

  /*DCT and quantization of entire image.*/
  FDct2D(Y,U,V);
//  if(PS.Motion)
//    FDct2DnoQ(crY,crU,crV);
//  else
//    FDct2DnoQ(rY,rU,rV);

  /*Code all partition structures.*/
  CodePartitions(Y,U,V);

  /*Code each of the data partitions as seperate entities.*/
  BitLimit = FrameBitLimit - CH263CODEC_EOI_BITS - 1; /*Allow some slack*/
  int pnum,coeffs_coded,pos;
  PartitionSize = Y_XBlk * Y_YBlk;
  EndMarker = 0;

  coeffs_coded = 0;
  for(pnum = 0; (pnum < PARTITIONS)&&(!RunOutOfBits); pnum++)
  {
    coeffs_coded = 0; /*How far we got within the active partition.*/
//    if(Partition[pnum].Base_coeff_pos.Y || Partition[pnum].Base_coeff_pos.X)
//    {/*AC coeffs.*/
      /*Start with the COD flag. Assume there is enough bit stream space.*/
      if((BitStreamSize + 1) < BitLimit)
      {
        BitStream = PutBits(BitStream,&BitPos,1,Partition[pnum].COD);
      }/*end if BitStreamSize...*/
      else
      {
        BitStream = PutBits(BitStream,&BitPos,1,1);
        RunOutOfBits = 1;
      }/*end else...*/
      BitStreamSize += 1;

      /*Now add the vlc to the bit stream for each element in the partition struct.*/
      if(Partition[pnum].COD &&(!RunOutOfBits))
      {
        for(pos = 0; (pos < Partition[pnum].structs_coded)&&(!RunOutOfBits); pos++)
        {
          int code;
          Bits = GetRunLevelBits(Partition[pnum].pCodes[pos].last,
                                 Partition[pnum].pCodes[pos].run,
                                 Partition[pnum].pCodes[pos].level,
                                 &code);
          if((BitStreamSize + Bits) < BitLimit)
          {
            /*Add the coded bits to the bit stream.*/
            BitStream = PutBits(BitStream,&BitPos,Bits,code);
            BitStreamSize += Bits;
            coeffs_coded += (Partition[pnum].pCodes[pos].run + 1);
          }/*end if BitStreamSize...*/
          else
          {
            RunOutOfBits = 1;
            /*Truncate the structure.*/
            Partition[pnum].structs_coded = pos;
          }/*end else...*/
        }/*end for pos...*/
      }/*end if COD...*/
//    }/*end if Base_coeff_pos...*/
//    else
//    {/*DC coeffs.*/
//    }/*end else...*/

    /*Mark the partition bit position.*/
    PartitionBitPos[PartitionCnt] = BitStreamSize;
    PartitionCnt++;

  }/*end for pnum...*/

  /*If RunOutOfBits, pnum-1 marks the partition */
  /*and coeffs_coded marks the coeff. */
  int partitions_coded = pnum;
  for(; pnum < PARTITIONS; pnum++)
  {
    /*Clean all remaining partition structs for correct decoding.*/
    if(RunOutOfBits)
    {
      Partition[pnum].structs_coded = 0;
      Partition[pnum].COD = 0;
    }/*end if RunOutOfBits...

    /*Mark the partition bit position.*/
    PartitionBitPos[PartitionCnt] = BitStreamSize;
    PartitionCnt++;
  }/*end for pnum...

  /*Clean remaining structures from the centre to the last coded block.*/
//  if(RunOutOfBits)
//  {
//    if(PS.Motion)
//    {
//      int lim;
//      /*Lum first.*/
//      lim = (int)(PartitionSize * CodedRatio);
//      for(blk = 0; blk < lim; blk++)
//      {
//        for(i = (pY8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(crY + Y_X*(DctBlk_176x144_8x8[blk].Y + ZZ_Order[i].Y) + 
//                      (DctBlk_176x144_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//      }/*end for blk...*/
//      /*Chr next.*/
//      lim = (int)(UVPartitionSize * CodedRatio);
//      for(blk = 0; blk < lim; blk++)
//      {
//        for(i = (pU8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(crU + UV_X*(DctBlk_88x72_8x8[blk].Y + ZZ_Order[i].Y) + 
//                       (DctBlk_88x72_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//        for(i = (pV8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(crV + UV_X*(DctBlk_88x72_8x8[blk].Y + ZZ_Order[i].Y) + 
//                       (DctBlk_88x72_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//      }/*end for blk...*/
//    }/*end if Motion...*/
//    else
//    {
//      int lim;
//      /*Lum first.*/
//      lim = (int)(PartitionSize * CodedRatio);
//      for(blk = 0; blk < lim; blk++)
//      {
//        for(i = (pY8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(rY + Y_X*(DctBlk_176x144_8x8[blk].Y + ZZ_Order[i].Y) + 
//                     (DctBlk_176x144_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//      }/*end for blk...*/
//      /*Chr next.*/
//      lim = (int)(UVPartitionSize * CodedRatio);
//      for(blk = 0; blk < lim; blk++)
//      {
//        for(i = (pU8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(rU + UV_X*(DctBlk_88x72_8x8[blk].Y + ZZ_Order[i].Y) + 
//                      (DctBlk_88x72_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//        for(i = (pV8x8Blk[blk].coeffs_coded + 1); i < 63; i++)
//        {
//          *(rV + UV_X*(DctBlk_88x72_8x8[blk].Y + ZZ_Order[i].Y) + 
//                      (DctBlk_88x72_8x8[blk].X + ZZ_Order[i].X)) = 0;
//        }/*end for i...*/
//      }/*end for blk...*/
//    }/*end else...*/
//  }/*end if RunOutOfBits...*/

  /*Finally add the end of image marker.*/
  BitStream = PutBits(BitStream,&BitPos,CH263CODEC_EOI_BITS,CH263CODEC_EOI_CODE);
  BitStreamSize += CH263CODEC_EOI_BITS;

  /*Decode all partition structures.*/
  DecodePartitions(Y,U,V);

  /*Inverse DCT and inverse quantization of entire image.*/
  IDct2D(Y,U,V);
//  if(PS.Motion)
//    IDct2DnoQ(crY,crU,crV);
//  else
//    IDct2DnoQ(rY,rU,rV);

  /*Add the decoded image to the reference.*/
  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Img = Y;
      rDst = rY;
      if(PS.Motion)
        rSrc = crY;
      else
        rSrc = rY;
      TotalPels = Y_X * Y_Y;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
      {
        Img = U;
        rDst = rU;
        if(PS.Motion)
          rSrc = crU;
        else
          rSrc = rU;
      }/*end if col...*/
      else /* col == Col_V */
      {
        Img = V;
        rDst = rV;
        if(PS.Motion)
          rSrc = crV;
        else
          rSrc = rV;
      }/*end if col...*/
      TotalPels = UV_X * UV_Y;
    }/*end else...*/

    for(i = TotalPels; i > 0 ; i--)
      *(rDst++) = *(rSrc++) + (*Img++);

  }/*end for col...*/

  FrameCounter++;
  return(1);
}/*end code.*/

int CH263CODEC::Decode(void *pCmp,unsigned int FrameBitSize,void *pI)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

  BitStream = (unsigned int *)pCmp;
  BitStreamSize = FrameBitSize;

  /*Decompresed image must be 24 bit RGB.*/
  PS.pRGB = pI;

  /*Extract bits from the input stream and construct the valid */
  /*data structures for motion compensation and Dct coefficient decoding.*/
  /* Process each partition in the correct order.*/
  int RunOutOfBits,BitPos;
  int i,p;
  int PartitionSize;
  int finished;
  int extractbits;
  int PartitionCnt = 0;

  RunOutOfBits = 0;
  BitPos = 0;

  /*The 1st partition consists of the motion vectors.*/
  if(PS.Motion)
  {
    if(FrameCounter > 1)
    {
      /*Decode the motion vectors as the first partition into the structure.*/
      int BitsX,BitsY;
      int MX,MY;
  
      if(MOTION_BLK_SIZE == 8)
      {
        PartitionSize = DctXBlks_176_8*DctYBlks_144_8;
      }/*end if MOTION_BLK_SIZE...*/
      else /*if(MOTION_BLK_SIZE == 16) */
      {
        PartitionSize = MotionXBlks_176_16*MotionYBlks_144_16;
      }/*end else...*/
  
      /*Only Lum vectors exist.*/
      finished = 0;
      for(p = 0; p < PartitionSize; p++)
      {
        if(!finished)
        {
          /*Extract the x motion vector and check for EOV marker.*/
          BitStream = ExtractMotionVecBits(BitStream,&BitPos,&MX,&BitsX,&extractbits);
          FrameBitSize -= BitsX;
          if( BitsX == 0 )
          {
            /*Loss of bit stream sync, catastrophic failure.*/
            return(0);
          }/*end if BitsX...*/
          if( MX == CH263CODEC_EOV_INDEX )
            finished = 1;
          else
          {
            /*Extract the y motion vector.*/
            BitStream = ExtractMotionVecBits(BitStream,&BitPos,&MY,&BitsY,&extractbits);
            FrameBitSize -= BitsY;
            if( BitsY == 0 )
            {
              /*Loss of bit stream sync, catastrophic failure.*/
              return(0);
            }/*end if BitsY...*/
            /*Load the motion vector structure.*/
            pMVD[p].x = MX;
            pMVD[p].y = MY;
          }/*end else...*/
        }/*end if !finished...*/

        if(finished)
        {
          pMVD[p].x = 0;
          pMVD[p].y = 0;
        }/*end if finished...*/

      }/*end for p...*/

      /*The next bits must be the EOV marker.*/
      if(!finished)
      {
        BitStream = ExtractMotionVecBits(BitStream,&BitPos,&MX,&BitsX,&extractbits);
        FrameBitSize -= BitsX;
        if( (MX != CH263CODEC_EOV_INDEX)||(BitsX == 0) )
        {
          /*Loss of bit stream sync, catastrophic failure.*/
          ErrorStr = "H263 Codec: End of vectors marker not found!";
          return(0);
        }/*end if MX...*/
      }/*end if !finished...*/

      MotionCompensation();

      PartitionCnt++;
    }/*end if FrameCounter...*/
    else
    {
      int size = Y_X * Y_Y;
      for(i = 0; i < size; i++)
        crY[i] = rY[i];
      size = UV_X * UV_Y;
      for(i = 0; i < size; i++)
      {
        crU[i] = rU[i];
        crV[i] = rV[i];
      }/*end for i...*/
    }/*end else...*/
  }/*end if Motion...*/

  /*The Dct run-length data structures are extracted from the data stream.*/
  int last,run,level,marker;
  int Bits;
  int pnum,coeffs_coded;

  /*Before doing the DCT coefficients, clean out the partition structures.*/
  for(pnum = 0; pnum < PARTITIONS; pnum++)
  {
    Partition[pnum].structs_coded = 0;
    Partition[pnum].structs_pos = 0;
    Partition[pnum].COD = 0;
    Partition[pnum].done = 0;
  }/*end for pnum...*/

  /*Load the structs from the bit stream.*/
  last = 0;
  run = 0;
  level = 0;
  marker = 0;
  coeffs_coded = 0;
  for(pnum = 0; pnum < PARTITIONS; pnum++)
  {
    coeffs_coded = 0; /*Reset for each partition.*/
    while((!Partition[pnum].done)&&(!RunOutOfBits)&&(Partition[pnum].structs_coded < Partition[pnum].CODE_STRUCT_SIZE))
    {
      /*If this is the 1st structure to be coded then start with */
      /*extracting the COD flag.    */
      if(Partition[pnum].structs_coded == 0)
      {
        BitStream = GetNextBit(BitStream,&BitPos,&(Partition[pnum].COD));
        FrameBitSize -= 1;
      }/*end if structs_pos...*/

      if(Partition[pnum].COD)
      {
        /*Extract the next structure values from the data stream.*/
        BitStream = ExtractRunLevelBits(BitStream,&BitPos,&last,&run,&level,&marker,&Bits,&extractbits);
        FrameBitSize -= Bits;

        if(marker == 0) /*Valid last,run,level values.*/
        {
          Partition[pnum].pCodes[Partition[pnum].structs_coded].last = last;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].run = run;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].level = level;
          coeffs_coded += (run + 1);
          Partition[pnum].structs_coded++;
          if(last)
            Partition[pnum].done = 1;
        }/*end if marker...*/
        else if(marker == 3) /*EOP marker.*/
        {
          /*Loss of bit stream sync, catastrophic failure.*/
          ErrorStr = "H263 Codec: Unexpected EOP marker found!";
          return(0);
        }/*end else if marker...*/
        else /*EOI marker.*/
        {
          RunOutOfBits = 1;
        }/*end else...*/
      }/*end if COD...*/
      else
      {
        Partition[pnum].done = 1;
      }/*end else...*/
    }/*end while !done...*/

    /*Clean remaining structures.*/
    if(RunOutOfBits)
    {
      if(Partition[pnum].structs_coded == 0)
        Partition[pnum].COD = 0;
    }/*end if RunOutOfBits...*/

    PartitionCnt++;
  }/*end for pnum...*/

  if(!RunOutOfBits) /*Last bits must be EOI marker.*/
  {
    BitStream = ExtractRunLevelBits(BitStream,&BitPos,&last,&run,&level,&marker,&Bits,&extractbits);
    FrameBitSize -= Bits;
    if(marker != 4) /*EOI marker is expected.*/
    {
      /*Loss of bit stream sync, catastrophic failure.*/
      ErrorStr = "H263 Codec: EOI marker not found!";
      return(0);
    }/*end if marker...*/
  }/*end if !RunOutOfBits...*/

  /*Decode all partiton structures.*/
  DecodePartitions(Y,U,V);

  /*Inverse DCT and inverse quantization of entire image.*/
  IDct2D(Y,U,V);

  /*Add the decoded image to the reference.*/
  pel *rSrc;
  pel *rDst;
  pel *Img;
  int TotalPels;
  int col;

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Img = Y;
      rDst = rY;
      if(PS.Motion)
        rSrc = crY;
      else
        rSrc = rY;
      TotalPels = Y_X * Y_Y;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
      {
        Img = U;
        rDst = rU;
        if(PS.Motion)
          rSrc = crU;
        else
          rSrc = rU;
      }/*end if col...*/
      else /* col == Col_V */
      {
        Img = V;
        rDst = rV;
        if(PS.Motion)
          rSrc = crV;
        else
          rSrc = rV;
      }/*end if col...*/
      TotalPels = UV_X * UV_Y;
    }/*end else...*/

    for(i = TotalPels; i > 0 ; i--)
      *(rDst++) = *(rSrc++) + (*Img++);

  }/*end for col...*/

  YUV411toRGB24(rY, rU, rV, PS.pRGB, PS.RGB_X, PS.RGB_Y);

  FrameCounter++;
  return(1);
}/*end Decode.*/

void CH263CODEC::Close(void)
{
	if(hY != NULL)
	{
		GlobalUnlock(hY);
 		GlobalFree(hY);
 		hY = NULL;
	}/*end if hY...*/
	Y = NULL;
	if(hU != NULL)
	{
		GlobalUnlock(hU);
 		GlobalFree(hU);
 		hU = NULL;
	}/*end if hU...*/
	U = NULL;
	if(hV != NULL)
	{
		GlobalUnlock(hV);
 		GlobalFree(hV);
 		hV = NULL;
	}/*end if hV...*/
	V = NULL;

	if(hrY != NULL)
	{
		GlobalUnlock(hrY);
 		GlobalFree(hrY);
 		hrY = NULL;
	}/*end if hrY...*/
	rY = NULL;
	if(hrU != NULL)
	{
		GlobalUnlock(hrU);
 		GlobalFree(hrU);
 		hrU = NULL;
	}/*end if hrU...*/
	rU = NULL;
	if(hrV != NULL)
	{
		GlobalUnlock(hrV);
 		GlobalFree(hrV);
 		hrV = NULL;
	}/*end if hrV...*/
	rV = NULL;

	if(hcrY != NULL)
	{
		GlobalUnlock(hcrY);
 		GlobalFree(hcrY);
 		hcrY = NULL;
	}/*end if hcrY...*/
	crY = NULL;
	if(hcrU != NULL)
	{
		GlobalUnlock(hcrU);
 		GlobalFree(hcrU);
 		hcrU = NULL;
	}/*end if hcrU...*/
	crU = NULL;
	if(hcrV != NULL)
	{
		GlobalUnlock(hcrV);
 		GlobalFree(hcrV);
 		hcrV = NULL;
	}/*end if hcrV...*/
	crV = NULL;

	if(hMVD != NULL)
	{
		GlobalUnlock(hMVD);
 		GlobalFree(hMVD);
 		hMVD = NULL;
	}/*end if hMVD...*/
	pMVD = NULL;
  
  DeletePartitions();

  CodecIsOpen = 0;
}/*end Close.*/

/* Call this function only after a successful code or decode.*/
void CH263CODEC::ZeroRefPartition(int partition)
{
  /*We will ignore zeroing the motion vector partition for now.*/
  if( (partition == 0)&&(PS.Motion)&&(FrameCounter > 2) )
    return;

  /*Make a compensated partition number if a motion vector partition exisits.*/
  int comp_part = partition;
  if((PS.Motion)&&(FrameCounter > 2))
    comp_part--;

  /*Convert the spatial domain reference image into the Dct domain.*/
  FDct2DnoQ(rY,rU,rV);

  /*Treat the partitions according to their application.*/
  if(comp_part < PARTITIONS)
  {
    int blk;
    pel *Img;
    int TotBlks,X;
    COORD *pBlk;

    if(Partition[comp_part].Colour == Col_Y)
    {
      Img = rY;
      pBlk = DctBlk_176x144_8x8;
      TotBlks = Y_XBlk * Y_YBlk;
      X = Y_X;
    }/*end if Colour...*/
    else
    {
      if(Partition[comp_part].Colour == Col_U)
      {
        Img = rU;
      }/*end if Colour...*/
      else /* Colour == Col_V */
      {
        Img = rV;
      }/*end if Colour...*/
      pBlk = DctBlk_88x72_8x8;
      TotBlks = UV_XBlk * UV_YBlk;
      X = UV_X;
    }/*end else...*/

    /*Zero one coeff from each block.*/
    for(blk = 0; blk < TotBlks; blk++)
    {
      *((pel *)(Img + X*(pBlk[blk].Y + Partition[comp_part].Base_coeff_pos.Y) + 
                        (pBlk[blk].X + Partition[comp_part].Base_coeff_pos.X))) = 0;
    }/*end for blk...*/

  }/*end if comp_part...*/

  /*Put the reference image space back into the spatial domain.*/
  IDct2DnoQ(rY,rU,rV);
}/*end ZeroRefPartition.*/

/*//////////////////////////////////////////////////////////*/
/* Private Implementation.                                  */
/*//////////////////////////////////////////////////////////*/

/////////////////////////////////////////////////////////////////
// In-place 2-dim DCT of the local YUV image pointed to by the 
// input parameters. The dim of the YUV image is assumed to be
// that of the parameter list.
/////////////////////////////////////////////////////////////////
void CH263CODEC::Dct2D(pel *locY,pel *locU,pel *locV)
{
  int col,i,j;
  int XBlk,YBlk,X;
  pel *Src;
  pel *TL;
  pel th = (pel)(PS.Thresh);

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Src = locY;
      X = Y_X;
      XBlk = Y_XBlk; /*Assume divisibility by 8.*/
      YBlk = Y_YBlk;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
        Src = locU;
      else /* col == Col_V */
        Src = locV;
      X = UV_X;
      XBlk = UV_XBlk; /*Assume divisibility by 8.*/
      YBlk = UV_YBlk;
    }/*end else...*/

    /*Step thru every 8x8 block.*/
    for(i = 0; i < YBlk; i++)
    {
      int row = (i << 3)*X;
      for(j = 0; j < XBlk; j++)
      {
        TL = Src + row + (j << 3);
        /* Do the Dct, Quantization and noise coring of */ 
        /* this block in-place.*/

	      long int r0,r1,r2,r3,r4,r5,r6,r7;
	      long int r8,r9,r10,r11;
	      long int w,y,z;
        pel x;

	      int m,n;
	      long int T[64];

	      /* Do 1-D DCT on each row and store the intermediate */
	      /* result in the T matrix. */
        for(m=0; m < 8; m++) /*for each row...*/
          {
	      	  z = m << 3;
            w = m*X;
	      		// Convert to fixed pt. numbers as read in.
	      		r8 = DCTFIXPOINT((long)(*(TL + w)));
	      		r9 = DCTFIXPOINT((long)(*(TL + w + 7)));
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = DCTFIXPOINT((long)(*(TL + w + 1)));
	      		r11 = DCTFIXPOINT((long)(*(TL + w + 6)));
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = DCTFIXPOINT((long)(*(TL + w + 2)));
	      		r9 = DCTFIXPOINT((long)(*(TL + w + 5)));
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = DCTFIXPOINT((long)(*(TL + w + 3)));
	      		r11 = DCTFIXPOINT((long)(*(TL + w + 4)));
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          *(T + z) = r8 + r9;
	          r0 = r10 + r11;
	          *(T + z + 4) = r8 - r9;
	          
	          r1 = DCTFIXPOINTADJUST(C_1 * r0);
	          
	          *(T + z + 2) = r11 + r1;
	          *(T + z + 6) = r11 - r1;
	          
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = DCTFIXPOINTADJUST(C_1 * r1);
	          r3 = C_2 * (r0 + r2);
	          r4 = -(DCTFIXPOINTADJUST((C_3 * r0) + r3));
	          r6 = DCTFIXPOINTADJUST((C_4 * r2) - r3);
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          *(T + z + 5) = r4 + r1;
	          *(T + z + 3) = r1 - r4;
	          *(T + z + 1) = r0 + r6;
	          *(T + z + 7) = r0 - r6;
          }/*end for m....*/
	      /* Do the 1-D DCT down each column, quantize, and */
        /* convert from fixed pt. */
        for(n=0; n < 8; n++) /*for each column...*/
          {
	      		r8 = *(T + n);
	      		r9 = *(T + 56 + n);
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = *(T + 8 + n);
	      		r11 = *(T + 48 + n);
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = *(T + 16 + n);
	      		r9 = *(T + 40 + n);
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = *(T + 24 + n);
	      		r11 = *(T + 32 + n);
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          y = r8 + r9; //[0]
	      		x = (pel)((y * QTable[n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + n) = x;
        
	          r0 = r10 + r11;
        
	          y = r8 - r9; //[4]
	      		x = (pel)((y * QTable[32 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 4*X + n) = x;
        
	          r1 = DCTFIXPOINTADJUST(C_1 * r0);
	          
	          y = r11 + r1; //[2]
	      		x = (pel)((y * QTable[16 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 2*X + n) = x;
        
	          y = r11 - r1; //[6]
	      		x = (pel)((y * QTable[48 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 6*X + n) = x;
        
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = DCTFIXPOINTADJUST(C_1 * r1);
	          r3 = C_2 * (r0 + r2);
	          r4 = -(DCTFIXPOINTADJUST((C_3 * r0) + r3));
	          r6 = DCTFIXPOINTADJUST((C_4 * r2) - r3);
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          y = r4 + r1; //[5]
	      		x = (pel)((y * QTable[40 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 5*X + n) = x;
        
	          y = r1 - r4; //[3]
	      		x = (pel)((y * QTable[24 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 3*X + n) = x;
        
	          y = r0 + r6; //[1]
	      		x = (pel)((y * QTable[8 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + X + n) = x;
        
	          y = r0 - r6; //[7]
	      		x = (pel)((y * QTable[56 + n]) >> DctFxPt2x);
//	      		if( (x > -th)&&(x < th) )
//              x = 0;
	      		*(TL + 7*X + n) = x;
          }/*end for n....*/

      }/*end for j...*/
    }/*end for i...*/

  }/*end for col...*/
}/*end Dct2D.*/

void CH263CODEC::FDct2D(pel *locY,pel *locU,pel *locV)
{
  int col,i,j;
  int XBlk,YBlk,X;
  pel *Src;
  pel *TL;
  pel th = (pel)(PS.Thresh);

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Src = locY;
      X = Y_X;
      XBlk = Y_XBlk; /*Assume divisibility by 8.*/
      YBlk = Y_YBlk;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
        Src = locU;
      else /* col == Col_V */
        Src = locV;
      X = UV_X;
      XBlk = UV_XBlk; /*Assume divisibility by 8.*/
      YBlk = UV_YBlk;
    }/*end else...*/

    /*Step thru every 8x8 block.*/
    for(i = 0; i < YBlk; i++)
    {
      int row = (i << 3)*X;
      for(j = 0; j < XBlk; j++)
      {
        TL = Src + row + (j << 3);
        /* Do the Dct, Quantization and noise coring of */ 
        /* this block in-place.*/

	      double r0,r1,r2,r3,r4,r5,r6,r7;
	      double r8,r9,r10,r11;
	      double y;
	      int w,z;
        pel x;

	      int m,n;
	      double T[64];

	      /* Do 1-D DCT on each row and store the intermediate */
	      /* result in the T matrix. */
        for(m=0; m < 8; m++) /*for each row...*/
          {
	      	  z = m << 3;
            w = m*X;
	      		// Convert to fixed pt. numbers as read in.
	      		r8 = (double)(*(TL + w));
	      		r9 = (double)(*(TL + w + 7));
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = (double)(*(TL + w + 1));
	      		r11 = (double)(*(TL + w + 6));
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = (double)(*(TL + w + 2));
	      		r9 = (double)(*(TL + w + 5));
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = (double)(*(TL + w + 3));
	      		r11 = (double)(*(TL + w + 4));
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          *(T + z) = r8 + r9;
	          r0 = r10 + r11;
	          *(T + z + 4) = r8 - r9;
	          
	          r1 = FC_1 * r0;
	          
	          *(T + z + 2) = r11 + r1;
	          *(T + z + 6) = r11 - r1;
	          
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = FC_1 * r1;
	          r3 = FC_2 * (r0 + r2);
	          r4 = -((FC_3 * r0) + r3);
	          r6 = (FC_4 * r2) - r3;
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          *(T + z + 5) = r4 + r1;
	          *(T + z + 3) = r1 - r4;
	          *(T + z + 1) = r0 + r6;
	          *(T + z + 7) = r0 - r6;
          }/*end for m....*/
	      /* Do the 1-D DCT down each column, quantize, and */
        /* convert from fixed pt. */
        for(n=0; n < 8; n++) /*for each column...*/
          {
	      		r8 = *(T + n);
	      		r9 = *(T + 56 + n);
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = *(T + 8 + n);
	      		r11 = *(T + 48 + n);
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = *(T + 16 + n);
	      		r9 = *(T + 40 + n);
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = *(T + 24 + n);
	      		r11 = *(T + 32 + n);
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          y = r8 + r9; //[0]
	      		x = (pel)(y * FQTable[n]);
	      		if( (x > -th)&&(x < th) )
              x = 0;
	      		*(TL + n) = x;
        
	          r0 = r10 + r11;
        
	          y = r8 - r9; //[4]
	      		x = (pel)(y * FQTable[32 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 4*X + n) = x;
        
	          r1 = FC_1 * r0;
	          
	          y = r11 + r1; //[2]
	      		x = (pel)(y * FQTable[16 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 2*X + n) = x;
        
	          y = r11 - r1; //[6]
	      		x = (pel)(y * FQTable[48 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 6*X + n) = x;
        
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = FC_1 * r1;
	          r3 = FC_2 * (r0 + r2);
	          r4 = -((FC_3 * r0) + r3);
	          r6 = (FC_4 * r2) - r3;
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          y = r4 + r1; //[5]
	      		x = (pel)(y * FQTable[40 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 5*X + n) = x;
        
	          y = r1 - r4; //[3]
	      		x = (pel)(y * FQTable[24 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 3*X + n) = x;
        
	          y = r0 + r6; //[1]
	      		x = (pel)(y * FQTable[8 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + X + n) = x;
        
	          y = r0 - r6; //[7]
	      		x = (pel)(y * FQTable[56 + n]);
            if(x < -th) /*Noise coring.*/
              x += th;
            else if(x > th)
              x -= th;
            else
              x = 0;
	      		*(TL + 7*X + n) = x;
          }/*end for n....*/

      }/*end for j...*/
    }/*end for i...*/

  }/*end for col...*/
}/*end FDct2D.*/

void CH263CODEC::FDct2DnoQ(pel *locY,pel *locU,pel *locV)
{
  int col,i,j;
  int XBlk,YBlk,X;
  pel *Src;
  pel *TL;

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Src = locY;
      X = Y_X;
      XBlk = Y_XBlk; /*Assume divisibility by 8.*/
      YBlk = Y_YBlk;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
        Src = locU;
      else /* col == Col_V */
        Src = locV;
      X = UV_X;
      XBlk = UV_XBlk; /*Assume divisibility by 8.*/
      YBlk = UV_YBlk;
    }/*end else...*/

    /*Step thru every 8x8 block.*/
    for(i = 0; i < YBlk; i++)
    {
      int row = (i << 3)*X;
      for(j = 0; j < XBlk; j++)
      {
        TL = Src + row + (j << 3);
        /* Do the Dct, Quantization and noise coring of */ 
        /* this block in-place.*/

	      double r0,r1,r2,r3,r4,r5,r6,r7;
	      double r8,r9,r10,r11;
	      double y;
	      int w,z;
        pel x;

	      int m,n;
	      double T[64];

	      /* Do 1-D DCT on each row and store the intermediate */
	      /* result in the T matrix. */
        for(m=0; m < 8; m++) /*for each row...*/
          {
	      	  z = m << 3;
            w = m*X;
	      		// Convert to fixed pt. numbers as read in.
	      		r8 = (double)(*(TL + w));
	      		r9 = (double)(*(TL + w + 7));
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = (double)(*(TL + w + 1));
	      		r11 = (double)(*(TL + w + 6));
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = (double)(*(TL + w + 2));
	      		r9 = (double)(*(TL + w + 5));
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = (double)(*(TL + w + 3));
	      		r11 = (double)(*(TL + w + 4));
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          *(T + z) = r8 + r9;
	          r0 = r10 + r11;
	          *(T + z + 4) = r8 - r9;
	          
	          r1 = FC_1 * r0;
	          
	          *(T + z + 2) = r11 + r1;
	          *(T + z + 6) = r11 - r1;
	          
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = FC_1 * r1;
	          r3 = FC_2 * (r0 + r2);
	          r4 = -((FC_3 * r0) + r3);
	          r6 = (FC_4 * r2) - r3;
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          *(T + z + 5) = r4 + r1;
	          *(T + z + 3) = r1 - r4;
	          *(T + z + 1) = r0 + r6;
	          *(T + z + 7) = r0 - r6;
          }/*end for m....*/
	      /* Do the 1-D DCT down each column, quantize, and */
        /* convert from fixed pt. */
        for(n=0; n < 8; n++) /*for each column...*/
          {
	      		r8 = *(T + n);
	      		r9 = *(T + 56 + n);
	          r0 = r8 + r9;
	          r7 = r8 - r9;
	      		r10 = *(T + 8 + n);
	      		r11 = *(T + 48 + n);
	          r1 = r10 + r11;
	          r6 = r10 - r11;
	      		r8 = *(T + 16 + n);
	      		r9 = *(T + 40 + n);
	          r2 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = *(T + 24 + n);
	      		r11 = *(T + 32 + n);
	          r3 = r10 + r11;
	          r4 = r10 - r11;
        
	          r8 = r0 + r3;
	          r11 = r0 - r3;
	          r9 = r1 + r2;
	          r10 = r1 - r2;
        
	          y = r8 + r9; //[0]
	      		x = (pel)(y * SDCT[n]);
	      		*(TL + n) = x;
        
	          r0 = r10 + r11;
        
	          y = r8 - r9; //[4]
	      		x = (pel)(y * SDCT[32 + n]);
	      		*(TL + 4*X + n) = x;
        
	          r1 = FC_1 * r0;
	          
	          y = r11 + r1; //[2]
	      		x = (pel)(y * SDCT[16 + n]);
	      		*(TL + 2*X + n) = x;
        
	          y = r11 - r1; //[6]
	      		x = (pel)(y * SDCT[48 + n]);
	      		*(TL + 6*X + n) = x;
        
	          r0 = -(r4 + r5);
	          r1 = r5 + r6;
	          r2 = r6 + r7;
	          
	          r5 = FC_1 * r1;
	          r3 = FC_2 * (r0 + r2);
	          r4 = -((FC_3 * r0) + r3);
	          r6 = (FC_4 * r2) - r3;
	          
	          r0 = r5 + r7;
	          r1 = r7 - r5;
	          
	          y = r4 + r1; //[5]
	      		x = (pel)(y * SDCT[40 + n]);
	      		*(TL + 5*X + n) = x;
        
	          y = r1 - r4; //[3]
	      		x = (pel)(y * SDCT[24 + n]);
	      		*(TL + 3*X + n) = x;
        
	          y = r0 + r6; //[1]
	      		x = (pel)(y * SDCT[8 + n]);
	      		*(TL + X + n) = x;
        
	          y = r0 - r6; //[7]
	      		x = (pel)(y * SDCT[56 + n]);
	      		*(TL + 7*X + n) = x;
          }/*end for n....*/

      }/*end for j...*/
    }/*end for i...*/

  }/*end for col...*/
}/*end FDct2DnoQ.*/

/////////////////////////////////////////////////////////////////
// In-place 2-dim IDCT of the local YUV image pointed to by the 
// input parameters. The dim of the YUV image is assumed to be
// that of the parameter list.
/////////////////////////////////////////////////////////////////
void CH263CODEC::IDct2D(pel *locY,pel *locU,pel *locV)
{
  int col,i,j;
  int XBlk,YBlk,X;
  pel *Src;
  pel *TL;
  long int th = PS.Thresh;

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Src = locY;
      X = Y_X;
      XBlk = Y_XBlk; /*Assume divisibility by 8.*/
      YBlk = Y_YBlk;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
        Src = locU;
      else /* col == Col_V */
        Src = locV;
      X = UV_X;
      XBlk = UV_XBlk; /*Assume divisibility by 8.*/
      YBlk = UV_YBlk;
    }/*end else...*/

    /*Step thru every 8x8 block.*/
    for(i = 0; i < YBlk; i++)
    {
      int row = (i << 3)*X;
      for(j = 0; j < XBlk; j++)
      {
        TL = Src + row + (j << 3);
        /* Do the IDct, Inverse Quantization and noise coring of */ 
        /* this block in-place.*/

	      long int r0,r1,r2,r3,r4,r5,r6,r7;
	      long int r8,r9,r10,r11;
	      long int w,x,y,z;

	      int m,n;
	      long int T[64];

	      /* Get the DCT coeff. convert to fixed pt., */
        /* inverse quantize and do 1-D IDCT on each row. */
        for(m=0; m < 8; m++) /*for each row...*/
          {
	      	  z = m << 3;
            w = m*X;
            x = (long)(*(TL + w));
            if(x < 0)
              x -= th;
            else if(x > 0)
              x += th;
	      		r8 = DCTFIXPOINT(x);
	      		r8 = DCTFIXPOINTADJUST(r8 * IQTable[z]);
            y = (long)(*(TL + w + 4));
            if(y < 0)
              y -= th;
            else if(y > 0)
              y += th;
	      		r9 = DCTFIXPOINT(y);
	      		r9 = DCTFIXPOINTADJUST(r9 * IQTable[z + 4]);
	          r4 = r8 + r9;
	          r5 = r8 - r9;
            x = (long)(*(TL + w + 2));
            if(x < 0)
              x -= th;
            else if(x > 0)
              x += th;
	      		r10 = DCTFIXPOINT(x);
	      		r10 = DCTFIXPOINTADJUST(r10 * IQTable[z + 2]);
            y = (long)(*(TL + w + 6));
            if(y < 0)
              y -= th;
            else if(y > 0)
              y += th;
	      		r11 = DCTFIXPOINT(y);
	      		r11 =	DCTFIXPOINTADJUST(r11 * IQTable[z + 6]);
	          r6 = DCTFIXPOINTADJUST(C_1 * (r10 - r11));
	          r1 = r10 + r11;
	          r7 = r6 + r1;
	          
	          r0 = r4 + r7;
	          r3 = r4 - r7;
	          r1 = r5 + r6;
	          r2 = r5 - r6;
        
            x = (long)(*(TL + w + 5));
            if(x < 0)
              x -= th;
            else if(x > 0)
              x += th;
	      		r8 = DCTFIXPOINT(x);
	      		r8 = DCTFIXPOINTADJUST(r8 * IQTable[z + 5]);
            y = (long)(*(TL + w + 3));
            if(y < 0)
              y -= th;
            else if(y > 0)
              y += th;
	      		r9 = DCTFIXPOINT(y);
	      		r9 = DCTFIXPOINTADJUST(r9 * IQTable[z + 3]);
	          r4 = r8 - r9;
	          r7 = r8 + r9;
            x = (long)(*(TL + w + 1));
            if(x < 0)
              x -= th;
            else if(x > 0)
              x += th;
	      		r10 = DCTFIXPOINT(x);
	      		r10 = DCTFIXPOINTADJUST(r10 * IQTable[z + 1]);
            y = (long)(*(TL + w + 7));
            if(y < 0)
              y -= th;
            else if(y > 0)
              y += th;
	      		r11 = DCTFIXPOINT(y);
	      		r11 =	DCTFIXPOINTADJUST(r11 * IQTable[z + 7]);
	          r5 = r10 + r11;
	          r6 = r10 - r11;
	          
	          r8 = DCTFIXPOINTADJUST(C_2 * (-(r4 + r6)));
	          r10 = DCTFIXPOINTADJUST(C_1 * (r5 - r7));
	          r9 = r8 + DCTFIXPOINTADJUST(C_3 * (-r4));
	          r11 = r8 + DCTFIXPOINTADJUST(C_4 * r6);
	          r8 = r5 + r7;
	          
	          r4 = -r9;
	          r5 = r10 - r9;
	          r6 = r10 + r11;
	          r7 = r11 + r8;
	          
	          *(T + z) = r0 + r7;
	          *(T + z + 7) = r0 - r7;
	          *(T + z + 1) = r1 + r6;
	          *(T + z + 6) = r1 - r6;
	          *(T + z + 2) = r2 + r5;
	          *(T + z + 5) = r2 - r5;
	          *(T + z + 3) = r3 + r4;
	          *(T + z + 4) = r3 - r4;
           }/*end for m....*/
        
        /*Copy the previous DCT coeff into temparary Vector and do */
        /*1-D IDCT on columns*/
        for(n=0; n < 8; n++) /*for each column...*/
          {
	      		r8 = *(T + n); //[0]
	      		r9 = *(T + 32 + n); //[4]
	          r4 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = *(T + 16 + n); //[2]
	      		r11 =	*(T + 48 + n); //[6]
	          r6 = DCTFIXPOINTADJUST(C_1 * (r10 - r11));
	          r1 = r10 + r11;
	          r7 = r6 + r1;
	          
	          r0 = r4 + r7;
	          r3 = r4 - r7;
	          r1 = r5 + r6;
	          r2 = r5 - r6;
	          
	      		r8 = *(T + 40 + n); //[5]
	      		r9 = *(T + 24 + n); //[3]
	          r4 = r8 - r9;
	          r7 = r8 + r9;
	      		r10 = *(T + 8 + n); //[1]
	      		r11 =	*(T + 56 + n);//[7]
	          r5 = r10 + r11;
	          r6 = r10 - r11;
	          
	          r8 = DCTFIXPOINTADJUST(C_2 * (-(r4 + r6)));
	          r10 = DCTFIXPOINTADJUST(C_1 * (r5 - r7));
	          r9 = r8 + DCTFIXPOINTADJUST(C_3 * (-r4));
	          r11 = r8 + DCTFIXPOINTADJUST(C_4 * r6);
	          r8 = r5 + r7;
	          
	          r4 = -r9;
	          r5 = r10 - r9;
	          r6 = r10 + r11;
	          r7 = r11 + r8;
	          
	      		x = DCTFIXPOINTADJUST(r0 + r7); //[0]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r0 - r7); //[7]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 7*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r1 + r6); //[1]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r1 - r6); //[6]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 6*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r2 + r5); //[2]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + 2*X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r2 - r5); //[5]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 5*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r3 + r4); //[3]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + 3*X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r3 - r4); //[4]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 4*X + n) = (pel)y;
          }/*end for n....*/

      }/*end for j...*/
    }/*end for i...*/

  }/*end for col...*/
}/*end IDct2D.*/

void CH263CODEC::IDct2DnoQ(pel *locY,pel *locU,pel *locV)
{
  int col,i,j;
  int XBlk,YBlk,X;
  pel *Src;
  pel *TL;

  for(col = 0; col < NUM_OF_COLOUR_COMP; col++)
  {
    if(col == Col_Y)
    {
      Src = locY;
      X = Y_X;
      XBlk = Y_XBlk; /*Assume divisibility by 8.*/
      YBlk = Y_YBlk;
    }/*end if col...*/
    else
    {
      if(col == Col_U)
        Src = locU;
      else /* col == Col_V */
        Src = locV;
      X = UV_X;
      XBlk = UV_XBlk; /*Assume divisibility by 8.*/
      YBlk = UV_YBlk;
    }/*end else...*/

    /*Step thru every 8x8 block.*/
    for(i = 0; i < YBlk; i++)
    {
      int row = (i << 3)*X;
      for(j = 0; j < XBlk; j++)
      {
        TL = Src + row + (j << 3);
        /* Do the IDct, Inverse Quantization and noise coring of */ 
        /* this block in-place.*/

	      long int r0,r1,r2,r3,r4,r5,r6,r7;
	      long int r8,r9,r10,r11;
	      long int w,x,y,z;

	      int m,n;
	      long int T[64];

	      /* Get the DCT coeff. convert to fixed pt., */
        /* inverse quantize and do 1-D IDCT on each row. */
        for(m=0; m < 8; m++) /*for each row...*/
          {
	      	  z = m << 3;
            w = m*X;
            x = (long)(*(TL + w));
	      		r8 = DCTFIXPOINT(x);
	      		r8 = DCTFIXPOINTADJUST(r8 * ITable[z]);
            y = (long)(*(TL + w + 4));
	      		r9 = DCTFIXPOINT(y);
	      		r9 = DCTFIXPOINTADJUST(r9 * ITable[z + 4]);
	          r4 = r8 + r9;
	          r5 = r8 - r9;
            x = (long)(*(TL + w + 2));
	      		r10 = DCTFIXPOINT(x);
	      		r10 = DCTFIXPOINTADJUST(r10 * ITable[z + 2]);
            y = (long)(*(TL + w + 6));
	      		r11 = DCTFIXPOINT(y);
	      		r11 =	DCTFIXPOINTADJUST(r11 * ITable[z + 6]);
	          r6 = DCTFIXPOINTADJUST(C_1 * (r10 - r11));
	          r1 = r10 + r11;
	          r7 = r6 + r1;
	          
	          r0 = r4 + r7;
	          r3 = r4 - r7;
	          r1 = r5 + r6;
	          r2 = r5 - r6;
        
            x = (long)(*(TL + w + 5));
	      		r8 = DCTFIXPOINT(x);
	      		r8 = DCTFIXPOINTADJUST(r8 * ITable[z + 5]);
            y = (long)(*(TL + w + 3));
	      		r9 = DCTFIXPOINT(y);
	      		r9 = DCTFIXPOINTADJUST(r9 * ITable[z + 3]);
	          r4 = r8 - r9;
	          r7 = r8 + r9;
            x = (long)(*(TL + w + 1));
	      		r10 = DCTFIXPOINT(x);
	      		r10 = DCTFIXPOINTADJUST(r10 * ITable[z + 1]);
            y = (long)(*(TL + w + 7));
	      		r11 = DCTFIXPOINT(y);
	      		r11 =	DCTFIXPOINTADJUST(r11 * ITable[z + 7]);
	          r5 = r10 + r11;
	          r6 = r10 - r11;
	          
	          r8 = DCTFIXPOINTADJUST(C_2 * (-(r4 + r6)));
	          r10 = DCTFIXPOINTADJUST(C_1 * (r5 - r7));
	          r9 = r8 + DCTFIXPOINTADJUST(C_3 * (-r4));
	          r11 = r8 + DCTFIXPOINTADJUST(C_4 * r6);
	          r8 = r5 + r7;
	          
	          r4 = -r9;
	          r5 = r10 - r9;
	          r6 = r10 + r11;
	          r7 = r11 + r8;
	          
	          *(T + z) = r0 + r7;
	          *(T + z + 7) = r0 - r7;
	          *(T + z + 1) = r1 + r6;
	          *(T + z + 6) = r1 - r6;
	          *(T + z + 2) = r2 + r5;
	          *(T + z + 5) = r2 - r5;
	          *(T + z + 3) = r3 + r4;
	          *(T + z + 4) = r3 - r4;
           }/*end for m....*/
        
        /*Copy the previous DCT coeff into temparary Vector and do */
        /*1-D IDCT on columns*/
        for(n=0; n < 8; n++) /*for each column...*/
          {
	      		r8 = *(T + n); //[0]
	      		r9 = *(T + 32 + n); //[4]
	          r4 = r8 + r9;
	          r5 = r8 - r9;
	      		r10 = *(T + 16 + n); //[2]
	      		r11 =	*(T + 48 + n); //[6]
	          r6 = DCTFIXPOINTADJUST(C_1 * (r10 - r11));
	          r1 = r10 + r11;
	          r7 = r6 + r1;
	          
	          r0 = r4 + r7;
	          r3 = r4 - r7;
	          r1 = r5 + r6;
	          r2 = r5 - r6;
	          
	      		r8 = *(T + 40 + n); //[5]
	      		r9 = *(T + 24 + n); //[3]
	          r4 = r8 - r9;
	          r7 = r8 + r9;
	      		r10 = *(T + 8 + n); //[1]
	      		r11 =	*(T + 56 + n);//[7]
	          r5 = r10 + r11;
	          r6 = r10 - r11;
	          
	          r8 = DCTFIXPOINTADJUST(C_2 * (-(r4 + r6)));
	          r10 = DCTFIXPOINTADJUST(C_1 * (r5 - r7));
	          r9 = r8 + DCTFIXPOINTADJUST(C_3 * (-r4));
	          r11 = r8 + DCTFIXPOINTADJUST(C_4 * r6);
	          r8 = r5 + r7;
	          
	          r4 = -r9;
	          r5 = r10 - r9;
	          r6 = r10 + r11;
	          r7 = r11 + r8;
	          
	      		x = DCTFIXPOINTADJUST(r0 + r7); //[0]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r0 - r7); //[7]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 7*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r1 + r6); //[1]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r1 - r6); //[6]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 6*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r2 + r5); //[2]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + 2*X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r2 - r5); //[5]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 5*X + n) = (pel)y;
	      		x = DCTFIXPOINTADJUST(r3 + r4); //[3]
	      		if( x == -1 )
	            x = 0; /*Small error -512/512 = 0*/
	          *(TL + 3*X + n) = (pel)x;
	      		y = DCTFIXPOINTADJUST(r3 - r4); //[4]
	      		if( y == -1 )
	            y = 0; /*Small error -512/512 = 0*/
	          *(TL + 4*X + n) = (pel)y;
          }/*end for n....*/

      }/*end for j...*/
    }/*end for i...*/

  }/*end for col...*/
}/*end IDct2DnoQ.*/

////////// Colour Space conversions ///////////////////

#define Y_MAX    (colfixedpoint)( 8191) /*(0.99999999 * FxPt_float) */
#define UV_MAX   (colfixedpoint)( 4096) /*(0.5 * FxPt_float)        */
#define UV_MIN   (colfixedpoint)(-4096) /*(-0.5 * FxPt_float)       */

#define R_Y_0    (colfixedpoint)( 2449) /*(0.299 * FxPt_float)  */
#define R_Y_1    (colfixedpoint)( 4809) /*(0.587 * FxPt_float)  */
#define R_Y_2    (colfixedpoint)(  934) /*(0.114 * FxPt_float)  */
#define R_Y_3    (colfixedpoint)( 3572) /*(0.436 * FxPt_float)  */
#define R_Y_4    (colfixedpoint)(-1204) /*(-0.147 * FxPt_float) */
#define R_Y_5    (colfixedpoint)(-2367) /*(-0.289 * FxPt_float) */
#define R_Y_6    (colfixedpoint)( 5038) /*(0.615 * FxPt_float)  */
#define R_Y_7    (colfixedpoint)(-4219) /*(-0.515 * FxPt_float) */
#define R_Y_8    (colfixedpoint)( -819) /*(-0.100 * FxPt_float) */

#define Y_R_0    (colfixedpoint)( 9339) /*(1.140 * FxPt_float)   */
#define Y_R_1    (colfixedpoint)(-3228) /*(-0.394 * FxPt_float)  */
#define Y_R_2    (colfixedpoint)(-4756) /*(-0.581 * FxPt_float)  */
#define Y_R_3    (colfixedpoint)(16646) /*(2.032 * FxPt_float)   */
#define FXROUND  (colfixedpoint)( 4096) /*(0.5 * FxPt_float)     */
#define CFXROUND (colfixedpoint)(   16) /*(1 << 4)     */

int CH263CODEC::RGB24toYUV411(void *pRGB,int RGB_X,int RGB_Y,
                             pel *pY,pel *pU,pel *pV)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* Y1 & Y2 have range 0..0.999,*/
	/* U & V have range -0.5..0.5.*/
	unsigned char *S;
	register int xb,yb,XB,YB;
  int xpix;
	pel u,v;
	colfixedpoint r,g,b;

	xpix = RGB_X;
	S = (unsigned char *)(pRGB);
	/*4 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y >> 1;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
    int C_Off = yb*XB + xb;
    int Y_Off = (yb*xpix + xb) << 1;
    unsigned char *T = S + Y_Off*3;

		/*Top left pix.  255->0.999.*/
		b = CFIXPOINT((colfixedpoint)( *(T) ));
		g = CFIXPOINT((colfixedpoint)( *(T + 1) ));
		r = CFIXPOINT((colfixedpoint)( *(T + 2) ));
    /*    RGBtoYUV(r,g,b,y,accu,accv);  */
		pY[Y_Off] = (pel)(CFIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b) + CFXROUND));

		u = (pel)(CFIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g) + CFXROUND));
		v = (pel)(CFIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b) + CFXROUND));

		/*Top right pix.*/
		b = CFIXPOINT((colfixedpoint)( *(T + 3) ));
		g = CFIXPOINT((colfixedpoint)( *(T + 4) ));
		r = CFIXPOINT((colfixedpoint)( *(T + 5) ));
    /*    RGBtoYUV(r,g,b,y,u,v);  */
		pY[Y_Off+1] = (pel)(CFIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b) + CFXROUND));

		u += (pel)(CFIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g) + CFXROUND));
		v += (pel)(CFIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b) + CFXROUND));

    Y_Off += xpix;
    T += xpix*3;
		/*Bottom left pix.  255->0.999.*/
		b = CFIXPOINT((colfixedpoint)( *(T) ));
		g = CFIXPOINT((colfixedpoint)( *(T + 1) ));
		r = CFIXPOINT((colfixedpoint)( *(T + 2) ));
    /*    RGBtoYUV(r,g,b,y,accu,accv);  */
		pY[Y_Off] = (pel)(CFIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b) + CFXROUND));

		u += (pel)(CFIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g) + CFXROUND));
		v += (pel)(CFIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b) + CFXROUND));

		/*Bottom right pix. */
		b = CFIXPOINT((colfixedpoint)( *(T + 3) ));
		g = CFIXPOINT((colfixedpoint)( *(T + 4) ));
		r = CFIXPOINT((colfixedpoint)( *(T + 5) ));
    /*    RGBtoYUV(r,g,b,y,u,v); */
		pY[Y_Off+1] = (pel)(CFIXPOINTADJUST((long)(R_Y_0*r) + (long)(R_Y_1*g) + (long)(R_Y_2*b) + CFXROUND));

		pU[C_Off] = (u + (pel)(CFIXPOINTADJUST((long)(R_Y_3*b) + (long)(R_Y_4*r) + (long)(R_Y_5*g) + CFXROUND))) >> 2;
		pV[C_Off] = (v + (pel)(CFIXPOINTADJUST((long)(R_Y_6*r) + (long)(R_Y_7*g) + (long)(R_Y_8*b) + CFXROUND))) >> 2;
 	}/*end for xb & yb...*/

  return(1);
}/*end RGB24toYUV411.*/

int CH263CODEC::YUV411toRGB24(pel *pY,pel *pU,pel *pV,
                             void *pRGB,int RGB_X,int RGB_Y)
{
  if(!CodecIsOpen)
  {
    ErrorStr = "Codec is not open!";
    return(0);
  }/*end if !CodecIsOpen...*/

	/* R, G & B have range 0..255,*/
	unsigned char *D;
	register int xb,yb,XB,YB;
	int xpix;
  int r,b,g;
	colfixedpoint y,u,v;

	xpix = RGB_X;
	D = (unsigned char *)(pRGB);
	/*4 pix per Block.*/
	XB = xpix >> 1;
	YB = RGB_Y >> 1;

	for(yb = 0; yb < YB; yb++)
   for(xb = 0; xb < XB; xb++)
	{
    int Y_Off = (yb*xpix + xb)<<1;
    int C_Off = yb*(xpix>>1) + xb;
    unsigned char *T = D + (yb*xpix + xb)*6;
		u = CFIXPOINT(pU[C_Off]);
		v = CFIXPOINT(pV[C_Off]);

		/*Top left pix.*/
		y = CFIXPOINT(pY[Y_Off]);
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_0*v))) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_3*u))) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 1) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 2) = (unsigned char)r;

		/*Top right pix. */
		y = CFIXPOINT(pY[Y_Off + 1]);
    /*    YUVtoRGB(y,u,v,r,g,b); */
		r = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_0*v))) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_3*u))) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T + 3) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 4) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 5) = (unsigned char)r;

    Y_Off += xpix;
    T += xpix*3;
		/*Bottom left pix.*/
		y = CFIXPOINT(pY[Y_Off]);
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_0*v))) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_3*u))) << 8) + FXROUND);
		if( b > 255 ) b = 255;
		if( b < 0 ) b = 0;
		*(T) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 1) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 2) = (unsigned char)r;

		/*Bottom right pix.*/
		y = CFIXPOINT(pY[Y_Off + 1]);
    /*    YUVtoRGB(y,u,v,r,g,b);   */
		r = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_0*v))) << 8) + FXROUND);
		g = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_1*u) + (long)(Y_R_2*v))) << 8) + FXROUND);
		b = FIXPOINTADJUST(((long)(y + FIXPOINTADJUST((long)(Y_R_3*u))) << 8) + FXROUND);
		if( b > 255 ) b = 255;                       
		if( b < 0 ) b = 0;
		*(T + 3) = (unsigned char)b;
		if( g > 255 ) g = 255;
		if( g < 0 ) g = 0;
		*(T + 4) = (unsigned char)g;
		if( r > 255 ) r = 255;
		if( r < 0 ) r = 0;
		*(T + 5) = (unsigned char)r;

	}/*end for xb & yb...*/

  return(1);
}/*end YUV411toRGB24.*/

/* Motion estimation and compensation. */
#define TopLeftHalfPelOffsetLen  3
static COORD TopLeftHalfPelOffset[TopLeftHalfPelOffsetLen] = {{1,0},{0,1},{1,1}};
#define TopHalfPelOffsetLen  5
static COORD TopHalfPelOffset[TopHalfPelOffsetLen] = {{-1,0},{1,0},{-1,1},{0,1},{1,1}};
#define LeftHalfPelOffsetLen  5
static COORD LeftHalfPelOffset[LeftHalfPelOffsetLen] = {{0,-1},{0,1},{1,0},{1,-1},{1,1}};
#define HalfPelOffsetLen  8
static COORD HalfPelOffset[HalfPelOffsetLen] = {{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}};

int CH263CODEC::MotionEstimationAndCompensation(pel *locY,pel *locU,pel *locV)
{
  int seq,seq_length;
  int index,i,j,x,y;
  pel *InTL;
  pel *crTL;
  pel *rTL;
  pel *crTL_U;
  pel *rTL_U;
  pel *crTL_V;
  pel *rTL_V;
  COORD *pYBlk;
  COORD *pUVBlk;
  int *BoundaryType;
  int COL_MOTION_BLK_SIZE;
  int bestdiff;
  int zerodiff;
  int weight = 80;

  if(MOTION_BLK_SIZE == 8)
  {
    seq_length = DctXBlks_176_8*DctYBlks_144_8;
    pYBlk = DctBlk_176x144_8x8;
    pUVBlk = ColourBlk_88x72_4x4;
    BoundaryType = BoundryType_176x144_8x8;
  }/*end if MOTION_BLK_SIZE...*/
  else /*if(MOTION_BLK_SIZE == 16) */
  {
    seq_length = MotionXBlks_176_16*MotionYBlks_144_16;
    pYBlk = MotionBlk_176x144_16x16;
    pUVBlk = DctBlk_88x72_8x8;
    BoundaryType = BoundryType_176x144_16x16;
  }/*end else...*/
  COL_MOTION_BLK_SIZE = MOTION_BLK_SIZE/2;

  for(seq = 0; seq < seq_length; seq++)
  {
    index = Y_X*pYBlk[seq].Y + pYBlk[seq].X;
    InTL = locY + index;
    crTL = crY + index;
    rTL = rY + index;

    index = UV_X*pUVBlk[seq].Y + pUVBlk[seq].X;
    crTL_U = crU + index;
    rTL_U = rU + index;
    crTL_V = crV + index;
    rTL_V = rV + index;

    /*Boundaries are treated uniquely.*/
    if(BoundaryType[seq] == 0) /* inner pels.*/
    {
      /*Full pel search first.*/
      int best_y = 0;
      int best_x = 0;
      /*Start with [0,0] motion vector dominance and give the zero */
      /*vector a premium.*/
      bestdiff =  0;
      for(y = 0; y < MOTION_BLK_SIZE; y++)
      {
        int k = Y_X*y;
        pel *locIn = (pel *)(InTL + k);
        pel *locR = (pel *)(rTL + k);
        for(x = 0; x < MOTION_BLK_SIZE; x++)
        {
          int diff = *(locIn++) - *(locR++);
          if(diff < 0)
            bestdiff -= diff;
          else
            bestdiff += diff;
        }/*end for x...*/
      }/*end for y...*/
      zerodiff = bestdiff;
      /*Now do all other locations.*/
      for(i = -8; i < 8; i++)  /*Vector range = -8 ... +7 */
      {
        for(j = -8; j < 8; j++)
        {
          /* The 16x16/8x8 block comparison.*/
          int absdiff = 0;
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locIn = (pel *)(InTL + Y_X*y);
            pel *locR = (pel *)(rTL + Y_X*(y+i) + j);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              int diff = *(locIn++) - *(locR++);
              if(diff < 0)
                absdiff -= diff;
              else
                absdiff += diff;
            }/*end for x...*/
          }/*end for y...*/
          if(absdiff < bestdiff)
          {
            bestdiff = absdiff;
            best_y = i;
            best_x = j;
          }/*end if absdiff...*/
        }/*end for j...*/
      }/*end for i...*/
      if( bestdiff > ((weight*zerodiff)/100) )
      {
        bestdiff = zerodiff;
        best_y = 0;
        best_x = 0;
      }/*end if bestdiff...*/

      /*Do half pel estimation by searching around the full pel location.*/
      if(PS.Motion == 2)
      {
        /*Note: bestdiff is carried in.*/
        int best_half_y = 0;
        int best_half_x = 0;
        int off_seq,off_seq_len;
        COORD *HalfPelOff;

        if((best_y == -8)&&(best_x == -8))
        {
          off_seq_len = TopLeftHalfPelOffsetLen;
          HalfPelOff = TopLeftHalfPelOffset;
        }/*end if best_y...*/
        else if (best_y == -8)
        {
          off_seq_len = TopHalfPelOffsetLen;
          HalfPelOff = TopHalfPelOffset;
        }/*end else if best_y...*/
        else if (best_x == -8)
        {
          off_seq_len = LeftHalfPelOffsetLen;
          HalfPelOff = LeftHalfPelOffset;
        }/*end else if best_x...*/
        else
        {
          off_seq_len = HalfPelOffsetLen;
          HalfPelOff = HalfPelOffset;
        }/*end else...*/

        for(off_seq = 0; off_seq < off_seq_len; off_seq++)
        {
          /* The 8x8 block comparison.*/
          i = HalfPelOff[off_seq].Y;
          j = HalfPelOff[off_seq].X;
          int absdiff = 0;
          /*Make this decision an outer loop for speed.*/
          if(i && j)
          {
            for(y = 0; y < MOTION_BLK_SIZE; y++)
            {
              pel *locIn = (pel *)(InTL + Y_X*y);
              int l = Y_X*(best_y+y) + best_x;
              pel *locR1 = (pel *)(rTL + l);
              pel *locR2 = (pel *)(rTL + l + j);
              int m = Y_X*(best_y+y+i) + best_x;
              pel *locR3 = (pel *)(rTL + m);
              pel *locR4 = (pel *)(rTL + m + j);
              for(x = 0; x < MOTION_BLK_SIZE; x++)
              {
                int half_pel = (*(locR1++) + *(locR2++) +
                                *(locR3++) + *(locR4++) + 2) >> 2;
                int diff = *locIn++ - half_pel;
                if(diff < 0)
                  absdiff -= diff;
                else
                  absdiff += diff;
              }/*end for x...*/
            }/*end for y...*/
          }/*end if i...*/
          else
          {
            for(y = 0; y < MOTION_BLK_SIZE; y++)
            {
              pel *locIn = (pel *)(InTL + Y_X*y);
              int l = best_y + y;
              pel *locR1 = (pel *)(rTL + Y_X*l + best_x);
              pel *locR2 = (pel *)(rTL + Y_X*(l+i) + best_x + j);
              for(x = 0; x < MOTION_BLK_SIZE; x++)
              {
                int half_pel = (*(locR1++) + *(locR2++) + 1) >> 1;
                int diff = *locIn++ - half_pel;
                if(diff < 0)
                  absdiff -= diff;
                else
                  absdiff += diff;
              }/*end for x...*/
            }/*end for y...*/
          }/*end else...*/
          if(absdiff < bestdiff)
          {
            bestdiff = absdiff;
            best_half_y = i;
            best_half_x = j;
          }/*end if absdiff...*/
        }/*end for off_seq...*/
        best_x = (best_x << 1) + best_half_x;
        best_y = (best_y << 1) + best_half_y;
      }/*end if Motion...*/

      /*Load best MVD.*/
      pMVD[seq].x = best_x;
      pMVD[seq].y = best_y;

      /* Compensate to compensated reference space.*/
      if(PS.Motion == 1) /*Full pel compensation.*/
      {
        /*Lum compensation.*/
        for(y = 0; y < MOTION_BLK_SIZE; y++)
        {
          pel *locCR = (pel *)(crTL + Y_X*y);
          pel *locR = (pel *)(rTL + Y_X*(best_y+y) + best_x);
          for(x = 0; x < MOTION_BLK_SIZE; x++)
          {
            *(locCR++) = *(locR++);
          }/*end for x...*/
        }/*end for y...*/
      }/*end if Motion...*/
      else /*Half pel compensation.*/
      {
        int half_x = best_x % 2;
        int half_y = best_y % 2;
        best_x /= 2;
        best_y /= 2;
        if(half_x && half_y)
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = Y_X*(best_y+y) + best_x;
            pel *locR1 = (pel *)(rTL + l);
            pel *locR2 = (pel *)(rTL + l + half_x);
            int m = Y_X*(best_y+half_y+y) + best_x;
            pel *locR3 = (pel *)(rTL + m);
            pel *locR4 = (pel *)(rTL + m + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locR1++) + *(locR2++) +
                            *(locR3++) + *(locR4++) + 2) >> 2;
            }/*end for x...*/
          }/*end for y...*/
        }/*end if half_x...*/
        else
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            pel *locR1 = (pel *)(rTL + Y_X*(best_y+y) + best_x);
            pel *locR2 = (pel *)(rTL + Y_X*(best_y+y+half_y) + best_x + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locR1++) + *(locR2++) + 1) >> 1;
            }/*end for x...*/
          }/*end for y...*/
        }/*end else...*/
      }/*end else...*/

      /*Chr compensation.*/
      /*Round down the motion vectors.*/
      best_x /= 2;
      best_y /= 2;
      for(y = 0; y < COL_MOTION_BLK_SIZE; y++)
      {
        int k = UV_X*y;
        pel *locCRU = (pel *)(crTL_U + k);
        pel *locCRV = (pel *)(crTL_V + k);
        int l = UV_X*(best_y+y) + best_x;
        pel *locRU = (pel *)(rTL_U + l);
        pel *locRV = (pel *)(rTL_V + l);
        for(x = 0; x < COL_MOTION_BLK_SIZE; x++)
        {
          *(locCRU++) = *(locRU++);
          *(locCRV++) = *(locRV++);
        }/*end for x...*/
      }/*end for y...*/
    }/*end if ...*/
    else /*if(BoundryType[seq] == others)*/ /* left right and corner blocks.*/
    {
      /*Construct a search window.*/
      y = pYBlk[seq].Y;
      x = pYBlk[seq].X;
      int rnglim = MOTION_BLK_SIZE + 8;
      int winx = MOTION_BLK_SIZE + 16;
      for(i = -8; i < rnglim; i++)  /*Pel range = -8 ... (8+8)or(16+8) */
      {
        int pel_y = y + i;
        if(pel_y >= Y_Y)
          pel_y = Y_Y-1;
        else if(pel_y < 0)
          pel_y = 0;
        pel *locSW = (pel *)(SrchWin + winx*(i+8) + 8);
        pel *locR = (pel *)(rY + Y_X*pel_y);  /*NB: Image pels not offset.*/
        for(j = -8; j < rnglim; j++)
        {
          int pel_x = x + j;
          if(pel_x >= Y_X)
            pel_x = Y_X-1;
          else if(pel_x < 0)
            pel_x = 0;
          /*Load the actual pel.*/
          *(locSW + j) = *(locR + pel_x);
        }/*end for j...*/
      }/*end for i...*/

      /*Full pel search from the search window first.*/
      int best_y = 0;
      int best_x = 0;
      /*Start with [0,0] motion vector dominance and give the zero */
      /*vector a premium.*/
      bestdiff =  0;
      for(y = 0; y < MOTION_BLK_SIZE; y++)
      {
        pel *locIn = (pel *)(InTL + Y_X*y);
        pel *locSW = (pel *)(SrchWin + winx*(y+8) + 8);
        for(x = 0; x < MOTION_BLK_SIZE; x++)
        {
          int diff = *(locIn++) - *(locSW++);
          if(diff < 0)
            bestdiff -= diff;
          else
            bestdiff += diff;
        }/*end for x...*/
      }/*end for y...*/
      zerodiff = bestdiff;
      /*Now do all other locations.*/
      for(i = -8; i < 8; i++)  /*Vector range = -8 ... +7 */
      {
        for(j = -8; j < 8; j++)
        {
          /* The 8x8 block comparison.*/
          int absdiff = 0;
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locIn = (pel *)(InTL + Y_X*y);
            pel *locSW = (pel *)(SrchWin + winx*(8+i+y) + 8+j);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              int diff = *(locIn++) - *(locSW++);
              if(diff < 0)
                absdiff -= diff;
              else
                absdiff += diff;
            }/*end for x...*/
          }/*end for y...*/
          if(absdiff < bestdiff)
          {
            bestdiff = absdiff;
            best_y = i;
            best_x = j;
          }/*end if absdiff...*/
        }/*end for j...*/
      }/*end for i...*/
      if( bestdiff > ((weight*zerodiff)/100) )
      {
        bestdiff = zerodiff;
        best_y = 0;
        best_x = 0;
      }/*end if bestdiff...*/

      /*Do half pel estimation by searching around the full pel location.*/
      if(PS.Motion == 2)
      {
        /*Note: bestdiff is carried in.*/
        int best_half_y = 0;
        int best_half_x = 0;
        int off_seq,off_seq_len;
        COORD *HalfPelOff;

        if((best_y == -8)&&(best_x == -8))
        {
          off_seq_len = TopLeftHalfPelOffsetLen;
          HalfPelOff = TopLeftHalfPelOffset;
        }/*end if best_y...*/
        else if (best_y == -8)
        {
          off_seq_len = TopHalfPelOffsetLen;
          HalfPelOff = TopHalfPelOffset;
        }/*end else if best_y...*/
        else if (best_x == -8)
        {
          off_seq_len = LeftHalfPelOffsetLen;
          HalfPelOff = LeftHalfPelOffset;
        }/*end else if best_x...*/
        else
        {
          off_seq_len = HalfPelOffsetLen;
          HalfPelOff = HalfPelOffset;
        }/*end else...*/

        for(off_seq = 0; off_seq < off_seq_len; off_seq++)
        {
          /* The 8x8 block comparison.*/
          i = HalfPelOff[off_seq].Y;
          j = HalfPelOff[off_seq].X;
          int absdiff = 0;
          /*Make this decision an outer loop for speed.*/
          if(i && j)
          {
            for(y = 0; y < MOTION_BLK_SIZE; y++)
            {
              pel *locIn = (pel *)(InTL + Y_X*y);
              int l = winx*(8+best_y+y) + best_x+8;
              pel *locSW1 = (pel *)(SrchWin + l);
              pel *locSW2 = (pel *)(SrchWin + l + j);
              int m = l + (winx*i);
              pel *locSW3 = (pel *)(SrchWin + m);
              pel *locSW4 = (pel *)(SrchWin + m + j);
              for(x = 0; x < MOTION_BLK_SIZE; x++)
              {
                int half_pel = (*(locSW1++) + *(locSW2++) +
                                *(locSW3++) + *(locSW4++) + 2) >> 2;
                int diff = *(locIn++) - half_pel;
                if(diff < 0)
                  absdiff -= diff;
                else
                  absdiff += diff;
              }/*end for x...*/
            }/*end for y...*/
          }/*end if i...*/
          else
          {
            for(y = 0; y < MOTION_BLK_SIZE; y++)
            {
              pel *locIn = (pel *)(InTL + Y_X*y);
              int l = winx*(8+best_y+y) + best_x+8;
              pel *locSW1 = (pel *)(SrchWin + l);
              pel *locSW2 = (pel *)(SrchWin + l + winx*i + j);
              for(x = 0; x < MOTION_BLK_SIZE; x++)
              {
                int half_pel = (*(locSW1++) + *(locSW2++) + 1) >> 1;
                int diff = *(locIn++) - half_pel;
                if(diff < 0)
                  absdiff -= diff;
                else
                  absdiff += diff;
              }/*end for x...*/
            }/*end for y...*/
          }/*end else...*/
          if(absdiff < bestdiff)
          {
            bestdiff = absdiff;
            best_half_y = i;
            best_half_x = j;
          }/*end if absdiff...*/
        }/*end for off_seq...*/
        best_x = (best_x << 1) + best_half_x;
        best_y = (best_y << 1) + best_half_y;
      }/*end if Motion...*/

      /*Load best MVD.*/
      pMVD[seq].x = best_x;
      pMVD[seq].y = best_y;

      /* Compensate to compensated reference space.*/
      if(PS.Motion == 1) /*Full pel compensation.*/
      {
        /*Lum compensation.*/
        for(y = 0; y < MOTION_BLK_SIZE; y++)
        {
          pel *locCR = (pel *)(crTL + Y_X*y);
          pel *locSW = (pel *)(SrchWin + winx*(8+best_y+y) + best_x+8);
          for(x = 0; x < MOTION_BLK_SIZE; x++)
          {
            *(locCR++) = *(locSW++);
          }/*end for x...*/
        }/*end for y...*/
      }/*end if Motion...*/
      else /*Half pel compensation.*/
      {
        int half_x = best_x % 2;
        int half_y = best_y % 2;
        best_x /= 2;
        best_y /= 2;
        if(half_x && half_y)
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = winx*(8+best_y+y) + best_x+8;
            pel *locSW1 = (pel *)(SrchWin + l);
            pel *locSW2 = (pel *)(SrchWin + l + half_x);
            int m = l + (winx*half_y);
            pel *locSW3 = (pel *)(SrchWin + m);
            pel *locSW4 = (pel *)(SrchWin + m + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locSW1++) + *(locSW2++) +
                            *(locSW3++) + *(locSW4++) + 2) >> 2;
            }/*end for x...*/
          }/*end for y...*/
        }/*end if half_x...*/
        else
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = winx*(8+best_y+y) + best_x+8;
            pel *locSW1 = (pel *)(SrchWin + l);
            pel *locSW2 = (pel *)(SrchWin + l + winx*half_y + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locSW1++) + *(locSW2++) + 1) >> 1;
            }/*end for x...*/
          }/*end for y...*/
        }/*end else...*/
      }/*end else...*/

      /*Chr compensation.*/
      /*Round down the motion vectors.*/
      i = pUVBlk[seq].Y;
      j = pUVBlk[seq].X;
      best_x /= 2;
      best_y /= 2;
      for(y = 0; y < COL_MOTION_BLK_SIZE; y++)
      {
        int k = UV_X*y;
        pel *locCRU = (pel *)(crTL_U + k);
        pel *locCRV = (pel *)(crTL_V + k);
        int pel_y = best_y + y + i;
        if(pel_y >= UV_Y)
          pel_y = UV_Y-1;
        else if(pel_y < 0)
          pel_y = 0;
        int l = UV_X*pel_y;
        pel *locRU = (pel *)(rU + l);
        pel *locRV = (pel *)(rV + l);
        for(x = 0; x < COL_MOTION_BLK_SIZE; x++)
        {
          int pel_x = best_x + x + j;
          if(pel_x >= UV_X)
            pel_x = UV_X-1;
          else if(pel_x < 0)
            pel_x = 0;
          *(locCRU++) = *(locRU + pel_x);
          *(locCRV++) = *(locRV + pel_x);
        }/*end for x...*/
      }/*end for y...*/

    }/*end else BoundryType...*/

  }/*end for seq...*/

  return(1);
}/*end MotionEstimationAndCompensation.*/

int CH263CODEC::MotionCompensation(void)
{
  int seq,seq_length;
  int index,i,j,x,y;
  pel *crTL;
  pel *rTL;
  pel *crTL_U;
  pel *rTL_U;
  pel *crTL_V;
  pel *rTL_V;
  COORD *pYBlk;
  COORD *pUVBlk;
  int *BoundaryType;
  int COL_MOTION_BLK_SIZE;

  if(MOTION_BLK_SIZE == 8)
  {
    seq_length = DctXBlks_176_8*DctYBlks_144_8;
    pYBlk = DctBlk_176x144_8x8;
    pUVBlk = ColourBlk_88x72_4x4;
    BoundaryType = BoundryType_176x144_8x8;
  }/*end if MOTION_BLK_SIZE...*/
  else /*if(MOTION_BLK_SIZE == 16) */
  {
    seq_length = MotionXBlks_176_16*MotionYBlks_144_16;
    pYBlk = MotionBlk_176x144_16x16;
    pUVBlk = DctBlk_88x72_8x8;
    BoundaryType = BoundryType_176x144_16x16;
  }/*end else...*/
  COL_MOTION_BLK_SIZE = MOTION_BLK_SIZE/2;

  for(seq = 0; seq < seq_length; seq++)
  {
    index = Y_X*pYBlk[seq].Y + pYBlk[seq].X;
    crTL = crY + index;
    rTL = rY + index;

    index = UV_X*pUVBlk[seq].Y + pUVBlk[seq].X;
    crTL_U = crU + index;
    rTL_U = rU + index;
    crTL_V = crV + index;
    rTL_V = rV + index;

    /*Boundaries are treated uniquely.*/
    if(BoundaryType[seq] == 0) /* inner pels.*/
    {
      /*Load best MVD.*/
      int best_x = pMVD[seq].x;
      int best_y = pMVD[seq].y;

      /* Compensate to compensated reference space.*/
      if(PS.Motion == 1) /*Full pel compensation.*/
      {
        /*Lum compensation.*/
        for(y = 0; y < MOTION_BLK_SIZE; y++)
        {
          pel *locCR = (pel *)(crTL + Y_X*y);
          pel *locR = (pel *)(rTL + Y_X*(best_y+y) + best_x);
          for(x = 0; x < MOTION_BLK_SIZE; x++)
          {
            *(locCR++) = *(locR++);
          }/*end for x...*/
        }/*end for y...*/
      }/*end if Motion...*/
      else /*Half pel compensation.*/
      {
        int half_x = best_x % 2;
        int half_y = best_y % 2;
        best_x /= 2;
        best_y /= 2;
        if(half_x && half_y)
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = Y_X*(best_y+y) + best_x;
            pel *locR1 = (pel *)(rTL + l);
            pel *locR2 = (pel *)(rTL + l + half_x);
            int m = Y_X*(best_y+half_y+y) + best_x;
            pel *locR3 = (pel *)(rTL + m);
            pel *locR4 = (pel *)(rTL + m + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locR1++) + *(locR2++) +
                            *(locR3++) + *(locR4++) + 2) >> 2;
            }/*end for x...*/
          }/*end for y...*/
        }/*end if half_x...*/
        else
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            pel *locR1 = (pel *)(rTL + Y_X*(best_y+y) + best_x);
            pel *locR2 = (pel *)(rTL + Y_X*(best_y+y+half_y) + best_x + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locR1++) + *(locR2++) + 1) >> 1;
            }/*end for x...*/
          }/*end for y...*/
        }/*end else...*/
      }/*end else...*/

      /*Chr compensation.*/
      /*Round down the motion vectors.*/
      best_x /= 2;
      best_y /= 2;
      for(y = 0; y < COL_MOTION_BLK_SIZE; y++)
      {
        int k = UV_X*y;
        pel *locCRU = (pel *)(crTL_U + k);
        pel *locCRV = (pel *)(crTL_V + k);
        int l = UV_X*(best_y+y) + best_x;
        pel *locRU = (pel *)(rTL_U + l);
        pel *locRV = (pel *)(rTL_V + l);
        for(x = 0; x < COL_MOTION_BLK_SIZE; x++)
        {
          *(locCRU++) = *(locRU++);
          *(locCRV++) = *(locRV++);
        }/*end for x...*/
      }/*end for y...*/
    }/*end if ...*/
    else /*if(BoundryType[seq] == others)*/ /* left right and corner blocks.*/
    {
      /*Construct a search window.*/
      y = pYBlk[seq].Y;
      x = pYBlk[seq].X;
      int rnglim = MOTION_BLK_SIZE + 8;
      int winx = MOTION_BLK_SIZE + 16;
      for(i = -8; i < rnglim; i++)  /*Pel range = -8 ... (8+8)or(16+8) */
      {
        int pel_y = y + i;
        if(pel_y >= Y_Y)
          pel_y = Y_Y-1;
        else if(pel_y < 0)
          pel_y = 0;
        pel *locSW = (pel *)(SrchWin + winx*(i+8) + 8);
        pel *locR = (pel *)(rY + Y_X*pel_y);  /*NB: Image pels not offset.*/
        for(j = -8; j < rnglim; j++)
        {
          int pel_x = x + j;
          if(pel_x >= Y_X)
            pel_x = Y_X-1;
          else if(pel_x < 0)
            pel_x = 0;
          /*Load the actual pel.*/
          *(locSW + j) = *(locR + pel_x);
        }/*end for j...*/
      }/*end for i...*/

      /*Load best MVD.*/
      int best_x = pMVD[seq].x;
      int best_y = pMVD[seq].y;

      /* Compensate to compensated reference space.*/
      if(PS.Motion == 1) /*Full pel compensation.*/
      {
        /*Lum compensation.*/
        for(y = 0; y < MOTION_BLK_SIZE; y++)
        {
          pel *locCR = (pel *)(crTL + Y_X*y);
          pel *locSW = (pel *)(SrchWin + winx*(8+best_y+y) + best_x+8);
          for(x = 0; x < MOTION_BLK_SIZE; x++)
          {
            *(locCR++) = *(locSW++);
          }/*end for x...*/
        }/*end for y...*/
      }/*end if Motion...*/
      else /*Half pel compensation.*/
      {
        int half_x = best_x % 2;
        int half_y = best_y % 2;
        best_x /= 2;
        best_y /= 2;
        if(half_x && half_y)
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = winx*(8+best_y+y) + best_x+8;
            pel *locSW1 = (pel *)(SrchWin + l);
            pel *locSW2 = (pel *)(SrchWin + l + half_x);
            int m = l + (winx*half_y);
            pel *locSW3 = (pel *)(SrchWin + m);
            pel *locSW4 = (pel *)(SrchWin + m + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locSW1++) + *(locSW2++) +
                            *(locSW3++) + *(locSW4++) + 2) >> 2;
            }/*end for x...*/
          }/*end for y...*/
        }/*end if half_x...*/
        else
        {
          /*Lum compensation.*/
          for(y = 0; y < MOTION_BLK_SIZE; y++)
          {
            pel *locCR = (pel *)(crTL + Y_X*y);
            int l = winx*(8+best_y+y) + best_x+8;
            pel *locSW1 = (pel *)(SrchWin + l);
            pel *locSW2 = (pel *)(SrchWin + l + winx*half_y + half_x);
            for(x = 0; x < MOTION_BLK_SIZE; x++)
            {
              *(locCR++) = (*(locSW1++) + *(locSW2++) + 1) >> 1;
            }/*end for x...*/
          }/*end for y...*/
        }/*end else...*/
      }/*end else...*/

      /*Chr compensation.*/
      /*Round down the motion vectors.*/
      i = pUVBlk[seq].Y;
      j = pUVBlk[seq].X;
      best_x /= 2;
      best_y /= 2;
      for(y = 0; y < COL_MOTION_BLK_SIZE; y++)
      {
        int k = UV_X*y;
        pel *locCRU = (pel *)(crTL_U + k);
        pel *locCRV = (pel *)(crTL_V + k);
        int pel_y = best_y + y + i;
        if(pel_y >= UV_Y)
          pel_y = UV_Y-1;
        else if(pel_y < 0)
          pel_y = 0;
        int l = UV_X*pel_y;
        pel *locRU = (pel *)(rU + l);
        pel *locRV = (pel *)(rV + l);
        for(x = 0; x < COL_MOTION_BLK_SIZE; x++)
        {
          int pel_x = best_x + x + j;
          if(pel_x >= UV_X)
            pel_x = UV_X-1;
          else if(pel_x < 0)
            pel_x = 0;
          *(locCRU++) = *(locRU + pel_x);
          *(locCRV++) = *(locRV + pel_x);
        }/*end for x...*/
      }/*end for y...*/

    }/*end else BoundryType...*/

  }/*end for seq...*/

  return(1);
}/*end MotionCompensation.*/

int CH263CODEC::CodePartitions(pel *locY,pel *locU,pel *locV)
{
  int blk,run;
  pel *Img;
  int TotBlks,X;
  COORD *pBlk;
  pel coeff,level,DCPred;
  int pnum;

  /*Step thru each partition and code across all 8x8 blocks */
  /*at the coeff offset in Base_coeff_pos.*/
  for(pnum = 0; pnum < PARTITIONS; pnum++)
  {
    if(Partition[pnum].Colour == Col_Y)
    {
      Img = locY;
      pBlk = DctBlk_176x144_8x8;
      TotBlks = Y_XBlk * Y_YBlk;
      X = Y_X;
    }/*end if Colour...*/
    else
    {
      if(Partition[pnum].Colour == Col_U)
      {
        Img = locU;
      }/*end if Colour...*/
      else /* Colour == Col_V */
      {
        Img = locV;
      }/*end if Colour...*/
      pBlk = DctBlk_88x72_8x8;
      TotBlks = UV_XBlk * UV_YBlk;
      X = UV_X;
    }/*end else...*/

    /*Prepare the partition variables.*/
    Partition[pnum].structs_coded = 0;
    Partition[pnum].structs_pos = 0;
    Partition[pnum].COD = 0;
    Partition[pnum].done = 0;

    /*Code one coeff from each block.*/
    run = 0;
    DCPred = 0;
    for(blk = 0; blk < TotBlks; blk++)
    {
      coeff = *((pel *)(Img + X*(pBlk[blk].Y + Partition[pnum].Base_coeff_pos.Y) + 
                                (pBlk[blk].X + Partition[pnum].Base_coeff_pos.X)));
      /*DC coeffs are treated differently.*/
      if(Partition[pnum].Base_coeff_pos.Y || Partition[pnum].Base_coeff_pos.X)
      {/*AC coeff.*/
        if(coeff == 0)
          run++;
        else
        {
          Partition[pnum].pCodes[Partition[pnum].structs_coded].last = 0;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].run = run;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].level = coeff;
          Partition[pnum].structs_coded++;
          run = 0;
        }/*end else...*/
      }/*end if ...*/
      else
      {/*DC coeff.*/
        level = coeff - DCPred;
        /*Limit values to table and ensure correct reconstruction of ref image.*/
        if(level > 127)
          level = 127;
        if(level < -127)
          level = -127;

        if(level == 0)
          run++;
        else
        {
          Partition[pnum].pCodes[Partition[pnum].structs_coded].last = 0;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].run = run;
          Partition[pnum].pCodes[Partition[pnum].structs_coded].level = level;
          Partition[pnum].structs_coded++;
          run = 0;
        }/*end else...*/
        DCPred = level + DCPred;
      }/*end else...*/
    }/*end for blk...*/

    /*At least one valid code.*/
    if(Partition[pnum].structs_coded)
    {
      Partition[pnum].COD = 1;
      Partition[pnum].pCodes[Partition[pnum].structs_coded - 1].last = 1;
    }/*end if structs_coded...*/

  }/*end for pnum...*/

  return(1);
}/*end CodePartitons.*/

int CH263CODEC::DecodePartitions(pel *locY, pel *locU, pel *locV)
{
  int blk,run;
  pel *Img;
  int TotBlks,X;
  COORD *pBlk;
  pel coeff,level,DCPred;
  int pnum;

  /*Step thru each partition and decode across all 8x8 blocks */
  /*at the coeff offset in Base_coeff_pos.*/
  for(pnum = 0; pnum < PARTITIONS; pnum++)
  {
    if(Partition[pnum].Colour == Col_Y)
    {
      Img = locY;
      pBlk = DctBlk_176x144_8x8;
      TotBlks = Y_XBlk * Y_YBlk;
      X = Y_X;
    }/*end if Colour...*/
    else
    {
      if(Partition[pnum].Colour == Col_U)
      {
        Img = locU;
      }/*end if Colour...*/
      else /* Colour == Col_V */
      {
        Img = locV;
      }/*end if Colour...*/
      pBlk = DctBlk_88x72_8x8;
      TotBlks = UV_XBlk * UV_YBlk;
      X = UV_X;
    }/*end else...*/

    /*Prepare the partition variables.*/
    Partition[pnum].structs_pos = 0;
    run = Partition[pnum].pCodes[Partition[pnum].structs_pos].run;

    /*Decode one coeff from each block.*/
    DCPred = 0;
    for(blk = 0; blk < TotBlks; blk++)
    {
      coeff = 0;
      if(Partition[pnum].COD && (Partition[pnum].structs_pos < Partition[pnum].structs_coded))
      {
        /*DC coeffs are treated differently.*/
        if(Partition[pnum].Base_coeff_pos.Y || Partition[pnum].Base_coeff_pos.X)
        {/*AC coeff.*/
          if(run)
            run--;
          else
          {
            coeff = Partition[pnum].pCodes[Partition[pnum].structs_pos].level;
            Partition[pnum].structs_pos++;
            run = Partition[pnum].pCodes[Partition[pnum].structs_pos].run;
          }/*end else...*/
        }/*end if ...*/
        else
        {/*DC coeff.*/
          coeff = DCPred;
          if(run)
            run--;
          else
          {
            level = Partition[pnum].pCodes[Partition[pnum].structs_pos].level;
            coeff = level + DCPred;
            DCPred = coeff;
            Partition[pnum].structs_pos++;
            run = Partition[pnum].pCodes[Partition[pnum].structs_pos].run;
          }/*end else...*/
        }/*end else...*/
      }/*end if COD...*/
      *((pel *)(Img + X*(pBlk[blk].Y + Partition[pnum].Base_coeff_pos.Y) + 
                        (pBlk[blk].X + Partition[pnum].Base_coeff_pos.X))) = coeff;
    }/*end for blk...*/

  }/*end for pnum...*/

  return(1);
}/*end DecodePartitons.*/

/*-------------------------------------------------------------------*/
/* Add BitsToAdd number of bits from the LSB of CodeBits, to the 32  */
/* bit word pointed to by CurrPtr from bit position NextBitPos up    */
/* towards the MSB.                                                  */       
/*-------------------------------------------------------------------*/
unsigned int *CH263CODEC::PutBits(unsigned int *CurrPtr,
												 int *NextBitPos,int BitsToAdd,
                         int CodeBits)
{
  int pos;
	int b,i;

  pos = *NextBitPos;
  b = CodeBits;

  for(i = BitsToAdd; i > 0; i--)
  {
    /* Strip out the LSB bit and write it to bit position pos.*/
    if(b & 1)
      *CurrPtr = *CurrPtr | (1 << pos);
    else
      *CurrPtr = *CurrPtr & ~(1 << pos);

    /* Point to next available bit.*/
    if(pos == 31)
    {
      pos = 0;
      CurrPtr++;
    }/*end if pos...*/
    else
      pos++;

    /* Update the bits still to add.*/
    b = b >> 1;
  }/*end for i...*/

  /* Update the global next bit position.*/
  *NextBitPos = pos;

  return(CurrPtr); /* Success.*/
}/*end PutBits.*/

/*-------------------------------------------------------------------*/
/* Get the next BitsToGet number of bits and return it in CodeBits.  */
/* This function is for multiple bit accesses.                       */
/*-------------------------------------------------------------------*/
unsigned int *CH263CODEC::GetBits(unsigned int *CurrPtr,
													int *NextBitPos,int BitsToGet,
                          int *CodeBits)
{
  int pos;
	int b,i;

  pos = *NextBitPos;
  b = 0;

  for(i = 0; i < BitsToGet; i++)
  {
    /* Strip out the next bit and update in the bit position i.*/
    if((*CurrPtr >> pos) & 1)
      b = (int)(b | (1 << i)); /*bit=1*/
    else
      b = (int)(b & ~(1 << i)); /*bit=0*/

    /* Point to next available bit.*/
    if(pos == 31)
    {
      pos = 0;
      CurrPtr++;
    }/*end if pos...*/
    else
      pos++;
  }/*end for i...*/

  /* Update the global next bit position.*/
  *NextBitPos = pos;
  /* Output the result.*/
  *CodeBits = b;

  return(CurrPtr); /* Success.*/
}/*end GetBits.*/

/*----------------------------------------------------------------------*/
/* Get the next bit and inc the bit counter.                            */
/*----------------------------------------------------------------------*/
unsigned int *CH263CODEC::GetNextBit(unsigned int *CurrPtr,
												 int *NextBitPos,int *CodeBit)
{
  int pos;

  pos = *NextBitPos;

  /* Strip out the bit. */
  *CodeBit = (int)(*CurrPtr >> pos) & 1;

  /* Point to next available bit.*/
  if(pos == 31)
  {
    pos = 0;
    CurrPtr++;
  }/*end if pos...*/
  else
    pos++;

  /* Update the global next bit position.*/
  *NextBitPos = pos;

  return(CurrPtr); /* Success.*/
}/*end GetNextBit.*/

/* Get the vlc code from the table and add the sign bit.*/
int CH263CODEC::GetMotionVecBits(int VecCoord,int *CdeWord)
{
  int abs_motion,bits;
  int code;

  if(VecCoord < 0)
    abs_motion = -(VecCoord);
  else
    abs_motion = VecCoord;

  if( abs_motion >= CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE )
  {
    ErrorStr = "Motion vector out of vlc table range!";
    return(0);
  }/*end if abs_motion...*/

  bits = CH263CODEC_MotionVlcTable[abs_motion].numbits;
  code = CH263CODEC_MotionVlcTable[abs_motion].bits;
  /*Add the sign bit to MSB if not zero.*/
  if(VecCoord < 0)
  {
    code = (1 << bits) | code; /*-ve.*/
    bits++;
  }/*end if VecCoord...*/
  else if(VecCoord > 0)
  {
    bits++;  /*+ve. zero sign bit is implicit.*/
  }/*end else if VecCoord...*/

  *CdeWord = code;
  return(bits);
}/*end GetMotionVecBits.*/

/*------------------------------------------------------------*/
/* Motion vector coordinate bit extraction from bit stream.*/
/*------------------------------------------------------------*/
unsigned int *CH263CODEC::ExtractMotionVecBits(unsigned int *CurrPtr,
												  int *NextBitPos,int *Vec,int *BitsExtracted,
                          int *Codeword)
{
  int bit;
  int bits;
  int tbl_pos,tbl_size;
  int Found;
  int bits_so_far,bits_needed;

  /* Decode the binary tree to determine the motion vector coord.*/
  /* The table must be in ascending bit length order. Bits */
  /* are extracted from the bit stream depending on the next */
  /* no. of bits in the table. Keep going through the table */
  /* until a match is found or the table ends. */
  bits = 0;
  tbl_pos = 0;
  bits_so_far = 0;
  bits_needed = 0;
  Found = 0;
  tbl_size = CH263CODEC_MOTIONVEC_VLC_TABLE_SIZE;

  while( (tbl_pos < tbl_size) && !Found )
  {
    bits_needed = CH263CODEC_MotionVlcTable[tbl_pos].numbits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (CH263CODEC_MotionVlcTable[tbl_pos].numbits == bits_so_far) && 
           (tbl_pos < tbl_size) && !Found )
    {
      if(CH263CODEC_MotionVlcTable[tbl_pos].bits == bits)
        Found = 1;
      else
        tbl_pos++;
    }/*end while numbits...*/
  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *Vec = 0;
    *BitsExtracted = 0; /* Implies an error. */
    *Codeword = bits;
		ErrorStr = "Non-existant motion vector Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  /*Extract sign bit.*/
  if(tbl_pos != 0)
  {
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far++;
    if(bit) /*1 => -ve.*/
    {
      tbl_pos = -(tbl_pos);
    }/*end if bit...*/
  }/*end if tbl_pos...*/

  *Vec = tbl_pos;
  *BitsExtracted = bits_so_far;
  *Codeword = bits;
  return(CurrPtr);
}/*end ExtractMotionVecBits.*/

/* Get the appropriate last,run,level vlc from the table.*/
int CH263CODEC::GetRunLevelBits(int Last,int Run,int Level,int *CdeWord)
{
  int abs_level,bits;
  int code,pos;

  if(Level < 0)
    abs_level = -(Level);
  else
    abs_level = Level;

  /*Test if input variables warrant a look at the table.*/
  if((!Last && (abs_level < 13)) || (Last && (abs_level < 4)))
  {
    for(pos = 0; pos < (CH263CODEC_COEFF_VLC_TABLE_SIZE-2); pos++) /*Exclude markers.*/
    {
      if(CH263CODEC_CoeffVlcTable[pos].last == Last)
      {
        if(CH263CODEC_CoeffVlcTable[pos].run == Run)
        {
          if(CH263CODEC_CoeffVlcTable[pos].level == abs_level)
          {
            bits = CH263CODEC_CoeffVlcTable[pos].numbits;
            code = CH263CODEC_CoeffVlcTable[pos].bits;
            /*Add the sign bit to MSB if not zero.*/
            if(Level < 0)
            {
              code = (1 << bits) | code; /*-ve.*/
              bits++;
            }/*end if Level...*/
            else /* if(Level > 0)... Level == 0 does not exist.*/
            {
              bits++;  /*+ve. zero sign bit is implicit.*/
            }/*end else if Level...*/
            *CdeWord = code;
            return(bits);
          }/*end if level...*/
        }/*end if run...*/
      }/*end if last...*/
    }/*end for pos...*/
  }/*end if ...*/

  /* If get this far, then ESC sequence required.*/
  *CdeWord = ( ( ( ( ((Level & CH263CODEC_ESC_LEVEL_MASK) << CH263CODEC_ESC_RUN_BITS) 
             |   (Run & CH263CODEC_ESC_RUN_MASK) ) << 1 )
             |   (Last & 0x00000001) ) << CH263CODEC_ESC_BITS )
             |   CH263CODEC_ESC_CODE;
  return(CH263CODEC_ESC_LEVEL_BITS + CH263CODEC_ESC_RUN_BITS + 1 + CH263CODEC_ESC_BITS);
}/*end GetRunLevelBits.*/

/*------------------------------------------------------------*/
/* Run-length and last bit extraction from bit stream.*/
/*------------------------------------------------------------*/
unsigned int *CH263CODEC::ExtractRunLevelBits(unsigned int *CurrPtr,int *NextBitPos,
                          int *Last,int *Run,int *Level,int *Marker,
                          int *BitsExtracted,int *Codeword)
{
  int bit;
  int bits;
  int tbl_pos,tbl_size;
  int Found;
  int bits_so_far,bits_needed;

  /* Decode the binary tree to determine the last,run,level values.*/
  /* The table must be in ascending bit length order. Bits */
  /* are extracted from the bit stream depending on the next */
  /* no. of bits in the table. Keep going through the table */
  /* until a match is found or the table ends. */
  bits = 0;
  tbl_pos = 0;
  bits_so_far = 0;
  bits_needed = 0;
  Found = 0;
  tbl_size = CH263CODEC_COEFF_VLC_TABLE_SIZE;

  while( (tbl_pos < tbl_size) && !Found )
  {
    bits_needed = CH263CODEC_CoeffVlcTable[tbl_pos].numbits - bits_so_far;
    /*Get the bits off the bit stream.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,bits_needed,&bit);
    bits = bits | (bit << bits_so_far);
    bits_so_far += bits_needed;

    /*Advance down the table checking the codes with the current no. of bits so far.*/
    while( (CH263CODEC_CoeffVlcTable[tbl_pos].numbits == bits_so_far) && 
           (tbl_pos < tbl_size) && !Found )
    {
      if(CH263CODEC_CoeffVlcTable[tbl_pos].bits == bits)
        Found = 1;
      else
        tbl_pos++;
    }/*end while numbits...*/
  }/*end while tbl_pos...*/

  /*If not found then there is an error.*/
  if( !Found )
  {
    *Last = 0;
    *Run = 0;
    *Level = 0;
    *Marker = 0;
    *BitsExtracted = 0; /* Implies an error. */
    *Codeword = bits;
		ErrorStr = "Non-existant motion vector Huffman code!";
    return(CurrPtr);
  }/*end if !Found...*/

  /*At this point there are 3 possibilities, ESC sequence, EOP and EOI marker.*/
  int last = CH263CODEC_CoeffVlcTable[tbl_pos].last;
  int run = CH263CODEC_CoeffVlcTable[tbl_pos].run;
  int level = CH263CODEC_CoeffVlcTable[tbl_pos].level;
//  char level = (char)(CH263CODEC_CoeffVlcTable[tbl_pos].level);

  if( (last == 0)||(last == 1) )
  {
    /*Extract sign bit.*/
    if(level != 0)
    {
      CurrPtr = GetNextBit(CurrPtr,NextBitPos,&bit);
      bits = bits | (bit << bits_so_far);
      bits_so_far++;
      if(bit) /*1 => -ve.*/
      {
        level = -(level);
      }/*end if bit...*/
    }/*end if level...*/
    *Marker = 0;
  }/*end if last...*/
  else if(last == 2) /*ESC sequence.*/
  {
    /*Next bit is last bit.*/
    CurrPtr = GetNextBit(CurrPtr,NextBitPos,&last);
    bits = bits | (last << bits_so_far);
    bits_so_far++;

    /*Next are run bits.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,CH263CODEC_ESC_RUN_BITS,&run);
    bits = bits | (run << bits_so_far);
    bits_so_far += CH263CODEC_ESC_RUN_BITS;

    /*Next are level bits.*/
    CurrPtr = GetBits(CurrPtr,NextBitPos,CH263CODEC_ESC_LEVEL_BITS,&level);
    bits = bits | (level << bits_so_far);
    bits_so_far += CH263CODEC_ESC_LEVEL_BITS;
//    level = (char)(bit);
    int t = (1 << (CH263CODEC_ESC_LEVEL_BITS-1));
    if(level > (t-1))
      level = level | (~CH263CODEC_ESC_LEVEL_MASK);

    *Marker = 0;
  }/*end else if last...*/
  else if(last == 3) /*EOP marker.*/
  {
    *Marker = 3;
  }/*end else if last...*/
  else /*EOI marker.*/
  {
    *Marker = 4;
  }/*end else...*/

  *Last = last;
  *Run = run;
  *Level = level;
//  *Level = (int)(level);
  *BitsExtracted = bits_so_far;
  *Codeword = bits;
  return(CurrPtr);
}/*end ExtractRunLevelBits.*/

int CH263CODEC::CreatePartitions(void)
{
  int size;

  /*3 partitions for each block coeff position.*/
  for(int b = 0; b < BLOCK_SIZE; b++)
  {
    int pbase = NUM_OF_COLOUR_COMP * b;
    /*Lum.*/
    Partition[pbase].Colour = Col_Y;
    Partition[pbase].Base_coeff_pos = ZZ_Order[b]; 
    Partition[pbase].structs_coded = 0;
    Partition[pbase].structs_pos = 0;
    Partition[pbase].COD = 0;
    Partition[pbase].done = 0;
    Partition[pbase].hCodes = NULL;
    Partition[pbase].pCodes = NULL;
    Partition[pbase].CODE_STRUCT_SIZE = 0;
    size = Y_XBlk * Y_YBlk * sizeof(H263_VLC_TYPE);
		Partition[pbase].hCodes = GlobalAlloc(GMEM_FIXED,size);
		if(!Partition[pbase].hCodes)
    {
      ErrorStr = "Luminance code memory unavailable!";
      DeletePartitions();
		  return(0);
    }/*end if !hCodes...*/
		Partition[pbase].pCodes = (H263_VLC_TYPE *)GlobalLock(Partition[pbase].hCodes);
    Partition[pbase].CODE_STRUCT_SIZE = Y_XBlk * Y_YBlk;

    /*U Chr.*/
    Partition[pbase+1].Colour = Col_U;
    Partition[pbase+1].Base_coeff_pos = ZZ_Order[b];
    Partition[pbase+1].structs_coded = 0;
    Partition[pbase+1].structs_pos = 0;
    Partition[pbase+1].COD = 0;
    Partition[pbase+1].done = 0;
    Partition[pbase+1].hCodes = NULL;
    Partition[pbase+1].pCodes = NULL;
    Partition[pbase+1].CODE_STRUCT_SIZE = 0;
    size = UV_XBlk * UV_YBlk * sizeof(H263_VLC_TYPE);
		Partition[pbase+1].hCodes = GlobalAlloc(GMEM_FIXED,size);
		if(!Partition[pbase+1].hCodes)
    {
      ErrorStr = "U Chrominance code memory unavailable!";
      DeletePartitions();
		  return(0);
    }/*end if !hCodes...*/
		Partition[pbase+1].pCodes = (H263_VLC_TYPE *)GlobalLock(Partition[pbase+1].hCodes);
    Partition[pbase+1].CODE_STRUCT_SIZE = UV_XBlk * UV_YBlk;

    /*V Chr.*/
    Partition[pbase+2].Colour = Col_V;
    Partition[pbase+2].Base_coeff_pos = ZZ_Order[b];
    Partition[pbase+2].structs_coded = 0;
    Partition[pbase+2].structs_pos = 0;
    Partition[pbase+2].COD = 0;
    Partition[pbase+2].done = 0;
    Partition[pbase+2].hCodes = NULL;
    Partition[pbase+2].pCodes = NULL;
    Partition[pbase+2].CODE_STRUCT_SIZE = 0;
    size = UV_XBlk * UV_YBlk * sizeof(H263_VLC_TYPE);
		Partition[pbase+2].hCodes = GlobalAlloc(GMEM_FIXED,size);
		if(!Partition[pbase+2].hCodes)
    {
      ErrorStr = "V Chrominance code memory unavailable!";
      DeletePartitions();
		  return(0);
    }/*end if !hCodes...*/
		Partition[pbase+2].pCodes = (H263_VLC_TYPE *)GlobalLock(Partition[pbase+2].hCodes);
    Partition[pbase+2].CODE_STRUCT_SIZE = UV_XBlk * UV_YBlk;

  }/*end for b...*/

  return(1);
}/*end CreatePartitions.*/

void CH263CODEC::DeletePartitions(void)
{
  /*Partition structures.*/
  for(int i = 0; i < PARTITIONS; i++)
  {
    if(Partition[i].hCodes != NULL)
    {
  		GlobalUnlock(Partition[i].hCodes);
 	  	GlobalFree(Partition[i].hCodes);
 		  Partition[i].hCodes = NULL;
    }/*end if hCodes...*/
    Partition[i].pCodes = NULL;
  }/*end for i...*/

}/*end DeletePartitions.*/


