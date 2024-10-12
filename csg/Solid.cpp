/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include <time.h>
#include "openmp_config.h"
#include "thread_omp_util.h"

#include "Solid.h"

#include "ML_Vector.h"

#include "VectorSet.h"
#include "IntSet.h"

#include "def.h"
#include "sse_func.h"

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

#include <fstream>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <tchar.h>

#include <GL/gl.h>
#include <GL/glu.h>


uad_double_library* uad_double_library_init(void)
{
	uad_double_library* uad_double_library_ = new uad_double_library;
	return uad_double_library_;
}
void uad_double_library_term(uad_double_library* uad_double_library_)
{
	delete uad_double_library_;
}

int	MatrixLocalAxisSystem2 ( mlFloat nvec[3],  mlFloat matrix[4][4], int invmx_calc, mlFloat matinv[4][4]);

Solid::Solid(const std::string & sFileName)
{
	loadCoordinateFile(sFileName);
	name_id = 0;
}

Solid::Solid(VectorSet * vertices, IntSet * indices)
{
	setData(*vertices, *indices);		
	name_id = 0;
}

Solid::Solid(VectorSet * vertices, IntSet * indices, ColorSet* colors)
{
	setData(*vertices, *indices);

	if ( colors )
	{
		const int sz = colors->GetSize();
		if ( colors->isDefaultColor())
		{
			this->colors.setDefaultColor();
		}else{
			this->colors.setUserColor();
			this->colors.Reserve(sz);
			this->colors.Assgin(*colors);
		}
	}
	name_id = 0;
}

Solid::Solid(VectorSet * vertices, IntSet * indices, int color)
{
	setData(*vertices, *indices);
	setColor( color );
	name_id = 0;
}

Solid::~Solid()
{
	//printf("Deleting Solid\n");
	name_id = 0;
	indices.m_pInts.clear();
	vertices.m_pVectors.clear();
	colors.m_pColors.clear();

	std::vector<int>().swap(indices.m_pInts);
	std::vector<mlVector3D>().swap(vertices.m_pVectors);
	std::vector<color_attr_type>().swap(colors.m_pColors);
}
//---------------------------------------GETS-----------------------------------//

VectorSet * Solid::getVertices()
{
	// This thing makes a fresh copy and hands the requestor the copy.

	VectorSet * newVertices = new VectorSet();

	const int sz = vertices.GetSize();
	newVertices->Resize(sz);

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		newVertices->SetVector(i, vertices[i]);
	}

	return newVertices;
}

IntSet * Solid::getIndices()
{
	IntSet * newIndices = new IntSet();

	const int sz = indices.GetSize();

	newIndices->Resize(sz);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		newIndices->SetInt(i, indices[i]);
	}
	return newIndices;
}

ColorSet * Solid::getColors()
{
	ColorSet * newColors = new ColorSet();

	const int sz = colors.GetSize();
	
	if ( colors.isDefaultColor() ) newColors->setDefaultColor();
	else
	{
		newColors->setUserColor();
		newColors->Resize(sz);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for(int i = 0; i < sz; i++)
		{
			newColors->SetColor(i, colors[i]);
		}
	}
	return newColors;
}

