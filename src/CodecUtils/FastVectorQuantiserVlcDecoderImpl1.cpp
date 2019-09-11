/** @file

MODULE						: FastVectorQuantiserVlcDecoderImpl1

TAG								: FVQVDI1

FILE NAME					: FastVectorQuantiserVlcDecoderImpl1.cpp

DESCRIPTION				: A fast vector quantiser Vlc decoder implementation with an
										IVlcDecoder Interface and derived from VectorQuantiserVlcDecoder.

REVISION HISTORY	:
									: 

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

#include "FastVectorQuantiserVlcDecoderImpl1.h"

#define FVQVDI1_TREE_SIZE	511

FastVectorQuantiserVlcDecoderImpl1::FastVectorQuantiserVlcDecoderImpl1() : VectorQuantiserVlcDecoder()
{
	_onesTree		= NULL;
	_zeroTree		= NULL;
	_indexTree	= NULL;

	Create();
}//end constructor.

FastVectorQuantiserVlcDecoderImpl1::~FastVectorQuantiserVlcDecoderImpl1()
{
	Destroy();
}//end destructor.

int FastVectorQuantiserVlcDecoderImpl1::Create(void)
{
	int i,j;
	int tableLen = VQVD_TABLE_SIZE;
	
	// Alloc mem for the fast huffman decode binary tree. 
	// Note: Only deleted in Destroy().
	_onesTree		= new int[FVQVDI1_TREE_SIZE];
	_zeroTree		= new int[FVQVDI1_TREE_SIZE];
	_indexTree	= new int[FVQVDI1_TREE_SIZE];
	
	if(!_onesTree || !_zeroTree || !_indexTree)
	  return(0);

	for(i = 0; i < FVQVDI1_TREE_SIZE; i++)
	{
		_onesTree[i]	= -1;
		_zeroTree[i]	= -1;
		_indexTree[i] = -1;
	}//end for i...

	// Loop through all of the run codes
	int treeNextIndex = 1;
	for (i = 0; i < tableLen; ++i)
	{
		// Get the code and depth (number of bits) for the current Huffman code
		int code	= VLC_TABLE[i][VQVD_BIT_CODE];
		int depth = VLC_TABLE[i][VQVD_NUM_BITS];
		
		// Create the path to the current code
		int cIndex = 0;
		for(j = 0; j < depth; j++)
		{
			// Get the next bit
			int bit = code & (1 << j);
			
			// Branch and allocate new tree member if necessary
			if(bit) // bit 1
			{
				if (_onesTree[cIndex] == -1)
					_onesTree[cIndex] = treeNextIndex++;
				
				cIndex = _onesTree[cIndex];
			}//end if bit...
			else // bit 0
			{
				if (_zeroTree[cIndex] == -1)
					_zeroTree[cIndex] = treeNextIndex++;
					
				cIndex = _zeroTree[cIndex];
			}//end else...

			if (j == depth-1)
				_indexTree[cIndex] = i;

		}//end for j...
	}//end for i...

	return(1);
}//end Create.

void FastVectorQuantiserVlcDecoderImpl1::Destroy(void)
{
	if(_onesTree != NULL)
		delete[] _onesTree;
	_onesTree = NULL;

	if(_zeroTree != NULL)
		delete[] _zeroTree;
	_zeroTree = NULL;

	if(_indexTree != NULL)
		delete[] _indexTree;
	_indexTree = NULL;

}//end Destroy();

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
int FastVectorQuantiserVlcDecoderImpl1::Decode(IBitStreamReader* bsr)
{
	int tblPos		= 0;
	int found			= 0;
	int index			= 0;

	while(1)
	{
		// Get the current bit
		int bit = bsr->Read();

		// Branch 
		if(bit) // bit 1
		{
			//if (_onesTree[index] == -1)
			//	break;	// Failed.
			//index = _onesTree[index];

			// Aid branch prediction by most likely being true.
			if (_onesTree[index] != -1)
				index = _onesTree[index];
			else
				break;

		}//end if bit...
		else // bit 0
		{
			//if (_zeroTree[index] == -1)
			//	break;	// Failed.
			//index = _zeroTree[index];

			if (_zeroTree[index] != -1)
				index = _zeroTree[index];
			else
				break;

		}//end else...

		if (_indexTree[index] != -1)
		{
			tblPos = _indexTree[index];
			found = 1;
			break;
		}//end if _indexTree...

	}//end while 1...

  // If not found then there is an error.
  if( !found )
  {
    _numCodeBits	= 0; // Implies an error. 
    return(0);
  }//end if !found...

  _numCodeBits = VLC_TABLE[tblPos][VQVD_NUM_BITS];
  return(tblPos);
}//end Decode.

