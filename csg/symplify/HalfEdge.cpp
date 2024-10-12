#include "HalfEdge.hpp"
#include "../kdtree\kdtree.hpp"
#include <chrono>

#include <omp.h>

Loop::Loop(Face* f)
{
	face_ = f;
}

void Face::New(Solid* solid, unsigned int faceid, unsigned int id1, unsigned int id2, unsigned int id3)
{
	error_ = false;
	alive = 1;
	id = faceid;
	solid_ = solid;
	if (loop) delete loop;
	loop = NULL;


	if ( id1 == id2 || id1 == id3 || id2 == id3 )
	{
		alive = 0;
		return;
	}

	index = &(solid->getMesh()->pIndices[3*faceid]);

	Vertex* v1 = solid->vertexList[id1];
	Vertex* v2 = solid->vertexList[id2];
	Vertex* v3 = solid->vertexList[id3];

	Edge* e1 = solid->NewEdge(v1, v2);
	Edge* e2 = solid->NewEdge(v2, v3);
	Edge* e3 = solid->NewEdge(v3, v1);
	
	std::pair<std::set<Edge*,EdgeSet>::iterator, bool>	 ee1 = solid->edgeList.insert(e1);
	std::pair<std::set<Edge*,EdgeSet>::iterator, bool>	 ee2 = solid->edgeList.insert(e2);
	std::pair<std::set<Edge*,EdgeSet>::iterator, bool>	 ee3 = solid->edgeList.insert(e3);
	 
	if ( !ee1.second )
	{
		e1 = *(ee1.first);
	}
	if ( !ee2.second )
	{
		e2 = *(ee2.first);
	}
	if ( !ee3.second )
	{
		e3 = *(ee3.first);
	}

	Edge* error_edge[3] = {NULL, NULL, NULL};
	if ( e1->HalfEdgeRefCount() >= 2 )
	{
		error_ = true;
		error_edge[0] = e1;
		//printf("\nERROR v%d -- v%d\n", e1->bound_.first->id_, e1->bound_.second->id_);
	}
	if ( e2->HalfEdgeRefCount() >= 2 )
	{
		error_ = true;
		error_edge[1] = e2;
		//printf("\nERROR v%d -- v%d\n", e2->bound_.first->id_, e2->bound_.second->id_);
	}
	if ( e3->HalfEdgeRefCount() >= 2 )
	{
		error_ = true;
		error_edge[2] = e3;
		//printf("\nERROR v%d -- v%d\n", e3->bound_.first->id_, e3->bound_.second->id_);
	}

	if ( error_ )
	{
		alive = 0;
		//std::vector<int> fids;
		//fids.push_back(faceid);
		//for ( int i = 0; i < 3; i++ )
		//{
		//	if ( error_edge[i] )
		//	{
		//		fids.push_back(	error_edge[i]->hedge[0]->loop_->face_->id);
		//		fids.push_back(	error_edge[i]->hedge[1]->loop_->face_->id);
		//	}
		//}
		//char fname[256];
		//sprintf(fname, "topology_%04d.obj", faceid);
		//solid->dump(fname, fids);
		return;
	}

	loop = new Loop(this);
	HEdge* he[3];

	he[0] = new HEdge(loop, e1, v1, v2);
	he[1] = new HEdge(loop, e2, v2, v3);
	he[2] = new HEdge(loop, e3, v3, v1);
	loop->hedge = he[0];
	he[0]->prev = he[2];
	he[0]->next = he[1];
	he[1]->prev = he[0];
	he[1]->next = he[2];
	he[2]->prev = he[1];
	he[2]->next = he[0];

}

double* Face::Normal() const
{
	return solid_->faceAttributeList[id].normal_;
}


class VertexS
{
public:
	int id;
};