bool Solid::isEmpty()
{
	if(indices.GetSize() <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//---------------------------------------SETS-----------------------------------//
void Solid::setColor(int color)
{
	const int sz = vertices.GetSize();
	colors.m_pColors.clear();

	if ( color < 0 )
	{
		colors.setDefaultColor();
	}else{
		colors.setUserColor();
		colors.Resize(sz);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for ( int i = 0; i < sz; i++ )
		{
			colors.SetColor(i, color);
		}
	}
}


/**
 * Sets the solid data. An exception may occur in the case of abnormal arrays 
 * (indices making references to inexistent vertices, there are less colors 
 * than vertices...)
 * 
 * @param vertices array of points defining the solid vertices
 * @param indices array of indices for a array of vertices
 * @param colors array of colors defining the vertices colors 
 */
void Solid::setData(VectorSet & vertices, IntSet & indices)
{
	// Clear them...
	//this->vertices = new VectorSet();
	//this->colors = new ColorSet();
	//this->indices = new IntSet();

	//this->vertices.Clear();
	//this->colors.Clear();
	//this->indices.Clear();

	const int indices_sz = indices.GetSize();
	if( indices_sz > 0)
	{
		const int sz = vertices.GetSize();
		this->vertices.Resize(sz);
		this->indices.Resize(indices_sz);

#ifdef USE_OPENMP
#pragma omp parallel
#endif
		{
			#ifdef USE_OPENMP
			#pragma omp  for
			#endif
			for(int i=0; i<sz; i++)
			{
				this->vertices.SetVector(i, vertices[i]);
			}
	
			#ifdef USE_OPENMP
			#pragma omp  for
			#endif
			for(int i=0; i<indices_sz; i++)
			{
				this->indices.SetInt(i, indices[i]);
			}
		}
	}
}


//-------------------------GEOMETRICAL_TRANSFORMATIONS-------------------------//

void Solid::Translate(const mlVector3D & t)
{
	const int sz = vertices.GetSize(); 

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		vertices.SetVector(i, vertices.GetVector(i) + t);
	}
}

void Solid::Rotate(const mlVector3D & a, const mlVector3D & b, mlFloat angle)
{
	const int sz = vertices.GetSize();
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		vertices.SetVector(i, mlVectorRotate(vertices.GetVector(i), a, b, angle));
	}
}
void Solid::Rotate(const mlVector3D & n)
{
	mlVector3D nn = n;
	nn.Normalise();
	mlFloat nvec[3];
	mlFloat matrix[4][4];
	mlFloat matinv[4][4];

	nvec[0] = nn.x;
	nvec[1] = nn.y;
	nvec[2] = nn.z;

	MatrixLocalAxisSystem2 ( nvec,  matrix, 0, matinv);
	
	const mlFloat* a = matrix[0];
	const mlFloat* b = matrix[1];
	const mlFloat* c = matrix[2];

	const int sz = vertices.GetSize();
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		mlVector3D& v = vertices.GetVector(i);

		vertices.SetVector(i, mlVector3D(
			v.x*a[0]+v.y*b[0]+v.z*c[0],
			v.x*a[1]+v.y*b[1]+v.z*c[1],
			v.x*a[2]+v.y*b[2]+v.z*c[2]));
	}
}

void Solid::Scale(const mlVector3D & s)
{
	const int sz = vertices.GetSize();
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		vertices.SetVector(i,mlVectorScale(vertices.GetVector(i), s));
	}
}

//-----------------------------------PRIVATES--------------------------------//

/**
 * Gets the solid mean
 * 
 * @return point representing the mean
 */
mlVector3D Solid::getMean()
{
	mlVector3D mean;
	const int sz = vertices.GetSize();
	for(int i=0;i<sz;i++)
	{
		mean.x += vertices[i].x;
		mean.y += vertices[i].y;
		mean.z += vertices[i].z;
	}
	mean.x /= vertices.GetSize();
	mean.y /= vertices.GetSize();
	mean.z /= vertices.GetSize();
	
	return mean;
}

/**
 * Loads a coordinates file, setting vertices and indices 
 * 
 * @param solidFile file used to create the solid
 * @param color solid color
 */
void Solid::loadCoordinateFile(const std::string & sFileName)
{
	std::ifstream modelFile;

	modelFile.open(sFileName.c_str());

	int nNumVertices = 0;
	modelFile >> nNumVertices;

	for(int i = 0; i < nNumVertices; i++)
	{
		// Read in a vector.
		int nVertexID = 0;

		mlVector3D vPosition;

		modelFile >> nVertexID;

		modelFile >> vPosition.x;
		modelFile >> vPosition.y;
		modelFile >> vPosition.z;

		//vPosition = mlVectorScale(vPosition, mlVector3D(0.5f, 0.5f, 0.5f));
		//vPosition = mlVectorScale(vPosition, mlVector3D(2, 2, 2));

		vertices.AddVector(vPosition);
	}

	// Now load up the indices.

	int nNumTriangles = 0;
	modelFile >> nNumTriangles;

	for(int i = 0; i < nNumTriangles; i++)
	{
		int nTriangleID = 0;

		int nTriIndex1 = 0;
		int nTriIndex2 = 0;
		int nTriIndex3 = 0;

		modelFile >> nTriangleID;

		modelFile >> nTriIndex1;
		modelFile >> nTriIndex2;
		modelFile >> nTriIndex3;

		indices.AddInt(nTriIndex1);
		indices.AddInt(nTriIndex2);
		indices.AddInt(nTriIndex3);
	}

	modelFile.close();
}


