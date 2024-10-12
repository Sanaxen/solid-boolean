/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ML_Types.h"

class Vertex;

class Bound;
class VertexSet;
class FaceSet;
class Solid;
class mlVector3D;
class gxColor;
class Segment;


#include "bound.h"
#include "FaceSet.h"
#include "ColorSet.h"
#include "uniformgrid.h"

class Face;
class octreecell;

//Ø’f‚³‚ê‚é‰ñ”‚ÍŒ©Ï‚à‚é‚±‚Æ‚ªo—ˆ‚é‚ªÚG‚É‹ß‚¢‚Æ‚»‚Ì”‚æ‚è‚à‘½‚­Ø’f‚³‚ê‚éŽ–‚ª‚ ‚é
//FACE_CUT_NUM_COEF”{‚ÉÝ’è‚µ‚Ä‚¨‚­
//‚±‚ê‚æ‚è‘½‚­‰½“x‚àØ’f‚³‚ê‚é‚Ì‚ÍˆÙí‚Èó‘Ô‚Æ”»’f‚·‚é
#define FACE_CUT_NUM_COEF	(6)

////”÷¬‚ÈFace‚Í”»’è‚ª”÷–­‚È‚Ì‚ÅŒã‰ñ‚µ‚·‚é
//#define POSTPONE_RAYTRACE

//Ø’fŒ‹‰Ê‚Ìd•¡Š®‘Sœ‹Ž‰ñ”‚ÌŒÀŠEðŒ
#define EXTEND_NUMFACEMAX_NUM2 (30)

//Ø’fŽc‚è”‚ÌˆÙí’l
#define FACE_SPLIT_REST_NUM_ABNORMAL_VALUE	(1000)

class Object3D
{
	Solid * solid_;
public:

	VertexSet * vertices;
	FaceSet * faces;

	Bound * bound;
	//static int cuttingFaceNumLoopMax;
	
	Object3D();
	Object3D(Solid * solid);
	~Object3D();
	
	void Clear();
	void CreateObject(Solid * solid);
	void CreateObject()
	{
		CreateObject(solid_);
	}

	//Object3D * clone()
	
	/**
	 * Gets the number of faces
	 * 
	 * @return number of faces
	 */
	inline int __fastcall getNumFaces() const
	{
		return faces->GetSize();
	}

	/**
	 * Gets a face reference for a given position
	 * 
	 * @param index required face position
	 * @return face reference , null if the position is invalid
	 */
	inline Face * __fastcall getFace(const int index) const
	{
#ifdef _STL_DEBUG
		if(index<0 || index>=faces->GetSize())
		{
			return 0;
		}
#endif
		return faces->GetFace(index);
	}

	/**
	 * Gets the solid bound
	 * 
	 * @return solid bound
	 */
	inline const Bound* __fastcall getBound() const
	{
		return bound;
	}

	int splitFaces(Object3D * pObject, const int extend_numFacesMax_Num);
	int splitFaces0(Object3D * object, const int cuttingFaceIndex, Face* face1, const int numthread, const int pre_num, int& post_num);
	void OverlapCheckInit(Object3D * object);
	int OverlapCheck(Object3D * object, const Bound* object_bound, const int startIndex, const int endIndex, const int num);

	int DuplicateFaceRemove(const int call_marker, const int is, const int ie, const int js, const int je, mlFloat* chktol=NULL);
	int DuplicateFaceRemove2(const int call_marker, const int is, const int ie, const int js, const int je);
	int ReverseFaceRemove(const int is, const int ie);
	int RestFaceCount(int curindex, int num);

	const static mlFloat TOL;
	const static mlFloat ATOL;
	const static mlFloat ATOL2;
	static mlFloat SPLIT_TOL;
//private:

	Face * __fastcall addFaceChk(Vertex * v1, Vertex * v2, Vertex * v3, Face* base, mlVector3D* normalvec = NULL);
	Face * __fastcall addFace(Vertex * v1, Vertex * v2, Vertex * v3, Face* base, mlVector3D* normalvec = NULL);
	Vertex * __fastcall addVertex(const mlVector3D & pos, const color_attr_type& color, const int status, const int flag);

private:
	Vertex * __fastcall addVertex(const mlVector3D & pos, const color_attr_type& color, const int status);
	Vertex * __fastcall addVertex(const int index, const mlVector3D & pos, const color_attr_type& color, const int status);
	void __fastcall CreateGrid(void);

public:
	mlFloat __fastcall computeDistance(Vertex* vertex, Face* face, mlVector3D* normalP);
	void __fastcall computeDistance(Vertex* vertex[3], Face* face, mlVector3D* normalP, mlFloat dist[3]);

	void splitFace(const int facePos, Segment segment[2]);
	  
	void __fastcall breakFaceInTwo(const int facePos, const mlVector3D & newPos, int splitEdge);
	
	void __fastcall breakFaceInTwo(const int facePos, const mlVector3D & newPos, Vertex & endVertex);
	
	void __fastcall breakFaceInThree(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, Vertex & startVertex, Vertex & endVertex);
	void __fastcall breakFaceInThree(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, int splitEdge);
	void __fastcall breakFaceInThree(const int facePos, const mlVector3D & newPos, Vertex & endVertex);
	void __fastcall breakFaceInThree(const int facePos, const mlVector3D & newPos);

	void __fastcall breakFaceInFour(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, Vertex & endVertex);
	
	void __fastcall breakFaceInFive(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, int linedVertex);
	
	int __fastcall simple_classify(Object3D* object, const Bound* object_bound, Face* face, int& rayTrace_time);
	int __fastcall classifyFaces(Object3D* pObject);
	
	void invertInsideFaces();

	octreecell* cell;
	void MakeCell();
	void ReMakeCell();
	void DeleteCell();

	UniformGrid<Vertex*>* unigrid;
};

#endif // OBJECT3D_H
