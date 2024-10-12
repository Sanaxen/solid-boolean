#ifndef _HalfEdge_HPP
#define _HalfEdge_HPP

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <set>
#include <vector>
#include <algorithm>

#include "cylinder.h"

#define USE_TriangleQuality 1

#ifndef M_PI
	#define M_PI			3.1415926535897932
#endif
#define M_TWO_PI			6.2831853071795865

#define VEC3_ADD(d,v,w)		(d)[0]=(v)[0]+(w)[0]; (d)[1]=(v)[1]+(w)[1]; (d)[2]=(v)[2]+(w)[2]
#define VEC3_SUB(d,v,w)		(d)[0]=(v)[0]-(w)[0]; (d)[1]=(v)[1]-(w)[1]; (d)[2]=(v)[2]-(w)[2]
#define VEC3_CROSS(d,v,w)	(d)[0]=(v)[1]*(w)[2] - (v)[2]*(w)[1]; \
							(d)[1]=(v)[2]*(w)[0] - (v)[0]*(w)[2]; \
							(d)[2]=(v)[0]*(w)[1] - (v)[1]*(w)[0]
#define VEC3_NORMALIZE(v)	{ double n=sqrtf((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2]); \
							if(fabs(n)>1e-16) { double m=1.0/n; (v)[0]*=m; (v)[1]*=m; (v)[2]*=m; } }

#define VEC3_NORMALIZE2(v,ln)	{ ln=sqrtf((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2]); \
							if(fabs(ln)>1e-16) { double m=1.0/ln; (v)[0]*=m; (v)[1]*=m; (v)[2]*=m; } }

#define SQR(a)	((a)*(a))

#define DOTPRODUCT(a)		((a)[0]*(a)[0] + (a)[1]*(a)[1] + (a)[2]*(a)[2])
#define DOTPRODUCT2(a,b)	((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])

#define ZERO_TOL	(1.0e-16)

#if defined(WIN32) && defined(_USRDLL)
# define MESHSIMPLIFICATION_DLLEXPORTS extern "C" __declspec( dllexport )
#else
# define MESHSIMPLIFICATION_DLLEXPORTS /* emptyr */
         
#endif

class Color
{
public:
	unsigned char attr;
	int rgb;
};

inline void Color_IntToRGB( int color, int rgb[3] )
{
	rgb[0] = (color & 0x000000FF);
	rgb[1] = (color & 0x0000FF00) >> 8;
	rgb[2] = (color & 0x00FF0000) >> 16;
}

inline int Color_RGBToInt( int rgb[3] )
{
	return ((rgb[2] << 16) | (rgb[1] << 8) | rgb[0]);
}

class ColorPalette
{
	int _rgb[256][3];
public:
	inline ColorPalette()
	{
		// 青→緑
		for (int i = 0; i < 64; i++)
		{
			int green = i * 4;
			_rgb[i][0] = 0;
			_rgb[i][1] = green;
			_rgb[i][2] = 255 - green;
		}
		// 緑→黄
		for (int i = 0; i < 64; i++)
		{
			int red = i * 4;
			_rgb[i+64][0] = red;
			_rgb[i+64][1] = 255;
			_rgb[i+64][2] = 0;

		}
		// 黄→赤
		for (int i = 0; i < 128; i++)
		{
			int green = 255 - i * 2;
			_rgb[i + 128][0] = 255;
			_rgb[i + 128][1] = green;
			_rgb[i + 128][2] = 0;
		}
	} 
	void getColor(int max, int index, int rgb[3])
	{
		double t = (double)(max - index)/(double)max;

		int i = (int)(t*255.0 + 0.5);

		rgb[0] = _rgb[i][0];
		rgb[1] = _rgb[i][1];
		rgb[2] = _rgb[i][2];
	}
};


class MeshV
{
public:
	bool vertexAttr;
	unsigned int *pIndices;
	int iNumVertices;
	int iNumIndices;
	double *pVertices;
	double *pNormals;
	Color* pColors;
	float min[3];
	float max[3];
	double diagonal_length;