class Vertex_tmp
{
public:
	mlVector3D* pos;
	color_attr_type color;
	int id;

	inline void set(mlVector3D* v, int idx, const color_attr_type& icolor)
	{
		pos = v;
		id = idx;
		color = icolor;
	}
};

#define USE_KDTREE	0
#include "octree_vtx.h"

#if USE_KDTREE
#include "kdtree\kdtree.hpp"
#endif

static std::vector<Vertex_tmp>* Solid_getPntBuffer;
extern "C" double Solid_getPnt(int id, int i)
{
	std::vector<Vertex_tmp>& p = *Solid_getPntBuffer;
	if (i == 0) return  to_double(p[id].pos->x);
	if (i == 1) return  to_double(p[id].pos->y);
	if (i == 2) return  to_double(p[id].pos->z);

	printf("Solid_getPnt call error\n");
	return to_double(p[id].pos->x);
}

void Solid::Simplify()
{
	clock_t ts = clock();

	const double tol = 1.0e-6;
	const double tol2 = tol*tol;

	const int sz = this->indices.GetSize();
	if ( sz == 0 ) return;

	std::vector<Vertex_tmp> vrtlist;
	vrtlist.resize(sz);
	Solid_getPntBuffer = &vrtlist;

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for ( int i = 0; i < sz; i++ )
	{
		//頂点インデックスを取得
		const int index = this->indices.GetInt(i);

		vrtlist[i].set(&(vertices[index]), index, this->colors.GetColor(index));
	}

#if USE_KDTREE
	KdTree<Vertex_tmp> kdtree;
	kdGetPoint = Solid_getPnt;
#else
	//Octreeへ頂点を登録
	octreeVertex<double,Vertex_tmp> vtxtree;
	vtxtree.buildoctree(vrtlist);

	//Octree構造を残して登録頂点を削除する
	vtxtree.clearVertex();
#endif

	//新たな頂点リストとインデックスリストを用意する
	VectorSet vertex_new;
	IntSet index_new;
	ColorSet color_new;

	if ( this->colors.isDefaultColor()) color_new.setDefaultColor();
	else color_new.setUserColor();

	for ( int i = 0; i < sz; i++ )
	{
		//頂点インデックスを取得
		const int index = vrtlist[i].id;

		//頂点を取得 
		mlVector3D* vtx = &(this->vertices[index]);

		const color_attr_type& color = this->colors.GetColor(index);

		//この頂点の近傍にある頂点を取得する
#if USE_KDTREE
		std::vector<int> list;
		kdtree.nearest_find(to_double(vrtlist[i].pos->x), to_double(vrtlist[i].pos->y), to_double(vrtlist[i].pos->z), tol*1.15f, list);
#else
		std::vector<Vertex_tmp> list;
		vtxtree.getpnts( vrtlist[i], list);
#endif
		//この頂点の近傍には他の頂点が無い
		if ( list.size() == 0 )
		{
			//頂点を登録
			vertex_new.AddVector( *vtx );
			color_new.AddColor( color );
			//その頂点のインデックス
			int vtx_index = vertex_new.GetSize()-1;

			//そのインデックスを登録する
			index_new.AddInt( vtx_index);

			Vertex_tmp p;
			p.set(vtx, vtx_index, color);
#if USE_KDTREE
			vrtlist.push_back(p);
			kdtree.insert(vrtlist.size()-1, 0);
#else
			//Octreeのnodeに登録
			vtxtree.addpnts(p);
#endif
			continue;
		}



		bool dup = false;
		const int sz2 = list.size();
		for ( int j = 0; j < sz2; j++ )
		{
			//近傍頂点との差
#if USE_KDTREE
			const double dxyz[3] = {
				to_double(vrtlist[i].pos->x)-to_double(vrtlist[list[j]].pos->x),
				to_double(vrtlist[i].pos->y)-to_double(vrtlist[list[j]].pos->y),
				to_double(vrtlist[i].pos->z)-to_double(vrtlist[list[j]].pos->z)
			};
#else
			const double dxyz[3] = {
				to_double(vrtlist[i].pos->x)-to_double(list[j].pos->x),
				to_double(vrtlist[i].pos->y)-to_double(list[j].pos->y),
				to_double(vrtlist[i].pos->z)-to_double(list[j].pos->z)
			};
#endif
			//２点間距離を評価
			double dst = dxyz[0]*dxyz[0] + dxyz[1]*dxyz[1] + dxyz[2]*dxyz[2];

			//距離が閾値より離れていたらこの頂点は離れているから無視
#if USE_KDTREE
			if ( dst > tol2 || color.color != vrtlist[list[j]].color.color)
			{
				continue;
			}
#else
			if ( dst > tol2 || color.color != list[j].color.color)
			{
				continue;
			}
#endif
			//同一頂点があった場合
#if USE_KDTREE
			index_new.AddInt(vrtlist[list[j]].id);
#else
			index_new.AddInt(list[j].id);
#endif
			dup = true;
			break;
		}

		//同一頂点は無かったので
		if (!dup )
		{
			//頂点を登録
			vertex_new.AddVector( *vtx );
			color_new.AddColor( color );
			//その頂点のインデックス
			int vtx_index = vertex_new.GetSize()-1;

			//そのインデックスを登録する
			index_new.AddInt( vtx_index);

			Vertex_tmp p;
			p.set(vtx, vtx_index, color);
#if USE_KDTREE
			vrtlist.push_back(p);
			kdtree.insert(vrtlist.size()-1, 0);
#else
			//Octreeのnodeに登録
			vtxtree.addpnts(p);
#endif
		}
	}
	printf("vertex_merge_num %d\n", this->vertices.GetSize()-vertex_new.GetSize());
	printf("Simplify Solid ==> %d/%d (%.3f %%) time %d ms\n", vertex_new.GetSize(), this->vertices.GetSize(), 
		100.0-100.0*(double)vertex_new.GetSize()/(double)this->vertices.GetSize(), clock()-ts);
	fflush(stdout);

	this->indices.Assgin(index_new);

	IntSet index_new2;
	const int n = this->indices.GetSize()/3;
	for ( int i = 0; i < n; i++ )
	{
		if (	this->indices[3*i+0] == this->indices[3*i+1] ||
				this->indices[3*i+1] == this->indices[3*i+2] ||
				this->indices[3*i+2] == this->indices[3*i+0] )
		{
			continue;
		}
		index_new2.AddInt(this->indices[3*i+0]);
		index_new2.AddInt(this->indices[3*i+1]);
		index_new2.AddInt(this->indices[3*i+2]);
	}
	this->indices.m_pInts.swap(index_new2.m_pInts);

	this->vertices.Assgin(vertex_new);
	if ( !this->colors.isDefaultColor() )
	{
		this->colors.Assgin( color_new );
	}
}