MeshV* _halfedge__MeshPtr;
extern "C" double halfedge__GetPoint(int pos, int i)
{
	return _halfedge__MeshPtr->pVertices[3*pos+i];
}
void Solid::ColorVertexMerge()
{
	//色違いの同一座標のマージ
	int merge_num = 0;
	const double tol = 1.0e-16;
	const double tol2 = tol*tol;
	const double serch_r = 0.01;

	_halfedge__MeshPtr = getMesh();
	if ( _halfedge__MeshPtr->pColors == NULL )
	{
		return;
	}

	kdGetPoint = halfedge__GetPoint;
	KdTree<VertexS> kdtree;

	int sz = _halfedge__MeshPtr->iNumIndices;
	for ( int i = 0; i < sz; i++ )
	{
			printf("[%d]      \r\r\r", sz-i-1);
		double *vertex[3];
		unsigned int* vertex_id = &(_halfedge__MeshPtr->pIndices[3*i]);
		Color* color[3] = {NULL, NULL, NULL};

		color[0] = &(_halfedge__MeshPtr->pColors[vertex_id[0]]);
		color[1] = &(_halfedge__MeshPtr->pColors[vertex_id[1]]);
		color[2] = &(_halfedge__MeshPtr->pColors[vertex_id[2]]);

		vertex[0] = &(_halfedge__MeshPtr->pVertices[3*vertex_id[0]]);
		vertex[1] = &(_halfedge__MeshPtr->pVertices[3*vertex_id[1]]);
		vertex[2] = &(_halfedge__MeshPtr->pVertices[3*vertex_id[2]]);

		for ( int j = 0; j < 3; j++ )
		{
			std::vector<int> nearest_list;
			kdtree.nearest_find(vertex[j], serch_r, nearest_list);

			if ( nearest_list.size() == 0 )
			{
				VertexS v;
				v.id = vertex_id[j];
				kdtree.insert(vertex_id[j], &v);
				continue;
			}

			bool merge = false;
			for ( int k = 0; k < nearest_list.size(); k++ )
			{
				int id = nearest_list[k];

				if ( vertex_id[j] == id ) continue;

				double* v = &(_halfedge__MeshPtr->pVertices[3*id]);
				Color* c = NULL;
				
				double dx = (v[0] - vertex[j][0]);
				double dy = (v[1] - vertex[j][1]);
				double dz = (v[2] - vertex[j][2]);

				//座標値が同じで
				//if ( dx*dx + dy*dy + dz*dz < tol2 )
				if( v[0] == vertex[j][0] && v[1] == vertex[j][1] && v[2] == vertex[j][2] )
				{
					//色が違う場合は普通のvertexマージで共通化されない箇所なので
					if (_halfedge__MeshPtr->pColors[id].rgb != color[j]->rgb)
					{
						//ここでは一旦マージする（処理の後で再ど分割する(VertexSplit))
						vertex_id[j] = id;
						merge = true;
						merge_num++;
						break;
					}
				}
			}
			if ( !merge )
			{
				VertexS v;
				v.id = vertex_id[j];
				kdtree.insert(vertex_id[j], &v);
			}
		}
	}
	printf("\nvertex Merge:%d\n", merge_num);
}

void Solid::VertexSplit()
{
	if ( mesh_->pColors == NULL )
	{
		return;
	}

	const int sz = org_iNumIndices;
	for ( int i = 0; i < sz; i++ )
	{
		unsigned int* cur = &(mesh_->pIndices[3*i]);
		unsigned int* org = &(org_pIndices[3*i]);

		if ( cur[0] == cur[1] || cur[0] == cur[2] || cur[1] == cur[2] )
		{
			continue;
		}
		if ( cur[0] == org[0] && cur[1] == org[1] && cur[2] == org[2] )
		{
			continue;
		}

		if (	(mesh_->pColors[cur[0]].rgb == mesh_->pColors[org[0]].rgb) &&
				(mesh_->pColors[cur[1]].rgb == mesh_->pColors[org[1]].rgb) && 
				(mesh_->pColors[cur[2]].rgb == mesh_->pColors[org[2]].rgb))
		{
			continue;
		}

		double pnt[3];
		int color;

		for ( int j = 0; j < 3; j++ )
		{
			if ( cur[j] != org[j] && (mesh_->pColors[cur[j]].rgb != mesh_->pColors[org[j]].rgb))
			{
				pnt[0] = mesh_->pVertices[3*cur[j]];
				pnt[1] = mesh_->pVertices[3*cur[j]+1];
				pnt[2] = mesh_->pVertices[3*cur[j]+2];

				mesh_->pVertices[3*org[j]] = pnt[0];
				mesh_->pVertices[3*org[j]+1] = pnt[1];
				mesh_->pVertices[3*org[j]+2] = pnt[2];
				cur[j] = org[j];
			}
		}
	}
}

void Solid::AddVertex(double* pnt, double* nor, int color, int attr)
{
	int pre_NumVertices = mesh_->iNumVertices;

	mesh_->iNumVertices++;

	double *new_vertex = new double[3*mesh_->iNumVertices];
	memcpy(new_vertex, mesh_->pVertices, 3*sizeof(double)*pre_NumVertices);
	delete [] mesh_->pVertices;

	double *new_normal = NULL;
	if ( mesh_->pNormals != NULL )
	{
		new_normal = new double[3*mesh_->iNumVertices];
		memcpy(new_normal, mesh_->pNormals, 3*sizeof(double)*pre_NumVertices);
		delete [] mesh_->pNormals;
	}

	Color *new_color = NULL;
	
	if ( mesh_->pColors != NULL )
	{
		new_color = new Color[mesh_->iNumVertices];
		memcpy(new_color, mesh_->pColors, sizeof(Color)*pre_NumVertices);
		delete [] mesh_->pColors;
	}

	mesh_->pVertices = new_vertex;
	mesh_->pColors = new_color;
	mesh_->pNormals = new_normal;

	new_vertex[3*pre_NumVertices  ] = pnt[0];
	new_vertex[3*pre_NumVertices+1] = pnt[1];
	new_vertex[3*pre_NumVertices+2] = pnt[2];
	if ( new_normal )
	{
		new_normal[3*pre_NumVertices  ] = nor[0];
		new_normal[3*pre_NumVertices+1] = nor[1];
		new_normal[3*pre_NumVertices+2] = nor[2];
	}
	if ( new_color != NULL )
	{
		new_color[pre_NumVertices].rgb = color;
		new_color[pre_NumVertices].attr = (unsigned char)attr;
	}
}