	std::set<unsigned int>* faceGroup;
	MeshV()
	{
		pIndices = NULL;
		pVertices = NULL;
		pNormals = NULL;
		pColors = NULL;
		iNumVertices = 0;
		iNumIndices = 0;
		vertexAttr = false;
		faceGroup = 0;
	}

	~MeshV()
	{
		Clear();
	}

	void Clear()
	{
		if ( pIndices ) delete [] pIndices;
		if ( pVertices ) delete [] pVertices;
		if ( pNormals ) delete [] pNormals;
		if ( pColors ) delete [] pColors;
		if (faceGroup) delete[] faceGroup;
		pIndices = NULL;
		pVertices = NULL;
		pNormals = NULL;
		pColors = NULL;
		faceGroup = NULL;
		iNumVertices = 0;
		iNumIndices = 0;

		laplacianSmoothing_flag.shrink_to_fit();
	}

	inline double* __fastcall VertexCoord(int id) const
	{
		return &(pVertices[3*id]);
	}
	inline void Box()
	{
		unsigned int* id = &(pIndices[0]);
		double* p = &(pVertices[3*id[0]]);

		min[0] = p[0];
		min[1] = p[1];
		min[2] = p[2];
		max[0] = min[0];
		max[1] = min[1];
		max[2] = min[2];
		for ( int i = 0; i < iNumIndices; i++ )
		{
			unsigned int* id = &(pIndices[3*i]);
			double* p = &(pVertices[3*id[0]]);

			p = &(pVertices[3*id[0]]);
			if ( p[0] < min[0] ) min[0] = p[0];
			if ( p[1] < min[1] ) min[1] = p[1];
			if ( p[2] < min[2] ) min[2] = p[2];
			if ( p[0] > max[0] ) max[0] = p[0];
			if ( p[1] > max[1] ) max[1] = p[1];
			if ( p[2] > max[2] ) max[2] = p[2];

			p = &(pVertices[3*id[1]]);
			if ( p[0] < min[0] ) min[0] = p[0];
			if ( p[1] < min[1] ) min[1] = p[1];
			if ( p[2] < min[2] ) min[2] = p[2];
			if ( p[0] > max[0] ) max[0] = p[0];
			if ( p[1] > max[1] ) max[1] = p[1];
			if ( p[2] > max[2] ) max[2] = p[2];

			p = &(pVertices[3*id[2]]);
			if ( p[0] < min[0] ) min[0] = p[0];
			if ( p[1] < min[1] ) min[1] = p[1];
			if ( p[2] < min[2] ) min[2] = p[2];
			if ( p[0] > max[0] ) max[0] = p[0];
			if ( p[1] > max[1] ) max[1] = p[1];
			if ( p[2] > max[2] ) max[2] = p[2];
		}
		diagonal_length = sqrt( (max[0]-min[0])*(max[0]-min[0])+(max[1]-min[1])*(max[1]-min[1])+(max[2]-min[2])*(max[2]-min[2]));
		printf("diagonal_length %f\n", diagonal_length);
	}

	std::vector<int> laplacianSmoothing_flag;
	void laplacianSmoothing_flag_clear()
	{
		laplacianSmoothing_flag.clear();
		laplacianSmoothing_flag.resize(iNumVertices, 0);

	}

};



class HEdge;
class Edge;
class Face;
class Vertex
{
	MeshV* mesh_;
public:
	unsigned int id_;
	int boundary_;

	Vertex() {}
	inline void New(MeshV* mesh, int id)
	{
		mesh_ = mesh;
		id_ = id;
		boundary_ = 0;
		outhedge.clear();
	}

	inline double* __fastcall coord() const
	{
		return mesh_->VertexCoord(id_);
	}
	std::set<HEdge*> outhedge;	//この頂点から出るハーフエッジ
};


class Edge
{
	int hedge_ref;
public:
	int alive;
	int used;
	std::pair< Vertex*, Vertex* > bound_;
	HEdge* hedge[4];

	Edge() {}

