/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "VertexSet.h"

#include "VertexSet.h"
#include "Vertex.h"

#include <stdio.h>

#include "openmp_config.h"

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif



//////////////////////////////////////////////////////////////////////

VertexSet::~VertexSet()
{
	//printf("Deleting VertexSet\n");
	//printf("Deleting vertex array\n");

	const int sz = m_pVertices.size();

#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		m_pVertices[i]->Release();
	}

	Vertices().swap(m_pVertices);
	m_pVertices.clear();
	//delete [] m_pVertices;
}

Vertex * __fastcall VertexSet::SetVertex( const int nIndex, const Vertex& vertex)
{
#ifdef _STL_DEBUG
	if(nIndex < 0) return 0;
	if(nIndex >= m_pVertices.size()) return 0;
#endif
	Vertex* vrt = new Vertex(vertex);
	m_pVertices[nIndex] = vrt;
	return vrt;
}

Vertex * __fastcall VertexSet::AddVertex(const Vertex & vertex)
{
	Vertex* vrt = new Vertex(vertex);

	m_pVertices.push_back(vrt);

	return vrt;
	//return &m_pVertices[m_nNumVertices - 1];
}

void __fastcall VertexSet::Add3Vertex( Vertex* vertex[3], const int n)
{
#if VERTEX_REF_COUNT
	m_pVertices[n+0] = vertex[0]->AddRef();
	m_pVertices[n+1] = vertex[1]->AddRef();
	m_pVertices[n+2] = vertex[2]->AddRef();
#else
	m_pVertices[n+0] = new Vertex(vertex[0]);
	m_pVertices[n+1] = new Vertex(vertex[1]);
	m_pVertices[n+2] = new Vertex(vertex[2]);
#endif
}

