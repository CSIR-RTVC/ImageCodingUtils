/** @file

MODULE				: Fifo

TAG						: FIFO

FILE NAME			: Fifo.h

DESCRIPTION		: A class to hold a fifo buffer in the form of an array with type double. 
                Add to the head, shuffle down and remove from the tail. Keep a running sum
                of the fifo contents as this is a commonly required function.

COPYRIGHT			: (c)CSIR 2007-2016 all rights resevered

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
#ifndef _FIFO_H
#define _FIFO_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class Fifo
{
public:
  Fifo(void) { _pBuff = NULL; _length = 0; _empty = 1; _sum = 0.0; }
  virtual ~Fifo() { Destroy(); }

// Setup and interface methods.
public:
  int		Create(int length) { Destroy(); _pBuff = new double[length]; if(_pBuff == 0) return(0); else {_length = length; return(1);} }
  void	Destroy(void) { if(_pBuff != NULL) {delete[] _pBuff; _pBuff = NULL;} _length = 0; _empty = 1; _sum = 0.0; }

  int     GetLength(void) { return(_length); }
  double* GetBuffer(void) { return(_pBuff); }
  int     Empty(void)     { return(_empty); }
  void    Clear(void) { if(_pBuff) {for(int i = 0; i < _length; i++) _pBuff[i] = 0.0; _empty = 1; _sum = 0.0;} }
  void    MarkAsEmpty(void) { _empty = 1; }
  double  GetSum(void)    { return(_sum); }

  /// Add to the start of the array and shuffle down. The last item is discarded. Destructive call that changes the array.
  /// If the fifo is empty then fill it with the input param value. Update the sum of the contents.
  void AddFirstIn(double x) 
  { 
    if(_pBuff) 
    {
      if(!_empty)  
      {
        _sum -= _pBuff[_length-1];  ///< Subtract the item about to be discarded.
        for(int i = (_length-1); i > 0; i--) 
          _pBuff[i] = _pBuff[i-1]; 
        _pBuff[0] = x;
        _sum += x;  ///< Add the newest item.
      }//end if !_empty 
      else         
      {
        _sum = x * (double)_length;
        for (int i = 0; i < _length; i++)
          _pBuff[i] = x;
        _empty = 0;
      }//end else...
    }//end if _pBuff...
  }//end AddFirstIn.

  /// Get from the end of the array. Non-destructive call. 
  double  GetFirstOut(void) {if(_pBuff) {return(_pBuff[_length-1]);} else {return(0);} }
  double  GetItem(int loc) { return(_pBuff[loc]); }

// Common members.
protected:
  int       _empty;
	int				_length;
	double*		_pBuff;
  double    _sum;

};// end class Fifo.

#endif	// _FIFO_H