void Solid::CreateFaceAttri()
{
	const int sz = mesh_->iNumIndices;
	for ( int ii = 0; ii < sz; ii++ )
	{
		const int I1 = 3*ii;
		const int I2 = I1+1;
		const int I3 = I2+1;
		int id[3];

		id[0] = mesh_->pIndices[I1];
		id[1] = mesh_->pIndices[I2];
		id[2] = mesh_->pIndices[I3];

		double* p[3];

		p[0] = &(mesh_->pVertices[3*id[0]]);
		p[1] = &(mesh_->pVertices[3*id[1]]);
		p[2] = &(mesh_->pVertices[3*id[2]]);

		if ( mesh_->vertexAttr && mesh_->pColors[id[0]].attr == 0 && mesh_->pColors[id[1]].attr == 0 && mesh_->pColors[id[2]].attr == 0 )
		{
			faceAttributeList[ii].skipp = true;
		}
		double v[3][3];

		VEC3_SUB(v[0], p[1], p[0]);
		VEC3_SUB(v[1], p[2], p[1]);
		VEC3_CROSS(faceAttributeList[ii].normal_, v[0], v[1]);
		faceAttributeList[ii].area = sqrt(DOTPRODUCT(faceAttributeList[ii].normal_))*0.5;
		VEC3_NORMALIZE(faceAttributeList[ii].normal_);
		faceAttributeList[ii].id_ = ii;

		faceAttributeList[ii].quality = TriangleQuality(mesh_, ii);

	}
}

#if 0
//#define START()		start = std::chrono::system_clock::now();
//#define END(n)		end = std::chrono::system_clock::now();fprintf(stderr, "\t(%d)time:%.6f\n", n, (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
#define START()		start = clock();
#define END(n)		end = clock();fprintf(stderr, "\t(%d)time:%.6f\n", n, (double)(end - start)/(double)CLOCKS_PER_SEC);
#else
#define START()
#define END(n)
#endif

Face* Solid::SolidPart(int face)
{
	
	clock_t  start, end;
	unsigned int* index;
	Face* ff = NULL;

	index = &(mesh_->pIndices[3*face]);

	if ( index[0] == index[1] || index[0] == index[2] || index[1] == index[2] )
	{
		printf("潰れた三角形 %d\n", face);
		return NULL;
	}


	const int triNum = mesh_->iNumIndices;

	//double *p1 = mesh_->VertexCoord(index[0]);
	//double *p2 = mesh_->VertexCoord(index[1]);
	//double *p3 = mesh_->VertexCoord(index[2]);
	//double minv[3] = { 9999999999.0,9999999999.0,9999999999.0 };
	//double maxv[3] = { -9999999999.0,-9999999999.0,-9999999999.0 };

	//for (int j = 0; j < 3; j++)
	//{
	//	if (p1[j] < minv[j]) minv[j] = p1[j];
	//	if (p2[j] < minv[j]) minv[j] = p2[j];
	//	if (p3[j] < minv[j]) minv[j] = p3[j];

	//	if (p1[j] > maxv[j]) maxv[j] = p1[j];
	//	if (p2[j] > maxv[j]) maxv[j] = p2[j];
	//	if (p3[j] > maxv[j]) maxv[j] = p3[j];
	//}

	START();
	for (int i = 0; i < 3; i++)
	{
		//三角形頂点index[#]に接続する全Face
		const int sz = mesh_->faceGroup[index[i]].size();
		for (std::set<unsigned int>::iterator it = mesh_->faceGroup[index[i]].begin(); it != mesh_->faceGroup[index[i]].end(); ++it)
		{
			// 三角形頂点index[#]に接続する Face[##]
			const unsigned int findex = *it;

			// Face[##]の頂点
			const unsigned int* pindex = &(mesh_->pIndices[3*findex]);

			//頂点座標を取得
			if (vertexList[pindex[0]] == NULL) vertexList[pindex[0]] = this->NewVertex(mesh_, pindex[0]);
			if (vertexList[pindex[1]] == NULL) vertexList[pindex[1]] = this->NewVertex(mesh_, pindex[1]);
			if (vertexList[pindex[2]] == NULL) vertexList[pindex[2]] = this->NewVertex(mesh_, pindex[2]);

			//Faceの実態を生成（エッジとFaceのループも生成）
			Face* f = NewFace(findex, pindex[0], pindex[1], pindex[2]);
			if (!f->alive)
			{
				faceAttributeList[findex].alive = 0;
				continue;
			}
			if ( findex == face)
			{
				ff = f;
			}
			faceList.push_back(f);
		}
	}
	END(1);
	return ff;
}



