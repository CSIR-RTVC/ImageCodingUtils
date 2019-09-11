//**********************************************************
// TITLE       :RUN-LENGTH CODEC CLASS FUNCTIONS
// VERSION     :1.0
// FILE        :Rlcodec.cpp
// DESCRIPTION :A class for implementing a run-length encoder
//              and bit estimator for a sub block of data
//              within an image. This is a derived class from 
//              the abstract class of type CODEC.
// DATE        :February 1998
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"
#include "stdio.h"

#include "Rlcodec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////
// Public Implementations.

CRLCODEC::CRLCODEC()
{
  CurrParams.ImgX = 0;
  CurrParams.ImgY = 0;
  CurrParams.WinX = 0;
  CurrParams.WinY = 0;
  CurrParams.Direction = HORIZONTAL;
  CurrParams.Differential = FALSE;
  CurrParams.BitAllocTable = H261;
  IncX = TRUE;
  IncY = TRUE;

  MakeRLStat = FALSE;
  DataFilename = "Rlstat.dat";
  ClearStatTables();
}//end CRLCODEC Constructor.

CRLCODEC::~CRLCODEC()
{
}//end CRLCODEC Destructor.

void CRLCODEC::SetParameters(void *Params)
{
  RL_PARAMTYPE *P;
  P = (RL_PARAMTYPE *)Params;

  //Set current set parameters.
  CurrParams = *P;

}//end SetParameters.

int CRLCODEC::OpenCODEC(void *Params)
{
  SetParameters(Params);
  return(1);
}//end OpenCODEC.

int CRLCODEC::OpenCODEC(void)
{
  return(1);
}//end OpenCODEC.

// The parameter will be used to return the bit estimation
// for this CODE implementation.
int CRLCODEC::CODE(void *Img)
{
  int acc;
  int *I;
  I = (int *)Img;

  int i,j; //i,j represents the current location.
  int Pixels;
  int prev,t;
  int run,value;
  int NumBits,Code;

  if(MakeRLStat)
  {
    ClearStatTables();
  }//end if MakeRLStat...

  acc = 0;
  prev = 0;
  Pixels = CurrParams.WinX * CurrParams.WinY;
  if(CurrParams.BitAllocTable == FLAT_8BPP)
    return(Pixels * 8);
  if(CurrParams.BitAllocTable == FLAT_9BPP)
    return(Pixels * 9);
  i = 0;  //Start in the corner.
  j = 0;
  run = 0;
  for(int k = 0; k < Pixels; k++)
  {
    t = GetNext(&i,&j,I);
    if(CurrParams.Differential)
    {
      t = t - prev;
      prev = t + prev;
    }//end if Differential...
    if(t == 0)
      run++;
    else
    {
      value = t;
      //Find the code bits associated with this run-length
      //and value combination.
      FindBits(run,value,&NumBits,&Code);
      if(MakeRLStat)
      {
        PutRLStat(run,value);
      }//end if MakeRLStat...
      acc += NumBits;
      //Reset run.
      run = 0;
    }//end else...
  }//end for k...

  if(MakeRLStat)
  {
    FileStat(DataFilename);
  }//end if MakeRLStat...

  //Put EOB code bits.
  acc += GetEOBBits();
  return(acc);
}//end CODE.

int CRLCODEC::DECODE(void *Data)
{
  error = 1; //RUN-LENGTH DECODER NOT IMPLEMENTED.
  return(0);
}//end DECODE.

void CRLCODEC::CloseCODEC(void)
{
  CurrParams.ImgX = 0;
  CurrParams.ImgY = 0;
  CurrParams.WinX = 0;
  CurrParams.WinY = 0;
}//end CloseCODEC.

const char *CRLCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													 //0
     "RUN-LENGTH DECODER NOT IMPLEMENTED"};//1

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

////////////////////////////////////////////////////////
// Protected utility functions.
////////////////////////////////////////////////////////

void CRLCODEC::ClearStatTables(void)
{
  int i,j;
  for(i = 0; i < RUNLENGTHS; i++)
   for(j = 0; j < RUNLENGTH_VALUES; j++)
   {
    Stat[i*RUNLENGTH_VALUES + j].Run = i;
    Stat[i*RUNLENGTH_VALUES + j].Value = j;
    Stat[i*RUNLENGTH_VALUES + j].Occurrence = 0;
   }//end for j...
  int Base = RUNLENGTHS*RUNLENGTH_VALUES;
  for(i = 0; i < EXTRA_1_VALUES; i++)
   {
    Stat[Base + i].Run = RUNLENGTHS + i;
    Stat[Base + i].Value = 1;
    Stat[Base + i].Occurrence = 0;
   }//end for i...
  for(i = 0; i < RUNMISS_TABLE_SIZE; i++)
  {
    RunMiss[i] = 0;
  }//end for i...
}//end ClearStatTables.

