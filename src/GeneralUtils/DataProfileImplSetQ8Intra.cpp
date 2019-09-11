/** @file

MODULE						: DataProfileImplSetQ8Intra

TAG								: DPISQ8I

FILE NAME					: DataProfileImplSetQ8Intra.cpp

DESCRIPTION				: A class to hold a profile (array) of numbers that 
										represents an arbitrary function with a max and min
										bound of some selectable period. This implementation 
										represents the bit rate of H.263+ Advanced Intra mode
										operating at QP=8 with every frame set to an I-frame.
										It implements the more general IDataProfile interface.

REVISION HISTORY	:

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <math.h>
#include <stdlib.h>
#include "DataProfileImplSetQ8Intra.h"

/// Profile for H.263+ Advanced Intra mode at QP=8 for the 'bear' clip
/// set to 176x144 @ 10fps and all frames coded as INTRA.
#define DPISQ8I_PROFILE_LEN 305
const int DataProfileImplSetQ8Intra::_pProfile[DPISQ8I_PROFILE_LEN] = 
{
	11976,	12680,	12632,	12704,	12784,	12792,	12784,	12680,	12744,	12816,
	12856,	12944,	12872,	12880,	12904,	12936,	12928,	12968,	13000,	12904,
	12912,	12984,	12280,	12456,	12872,	11824,	11512,	11464,	11584,	12640,
	12544,	12728,	13144,	12632,	12400,	12752,	12600,	12528,	12024,	12224,
	12064,	11688,	11872,	12120,	12688,	12720,	12952,	13360,	13816,	13960,
	13824,	13704,	13840,	14024,	13552,	13096,	14024,	14008,	13840,	13904,
	13568,	13840,	14232,	13712,	13736,	13720,	13496,	14080,	13864,	14448,
	14160,	14128,	14504,	14440,	14160,	14296,	14568,	14920,	14912,	14904,
	14704,	14424,	14280,	14616,	14728,	15016,	14832,	14720,	14552,	14752,
	14416,	14224,	14848,	14992,	14384,	13880,	13976,	13176,	13160,	13128,	//< 100
	12976,	13320,	13632,	13560,	13264,	12632,	12320,	12600,	12784,	12480,
	12080,	12232,	12488,	12360,	12368,	12824,	11896,	12952,	13440,	13680,
	14576,	14376,	14960,	14712,	14728,	15512,	15448,	15080,	15768,	15792,
	14648,	14176,	14808,	15144,	15416,	15024,	15576,	15600,	16016,	15632,
	15176,	15296,	15440,	15536,	15760,	15824,	15656,	15616,	15656,	15608,
	14992,	15112,	15112,	14896,	14824,	15120,	15136,	15712,	15592,	15752,
	16088,	15768,	15864,	15904,	15032,	15192,	15304,	15768,	15912,	15208,
	16088,	15776,	15832,	16784,	16784,	16560,	16600,	15712,	16464,	16816,
	16840,	16696,	16536,	16160,	16208,	16528,	16400,	16616,	16584,	16688,
	16712,	16696,	16880,	15984,	15376,	15200,	14936,	15024,	14888,	14976,	///< 200
	14952,	14712,	14888,	14456,	15584,	15864,	15632,	15664,	15984,	15824,
	15864,	14984,	15328,	15472,	14864,	14696,	15208,	14888,	15128,	15648,
	16224,	16208,	15320,	15136,	16016,	15920,	14904,	15088,	15432,	15864,	
	15512,	15800,	16168,	16040,	15824,	16088,	15984,	16400,	16584,	15592,
	16120,	15264,	15856,	15480,	15264,	15616,	15816,	15744,	15872,	15816,
	15024,	15032,	14816,	15224,	14896,	14672,	15064,	14544,	14520,	13912,
	14208,	15128,	14624,	15088,	15384,	15768,	21376,	21624,	21680,	21864,
	22032,	22000,	21848,	22032,	22320,	22376,	22448,	22336,	22032,	22112,
	22216,	22328,	22424,	22464,	22168,	22144,	22248,	22224,	22360,	22360,
	22328,	22064,	22320,	22280,	22336,	22416,	22472,	22264,	22520,	22488,	///< 300
	23000,	23000,	23000,	23000,	23000
};

DataProfileImplSetQ8Intra::DataProfileImplSetQ8Intra()
{
}//end constructor.

DataProfileImplSetQ8Intra::~DataProfileImplSetQ8Intra()
{
	Destroy();
}//end destructor.

int	DataProfileImplSetQ8Intra::Create(int period, int min, int max)
{
	if(!DataProfileImplBase::CreateMem(DPISQ8I_PROFILE_LEN))
		return(0);

	for(int i = 0; i < DPISQ8I_PROFILE_LEN; i++)
		_pSample[i]	= _pProfile[i] + 16;

	_nextPos					= 0;
	_maxSampleVal			= 0;
	_minSampleVal			= 0;

	return(1);
}//end Create.