int Solid::boundaryLoop(std::vector<std::set<HEdge*> >& boundarys)
{
	int stat = 0;
	const int edgeNum = edgeList.size();

	std::set<Edge*,EdgeSet> flag;
	std::set<Edge*,EdgeSet>::iterator it = edgeList.begin();

	//エッジリストの内で隣の内エッジを辿ってループするものを集める
	//ループが１本なら分離していない領域を取得出来ている筈。
	for ( int i = 0; i < edgeNum; i++ )
	{
		std::set<HEdge*> cloop;
		//片側だけにFaceが付随していて一度もたどっていないエッジで潰されていないならループの開始
		if ( (*it)->HalfEdgeRefCount() == 1 && (*it)->hedge[0]->edge_->used == 0 && (*it)->hedge[0]->edge_->alive)
		{
			cloop.insert((*it)->hedge[0]);
			(*it)->hedge[0]->edge_->used = 1;
			flag.insert((*it)->hedge[0]->edge_);

			Edge* pre = (*it)->hedge[0]->edge_;

			//エッジの始点
			Vertex* s0 =  (*it)->hedge[0]->StartPos();

			//エッジの終点
			Vertex* e0 =  (*it)->hedge[0]->EndPos();
			Vertex* e =  e0;
			Edge* next_edge = NULL;	//次のエッジ
			do{
				next_edge = NULL;

				//終点から出て行くエッジで
				std::set<HEdge*>::iterator jt = e->outhedge.begin();
				for ( ; jt != e->outhedge.end(); ++jt)
				{
					if ( (*jt)->edge_ == pre )
					{
						//折り返しは駄目
						continue;
					}
					//片側だけにFaceが付随していてエッジで潰されていない
					if ( (*jt)->edge_->HalfEdgeRefCount() == 1 && (*jt)->edge_->alive )
					{
						//一度たどっていたら違う
						if ( (*jt)->edge_->used >= 1 )
						{
							next_edge = NULL;
							continue;
						}

						next_edge = (*jt)->edge_;
						Vertex* etmp = next_edge->hedge[0]->EndPos();
						e = etmp;
						cloop.insert(next_edge->hedge[0]);
						next_edge->hedge[0]->edge_->used = 1;
						flag.insert(next_edge->hedge[0]->edge_);
						pre = next_edge->hedge[0]->edge_;
						break;
					}
					if ( next_edge ) break;
				}
				if ( next_edge == NULL )
				{
					//printf("slit %d\n", e0->id_);
					stat = 1;
					break;
				}
			}while( e != s0 );	//元の位置に戻ったら「閉じた」

			//辿った印を消す
			std::set<Edge*,EdgeSet>::iterator ks = flag.begin();
			for ( ; ks != flag.end(); ++ks )
			{
				if ( (*ks)->used == 1 ) (*ks)->used = 0;
			}
			flag.clear();

			//ループを辿ったら
			if ( e == s0 && next_edge != NULL )
			{
				//辿った箇所は印を付けておく
				std::set<HEdge*>::iterator kt = cloop.begin();
				for ( ; kt != cloop.end(); ++kt )
				{
					(*kt)->edge_->used = 2;
					flag.insert((*kt)->edge_);
				}
				if ( next_edge != NULL )
				{
					//外周ループの登録
					boundarys.push_back(cloop);
				}
			}
			//if ( boundarys.size() > 2 ) break;
		}
		++it;
	}

	bool error_trace = false;
	//辿った印を消す
	std::set<Edge*,EdgeSet>::iterator kt = flag.begin();
	for ( ; kt != flag.end(); ++kt )
	{
		if ( (*kt)->used != 2 ) error_trace = true;
		(*kt)->used = 0;
	}
	if ( boundarys.size() ) stat = 0;
	if ( error_trace )
	{
		printf("error trace\n");
		stat = -1;
	}

	//printf("loop - %d\n", boundarys.size());
	return stat;
}