#include "export.h"
void EdgeCollapse::Construction()
{

	//ファセット列を構築する
	faceList.resize(FaceNum);
	for ( int i = 0; i < FaceNum; i++ )
	{
		simFace f;
		simEdge e[3];

		e[0].start_vtx = 3*i+0;
		e[0].end_vtx   = 3*i+1;

		e[1].start_vtx = 3*i+1;
		e[1].end_vtx   = 3*i+2;

		e[2].start_vtx = 3*i+2;
		e[2].end_vtx   = 3*i+0;

		e[0].Flush(solid_->vertices, solid_->indices);
		e[1].Flush(solid_->vertices, solid_->indices);
		e[2].Flush(solid_->vertices, solid_->indices);
		
		f.e[0] = e[0];
		f.e[1] = e[1];
		f.e[2] = e[2];
		if ( e[0].start_vtx == e[1].start_vtx || e[1].start_vtx == e[2].start_vtx || e[2].start_vtx == e[0].start_vtx )
		{
			f.collapse_edge = true;
		}
		f.Flush(solid_->vertices, solid_->indices);
		faceList[i] = f;
	}
}

void EdgeCollapse::Adjacency()
{
	int rinnum = 0;
	for ( int i = 0; i < FaceNum; i++ )
	{
		if ( faceList[i].e[0].side_face_id >= 0 && faceList[i].e[1].side_face_id >= 0 && faceList[i].e[2].side_face_id >= 0 )
		{
			continue;
		}

		for ( int j = i+1; j < FaceNum; j++ )
		{
			for ( int k = 0; k < 3; k++ )
			{
				if ( solid_->indices[faceList[i].e[0].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] && solid_->indices[faceList[i].e[0].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] )
				{
					faceList[i].e[0].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}else
				if ( solid_->indices[faceList[i].e[1].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] && solid_->indices[faceList[i].e[1].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] )
				{
					faceList[i].e[1].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}else
				if ( solid_->indices[faceList[i].e[2].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] && solid_->indices[faceList[i].e[2].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] )
				{
					faceList[i].e[2].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}else
				if ( solid_->indices[faceList[i].e[0].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] && solid_->indices[faceList[i].e[0].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] )
				{
					faceList[i].e[0].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}else
				if ( solid_->indices[faceList[i].e[1].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] && solid_->indices[faceList[i].e[1].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] )
				{
					faceList[i].e[1].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}else
				if ( solid_->indices[faceList[i].e[2].end_vtx] == solid_->indices[faceList[j].e[k].start_vtx] && solid_->indices[faceList[i].e[2].start_vtx] == solid_->indices[faceList[j].e[k].end_vtx] )
				{
					faceList[i].e[2].side_face_id = j;
					faceList[j].e[k].side_face_id = i;
					rinnum++;
				}
			}
		}
	}
	//printf("ファセットの隣接関係 %d\n", rinnum);

}