	inline void New( Vertex* v1, Vertex* v2)
	{
		alive = 1;
		used = 0;
		hedge_ref = 0;
		hedge[0] = NULL;
		hedge[1] = NULL;
		if ( v1->id_ < v2->id_ )
		{
			bound_ = std::pair< Vertex*, Vertex* >(v1, v2);
		}else
		{
			bound_ = std::pair< Vertex*, Vertex* >(v2, v1);
		}
	}
	inline int __fastcall HalfEdgeRefCount() const
	{
		return hedge_ref;
	}

	inline double __fastcall length_sqr() const
	{
		const double* p1 = bound_.first->coord();
		const double* p2 = bound_.second->coord();
		double q[3];

		VEC3_SUB(q, p1, p2);
		return DOTPRODUCT(q);
	}

	//Edge(const Edge& e)
	//{
	//	v1_ = e.v1_;
	//	v2_ = e.v2_;
	//}
	inline bool operator==(const Edge& edge) const {
		if ( this->bound_.first->id_  == edge.bound_.first->id_ && this->bound_.second->id_  == edge.bound_.second->id_) return true;
		if ( this->bound_.first->id_  == edge.bound_.second->id_ && this->bound_.second->id_  == edge.bound_.first->id_) return true;
		return false;
    }

	inline void __fastcall refHede(HEdge* he)
	{
		if ( hedge_ref >= 4 )
		{
			hedge_ref++;
			//printf("error.\n");
			return;
		}
		hedge[hedge_ref] = he;
		hedge_ref++;
	}
};
class EdgeSet
{
public:
    inline bool __fastcall operator()(  Edge* p_rcIntL,  Edge* p_rcIntR ) const
    {
		if ( p_rcIntL->bound_.first->id_  < p_rcIntR->bound_.first->id_ ) return true;
		if ( p_rcIntL->bound_.first->id_  == p_rcIntR->bound_.first->id_ && p_rcIntL->bound_.second->id_  < p_rcIntR->bound_.second->id_ ) return true;
		return false;
		//return ( p_rcIntL->v1_->id_  < p_rcIntR->v1_->id_ || (p_rcIntL->v1_->id_  == p_rcIntR->v2_->id_) && (p_rcIntL->v2_->id_  < p_rcIntR->v2_->id_));
    }
};

class Loop;
class HEdge
{
public:
	int dir_;
	Edge* edge_;
	Loop* loop_;

	inline  HEdge(Loop* loop, Edge* edge, Vertex* v1, Vertex* v2)
	{
		loop_ = loop;
		edge_ = edge;
		if ( edge->bound_.first == v1 && edge->bound_.second == v2 )
		{
			dir_ = 1;
			edge->bound_.first->outhedge.insert(this);
		}else if ( edge->bound_.first == v2 && edge->bound_.second == v1 )
		{
			dir_ = -1;
			edge->bound_.second->outhedge.insert(this);
		}
		edge->refHede(this);
	}

	inline Vertex* __fastcall StartPos() const
	{
		if ( dir_ == 1 ) return edge_->bound_.first;
		return edge_->bound_.second;
	}
	inline Vertex* __fastcall EndPos() const
	{
		if ( dir_ == 1 ) return edge_->bound_.second;
		return edge_->bound_.first;
	}

	HEdge* prev;
	HEdge* next;

};

class Loop
{
public:
	Face* face_;
	HEdge* hedge;
	Loop(Face* f);

	inline ~Loop()
	{
		HEdge* he1 = hedge;
		HEdge* he2 = he1->next;
		HEdge* he3 = he2->next;

		delete he1;
		delete he2;
		delete he3;
	}
};

class FaceAttribute;
class Solid;
class Face
{
public:
	unsigned int id;
	Loop* loop;
	Solid* solid_;
	int alive;
	unsigned int* index;
	bool error_;

	Face() { loop = 0; }

	void New(Solid* solid, unsigned int id, unsigned int id1, unsigned int id2, unsigned int id3);

	~Face()
	{
		if ( loop ) delete loop;
		loop = NULL;
	}

	double* Normal() const;