void Solid::dump(char* filename)
{
	FILE* fp = fopen( filename, "w");

	int vn = 1;
	for ( int i = 0; i < this->faceList.size(); i++ )
	{
		double* p;
		HEdge* he;

		he = faceList[i]->loop->hedge;
		p = he->StartPos()->coord();
		fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
		he = he->next;
		p = he->StartPos()->coord();
		fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
		he = he->next;
		p = he->StartPos()->coord();
		fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
		fprintf(fp, "f %d %d %d\n", vn, vn+1, vn+2);
		vn += 3;
	}
	fclose(fp);
}

void Solid::dump(char* filename, std::vector<int> ids)
{
	FILE* fp = fopen( filename, "w");

	int vn = 1;
	for ( int i = 0; i < ids.size(); i++ )
	{
		double* p;
		HEdge* he;

		for ( int j = 0; j < this->faceList.size(); j++ )
		{
			Face* f = this->faceList[j];
			if ( f->id == ids[i] )
			{
				fprintf(fp, "%d\n", f->id);
				he = faceList[j]->loop->hedge;
				p = he->StartPos()->coord();
				fprintf(fp, "# vertex %d\n", he->StartPos()->id_);
				fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
				he = he->next;
				p = he->StartPos()->coord();
				fprintf(fp, "# vertex %d\n", he->StartPos()->id_);
				fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
				he = he->next;
				p = he->StartPos()->coord();
				fprintf(fp, "# vertex %d\n", he->StartPos()->id_);
				fprintf(fp, "v %f %f %f 255 30 10\n", p[0], p[1], p[2]);
				fprintf(fp, "f %d %d %d\n", vn, vn+1, vn+2);
				fprintf(fp, "############\n\n");
				vn += 3;
			}
		}
	}
	fclose(fp);
}

double FaceAttribute::CalcArea(MeshV* mesh)
{
	int ii = id_;
	unsigned int* id;

	if (alive != 1 )
	{
		return DBL_MAX;
	}

	id = &(mesh->pIndices[3*ii]);

	if ( id[0] == id[1] || id[0] == id[2] || id[1] == id[2] )
	{
		alive = 0;
		return DBL_MAX;
	}

	double* p[3];

	p[0] = &(mesh->pVertices[3*id[0]]);
	p[1] = &(mesh->pVertices[3*id[1]]);
	p[2] = &(mesh->pVertices[3*id[2]]);

	double v[3][3];
	double nn[3];

	VEC3_SUB(v[0], p[1], p[0]);
	VEC3_SUB(v[1], p[2], p[1]);
	VEC3_CROSS(nn, v[0], v[1]);
	double ln = 0.0;
	VEC3_NORMALIZE2(nn, ln);
	area = ln*0.5;

	normal_[0] = nn[0];
	normal_[1] = nn[1];
	normal_[2] = nn[2];

	quality = TriangleQuality(mesh, id_);

	return area;
}

double TriangleArea(MeshV* mesh, int faceid)
{
	int id[3];

	id[0] = mesh->pIndices[3*faceid];
	id[1] = mesh->pIndices[3*faceid+1];
	id[2] = mesh->pIndices[3*faceid+2];

	if ( id[0] == id[1] || id[0] == id[2] || id[1] == id[2] )
	{
		return DBL_MAX;
	}

	double* p[3];

	p[0] = &(mesh->pVertices[3*id[0]]);
	p[1] = &(mesh->pVertices[3*id[1]]);
	p[2] = &(mesh->pVertices[3*id[2]]);

	double v[3][3];
	double n[3];

	VEC3_SUB(v[0], p[1], p[0]);
	VEC3_SUB(v[1], p[2], p[1]);
	VEC3_CROSS(n, v[0], v[1]);
	
	return sqrt(DOTPRODUCT(n))*0.5;
}

//正三角形に近いかどうかの指標を計算する
double TriangleQuality( MeshV* mesh, int faceid)
{
	double quality = -1.0;
#if USE_TriangleQuality

	unsigned int* id;

	id = &(mesh->pIndices[3*faceid]);

	double* p[3];

	p[0] = &(mesh->pVertices[3*id[0]]);
	p[1] = &(mesh->pVertices[3*id[1]]);
	p[2] = &(mesh->pVertices[3*id[2]]);

    double a [3] = {p[2][0] - p[1][0], p[2][1] - p[1][1], p[2][2] - p[1][2]};
    double b [3] = {p[0][0] - p[2][0], p[0][1] - p[2][1], p[0][2] - p[2][2]};
    double c [3] = {p[1][0] - p[0][0], p[1][1] - p[0][1], p[1][2] - p[0][2]};

	VEC3_NORMALIZE(a);
	VEC3_NORMALIZE(b);
	VEC3_NORMALIZE(c);

    double pva [3]; VEC3_CROSS(pva, b, c); const double sina = sqrt(DOTPRODUCT(pva)); 
    double pvb [3]; VEC3_CROSS(pvb, c, a); const double sinb = sqrt(DOTPRODUCT(pvb));
    double pvc [3]; VEC3_CROSS(pvc, a, b); const double sinc = sqrt(DOTPRODUCT(pvc));
      
    if (sina == 0.0 && sinb == 0.0 && sinc == 0.0) quality = 0.0;
    else quality = 2 * (2 * sina * sinb * sinc / (sina + sinb + sinc));
#else
	quality = 0.5;
#endif
	return quality;
}


