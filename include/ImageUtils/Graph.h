// Definition of a CGraph class that holds a graph that
// plots itself to the device context. 
#ifndef _GRAPH_H
#define _GRAPH_H

#include "afxwin.h"

class CGraph
{
// Attributes
protected:

public:

// Operations
public:

// Implementation
public:
	int DrawGraph(CDC *pDC);
	CRect GphWin;
	CGraph();
	~CGraph();

protected:

};//CGraph class.

#endif