bool EdgeCollapse::isSurrounded(int face)
{
	//ファセットが他のファセットに取り囲まれているか
	if (	faceList[face].e[0].side_face_id < 0	||
			faceList[face].e[1].side_face_id < 0	||
			faceList[face].e[2].side_face_id < 0 )
	{
		return false;
	}
	return true;
}

int EdgeCollapse::LoockUpTarget( int face )
{
	//潰したファセット無視
	if ( faceList[face].collapse_edge )
	{
		return 0;
	}
	//ファセットが他のファセットに取り囲まれていない場合は処理しない
	if ( !isSurrounded(face) )
	{
		return 0;
	}


	//周辺でedge collapse (ecol)されていたらやらない
	bool can = false;
	for ( int k = 0; k < 3; k++ )
	{
		if ( faceList[faceList[face].e[k].side_face_id].collapse_edge )
		{
			can = true;
			break;
		}
	}
	if ( can ) return 0;

	return 1;
}

void EdgeCollapse::AroundFacet(simEdge& edge, std::vector<flistS>& flist)
{
	//頂点周りのファセットを集める
	for ( int kk = 0; kk < FaceNum; kk++ )
	{
		for ( int kkk = 0; kkk < 3; kkk++ )
		{
			if ( solid_->indices[faceList[kk].e[kkk].start_vtx] == solid_->indices[edge.start_vtx])
			{
				flistS f;
				f.face_id = kk;
				f.edge = kkk;
				f.start = true;
				f.chk[0] = false;
				f.chk[1] = false;

				flist.push_back(f);
				break;
			}
			if ( solid_->indices[faceList[kk].e[kkk].end_vtx] == solid_->indices[edge.start_vtx])
			{
				flistS f;
				f.face_id = kk;
				f.edge = kkk;
				f.start = false;
				f.chk[0] = false;
				f.chk[1] = false;
					
				flist.push_back(f);
				break;
			}
		}
		for ( int kkk = 0; kkk < 3; kkk++ )
		{
			if ( solid_->indices[faceList[kk].e[kkk].start_vtx] == solid_->indices[edge.end_vtx])
			{
				flistS f;
				f.face_id = kk;
				f.edge = kkk;
				f.start = true;
				f.chk[0] = false;
				f.chk[1] = false;
					
				flist.push_back(f);
				break;
			}
			if ( solid_->indices[faceList[kk].e[kkk].end_vtx] == solid_->indices[edge.end_vtx])
			{
				flistS f;
				f.face_id = kk;
				f.edge = kkk;
				f.start = false;
				f.chk[0] = false;
				f.chk[1] = false;
					
				flist.push_back(f);
				break;
			}
		}
	}
}