int VertexRoundFaces(Vertex* v, std::vector<Face*>& faceList, std::set<Face*>& faces)
{
	const int vid = v->id_;
	const int n = faceList.size();
	for ( int i = 0; i < n; i++ )
	{
		if ( faceList[i]->index[0] == vid || faceList[i]->index[1] == vid || faceList[i]->index[2] == vid )
		{
			faces.insert(faceList[i]);
		}
	}
	return (int)faces.size();
}


void remove_unreferenced_vertex(MeshV& mesh)
{
	char* wrkfile ="remove_unreferenced_vertex$$$.tmp";
	FILE* fp = fopen(wrkfile, "w");

	int* new_vertex_id = new int[mesh.iNumVertices];
	const int vnum = mesh.iNumVertices;
	for ( int i = 0; i < vnum; i++ )
	{
		new_vertex_id[i] = -1;
	}

	int use_color = 0;
	if ( mesh.pColors ) use_color = 1;

	int vertex_count = 0;
	const int sz = mesh.iNumIndices;
	for ( int i = 0; i < sz; i++ )
	{
		unsigned int* vertex_id = &(mesh.pIndices[3*i]);
		double* vertex[3];

		if ( vertex_id[0] == vertex_id[1] || vertex_id[0] == vertex_id[2] || vertex_id[1] == vertex_id[2] )
		{
			continue;
		}

		double* normal[3] = {NULL, NULL, NULL};
		vertex[0] = &(mesh.pVertices[3*vertex_id[0]]);
		vertex[1] = &(mesh.pVertices[3*vertex_id[1]]);
		vertex[2] = &(mesh.pVertices[3*vertex_id[2]]);

		if (mesh.pNormals )
		{
			normal[0] = &(mesh.pNormals[3*vertex_id[0]]);
			normal[1] = &(mesh.pNormals[3*vertex_id[1]]);
			normal[2] = &(mesh.pNormals[3*vertex_id[2]]);
		}

		for ( int j = 0; j < 3; j++ )
		{
			if ( new_vertex_id[vertex_id[j]] == -1 )
			{
				if ( use_color )
				{
					int color[3];
					Color_IntToRGB(mesh.pColors[vertex_id[j]].rgb, color);
					fprintf(fp, "v %.16f %.16f %.16f %d %d %d %d\n", vertex[j][0], vertex[j][1], vertex[j][2], color[0], color[1], color[2], (int)mesh.pColors[vertex_id[j]].attr);
					if ( mesh.pNormals )
					{
						fprintf(fp, "vn %.16f %.16f %.16f\n", normal[j][0], normal[j][1], normal[j][2]);
					}
				}else
				{
					fprintf(fp, "v %.16f %.16f %.16f\n", vertex[j][0], vertex[j][1], vertex[j][2]);
					if ( mesh.pNormals )
					{
						fprintf(fp, "vn %.16f %.16f %.16f\n", normal[j][0], normal[j][1], normal[j][2]);
					}
				}
				new_vertex_id[vertex_id[j]] = vertex_count;
				vertex_count++;
			}
		}
		vertex_id[0] = new_vertex_id[vertex_id[0]];
		vertex_id[1] = new_vertex_id[vertex_id[1]];
		vertex_id[2] = new_vertex_id[vertex_id[2]];
	}
	fclose(fp);

	double* new_vertex = new double[3*vertex_count];
	double* new_normal = NULL;
	
	if ( mesh.pNormals ) new_normal = new double[3*vertex_count];

	Color* new_color = NULL;
	if ( use_color )
	{
		new_color = new Color[vertex_count];
	}

	fp = fopen( wrkfile, "r");
	char buf[256];


	for ( int i = 0; i < vertex_count; i++ )
	{
		double x, y, z;
		double nx, ny, nz;
		int color[3];
		int attr = 0;

		fgets(buf, 256, fp);
		if ( use_color )
		{
			sscanf(buf, "v %lf %lf %lf %d %d %d %d", &x, &y, &z, color, color+1, color+2, &attr);
			new_color[i].rgb = Color_RGBToInt( color );
			new_color[i].attr = (unsigned char)attr;
			if ( mesh.pNormals )
			{
				fgets(buf, 256, fp);
				sscanf(buf, "vn %lf %lf %lf", &nx, &ny, &nz);
			}
		}else
		{
			sscanf(buf, "v %lf %lf %lf", &x, &y, &z);
			if ( mesh.pNormals )
			{
				fgets(buf, 256, fp);
				sscanf(buf, "vn %lf %lf %lf", &nx, &ny, &nz);
			}
		}
		new_vertex[3*i] = x;
		new_vertex[3*i+1] = y;
		new_vertex[3*i+2] = z;
		if ( mesh.pNormals )
		{
			new_normal[3*i] = nx;
			new_normal[3*i+1] = ny;
			new_normal[3*i+2] = nz;
		}
	}
	fclose(fp);
	remove(wrkfile);

	printf("remove unreferenced vertex[%d] %d --> %d\n", mesh.iNumVertices-vertex_count, mesh.iNumVertices, vertex_count);
	delete [] mesh.pVertices;
	if ( use_color ) delete [] mesh.pColors;

	mesh.pVertices = new_vertex;
	mesh.pColors = new_color;
	mesh.pNormals = new_normal;
	mesh.iNumVertices = vertex_count;
}