	inline double* __fastcall CalcNormal(double* normal) const
	{
		double *p[3];

		p[0] = loop->hedge->StartPos()->coord();
		p[1] = loop->hedge->next->StartPos()->coord();
		p[2] = loop->hedge->next->next->StartPos()->coord();

		double v[2][3];

		VEC3_SUB(v[0], p[1], p[0]);
		VEC3_SUB(v[1], p[2], p[1]);

		VEC3_CROSS(normal, v[0], v[1]);

		VEC3_NORMALIZE(normal);
		return normal;
	}

	inline bool __fastcall equal(Face* f)
	{
		int id1[3];
		int id2[3];

		id1[0] = loop->hedge->StartPos()->id_;
		id1[1] = loop->hedge->next->StartPos()->id_;
		id1[2] = loop->hedge->next->next->StartPos()->id_;
		id2[0] = f->loop->hedge->StartPos()->id_;
		id2[1] = f->loop->hedge->next->StartPos()->id_;
		id2[2] = f->loop->hedge->next->next->StartPos()->id_;

		if ( id1[0] == id2[0] && id1[1] == id2[1] && id1[2] == id2[2] ) return true;
		if ( id1[0] == id2[0] && id1[1] == id2[2] && id1[2] == id2[1] ) return true;
		if ( id1[0] == id2[1] && id1[1] == id2[0] && id1[2] == id2[2] ) return true;
		if ( id1[0] == id2[1] && id1[1] == id2[2] && id1[2] == id2[0] ) return true;
		if ( id1[0] == id2[2] && id1[1] == id2[0] && id1[2] == id2[1] ) return true;
		if ( id1[0] == id2[2] && id1[1] == id2[1] && id1[2] == id2[0] ) return true;
		return false;
	}

	inline double __fastcall distance(double *p )
	{
		double* org = loop->hedge->StartPos()->coord();
		double* n = Normal();
		double d = -DOTPRODUCT2(n, org);

		double dist = DOTPRODUCT2(n, p) + d;

		return dist;
	}
};

class FaceAttribute
{
public:
	int id_;
	double normal_[3];
	double area;
	double quality;
	int alive;
	bool skipp;

	inline  FaceAttribute()
	{
		id_ = -1;
		alive = 1;
		quality = 1.0;
		skipp = false;
	}

	inline bool __fastcall operator <(const FaceAttribute& t) { // 大小比較用
        return area < t.area;
    }

	double CalcArea(MeshV* mesh);
};


class TVertex
{
public:
	Vertex* v;
	Edge* onedge;
	double onp[3];
};


class Solid
{
	MeshV* mesh_;
	int org_iNumIndices;
	unsigned int *org_pIndices;	//編集前のFace情報

public:
	int* vertexListIndex;
	Vertex** vertexList;
	std::vector<Face*> faceList;
	std::set<Edge*, EdgeSet> edgeList;

	std::vector<FaceAttribute> faceAttributeList;

	inline  Solid(MeshV* mesh)
	{
		org_pIndices = NULL;
		mesh_ = mesh;
		faceAttributeList.resize(mesh_->iNumIndices);

		vertexList = new Vertex*[mesh->iNumVertices];
		memset(vertexList, '\0', sizeof(Vertex*)*mesh->iNumVertices);
		vertexListIndex = new int[mesh->iNumVertices];
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for ( int i = 0; i < mesh->iNumVertices; i++ )
		{
			vertexListIndex[i] = -1;
		}

		org_pIndices = NULL;
		if ( mesh_->pColors )
		{
			org_pIndices = new unsigned int[3*mesh_->iNumIndices];
			memcpy(org_pIndices, mesh_->pIndices, sizeof(unsigned int)*(3*mesh_->iNumIndices));
			org_iNumIndices = mesh_->iNumIndices;
		}

		vertexBuffer = 0;
		faceBuffer   = 0;
		edgeBuffer   = 0;

		vertex_count = 0;
		face_count   = 0;
		edge_count   = 0;
	}

	int vertex_count;
	int vertex_max;
	Vertex* vertexBuffer;

	int face_count;
	int face_max;
	Face* faceBuffer;

	int edge_count;
	int edge_max;
	Edge* edgeBuffer;

