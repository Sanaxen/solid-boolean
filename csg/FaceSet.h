/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef FACESET_H
#define FACESET_H

#include <vector>

#pragma warning ( disable : 4150 ) /* 'Face' 型を削除するため delete 演算子が呼び出されましたが、定義がありません。 */
class Face;

class FaceSet
{
public:
	inline FaceSet(){};
	~FaceSet();

	inline Face * __fastcall GetFace(const int i) const
	{
#ifdef _STL_DEBUG
	if(i < 0) return 0;
	if(i >= m_pFaces.size()) return 0;
#endif
		return m_pFaces[i];
	}

	inline void __fastcall SetFace(const int i, Face* f)
	{
#ifdef _STL_DEBUG
	if(i < 0) return 0;
	if(i >= m_pFaces.size()) return 0;
#endif
		m_pFaces[i] = f;
	}

	void __fastcall SetFace(const int i, Face & vFace);
	Face * __fastcall CreateFace(Face & vFace, mlVector3D* normalvec);
	Face * __fastcall AddFace(Face & vFace, mlVector3D* normalvec=NULL);
	Face * __fastcall InsertFace(int i, Face & vFace);
	void __fastcall RemoveFace(int i);

	inline Face * __fastcall AddFace2(Face* vFace)
	{
		m_pFaces.push_back(vFace);
		return vFace;
	}
	void __fastcall RemoveFace2(Face* face);
	void __fastcall RemoveFace2(const int i);

	inline int __fastcall GetSize() const
	{
		return m_pFaces.size();
	}
	inline void __fastcall Reserve(const int size)
	{
		m_pFaces.reserve(size);
	}
	inline void __fastcall Resize(const int size)
	{
		m_pFaces.resize(size, NULL);
	}


	std::vector<Face *> m_pFaces;
private:

};

#endif // FACESET_H

