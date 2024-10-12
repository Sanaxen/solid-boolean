/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include <time.h>

#include "BooleanModeller.h"

#include "Object3D.h"
#include "Solid.h"
#include "Face.h"

#include "VertexSet.h"
#include "VectorSet.h"
#include "IntSet.h"

#include "octreecell.h"
#include "openmp_config.h"
#include "sse_func.h"

#include "stopwatch.h"
#include "debug_print.h"

#include <random>

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

#include <Windows.h>
static int call_BooleanModeller = 0;
static long call_BooleanModeller_time = 0;

extern "C" void SaveSolid(void* solid, char* filename);
extern "C" void GetCSG_solid_CallID(int* id1, int* id2);

#define SPLITFACE_RETRY_MAX0	1
#define SPLITFACE_RETRY_MAX1	40

static mlFloat eps_rotatin_ang = 1.0e-10;
static mlFloat eps_trans_dst = 1.0e-12;
static int splitObject_retry = 1;
void set_splitObject_retry_flag(int flg)
{
	splitObject_retry = flg;
}
int get_splitObject_retry_flag()
{
	return splitObject_retry;
}

BooleanModeller::BooleanModeller(Solid * solid1, Solid * solid2)
{
#ifdef _OPENMP
//printf("OpenMP : Enabled (Max # of threads = %d\n", omp_get_max_threads());
#endif
	
	int solid1_split_time = 0;
	int solid2_split_time = 0;
	int solid1_classify_time = 0;
	int solid2_classify_time = 0;

	call_BooleanModeller++;

	split_status = true;
	create_status = false;
	time_out = false;

	printf("split the faces start\n");
	//printf("\nConstruction Object3D from Solid\n");

	m_pObject1 = NULL;
	m_pObject2 = NULL;
	if ( solid1 == NULL || solid1 != NULL && solid1->indices.GetSize() == 0 )
	{
		split_status = false;
		return;
	}
	if ( solid2 == NULL || solid2 != NULL && solid2->indices.GetSize() == 0 )
	{
		split_status = false;
		return;
	}

	//SaveSolid((void*)solid1, "1.obj");
	//SaveSolid((void*)solid2, "2.obj");

	default_color1 = solid1->colors.isDefaultColor();
	default_color2 = solid2->colors.isDefaultColor();

	//printf("solid1 color default? :%d\n", (int)default_color1);
	//printf("solid2 color default? :%d\n", (int)default_color2);
	//fflush(stdout);

	Solid* solid1_sv = solid1;
	Solid* solid2_sv = solid2;

	Solid* solid1_new = NULL;
	Solid* solid2_new = NULL;

	std::mt19937 mt;
	std::uniform_real_distribution<double> d_rand(-1.0, 1.0);

	int ts = GetTickCount();
	{
		mlVector3D eps_trans;
		mlVector3D eps_trans_tot(0.0,0.0,0.0);
		int retry = 0;
		int retry_case = 0;
		const int retry_case_num =2;
		const int retryMax[retry_case_num] = {SPLITFACE_RETRY_MAX0, SPLITFACE_RETRY_MAX1};
		int error_obj_id = 0;
		//int cutting_face_num_loop_max = Object3D::cuttingFaceNumLoopMax;
		int extend_numFacesMax_Num = 10;

		do
		{
			if (m_pObject1)	delete m_pObject1; m_pObject1 = NULL;
			if (m_pObject2)	delete m_pObject2; m_pObject2 = NULL;

			//representation to apply boolean operations
			m_pObject1 = new Object3D(solid1);
			m_pObject2 = new Object3D(solid2);

			float len1, len2;
			float dx = m_pObject1->bound->xyz_Max[0] - m_pObject1->bound->xyz_Min[0];
			float dy = m_pObject1->bound->xyz_Max[1] - m_pObject1->bound->xyz_Min[1];
			float dz = m_pObject1->bound->xyz_Max[2] - m_pObject1->bound->xyz_Min[2];

			len1 = dx*dx + dy*dy + dz*dz;

			dx = m_pObject2->bound->xyz_Max[0] - m_pObject2->bound->xyz_Min[0];
			dy = m_pObject2->bound->xyz_Max[1] - m_pObject2->bound->xyz_Min[1];
			dz = m_pObject2->bound->xyz_Max[2] - m_pObject2->bound->xyz_Min[2];

			len2 = dx*dx + dy*dy + dz*dz;

			if (RAYBOX_BOX_EXT_TOL_value >= 0)
			{
				RAYBOX_BOX_EXT_TOL_value = sqrt(min(len1, len2))*0.005;
				//if (RAYBOX_BOX_EXT_TOL_value > RAYBOX_BOX_EXT_TOL)
				//{
				//	RAYBOX_BOX_EXT_TOL_value = RAYBOX_BOX_EXT_TOL;
				//}
			}
			else
			{
				RAYBOX_BOX_EXT_TOL_value = -RAYBOX_BOX_EXT_TOL_value;
			}
			printf("BOX_BOX_EXT_TOL=%f\n", RAYBOX_BOX_EXT_TOL_value);


			if (OCTREE_CELL_TOL_value >= 0)
			{
				OCTREE_CELL_TOL_value = sqrt(min(len1, len2))*0.01;
				if (OCTREE_CELL_TOL_value > OCTREE_CELL_TOL)
				{
					OCTREE_CELL_TOL_value = OCTREE_CELL_TOL;
				}
			}
			else
			{
				OCTREE_CELL_TOL_value = -OCTREE_CELL_TOL_value;
			}
			printf("OCTREE_CELL_TOL=%f\n", OCTREE_CELL_TOL_value);
			//Object3D::cuttingFaceNumLoopMax = cutting_face_num_loop_max;

			error_obj_id = 0;
			printf("create object %d ms\n", GetTickCount() - ts);
	

			int stat1 = 0;
			int stat2 = 0;

			//オブジェクト同士で干渉しないファセットにフラグビットを立てる
			//※スプリットの対象から除外できる
			Object_Object_Interference();

			//m_pObject1のファセットに干渉する相手をOctreeで管理してすばやく相手を見つける準備
			m_pObject2->MakeCell();

			printf("split start.\n");
			ts = GetTickCount();
			stat1 = m_pObject1->splitFaces(m_pObject2, extend_numFacesMax_Num);
			printf("split end.\n");

			if ( stat1 < 0 )
			{
				error_obj_id = 1;
				printf("切断エラー(%d)\n", error_obj_id);
				delete m_pObject1; m_pObject1 = NULL;
				delete m_pObject2; m_pObject2 = NULL;
				if ( solid1_new ) delete solid1_new; solid1_new = NULL;
				if ( solid2_new ) delete solid2_new; solid2_new = NULL;
			}
			if ( stat1 == -2 || splitObject_retry == 0 || retry > retryMax[retry_case] && retry_case > retry_case_num-1)
			{
				//エラー
				printf("===>Solid split separation[%d] error.\n", error_obj_id);
				if ( stat1 == -2 ) time_out = true;
				split_status = false;
				return;
			}

			if ( stat1 < 0 )
			{
				extend_numFacesMax_Num += 10;
				//最初のリトライパタンでリトライ回数を超えた
				if ( retry_case == 0 && retry >= retryMax[retry_case] )
				{
					retry_case++;
					retry = 0;
				}

				//最初のリトライパタンは処理ループを増やしてみる
				if ( retry_case == 0 )
				{
					//cutting_face_num_loop_max = Object3D::cuttingFaceNumLoopMax*10;
				}

				//2度目のリトライパタンは位置を少しだけずらしてみる
				if ( retry_case == 1 )
				{
					//cutting_face_num_loop_max = SPLIT_FACSE_MAX;
					if ( solid1_new ) delete solid1_new;
					mlFloat r[3];
					r[0] = d_rand(mt);
					r[1] = d_rand(mt);
					r[2] = d_rand(mt);
#ifdef USE_QDOUBLE
					printf("%f\n%f\n%f\n", to_double(r[0]), to_double(r[1]), to_double(r[2]));
#else
					printf("%f\n%f\n%f\n", (double)(r[0]), (double)(r[1]), (double)(r[2]));
#endif
					eps_trans.x = (r[0])*eps_trans_dst;
					eps_trans.y = (r[1])*eps_trans_dst;
					eps_trans.z = (r[2])*eps_trans_dst;
					eps_trans_tot += eps_trans;
					solid1 = new Solid(&solid1_sv->refVertices(), &solid1_sv->refIndices(), &solid1_sv->refColors());
					solid1->Translate( eps_trans );

					//mlVector3D p1, p2;
					//p1.x = ((double)rand() / ((double)RAND_MAX + 1));
					//p1.y = ((double)rand() / ((double)RAND_MAX + 1));
					//p1.z = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.x = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.y = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.z = ((double)rand() / ((double)RAND_MAX + 1));
					//mlFloat ang = eps_rotatin_ang*((double)rand() / ((double)RAND_MAX + 1));
					//solid1->Rotate(p1, p2, ang);

					solid1_new = solid1;
					_CVTBUFFFER;
					printf("Recalculation[%d:%d]:%s\n", retry_case, retry, ToString(eps_trans.Magnitude()).c_str());
				}
				retry++;
				printf("リトライ:%d\n", retry);
				continue;
			}

			m_pObject2->DeleteCell();
			solid1_split_time = GetTickCount()-ts;
			//printf("	=>ソリッドSplit time %d\n", solid1_split_time);
	
			//m_pObject1のファセットは分解されているため再度干渉しないファセットにフラグビットを立てる
			Object_Object_Interference();

			//m_pObject2のファセットに干渉する相手をOctreeで管理してすばやく相手を見つける準備
			m_pObject1->MakeCell();

			ts = GetTickCount();
			printf("split start.\n");
			stat2 = m_pObject2->splitFaces(m_pObject1, extend_numFacesMax_Num);
			printf("split end.\n");

			if ( stat2 < 0 )
			{
				error_obj_id = 2;
				printf("切断エラー(%d)\n", error_obj_id);
				delete m_pObject1; m_pObject1 = NULL;
				delete m_pObject2; m_pObject2 = NULL;
				if ( solid1_new ) delete solid1_new; solid1_new = NULL;
				if ( solid2_new ) delete solid2_new; solid2_new = NULL;
			}

			if ( stat2 == -2 || splitObject_retry == 0 || retry == retryMax[retry_case] && retry_case == retry_case_num-1)
			{
				//エラー
				printf("===>Solid split separation[%d] error.\n", error_obj_id);
				if ( stat1 == -2 ) time_out = true;
				split_status = false;
				return;
			}

			if ( stat2 < 0 )
			{
				extend_numFacesMax_Num += 10;
				printf("リトライ:%d\n", retry);

				//最初のリトライパタンでリトライ回数を超えた
				if ( retry_case == 0 && retry >= retryMax[retry_case] )
				{
					retry_case++;
					retry = 0;
				}
				//最初のリトライパタンは処理ループを増やしてみる
				if ( retry_case == 0 )
				{
					//cutting_face_num_loop_max = Object3D::cuttingFaceNumLoopMax * 1;
				}

				//2度目のリトライパタンは位置を少しだけずらしてみる
				if ( retry_case == 1 )
				{
					//cutting_face_num_loop_max = SPLIT_FACSE_MAX;
					if ( solid2_new ) delete solid2_new;
					mlFloat r[3];
					r[0] = d_rand(mt);
					r[1] = d_rand(mt);
					r[2] = d_rand(mt);
#ifdef USE_QDOUBLE
					printf("%f\n%f\n%f\n", to_double(r[0]), to_double(r[1]), to_double(r[2]));
#else
					printf("%f\n%f\n%f\n", (double)(r[0]), (double)(r[1]), (double)(r[2]));
#endif
					eps_trans.x = (r[0])*eps_trans_dst;
					eps_trans.y = (r[1])*eps_trans_dst;
					eps_trans.z = (r[2])*eps_trans_dst;
					eps_trans_tot += eps_trans;
					solid2 = new Solid(&solid2_sv->refVertices(), &solid2_sv->refIndices(), &solid2_sv->refColors());
					solid2->Translate( eps_trans );

					//mlVector3D p1, p2;
					//p1.x = ((double)rand() / ((double)RAND_MAX + 1));
					//p1.y = ((double)rand() / ((double)RAND_MAX + 1));
					//p1.z = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.x = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.y = ((double)rand() / ((double)RAND_MAX + 1));
					//p2.z = ((double)rand() / ((double)RAND_MAX + 1));
					//mlFloat ang = eps_rotatin_ang*((double)rand() / ((double)RAND_MAX + 1));
					//solid2->Rotate(p1, p2, ang);

					solid2_new = solid2;
					_CVTBUFFFER;
					printf("Recalculation[%d:%d]:%s\n", retry_case, retry, ToString(eps_trans.Magnitude()).c_str());
				}
				retry++;
				continue;
			}

			m_pObject1->DeleteCell();
			solid2_split_time = GetTickCount()-ts;
			_CVTBUFFFER;
			if (retry_case && retry) printf("再計算成功 形状変移:%s\n", ToString(eps_trans_tot.Magnitude()).c_str());
			break;
		}while( true );

		if ( solid1_new ) delete solid1_new;
		if ( solid2_new ) delete solid2_new;
	}
	printf("split the faces end.\n");


	//classify faces as being inside or outside the other solid

	printf("classify(1) faces as being inside or outside ray trace start\n");

	//レイトーレースでoctreeで管理したファセットでレイが刺さる相手をすばやく見つけるための準備
	m_pObject2->ReMakeCell();
	ts = GetTickCount();
	int stat1 = m_pObject1->classifyFaces(m_pObject2);
	if (stat1 < 0 ) split_status = false;
	solid1_classify_time = GetTickCount()-ts;
	//printf("\nObject2 - Classify faces of Object1\n");

	printf("classify(2) faces as being inside or outside ray trace start\n");
	//レイトーレースでoctreeで管理したファセットでレイが刺さる相手をすばやく見つけるための準備
	m_pObject1->ReMakeCell();
	ts = GetTickCount();
	int stat2 = m_pObject2->classifyFaces(m_pObject1);
	if (stat2 < 0 ) split_status = false;
	//printf("split the faces end.\n");
	solid2_classify_time = GetTickCount()-ts;

	printf("classify faces end.\n");
	//printf("\tsolid(1)split time    :%d \tsolid(2)split time    :%d (%d)\n", solid1_split_time, solid2_split_time, solid1_split_time+solid2_split_time);
	//printf("\tsolid(1)classify time :%d \tsolid(2)classify time :%d (%d)\n\n", solid2_classify_time, solid2_classify_time, solid1_classify_time+solid2_classify_time);
	//call_BooleanModeller_time += solid1_split_time+solid2_split_time+solid1_classify_time+solid2_classify_time;
	
#ifdef _OPENMP
	printf("[%04d]split time :%d ms classify time :%d ms(omp threads %d)\n", call_BooleanModeller, solid1_split_time+solid2_split_time, solid1_classify_time+solid2_classify_time, omp_get_max_threads());
#else
	printf("[%04d]split time :%d ms classify time :%d ms(omp threads --)\n", call_BooleanModeller, solid1_split_time+solid2_split_time, solid1_classify_time+solid2_classify_time);
#endif
	fflush(stdout);
	create_status = true;
	if (!split_status) create_status = false;
}