	inline void createVertexBuffer()
	{
		vertex_count = 0;
		vertex_max = mesh_->iNumVertices;
		if (vertexBuffer) return;

		vertexBuffer = new Vertex[vertex_max];
	}
	inline Vertex* NewVertex(MeshV* mesh, int id)
	{
		if (vertex_count >= vertex_max)
		{
			fprintf(stderr, "vertex over!!");
		}
		Vertex*p = &vertexBuffer[vertex_count];
		vertexListIndex[vertex_count] = id;
		vertex_count++;

		p->New(mesh, id);
		return p;
	}
	inline void createFaceBuffer()
	{
		face_count = 0;
		face_max = mesh_->iNumVertices;
		if (faceBuffer) return;

		faceBuffer = new Face[face_max];
	}
	inline Face* NewFace(unsigned int faceid, unsigned int id1, unsigned int id2, unsigned int id3)
	{
		if (face_count >= face_max)
		{
			fprintf(stderr, "face over!!");
		}
		Face*p = &faceBuffer[face_count];
		face_count++;
		
		p->New(this, faceid, id1, id2, id3);
		return p;
	}

	inline void createEdgeBuffer()
	{
		edge_count = 0;
		edge_max = mesh_->iNumVertices;
		if (edgeBuffer) return;

		edgeBuffer = new Edge[edge_max];
	}
	inline Edge* NewEdge(Vertex* v1, Vertex* v2)
	{
		if (edge_count >= edge_max)
		{
			fprintf(stderr, "edge over!!");
		}
		Edge*p = &edgeBuffer[edge_count];
		edge_count++;

		p->New(v1, v2);
		return p;
	}


	inline MeshV* __fastcall getMesh()
	{
		return mesh_;
	}
	void ColorVertexMerge();
	void VertexSplit();
	void AddVertex(double* pnt, double* nor, int color, int attr);

	void CreateFaceAttri();
	Face* SolidPart(int face);

	int boundaryLoop(std::vector<std::set<HEdge*> >& boundarys);
	
	~Solid()
	{
		Clear();
		faceAttributeList.clear();
		if ( org_pIndices ) delete [] org_pIndices;
		org_pIndices = NULL;

		delete[] vertexBuffer;
		delete[] faceBuffer;
		delete[] edgeBuffer;
		vertexBuffer = 0;
		faceBuffer = 0;
		edgeBuffer = 0;

		delete[] vertexList;
		vertexList = 0;

		delete[] vertexListIndex;
		vertexListIndex = 0;
	}

	void dump(char* filename);
	void dump(char* filename, std::vector<int> ids);

	inline void __fastcall Clear()
	{
		edgeList.clear();
		//faceList.clear();
		faceList.resize(0);
	}

	inline double areasum(bool recalc)
	{
		double a = 0.0;
		const int faceNum = faceList.size();
		for ( int i = 0; i < faceNum; i++ )
		{
			if ( faceAttributeList[faceList[i]->id].alive == 0 )
			{
				continue;
			}
			if ( recalc ) a += faceAttributeList[faceList[i]->id].CalcArea(mesh_);
			else a += faceAttributeList[faceList[i]->id].area;
		}
		return a;
	}
	inline void __fastcall Initial()
	{
		const int sz = vertex_count;
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (int i = 0; i < sz; i++)
		{
			vertexList[vertexListIndex[i]] = NULL;
		}

		Clear();

		createVertexBuffer();
		createFaceBuffer();
		createEdgeBuffer();
	}
};

int VertexRoundFaces(Vertex* v, std::vector<Face*>& faceList, std::set<Face*>& faces);
int VertexMerge(MeshV& mesh, double tol);
void RemoveMinAreaFace(MeshV& mesh, double tol);
void remove_unreferenced_vertex(MeshV& mesh);
double TriangleQuality( MeshV* mesh, int faceid);
double TriangleArea(MeshV* mesh, int faceid);

MESHSIMPLIFICATION_DLLEXPORTS void* NewMeshV();
MESHSIMPLIFICATION_DLLEXPORTS void* NewSolidV(void* mesh);
MESHSIMPLIFICATION_DLLEXPORTS void DeleteMeshV(void* mesh);
MESHSIMPLIFICATION_DLLEXPORTS void DeleteSolidV(void* mesh);

#endif