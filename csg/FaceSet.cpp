/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "ML_Vector.h"

#include "FaceSet.h"

#include "Face.h"
#include "openmp_config.h"

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif


FaceSet::~FaceSet()
{
	//printf("Deleting FaceSet\n");
	//delete [] m_pFaces;
#if 10
	const int sz = m_pFaces.size();
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for ( int i = 0; i < sz; i++ )
	{
		if ( m_pFaces[i] ) 
		{
			delete m_pFaces[i];
			m_pFaces[i] = NULL;
		}
	}
#else
	std::vector<Face*>::iterator end = m_pFaces.end();
	for ( std::vector<Face*>::iterator it = m_pFaces.begin(); it != end; ++it)
	{
		if ((*it) )
		{
			delete *it;
		}
	}
#endif
	std::vector<Face *>().swap(m_pFaces);
	m_pFaces.clear();
}


void __fastcall FaceSet::SetFace(const int i, Face & vFace)
{
#ifdef _STL_DEBUG
	if(i < 0) return;
	if(i >= m_pFaces.size()) return;
#endif

	*m_pFaces[i] = vFace;
}

Face * __fastcall FaceSet::AddFace(Face & vFace, mlVector3D* normalvec)
{
	Face* f = CreateFace(vFace, normalvec);
	m_pFaces.push_back( f );
	return f;
}

Face * __fastcall FaceSet::CreateFace(Face & vFace, mlVector3D* normalvec)
{
	Face *f = new Face;

	*f = vFace;
	if ( normalvec )
	{
		f->setNormal(*normalvec);
	}
	return f;
}


Face * __fastcall FaceSet::InsertFace(const int i, Face & vFace)
{
	//if(m_nSize >= m_nMaxSize)
	//{
	//	return 0;
	//}


	const int size = m_pFaces.size();
	m_pFaces.push_back(new Face);

	// Shift everything along
	for(int j = size; j >= i+1; j--)
	{
		*m_pFaces[j] = *m_pFaces[j-1];
	}

	//if ( m_pFaces[i] ) delete m_pFaces[i];
	//m_pFaces[i] = new Face;
	*m_pFaces[i] = vFace;
	return m_pFaces[i];
}

void __fastcall FaceSet::RemoveFace(const int i)
{
	const int sz = m_pFaces.size();
#ifdef _STL_DEBUG
	if(sz <= 0 )
	{
		return;
	}
#endif

	const int n = sz -1;
	for(int j = i; j < n; j++)
	{
		*m_pFaces[j] = *m_pFaces[j+1];
	}
	delete m_pFaces[n];
	m_pFaces[n] = NULL;
	m_pFaces.pop_back();
}

void __fastcall FaceSet::RemoveFace2(const int i)
{
	DELETE_FACE(m_pFaces[i]);
}
void __fastcall FaceSet::RemoveFace2(Face* face)
{
	DELETE_FACE(face);
}
