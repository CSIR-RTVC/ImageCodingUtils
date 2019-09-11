// Implementation of the CImage class methods.
#include "stdafx.h"
#include <memory.h>

#include "Graph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////
// Public Implementations.
///////////////////////////////////////////////////

CGraph::CGraph()
{
  RECT r = {0,0,0,0};
  GphWin = r;

}//end CGraph Constructor.

CGraph::~CGraph()
{
}//end CGraph Destructor.

int CGraph::DrawGraph(CDC *pDC)
{
  pDC->MoveTo(GphWin.left,GphWin.top);
  CPen P;
  CPen *pOldP;
  pOldP = (CPen *)(pDC->SelectStockObject(BLACK_PEN));
  pDC->SelectStockObject(WHITE_BRUSH);
  pDC->Rectangle(GphWin);

  pDC->SelectObject(pOldP);
  return(1);
}//end DrawGraph.