bool EdgeCollapse::ChecksAdjacency(std::vector<flistS>& flist)
{
	//各ファセットのエッジがどれかと必ず隣接関係にあるかチェックする
	for ( int kk = 0; kk < flist.size(); kk++ )
	{
		//flist[kk].face_id のファセットのエッジがflistのどれかと隣接関係にあるか？
		int cnt = 0;
		for ( int jj = 0; jj < 3; jj++ )
		{
			//flist[kk].face_id のファセットのエッジ(jj)に隣接すファセットID(id)
			int id = faceList[flist[kk].face_id].e[jj].side_face_id;

			//集めたファセット群のどれかにこのファセットがあるか？
			for ( int kkk = 0; kkk < flist.size(); kkk++ )
			{
				if ( cnt == 2) break;
				if ( kk == kkk ) continue;
				if ( flist[kkk].face_id == id )
				{
					//あった
					flist[kk].chk[cnt] = true;
					cnt++;
				}
			}
			if ( cnt == 2) break;
		}
	}

	//隙間がある場合は処理しない
	bool err = false;
	for ( int kk = 0; kk < flist.size(); kk++ )
	{
		if ( !flist[kk].chk[0]  || !flist[kk].chk[1])
		{
			err = true;
			break;
		}
	}
	if ( err )
	{
		return false;
	}

	return true;
}

bool EdgeCollapse::CheckFaceEdge(int face, double tol2, double angle, edge_collapse_target& target)
{
	int flat_check = -1;

	target.f1 = face;
	for ( int ek = 0; ek < 3; ek++ )
	{
		target.f2 = -1;
		target.e1 = -1;
		target.e2 = -1;
		target.x = false;
		flat_check = -1;

		//微小エッジ
		if ( faceList[face].e[ek].length2 < tol2 )
		{
			//このエッジに隣接するファセット
			target.f2 =  faceList[face].e[ek].side_face_id;
			if ( target.f2 < 0 )
			{
				//隣接ファセットが無い
				continue;
			}

			//潰したファセット無視
			if ( faceList[target.f2].collapse_edge )
			{
				continue;
			}

			//この隣接ファセットも他のファセットに取り囲まれていない場合は処理しない
			if (! isSurrounded(target.f2) )
			{
				continue;
			}
			target.e1 = ek;
		}
		
		if ( target.e1 == -1 )
		{
			continue;
		}

		//周辺でedge collapse (ecol)されていたらやらない
		bool can = false;
		for ( int k = 0; k < 3; k++ )
		{
			if ( faceList[faceList[target.f2].e[k].side_face_id].collapse_edge )
			{
				can = true;
				break;
			}
		}
		if ( can ) continue;
		

		for ( int k = 0; k < 3; k++ )
		{
			if ( 
				solid_->indices[faceList[face].e[target.e1].start_vtx] == solid_->indices[faceList[target.f2].e[k].end_vtx] &&
				solid_->indices[faceList[face].e[target.e1].end_vtx] == solid_->indices[faceList[target.f2].e[k].start_vtx] )
			{
				target.e2 = k;
				break;
			}
		}
		if ( target.e2 == -1 )
		{
			continue;
		}


		//頂点周りのファセットを集める
		AroundFacet(faceList[face].e[target.e1], target.flist);

		//各ファセットのエッジがどれかと必ず隣接関係にあるかチェックする
		if ( !ChecksAdjacency(target.flist) )
		{
			continue;
		}
		
		//edge collapse (ecol)するファセット間が折れていたらやらない
		mlVector3D n1 = faceList[face].getNormal();
		mlVector3D n2 = faceList[target.f2].getNormal();

		if ( VECTOR3D_DOT(n1, n2) < angle )
		{
			target.x = true;
			//continue;
		}

#if 10

		n1 = 0.5*(n1 + n2);
		n1.Normalise();

		//edge collapse (ecol)するファセット周りのファセットが折れていたらやらない
		bool err = false;
		for ( int jj = 0; jj < target.flist.size(); jj++ )
		{
			mlVector3D n3 = faceList[ target.flist[jj].face_id ].getNormal();
			if ( VECTOR3D_DOT(n1, n3) < angle )
			{
				err = true;
				break;
			}
		}
		if ( err )
		{
			target.x = true;
		}
#endif
		flat_check = 1;
		break;
	}

	if ( flat_check != 1 || target.e1 == -1 || target.e2 == -1 || target.f2 == -1 )
	{
		return false;
	}
	return true;
}