MeshV* _MeshPtr;
extern "C" double GetPoint(int pos, int i)
{
	return _MeshPtr->pVertices[3*pos+i];
}

int VertexMerge(MeshV& mesh, double tol)
{
	//FILE* fp = fopen("vertexmerge.txt", "w");
	int merge_num = 0;
	double tol2 = tol*tol;
	double serch_r = tol*3.0;

	_MeshPtr = &mesh;
	kdGetPoint = GetPoint;
	KdTree<VertexS> kdtree;

	int sz = mesh.iNumIndices;
	for ( int i = 0; i < sz; i++ )
	{
		printf("[%d]      \r\r\r", sz-i-1);
		double *vertex[3];
		unsigned int* vertex_id = &(mesh.pIndices[3*i]);
		Color* color[3] = {NULL, NULL, NULL};
		
		if ( mesh.pColors )
		{
			color[0] = &(mesh.pColors[vertex_id[0]]);
			color[1] = &(mesh.pColors[vertex_id[1]]);
			color[2] = &(mesh.pColors[vertex_id[2]]);

			if ( mesh.vertexAttr && color[0]->attr == 0 && color[1]->attr == 0 && color[2]->attr == 0 )
			{
				continue;
			}
		}

		vertex[0] = &(mesh.pVertices[3*vertex_id[0]]);
		vertex[1] = &(mesh.pVertices[3*vertex_id[1]]);
		vertex[2] = &(mesh.pVertices[3*vertex_id[2]]);

		for ( int j = 0; j < 3; j++ )
		{
			std::vector<int> nearest_list;
			kdtree.nearest_find(vertex[j], serch_r, nearest_list);

			if ( nearest_list.size() == 0 )
			{
				VertexS v;
				v.id = vertex_id[j];
				kdtree.insert(vertex_id[j], &v);
				continue;
			}

			bool merge = false;
			const int nearest_list_num = nearest_list.size();
			for ( int k = 0; k < nearest_list_num; k++ )
			{
				int id = nearest_list[k];

				if ( vertex_id[j] == id ) continue;

				double* v = &(mesh.pVertices[3*id]);
				Color* c = NULL;
				
				double dx = (v[0] - vertex[j][0]);
				double dy = (v[1] - vertex[j][1]);
				double dz = (v[2] - vertex[j][2]);

				if ( dx*dx + dy*dy + dz*dz < tol2 )
				{
					//同一座標にする
					vertex[j][0] = v[0];
					vertex[j][1] = v[1];
					vertex[j][2] = v[2];
					if ( mesh.pColors )
					{
						if (mesh.pColors[id].rgb != color[j]->rgb)
						{
							//同一座標にする
							//vertex[j][0] = v[0];
							//vertex[j][1] = v[1];
							//vertex[j][2] = v[2];
							//fprintf(fp, "%d %d\n#%f %f %f\n#%f %f %f\n", 
							//	vertex_id[j], id, vertex[j][0], vertex[j][1], vertex[j][2],
							//	v[0], v[1], v[2] );
							continue;
						}
					}
					vertex_id[j] = id;
					merge = true;
					merge_num++;
					break;
				}
			}
			if ( !merge )
			{
				VertexS v;
				v.id = vertex_id[j];
				kdtree.insert(vertex_id[j], &v);
			}
		}
	}
	//fclose(fp);
	printf("\nMerge:%d\n", merge_num);
	return merge_num;
}