BooleanModeller::~BooleanModeller()
{
	//printf("Deleting BooleanModeller\n");
	//printf("Deleting object 1\n");
	delete m_pObject1;
	//printf("Deleting object 2\n");
	delete m_pObject2;
}

void BooleanModeller::Object_Object_Interference()
{
	const int sz1 = m_pObject1->getNumFaces();
	const int sz2 = m_pObject2->getNumFaces();

	const Bound* object1_bound = m_pObject1->getBound();
	const Bound* object2_bound = m_pObject2->getBound();
	FaceSet* fset1 = m_pObject1->faces;
	FaceSet* fset2 = m_pObject2->faces;

#ifdef USE_OPENMP
#pragma omp parallel
#endif
	{
		#ifdef USE_OPENMP
		#pragma omp  for
		#endif
		for ( int i = 0; i < sz1; i++ )
		{
			Face* f1 = fset1->GetFace(i);
			if ( IS_OVERLAP_FLAG(f1) || f1->getBound()->overlap( object2_bound ))
			{
				continue;
			}
			//誰とも干渉しない
			OVERLAP_NO_SET(f1);
		}
		#ifdef USE_OPENMP
		#pragma omp  for
		#endif
		for ( int i = 0; i < sz2; i++ )
		{
			Face* f2 = fset2->GetFace(i);
			if ( IS_OVERLAP_FLAG(f2) || f2->getBound()->overlap( object1_bound ) )
			{
				continue;
			}
			//誰とも干渉しない
			OVERLAP_NO_SET(f2);
		}
	}
}