double EdgeCollapse::CalcCost(mlVector3D& vtx1, mlVector3D& vtx2, mlVector3D& midVtx, edge_collapse_target& target)
{
	//変形コストを計算する
	double cost = 0.0;
	for ( int jj = 0; jj < target.flist.size(); jj++ )
	{
		double d1 = faceList[target.flist[jj].face_id].computeDistance(midVtx, solid_->vertices, solid_->indices);

		d1 += 0.001;

		double area1 = faceList[target.flist[jj].face_id].getArea();
		double qes1 = faceList[target.flist[jj].face_id].getQes();

		//一時的にedge collapse (ecol)を行う
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].start_vtx]] = midVtx;
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].end_vtx]] = midVtx;

		double area2 = faceList[target.flist[jj].face_id].calcArea(solid_->vertices, solid_->indices);
		double qes2 = faceList[target.flist[jj].face_id].calcEquiAnglSkew(solid_->vertices, solid_->indices);

		cost += (d1*fabs(area1-area2)*(qes2 - qes1))*(d1*fabs(area1-area2)*(qes2 - qes1));

		//元に戻す
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].start_vtx]] = vtx1;
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].end_vtx]] = vtx2;
	}
	
	return cost;
}

bool EdgeCollapse::Collapse(double angle, edge_collapse_target& target, int& collapse)
{
	int tget_face = target.f1;
	int side_face = target.f2;
	int collapse_edge1 = target.e1;
	int collapse_edge2 = target.e2;

	//頂点を取得 
	mlVector3D vtx1 = solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].start_vtx]];
	mlVector3D vtx2 = solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].end_vtx]];
	
	//中点を計算
	mlVector3D midVtx = target.c*(vtx2 + vtx1);

	mlVector3D newVtx[3] = {midVtx, vtx1, vtx2};
	
	for ( int ii = 0; ii < 3; ii++ )
	{
		collapse++;
		//edge collapse (ecol)を行う
		solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].start_vtx]] = newVtx[ii];
		solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].end_vtx]] = newVtx[ii];

		//edge collapse (ecol)して周囲のファセットのどれかが反転するならやらない
		bool err = false;
		for ( int jj = 0; jj < target.flist.size(); jj++ )
		{
			if ( target.flist[jj].face_id == tget_face || target.flist[jj].face_id == side_face )
			{
				continue;
			}
			mlVector3D n1 = faceList[ target.flist[jj].face_id ].getNormal();
			mlVector3D n2 = faceList[ target.flist[jj].face_id ].calcNormal(solid_->vertices, solid_->indices);;
			
			if ( n2.MagnitudeSquared() < 1.0e-16 )
			{
				//完全に潰れたなら無視
				continue;
			}
			double a = angle;
			if (target.x )
			{
				a = 0.9999999;
			}
			if ( VECTOR3D_DOT(n1, n2) < a )
			{
				err = true;
				break;
			}

			double qes1 = faceList[ target.flist[jj].face_id ].getQes();
			double qes2 = faceList[ target.flist[jj].face_id ].calcEquiAnglSkew(solid_->vertices, solid_->indices);
			
			if ( (qes2-qes1)*(qes2-qes1) > 0.8 )
			{
				err = true;
				break;
			}
		}
		if ( err )
		{
			//元に戻す
			solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].start_vtx]] = vtx1;
			solid_->vertices[solid_->indices[faceList[tget_face].e[collapse_edge1].end_vtx]] = vtx2;
			collapse--;
		}else
		{
			//edge collapse (ecol)成功
			return true;
		}
	}
	return false;
}