void RemoveMinAreaFace(MeshV& mesh, double tol)
{
	const double zero_tol2 = ZERO_TOL*ZERO_TOL;
	const double tol2 = tol*tol;

	int numface = mesh.iNumIndices;
	for ( int i = 0; i < numface; i++ )
	{
		int target_face_id = i;
		unsigned int edge[3][2];

		unsigned int* index = &(mesh.pIndices[3*i]);
		edge[0][0] = index[0];
		edge[0][1] = index[1];
		edge[1][0] = index[1];
		edge[1][1] = index[2];
		edge[2][0] = index[2];
		edge[2][1] = index[0];

		if ( index[0] == index[1] || index[0] == index[2] || index[1] == index[2])
		{
			continue;
		}

		if ( mesh.vertexAttr &&  mesh.pColors[index[0]].attr == 0 && mesh.pColors[index[1]].attr == 0 && mesh.pColors[index[2]].attr == 0 )
		{
			continue;
		}

		double p1[3];
		double p2[3];

		for ( int j = 0; j < 3; j++ )
		{
			p1[0] = mesh.pVertices[3*edge[j][0]];
			p1[1] = mesh.pVertices[3*edge[j][0]+1];
			p1[2] = mesh.pVertices[3*edge[j][0]+2];
			p2[0] = mesh.pVertices[3*edge[j][1]];
			p2[1] = mesh.pVertices[3*edge[j][1]+1];
			p2[2] = mesh.pVertices[3*edge[j][1]+2];

			double dir[3];

			dir[0] = p2[0] - p1[0]; 
			dir[1] = p2[1] - p1[1]; 
			dir[2] = p2[2] - p1[2];
			double ln = ( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );
			if ( ln < 1.0e-24 )
			{
				if ( j == 0 ) index[1] = index[0];
				if ( j == 1 ) index[2] = index[1];
				if ( j == 2 ) index[0] = index[2];
				printf("vertex merge(1)\n");
				continue;
			}

			dir[0] /= ln;
			dir[1] /= ln;
			dir[2] /= ln;

			double* pnt;
			if ( j == 0 )
			{
				pnt = &(mesh.pVertices[3*index[2]]);
			}
			if ( j == 1 )
			{
				pnt = &(mesh.pVertices[3*index[0]]);
			}
			if ( j == 2 )
			{
				pnt = &(mesh.pVertices[3*index[0]]);
			}

			double vec[3];

			vec[0] = pnt[0] - p1[0];
			vec[1] = pnt[1] - p1[1];
			vec[2] = pnt[2] - p1[2];

			double t = dir[0]*vec[0] + dir[1]*vec[1] + dir[2]*vec[2];
			if ( t < 0.0 || t > ln )
			{
				continue;
			}
			double dist1 =	(p1[0] - pnt[0])*(p1[0] - pnt[0]) +
							(p1[1] - pnt[1])*(p1[1] - pnt[1]) +
							(p1[2] - pnt[2])*(p1[2] - pnt[2]);
			if ( dist1 < zero_tol2 )
			{
				continue;
			}
			double dist2 =	(p2[0] - pnt[0])*(p2[0] - pnt[0]) +
							(p2[1] - pnt[1])*(p2[1] - pnt[1]) +
							(p2[2] - pnt[2])*(p2[2] - pnt[2]);
			if ( dist2 < zero_tol2 )
			{
				continue;
			}

			double edge_on_p[3];

			edge_on_p[0] = p1[0] + dir[0]*t;
			edge_on_p[1] = p1[1] + dir[1]*t;
			edge_on_p[2] = p1[2] + dir[2]*t;
			double dist =	(edge_on_p[0] - pnt[0])*(edge_on_p[0] - pnt[0]) +
							(edge_on_p[1] - pnt[1])*(edge_on_p[1] - pnt[1]) +
							(edge_on_p[2] - pnt[2])*(edge_on_p[2] - pnt[2]);

			if ( dist < tol2 )
			{
				if ( j == 0 ) index[1] = index[0];
				if ( j == 1 ) index[2] = index[1];
				if ( j == 2 ) index[0] = index[2];
				printf("vertex merge(2)\n");
			}
		}
	}
}

void* NewMeshV()
{
	MeshV* mesh = new MeshV;

	return (void*)mesh;
}
void* NewSolidV(void* mesh)
{
	if ( mesh == NULL ) return NULL;
	MeshV* m = (MeshV*)mesh;

	Solid* solid = new Solid(m);
	solid->CreateFaceAttri();

	return (void*)solid;
}
void DeleteMeshV(void* mesh)
{
	if ( mesh == NULL ) return;

	MeshV* m = (MeshV*)mesh;
	delete m;
}
void DeleteSolidV(void* solid)
{
	if ( solid == NULL ) return;
	Solid* s = (Solid*)solid;
	delete s;
}

