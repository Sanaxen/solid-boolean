/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef INTSET_H
#define INTSET_H

#include <vector>

class IntSet
{
public:
	inline IntSet(){};

	inline ~IntSet()
	{
		//printf("Deleting IntSet\n");
		IntVector().swap(m_pInts);
		m_pInts.clear();
	}


	inline int __fastcall GetInt(const int i) const
	{
#ifdef _STL_DEBUG
		if(i < 0) return 0;
		if(i >= m_pInts.size()) return 0;
#endif
		return m_pInts[i];
	}

	inline void __fastcall SetInt(const int i, const int nInt)
	{
#ifdef _STL_DEBUG
		if(i < 0) return;
		if(i >= m_pInts.size()) return;
#endif
		m_pInts[i] = nInt;
	}

	inline void __fastcall AddInt(const int nInt)
	{
		m_pInts.push_back(nInt);
	}
	inline void __fastcall Add3Int(const int nInt)
	{
		int nn = nInt;
		m_pInts[nn] = nn;
		m_pInts[nn+1] = nn+1;
		m_pInts[nn+2] = nn+2;
	}



	inline int & __fastcall operator[](const int index)
	{
		int & pInt = m_pInts[index];

		// If its null, we're in trouble...
		return pInt;
	}

	inline int __fastcall GetSize() const
	{
		return m_pInts.size();
	}
	inline void __fastcall Reserve(const int size) 
	{
		m_pInts.reserve(size);
	}
	inline void __fastcall Resize(const int size) 
	{
		m_pInts.resize(size);
	}

	typedef std::vector<int> IntVector;

	IntVector m_pInts;

	inline void __fastcall Assgin(IntSet & vIntVectors)
	{
		m_pInts.assign(vIntVectors.m_pInts.begin(), vIntVectors.m_pInts.end());
	}

	inline void __fastcall Assgin(const int num, int* IntVectors)
	{
		m_pInts.assign(IntVectors, IntVectors+num);
	}
//private:
//
//	int * m_pInts;
//	int m_nMaxSize;
//	int m_nSize;
};

#endif // INTSET_H