Solid * BooleanModeller::get1()
{
	return composeSolid(-999, 0, 0, 0);
}
Solid * BooleanModeller::get2()
{
	return composeSolid(0, -999, 0, 0);
}

Solid * BooleanModeller::getUnion()
{
	return composeSolid(Face_OUTSIDE, Face_SAME, Face_OUTSIDE, 1);
}

Solid * BooleanModeller::getIntersection()
{
	return composeSolid(Face_INSIDE, Face_SAME, Face_INSIDE, 2);
}

Solid * BooleanModeller::getDifference()
{
	StopWach stopwatch("getDifference");
	m_pObject2->invertInsideFaces();
	Solid * result = composeSolid(Face_OUTSIDE, Face_OPPOSITE, Face_INSIDE, 3);
	m_pObject2->invertInsideFaces();

	stopwatch.stop();
	return result;
}

Solid * BooleanModeller::composeSolid(int faceStatus1, int faceStatus2, int faceStatus3, int op) 
{
	int ts = GetTickCount();
	//StopWach stopwatch("composeSolid");
	VertexSet vertices;
	IntSet indices;
	ColorSet colors;
	
	Solid *solid = new Solid;

	int n = m_pObject1->getNumFaces()+m_pObject2->getNumFaces();
	vertices.Reserve(3*n);

	bool is_default_color = false;
	if ( default_color1 && default_color2 )
	{
		is_default_color = true;
		solid->colors.setDefaultColor();
		colors.setDefaultColor();
	}else{
		solid->colors.setUserColor();
		colors.setUserColor();
		colors.Reserve(3*n);
	}
	//printf("solid oolean color default? :%d,%d\n", (int)solid->colors.isDefaultColor(), (int)colors.isDefaultColor());
	//fflush(stdout);

	solid->refIndices().Reserve(3*n);

	//group the elements of the two solids whose faces fit with the desired status 
	if ( faceStatus1 == -999 || faceStatus2 == -999 )
	{
		if ( faceStatus1 == -999 )
		{
			groupObjectComponents(m_pObject1, vertices, solid->refIndices(), colors, faceStatus1, 0, 0);
		}else
		{
			groupObjectComponents(m_pObject2, vertices, solid->refIndices(), colors, faceStatus2, 0, 0);
		}
	}else
	{
		groupObjectComponents(m_pObject1, vertices, solid->refIndices(), colors, faceStatus1, faceStatus2, 0);
		groupObjectComponents(m_pObject2, vertices, solid->refIndices(), colors, faceStatus3, faceStatus3, op);
	}


	const int sz = vertices.GetNumVertices();

	VectorSet& vectors = solid->refVertices();
	vectors.Resize(sz);

	ColorSet& colors2 = solid->refColors();
	if ( !is_default_color ) colors2.Resize(sz);

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = 0; i < sz; i++)
	{
		vectors[i] = vertices.GetVertex(i)->xyz;
		if ( !is_default_color ) colors2[i] = colors.GetColor(i);
	}

	//SaveSolid((void*)solid, "aaaa.obj");
	//stopwatch.stop();
	
	int id1 = -1;
	int id2 = -1;
	GetCSG_solid_CallID(&id1, &id2);

	if ( id1 < 0 || id2 < 0 )
	{
		printf("  [%04d]BooleanOP time :%d ms\n-----------------------------\n", call_BooleanModeller, GetTickCount() - ts);
	}else
	{
		printf("  [%04d]BooleanOP time :%d ms\n----------------------%d/%d--\n", call_BooleanModeller, GetTickCount() - ts, id1, id2);
	}
	return solid;
}

