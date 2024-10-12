/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef VERTEXSET_H
#define VERTEXSET_H

#include <algorithm>
#include <vector>
//#include <hash_set>
#include <set>
#include <map>
#include <unordered_set>

#define VERTEX_PTR_USE_VECTOR
//#define VERTEX_PTR_USE_SET
//#define VERTEX_PTR_USE_HASH_SET
//#define VERTEX_PTR_USE_UNORDERED_SET

class Vertex;

class VertexPointerSet
{
public:
	inline VertexPointerSet(){}

	inline ~VertexPointerSet()
	{
		//printf("Deleting VertexPointerSet\n");
		VerticesPtrSet().swap(m_pPointers);
		m_pPointers.clear();
	}

	inline void __fastcall AddVertexPointer(Vertex * pPointer)
	{
#ifdef VERTEX_PTR_USE_VECTOR
		m_pPointers.push_back(pPointer);
#else
		m_pPointers.insert(pPointer);
#endif
	}

	inline void Add(const VertexPointerSet& src)
	{
		VerticesPtrSet::const_iterator it = src.m_pPointers.begin();
		VerticesPtrSet::const_iterator end = src.m_pPointers.end();
		for ( ; it != end; ++it)
		{
#ifdef VERTEX_PTR_USE_VECTOR
			m_pPointers.push_back(*it);
#else
			m_pPointers.insert(*it);
#endif
		}

	}

	inline bool __fastcall FindVertexPtr(const Vertex * v) const
	{
#ifdef VERTEX_PTR_USE_VECTOR
		if ( std::find(m_pPointers.begin(), m_pPointers.end(), v) == m_pPointers.end()) return false;
#else
		if ( m_pPointers.find(v) == m_pPointers.end()) return false;
#endif
		return true;
	}


#ifdef VERTEX_PTR_USE_SET
	typedef std::set<Vertex *> VerticesPtrSet;
#endif
#ifdef VERTEX_PTR_USE_HASH_SET
	typedef std::hash_set<Vertex *> VerticesPtrSet;
#endif
#ifdef VERTEX_PTR_USE_VECTOR
	typedef std::vector<Vertex *> VerticesPtrSet;
#endif
#ifdef VERTEX_PTR_USE_UNORDERED_SET
	typedef std::unordered_set<Vertex *> VerticesPtrSet;
#endif
	

	inline VerticesPtrSet& __fastcall Get()
	{
		return m_pPointers;
	}
private:


	VerticesPtrSet m_pPointers;
};


class VertexSet
{
public:
	inline VertexSet(){};
	~VertexSet();

	inline Vertex * __fastcall GetVertex(const int nIndex) const
	{
#ifdef _STL_DEBUG
		if(nIndex < 0) return 0;
		if(nIndex >= m_pVertices.size()) return 0;
#endif
		return m_pVertices[nIndex];
	}

	Vertex * __fastcall SetVertex( const int nIndex, const Vertex& vertex);
	Vertex * __fastcall AddVertex(const Vertex & vertex);
	void __fastcall Add3Vertex( Vertex* vertex[3], const int n);


	inline int __fastcall GetNumVertices() const
	{
		return m_pVertices.size();
	}


	inline void __fastcall Reserve(const int size) 
	{
		m_pVertices.reserve(size);
	}
	inline void __fastcall Resize(const int size) 
	{
		m_pVertices.resize(size);
	}


private:

	typedef std::vector<Vertex *> Vertices;
	Vertices m_pVertices;

};

#endif // VERTEXSET_H