void CRLCODEC::PutRLStat(int Run,int Value)
{
  //Integer absolute.
  if( Value < 0)
    Value = -Value;

  //Parse table with Run-length and value.
  unsigned int i;
  i = 0;
  BOOL Found;
  Found = FALSE;
  while( (i < STAT_TABLE_SIZE)&&(!Found) )
  {
    //Does run-length match?
    if( Run == Stat[i].Run )
    {
      //Does value match?
      if( Value == Stat[i].Value )
      {
        Stat[i].Occurrence++;
        Found = TRUE;
      }//end if Value...
    }//end if Run...
    i++;
  }//end while i...
  //Run-length and value combination was not in table.
  if( !Found )
  {
    if( Run < (RUNLENGTHS + EXTRA_1_VALUES) )
      RunMiss[Run]++;
    else
      RunMiss[RUNLENGTHS + EXTRA_1_VALUES]++;
  }//end if !Found...
}//end PutRLStat.

void CRLCODEC::FileStat(CString &Filename)
{
  int i;
	CStdioFile Fl;
	CString Line;
	int CheckFile;
	CheckFile = Fl.Open(Filename,CStdioFile::modeCreate | 
		CStdioFile::modeWrite | CStdioFile::typeText);

  if(!CheckFile)
    return;

  //Store Statistic table.
  Line = "Run-length Statistic Table\n";
	Fl.WriteString(Line);
  Line = "Run,Level,Occurrence\n";
	Fl.WriteString(Line);
  for(i = 0; i < STAT_TABLE_SIZE; i++)
  {
		Line = "";
    Line.Format(_T("%d,%d,%d\n"),
      Stat[i].Run,Stat[i].Value,Stat[i].Occurrence);
		Fl.WriteString(Line);
  }//end for i...
  Line = "\nTable Misses\n";
	Fl.WriteString(Line);
  Line = "Run,Misses\n";
	Fl.WriteString(Line);
  for(i = 0; i < (RUNLENGTHS + EXTRA_1_VALUES); i++)
  {
		Line = "";
    Line.Format(_T("%d,%d\n"),i,RunMiss[i]);
		Fl.WriteString(Line);
  }//end for i...
	Line = "";
  Line.Format(_T("Other,%d\n"),RunMiss[i]);
	Fl.WriteString(Line);

  Fl.Close();
}//end FileStat...

// Depending on the direction, get the next value at
// location i,j in the windowed sub-image y,x. Advance
// the i,j location to the next valid one depending on
// the direction.
int CRLCODEC::GetNext(int *i,int *j,int *Base)
{
  int X,x,y;
  int Val;

  if( (*i == 0)&&(*j == 0) )
  {
    IncX = TRUE;
    IncY = TRUE;
  }//end if *i...

  X = CurrParams.ImgX;
  x = CurrParams.WinX;
  y = CurrParams.WinY;
  Val = *(Base + (*i)*X + (*j));
  if(CurrParams.Direction == HORIZONTAL) //Always IncY.
  {
    //Moving right condition.
    if( IncX )
    {
      //Last right pixel condition.
      if( (*i == (y-1)) && (*j == (x-1)) )
      {
        *i = 0;
        *j = 0;
      }//end if...
      //Right end of line condition.
      else if( *j == (x-1) )
      {
        *i = *i + 1; //Next line.
        IncX = FALSE; //Move backwards.
      }//end else if *j...
      //All other right pixels.
      else
        *j = *j + 1; //Next pixel.
    }//end if IncX...
    else // !IncX.
    {
      //Last left pixel condition.
      if( (*i == (y-1)) && (*j == 0) )
      {
        *i = 0;
        *j = 0;
        IncX = TRUE; //Move forwards.
      }//end if...
      else if( *j == 0 )
      {//Left end of line condition.
        *i = *i + 1; //Next line.
        IncX = TRUE; //Move forwards.
      }//end else if...
      //All other left pixels.
      else
        *j = *j - 1; //Next pixel.
    }//end else...
  }//end if Direction...
  else if(CurrParams.Direction == VERTICAL) //Always IncX.
  {
    //Moving down condition.
    if( IncY )
    {
      //Last down right pixel condition.
      if( (*i == (y-1)) && (*j == (x-1)) )
      {
        *i = 0;
        *j = 0;
      }//end if...
      //Bottom pixel condition.
      else if( *i == (y-1) )
      {
        *j = *j + 1; //Next pixel.
        IncY = FALSE; //Move up.
      }//end else if *i...
      //All other down pixels.
      else
        *i = *i + 1; //Next line.
    }//end if IncY...
    else // !IncY.
    {
      //Last up right pixel condition.
      if( (*i == 0) && (*j == (x-1)) )
      {
        *i = 0;
        *j = 0;
        IncY = TRUE; //Move down.
      }//end if...
      //Top pixel condition.
      else if( *i == 0 )
      {
        *j = *j + 1; //Next pixel.
        IncY = TRUE; //Move down.
      }//end else if...
      //All other up pixels.
      else
        *i = *i - 1; //Next line up.
    }//end else...
  }//end else if Direction...
  else //DIAGONAL.
  {
    //Top left corner.
    if( (*i == 0)&&(*j == 0) )
    {
      *j = *j + 1; //Move right 1.
      IncX = FALSE; //Down to the left next.
      IncY = TRUE;
    }//end if *i...
    //Top right corner.
    else if( (*i == 0)&&(*j == (x-1)) )
    {
      if( !IncX && IncY) //Moving down and left.
      {
        *i = *i + 1; //Down 1 left 1;
        *j = *j - 1;
      }//end if IncX...
      else //Moving right.
      {
        *i = *i + 1; //Move down 1.
        IncX = FALSE; //Down to the left next.
        IncY = TRUE;
      }//end else...
    }//end else if *i...
    //Bottom left corner.
    else if( (*i == (y-1))&&(*j == 0) )
    {
      if( IncX && !IncY ) //Moving up and right.
      {
        *i = *i - 1; //Up 1 right 1.
        *j = *j + 1;
      }//end if IncX...
      else
      {
        *j = *j + 1; //Move right 1.
        IncX = TRUE; //Up to the right next.
        IncY = FALSE;
      }//end else...
    }//end else if *i...
    //Bottom right corner.
    else if( (*i == (y-1))&&(*j == (x-1)) )
    {
      *i = 0; //Start again.
      *j = 0;
      IncX = TRUE;
      IncY = TRUE;
    }//end else if *i...
    //Top boundary.
    else if( *i == 0 )
    {
      if( !IncX && IncY) //Moving down and left.
      {
        *i = *i + 1; //Down 1 left 1;
        *j = *j - 1;
      }//end if IncX...
      else //Moving right.
      {
        *j = *j + 1; //Move right 1.
        IncX = FALSE; //Down to the left next.
        IncY = TRUE;
      }//end else...
    }//end else if *i...
    //Bottom boundary.
    else if( *i == (y-1) )
    {
      if( IncX && !IncY ) //Moving up and right.
      {
        *i = *i - 1; //Up 1 right 1.
        *j = *j + 1;
      }//end if IncX...
      else
      {
        *j = *j + 1; //Move right 1.
        IncX = TRUE; //Up to the right next.
        IncY = FALSE;
      }//end else...
    }//end else if *i...
    //Left boundary.
    else if( *j == 0 )
    {
      if( IncX && !IncY ) //Moving up and right.
      {
        *i = *i - 1; //Up 1 right 1.
        *j = *j + 1;
      }//end if IncX...
      else
      {
        *i = *i + 1; //Move down 1.
        IncX = TRUE; //Up to the right next.
        IncY = FALSE;
      }//end else...
    }//end else if *j...
    //Right boundary.
    else if( *j == (x-1) )
    {
      if( !IncX && IncY) //Moving down and left.
      {
        *i = *i + 1; //Down 1 left 1;
        *j = *j - 1;
      }//end if IncX...
      else //Moving right.
      {
        *i = *i + 1; //Move down 1.
        IncX = FALSE; //Down to the left next.
        IncY = TRUE;
      }//end else...
    }//end else if *j...
    else
    {
      //Moving right and up.
      if( IncX && !IncY )
      {
        *i = *i - 1;
        *j = *j + 1;
      }//end if IncX && !IncY...
      //Moving left and down.
      else
      {
        *i = *i + 1;
        *j = *j - 1;
      }//end else...
    }//end else...
  }//end else...
  return(Val);
}//end GetNext.

