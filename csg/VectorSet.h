/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef VECTORSET_H
#define VECTORSET_H

#include <vector>

#include "ML_Vector.h"

class VectorSet
{
public:
	inline VectorSet(){};

	inline ~VectorSet()
	{
		//printf("Deleting VectorSet\n");
		VectorVector().swap(m_pVectors);
		m_pVectors.clear();
	}

	inline mlVector3D __fastcall GetVector(const int i) const
	{
#ifdef _STL_DEBUG
		if(i < 0) return mlVector3D();
		if(i >= m_pVectors.size()) return mlVector3D();
#endif
		return m_pVectors[i];
	}

	inline void __fastcall SetVector(const int i, const mlVector3D & vVector)
	{
#ifdef _STL_DEBUG
		if(i < 0) return;
		if(i >= m_pVectors.size()) return;
#endif
		m_pVectors[i] = vVector;
	}

	inline void __fastcall AddVector(const mlVector3D & vVector)
	{
		m_pVectors.push_back(vVector);
	}

	inline void __fastcall RemoveVector(const int i)
	{
		m_pVectors.erase(m_pVectors.begin()+i);
	}



	inline  mlVector3D & __fastcall operator[](const int index)
	{
		mlVector3D & pVector = m_pVectors[index];

		// If its null, we're in trouble...

		return pVector;
	}

	inline int __fastcall GetSize() const
	{
		return m_pVectors.size();
	}
	inline void __fastcall Reserve(const int size) 
	{
		m_pVectors.reserve(size);
	}
	inline void __fastcall Resize(const int size) 
	{
		m_pVectors.resize(size);
	}

	typedef std::vector<mlVector3D> VectorVector;

	VectorVector m_pVectors;

	inline void Assgin(VectorSet & vVectors)
	{
		m_pVectors.assign(vVectors.m_pVectors.begin(), vVectors.m_pVectors.end());
	}

//private:
//
//	mlVector3D * m_pVectors;
//	int m_nMaxSize;
//	int m_nSize;
};

#endif // VECTORSET_H