int EdgeCollapse::Simplify(const double collapse_tol, const double angle, const double tol)
{
	if ( FaceNum == 0 )
	{
		return 0;
	}

	clock_t ts = clock();
	
	//solid_->Simplify();

	const double tol2 = collapse_tol*collapse_tol;

	//ファセット列を構築する
	Construction();

	//ファセットの隣接関係を構築する
	Adjacency();


	int collapse = 0;
	int errnum = 0;

	std::vector<edge_collapse_target> edge_collapse_target_List;

	//edge collapse (ecol)可能なファセットを探す
	for ( int i = 0; i < FaceNum; i++ )
	{
		int tget_face = i;
		if ( !LoockUpTarget(tget_face) )
		{
			continue;
		}

		edge_collapse_target target;

		//edge collapse (ecol)可能なファセットか？
		if ( !CheckFaceEdge(tget_face, tol2, angle, target) )
		{
			continue;
		}

		//頂点を取得 
		mlVector3D vtx1 = solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].start_vtx]];
		mlVector3D vtx2 = solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].end_vtx]];

		double cost_min = 999999999999999999999.0;
		int cost_min_id = -1;
		double coef[] = { 0.25, 0.5, 0.75};

		for ( int kk = 0; kk < 3; kk++ )
		{
			//中点を計算
			mlVector3D midVtx = (1.0 - coef[kk])*vtx1 + coef[kk]*vtx2;

			//変形コストを計算する
			double cost = 0.0;
			for ( int jj = 0; jj < target.flist.size(); jj++ )
			{
				cost +=  CalcCost( vtx1, vtx2, midVtx, target);
			}
			if ( cost < cost_min )
			{
				cost_min = cost;
				cost_min_id = kk;
			}
		}
		//元に戻す
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].start_vtx]] = vtx1;
		solid_->vertices[solid_->indices[faceList[target.f1].e[target.e1].end_vtx]] = vtx2;

		if ( cost_min_id == -1 )
		{
			continue;
		}

		target.c = coef[cost_min_id];
		edge_collapse_target_List.push_back(target);

#if 0
		//情報が編集されたので関連するファセット（周辺ファセット）でedge collapse (ecol)をしないようにする。
		faceList[tget_face].collapse_edge = true;
		faceList[side_face].collapse_edge = true;
#if 0
		for (int kk = 0; kk < 3; kk++ )
		{
			if (faceList[tget_face].e[kk].side_face_id >= 0 )
			{
				faceList[ faceList[tget_face].e[kk].side_face_id ].collapse_edge = true;
			}
			if (faceList[side_face].e[kk].side_face_id >= 0)
			{
				faceList[ faceList[side_face].e[kk].side_face_id ].collapse_edge = true;
			}
			faceList[tget_face].e[kk].side_face_id = -1;
			faceList[side_face].e[kk].side_face_id = -1;
		}
#endif
#endif
	}
	std::sort(edge_collapse_target_List.begin(), edge_collapse_target_List.end());


	for ( int i = 0; i < edge_collapse_target_List.size(); i++ )
	{
		Collapse( angle, edge_collapse_target_List[i], collapse);
	}
	printf("collapse %d\n", collapse);


	IntSet new_indices;
	for ( int i = 0; i < FaceNum; i++ )
	{
		if (	solid_->indices[3*i+0] == solid_->indices[3*i+1] ||
				solid_->indices[3*i+1] == solid_->indices[3*i+2] ||
				solid_->indices[3*i+2] == solid_->indices[3*i+0] )
		{
			continue;
		}
		new_indices.AddInt(solid_->indices[3*i+0]);
		new_indices.AddInt(solid_->indices[3*i+1]);
		new_indices.AddInt(solid_->indices[3*i+2]);
	}
	solid_->indices.m_pInts.swap(new_indices.m_pInts);
	
	if ( collapse )
	{
		solid_->Simplify();
	}

	return collapse;
}

