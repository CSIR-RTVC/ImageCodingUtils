//****************************************************************
// This function sets up a CSAMPLER object with a set of image
// files generated from the Video.exe program.
// FILE NAME: ImgSmp.h
// AUTHOR   : K.L. Ferguson
// DATE     : October 1999
//****************************************************************
#ifndef _IMGSMP_H
#define _IMGSMP_H

#include <stdio.h>
#include <math.h>

#include "sampler.h"

// A set of video difference images.
#define NUMIMAGES 48

extern int SetImgSampler(CSAMPLER *Sp,int lev,int inxdim,int inydim,
                         int colcomp,int dirsubband);

#endif