typedef struct faceComponet_type
{
	Face* face;
	Vertex** vertex;
	color_attr_type color[3];
}faceComponet;

void BooleanModeller::groupObjectComponents(Object3D* object, VertexSet & vertices, IntSet & indices, ColorSet & colors, int faceStatus1, int faceStatus2, int op)
{
	//for each face..
	int ts = GetTickCount();
	int tts = ts;
	
	bool is_default_color = false;
	if ( default_color1 && default_color2 )
	{
		is_default_color = true;
	}

	const int sz = object->getNumFaces();

	std::vector<faceComponet> facelist;
	facelist.resize(sz);

	int num = 0;
#ifdef USE_OPENMP
#pragma omp parallel for reduction(+:num)
#endif
	for(int i=0;i<sz;i++)
	{
		faceComponet fc;
		fc.face = NULL;

		Face* face = object->getFace(i);
		//if the face status fits with the desired status...
		const int facestat = face->getStatus();

		if(facestat==faceStatus1 || facestat==faceStatus2 || faceStatus1 == -999 || faceStatus2 == -999 )
		{
			Vertex** vrt = face->getVertex();
			fc.face = face;
			fc.vertex = vrt;
			fc.color[0] = vrt[0]->getColor();
			fc.color[1] = vrt[1]->getColor();
			fc.color[2] = vrt[2]->getColor();
			if ( op == 3 )
			{
				fc.color[0].attr = 1;
				fc.color[1].attr = 1;
				fc.color[2].attr = 1;
#if 0	/* debug */
				int color = 0xC85A5A;
				fc.color[0].color = color;
				fc.color[1].color = color;
				fc.color[2].color = color;
#endif
			}
			num++;
		}
		facelist[i] = fc;
	}

	DEBUG_PRINT(("===> time %d ms\n", GetTickCount() - tts));

	tts = GetTickCount();
	
	int indices_num = indices.GetSize();
	int vertices_num = vertices.GetNumVertices();

	indices.Resize(indices_num+3*num);
	vertices.Resize(vertices_num+3*num);
	if ( !is_default_color )
	{
		colors.Resize(vertices_num+3*num);
	}

	for(int i=0;i<sz;i++)
	{
		faceComponet& fc = facelist[i];
		if ( fc.face )
		{
			for ( int j = 0; j < 3; j++ )
			{
				if ( fc.vertex[j]->id == -1 )
				{
					fc.vertex[j]->id = vertices_num;
					vertices.SetVertex(vertices_num, fc.vertex[j]);
					if ( !is_default_color )
					{
						colors.SetColor(vertices_num, fc.color[j]);
					}
					vertices_num++;
				}
				indices.SetInt(indices_num, fc.vertex[j]->id);
				indices_num++;
			}
		}
	}

	vertices.Resize(vertices_num);
	if ( !is_default_color )
	{
		colors.Resize(vertices_num);
	}

	DEBUG_PRINT(("    [%04d]grouping time :%d ms\n", call_BooleanModeller, GetTickCount() - ts));
}