void CRLCODEC::FindBits(int Run,int Value,
                        int *NumCodeBits,int *CdeWord)
{
  CodeWordPtr Table;
  unsigned int TableLen;
  unsigned int ESCAPEBits;

  switch(CurrParams.BitAllocTable)
  {
  case H261:
    Table = H261Table;
    TableLen = H261TableSize;
    ESCAPEBits = H261ESCAPEBits;
    break;
  case H263: //Use H261 for now.
    Table = H261Table;
    TableLen = H261TableSize;
    ESCAPEBits = H261ESCAPEBits;
    break;  
  case H261EXT1:
    Table = H261EXT1Table;
    TableLen = H261EXT1TableSize;
    ESCAPEBits = H261EXT1ESCAPEBits;
    break;
  default :
    Table = H261Table;
    TableLen = H261TableSize;
    ESCAPEBits = H261ESCAPEBits;
    break;
  }//end switch...

  //Parse table with Run-length and value.
  unsigned int i;
  i = 0;
  BOOL Found;
  Found = FALSE;
  while( (i < TableLen)&&(!Found) )
  {
    //Does run-length match?
    if( Run == Table[i].Run )
    {
      //Does value match?
      if( Value == Table[i].Value )
      {
        *NumCodeBits = Table[i].NumBits;
        *CdeWord = Table[i].Code;
        Found = TRUE;
      }//end if Value...
    }//end if Run...
    i++;
  }//end while i...
  //Run-length and value combination was not in table.
  if( !Found )
  {
    *NumCodeBits = ESCAPEBits;
    *CdeWord = 0;
  }//end if !Found...
}//end FindBits.

unsigned long int CRLCODEC::GetEOBBits(void)
{
  unsigned long int B;

  switch(CurrParams.BitAllocTable)
  {
  case H261:
    B = H261EOBBits;
    break;
  case H263:
    B = 5;
    break;
  case H261EXT1:
    B = H261EXT1EOBBits;
    break;
  default :
    B = 5;
    break;
  }//end switch...
  return(B);
}//end GetEOBBits.

