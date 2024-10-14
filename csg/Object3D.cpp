/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "Object3D.h"

#include "Solid.h"
#include "VectorSet.h"
#include "IntSet.h"
#include "Vertex.h"
#include "VertexSet.h"
#include "Face.h"
#include "FaceSet.h"
#include "Bound.h"

#include "Line.h"
#include "Segment.h"

#include "octreecell.h"

#include <math.h>
#include "def.h"
#include "openmp_config.h"
#include "thread_omp_util.h"

#include "stopwatch.h"
#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
#include <Windows.h>

#include "debug_print.h"

//#undef USE_OPENMP

const mlFloat Object3D::TOL = Object3D_TOL;
const mlFloat Object3D::ATOL = 1.0e-8;
const mlFloat Object3D::ATOL2 = (Object3D::ATOL2)*(Object3D::ATOL2);
mlFloat Object3D::SPLIT_TOL = Object3D_SPLIT_TOL;

//int Object3D::cuttingFaceNumLoopMax = SPLIT_FACSE_MAX;
//mlFloat Object3D::TOL = 1e-8f;
//mlFloat Object3D::TOL = 0.5e-5f;
//mlFloat Object3D::TOL = 1e-3f;

Object3D::Object3D():vertices(0),faces(0),bound(0),cell(0),unigrid(0)
{
	//vertices = 0;
	//faces = 0;
	//bound = 0;
	//cell = 0;
	//unigrid = 0;
}

Object3D::Object3D(Solid * solid):solid_(solid)
{
	CreateObject(solid);
	//this->cuttingFaceNumLoopMax = SPLIT_FACSE_MAX;
}

Object3D::~Object3D()
{
	Clear();
	solid_ = NULL;
}

void Object3D::CreateObject(Solid * solid)
{
	Vertex * vertex = 0;
	cell = 0;
	unigrid = 0;

	VectorSet & verticesPoints = solid->refVertices();
	IntSet & indices = solid->refIndices();

	if ( solid->refColors().GetSize() == 0 )
	{
		solid->colors.setDefaultColor();
	}else
	{
		solid->colors.setUserColor();
	}

	ColorSet & colors = solid->refColors();

	//create bound
	bound = new Bound(&verticesPoints);

	std::vector<Vertex*> verticesTemp;
	
	//create vertices
	vertices = new VertexSet();
	const int sz = verticesPoints.GetSize();

	const int indexnum = indices.GetSize();

	int ts = GetTickCount();
	verticesTemp.resize(sz, NULL);

#if 10
	CreateGrid();
	vertices->Resize(sz);
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for(int i=0;i<sz;i++)
	{
		verticesTemp[i] = addVertex(i, verticesPoints[i], colors[i], Vertex_UNKNOWN);
	}

#else
	for(int i=0;i<sz;i++)
	{
		verticesTemp[i] = addVertex(verticesPoints[i], colors[i], Vertex_UNKNOWN, 0);
	}
#endif

	DEBUG_PRINT_D((" create vertex %d ms\n", GetTickCount() -ts));

	int skippNum = 0;
	int zeroAreaNum = 0;

	//create faces
	const int sz2 = indexnum/3;
	
	std::vector<Face*> faceList;
	faceList.resize(sz2);

	int num = 0;
	ts = GetTickCount();

#ifdef USE_OPENMP
#pragma omp parallel for reduction(+:num, skippNum, zeroAreaNum)
#endif
	for(int i=0; i<sz2; i++)
	{
		const int id[3] = {indices[3*i], indices[3*i+1], indices[3*i+2]};
		if ( id[0] == id[1] || id[1] == id[2] || id[0] == id[2] ) 
		{
			skippNum++;
			continue;
		}

		Face* f = NULL;
		if ( (f = addFaceChk(verticesTemp[id[0]], verticesTemp[id[1]], verticesTemp[id[2]], NULL)) == NULL )
		{
			zeroAreaNum++;
		}else
		{
			faceList[i] = f;
			num++;
		}
	}
	DEBUG_PRINT_D((" create face step1 %d ms\n", GetTickCount() -ts));
	ts = GetTickCount();
	
	faces = new FaceSet();
	faces->Resize( num );

	int n = 0;
	int* indexList = new int[sz2];
	for(int i=0; i<sz2; i++)
	{
		indexList[i] = -1;
		if ( faceList[i] != NULL )
		{
			indexList[i] = n;
			n++;
		}
	}

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i=0; i<sz2; i++)
	{
		if ( indexList[i] >= 0 )
		{
			faces->SetFace( indexList[i], faceList[i] );
		}
	}
	delete [] indexList;

	DEBUG_PRINT_D((" create face step2 %d ms\n", GetTickCount() -ts));

	if ( skippNum ) printf("skipp Degeneracy Face %d\n", skippNum);	fflush(stdout);

	if ( zeroAreaNum ) printf("skipp zeroArea Face %d\n", zeroAreaNum);	fflush(stdout);
}

void Object3D::Clear()
{
	//printf("Deleting Object3D\n");
	//printf("Deleting vertex set\n");
	delete vertices;
	vertices = NULL;

	//printf("Deleting face set\n");
	delete faces;
	faces = NULL;

	//printf("Deleting bound\n");
	delete bound;
	bound = NULL;
	
	delete cell;
	cell = NULL;

	delete unigrid;
	unigrid = NULL;
}

void Object3D::DeleteCell()
{
	if ( cell ) delete cell;
	cell = NULL;
	
	//if (unigrid) delete unigrid;
	//unigrid = 0;
}

void Object3D::MakeCell()
{
	DeleteCell();
	cell = new octreecell;
	cell->buildoctree(this, 1);
}

void Object3D::ReMakeCell()
{
	DeleteCell();
	cell = new octreecell;
	cell->buildoctree(this, 0);
}

//------------------------------------ADDS----------------------------------------//

// There may be missing functions in the container classes.
	
/**
 * Method used to add a face properly for internal methods
 * 
 * @param v1 a face vertex
 * @param v2 a face vertex
 * @param v3 a face vertex
 */
Face * __fastcall Object3D::addFace(Vertex * v1, Vertex * v2, Vertex * v3, Face* base, mlVector3D* normalvec)
{
	Face * pAddedFace = addFaceChk(v1, v2, v3, base, normalvec);
	if ( pAddedFace != NULL )
	{
		faces->AddFace2(pAddedFace);
	}
	return pAddedFace;
}

Face * __fastcall Object3D::addFaceChk(Vertex * v1, Vertex * v2, Vertex * v3, Face* base, mlVector3D* normalvec)
{
	if(!(v1->equals(v2)||v1->equals(v3)||v2->equals(v3)))
	{
		mlVector3D post_normalvec;
		mlFloat area;
		Face face(v1, v2, v3);

		bool isOk = face.chkNormal(FaceNorm_TOL, area, post_normalvec);

		//if(face.getArea()>FaceArea_TOL)
		// Tried removing this check, just for fun.
		// I should probably use mlFloat precision for everything.
		//if(true)
		if(isOk)
		{
			unsigned int attr = 0;
			const mlFloat atol2 = ATOL;

			if ( area < atol2 )
			{
				//printf("   extremely small facet %s\n", area.to_string().c_str());
				BIT_ON(attr,D_MICRO_FACE_BIT);
			}
			if ( normalvec != NULL )
			{
				if ( VECTOR3D_DOT(post_normalvec, (*normalvec))  < 0.0 )
				{
					//printf("   became in the opposite direction %s\n", area.to_string().c_str());
					BIT_ON(attr, D_REVERS_FACE_BIT);

					//反転したFaceを再反転しなおす
					{
						face.invert();
						mlVector3D post_normalvec;
						mlFloat area;
						face.chkNormal(FaceNorm_TOL, area, post_normalvec);
						if ( VECTOR3D_DOT(post_normalvec, (*normalvec))  < 0.0 )
						{
							//printf("----------------------------NG\n");
						}else
						{
							//BIT_OFF(attr, D_REVERS_FACE_BIT);
							//printf("----------------------------OK\n");
						}
					}
				}
			}

			Face * pAddedFace = faces->CreateFace(face, normalvec);
			if ( base != NULL )
			{
#if 0
				//切断されて出来たファセットなので切断回数は引き継がない(本来はこっち)
				pAddedFace->split_count = 0;
#else
				//切断回数を継承する場合（異常な場合に対するセーフ設定)
				pAddedFace->split_count = base->split_count;
#endif
				pAddedFace->split_max = base->split_max;
				pAddedFace->attr = attr;
			}
			return pAddedFace;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

void __fastcall Object3D::CreateGrid()
{
	//一様グリッド作成
	if ( !unigrid )
	{
		unigrid = new UniformGrid<Vertex*>
							(	bound->xyz_Min[0]-Bound_TOL, bound->xyz_Min[1]-Bound_TOL, bound->xyz_Min[2]-Bound_TOL, 
								bound->xyz_Max[0]+Bound_TOL, bound->xyz_Max[1]+Bound_TOL, bound->xyz_Max[2]+Bound_TOL);
	}
}

Vertex * __fastcall Object3D::addVertex(const int index, const mlVector3D & pos, const color_attr_type& color, const int status)
{
	Vertex vertex(pos, color, status);

	const ALIGN16 float fxyz[4] ={
	 static_cast<float>(to_double(vertex.xyz.x)),
	 static_cast<float>(to_double(vertex.xyz.y)),
	 static_cast<float>(to_double(vertex.xyz.z)), 0.0f};

	Vertex * pVertexInList = vertices->SetVertex(index, vertex);

#ifdef USE_OPENMP
#//pragma omp critical
#endif
	{
		unigrid->Add(fxyz, pVertexInList);
	}
	return pVertexInList;
}


Vertex * __fastcall Object3D::addVertex(const mlVector3D & pos, const color_attr_type& color, const int status)
{
	Vertex vertex(pos, color, status);

	const ALIGN16 float fxyz[4] ={
	 static_cast<float>(to_double(vertex.xyz.x)),
	 static_cast<float>(to_double(vertex.xyz.y)),
	 static_cast<float>(to_double(vertex.xyz.z)), 0.0f};

	Vertex * pVertexInList = vertices->AddVertex(vertex);
	unigrid->Add(fxyz, pVertexInList);
	return pVertexInList;
}


/**
 * Method used to add a vertex properly for internal methods
 * 
 * @param pos vertex position
 * @param color vertex color
 * @param status vertex status
 * @return the vertex inserted (if a similar vertex already exists, this is returned)
 */
Vertex * __fastcall Object3D::addVertex(const mlVector3D & pos, const color_attr_type& color, const int status, const int flag)
{
	int lookup_index = -1;

	//一様グリッド作成
	if ( !unigrid )
	{
		CreateGrid();
	}

	if ( flag == 0 )
	{
		Vertex * pVertexInList = addVertex(pos, color, status);
		return pVertexInList;
	}


	Vertex vertex(pos, color, status);

	const ALIGN16 float fxyz[4] ={
	 static_cast<float>(to_double(vertex.xyz.x)),
	 static_cast<float>(to_double(vertex.xyz.y)),
	 static_cast<float>(to_double(vertex.xyz.z)), 0.0f};

	std::vector<Vertex*>& list = unigrid->LockUp(fxyz);

	if ( list.size() != 0 )
	{
		const mlFloat tol = MERGE_VTX_TOL;
		const std::vector<Vertex*>& listv = list;
		const int sz2 = listv.size();

#ifdef USE_OPENMP
		if ( sz2 > 5000 )
		{
			//printf("###\n");
			int break_flg = 0;
			Vertex* vtx = NULL;

#pragma omp parallel
			{
				Thread_omp_LoopInBreak omp(omp_get_max_threads(), sz2, 0, &break_flg);
				for(int j=omp.Begin(); omp.Condition(j); j++)
				{
					Vertex* exist_vertex = listv[j];
					if(vertex.equals(exist_vertex, tol) )
					{
						#pragma omp critical
						{
							exist_vertex->setStatus(status);
							vtx = exist_vertex;
						}
						MY_LOOP_BREAK(omp);
					}
				}
			}
			if ( vtx ) return vtx;
		}else
#endif
		{
			for(int j=0;j<sz2;j++)
			{
				Vertex* exist_vertex = listv[j];
				if(vertex.equals(exist_vertex, tol) )
				{
					exist_vertex->setStatus(status);
					return exist_vertex;
				}
			}
		}
	}

	Vertex * pVertexInList = vertices->AddVertex(vertex);
	unigrid->Add(fxyz, pVertexInList);
	return pVertexInList;
}

//-------------------------FACES_SPLITTING_METHODS------------------------------//
static int splitObject_time_course_limit = -1;
void set_splitObject_time_course_limit( int limit )
{
	splitObject_time_course_limit = limit;
}
int get_splitObject_time_course_limit(void)
{
	return splitObject_time_course_limit;
}

//#ifdef USE_OPENMP
//#define RAY_THREAD_MAX		(FACE_TMP_VAR_MAX)
//#else
//#define RAY_THREAD_MAX		(1)
//#endif

#define OVERLAP_CHEK_STEP_MAX	(256)

/**
 * Split faces so that none face is intercepted by a face of other object
 * 
 * @param object the other object 3d used to make the split 
 */
int Object3D::splitFaces(Object3D * object, const int extend_numFacesMax_Num)
{
	int stat = 0;

	mlFloat org_SPLIT_TOL = SPLIT_TOL;
	const Bound* object_bound = object->getBound();

	if(!getBound()->overlap(object_bound))
	{
		return 0;
	}

	const int numFacesStart  = getNumFaces();

	int NumStep = numFacesStart/3;								//切断結果増分余裕
	int numFacesMax = numFacesStart*SPLIT_FACSE_MAX;			//切断結果最大数
	int extend_numFacesMax_Num_count = 0;						//切断結果最大数の変更回数
	int extend_numFacesMax_Num_wrk = extend_numFacesMax_Num;	//切断結果最大数の変更回数上限
	int extend_numFacesMax_Num_count2 = 0;						//切断結果の重複完全除去回数
	
	int cuttingFaceNum = numFacesStart;
	int startIndex = 0;
	
	static int error_count = 0;
	char dmpfile[256];

	//printf("numFacesStart %d numFacesMax:%d\n", numFacesStart, numFacesMax);

	OverlapCheckInit(object);

	int numthread = 1;	//計算スレッド数初期値

#ifdef USE_OPENMP
	//スレッド数制限
	Thread_omp_limit omp_limit(omp_get_max_threads());
	numthread = omp_limit.numthread;
#endif

	int ts = GetTickCount();
	int overlap_chk_time = 0;
	//printf("split start [FaceNum %d]\n", cuttingFaceNum);
	
	int pre_rest_num = 0;
	
	float diff_num = 0.0;
	int diff_count = 0;
	
	int nn_max = 5;

	int nn = 0;
	StopWach timer;
	do{
		printf(" N=%d (%d)\n", nn, cuttingFaceNum-startIndex);
		diff_num += cuttingFaceNum-startIndex;
		diff_count++;
		
		nn++;
		if ( diff_count == nn_max )
		{
			if ( diff_num/(float)diff_count <= cuttingFaceNum-startIndex && nn > nn_max*20*2)
			{
				stat = 0;
				break;
			}
			diff_count = 0;
			diff_num = 0;
		}

		//for each object1 face...
		for(int i = startIndex; i < cuttingFaceNum;i++)
		{
			Face * face1 = getFace(i);
			if ( IS_NO_OVERLAP_FACE(face1) || IS_DELETE_FACE(face1) )
			{
				continue;
			}

			if ( !(IS_OVERLAP_FACE_LIST_BITE(face1)))
			{
				//ラフチェックの実施
				int tts = GetTickCount();
				OverlapCheck( object, object_bound, i, i+OVERLAP_CHEK_STEP_MAX, cuttingFaceNum);
				overlap_chk_time += ( GetTickCount() - tts );
			}
			if ( IS_NO_OVERLAP_FACE(face1))
			{
				continue;
			}
	
			//切断に関与するfaceの数（face1->split_max）で全て切断されたとしても切断数がそれを超えるのは異常
			//何度も切断されている可能性がある。
			if ( IS_SPLIT_MAX_BIT(face1) && face1->split_max > 0 && (face1->split_count > face1->split_max) )
			{
				printf("Warning Face[%d][may be cut indefinitely] split %d > %d\n", i, face1->split_count, face1->split_max);
				continue;
			}

			int pre_num  = getNumFaces();
			int post_num = 0;
			int split_status = splitFaces0(object, i, face1, numthread, pre_num, post_num);
			if ( split_status < 0 )
			{
				stat = -1;
			}
			face1->clearItfList();


#if 0
			if ( split_status > 0 )
			{
				//微小なFaceが生成されたら重複Faceが存在する可能性があるのでそれを削除する
				//DuplicateFaceRemove2( 9, 0, pre_num, pre_num, post_num);
				//ReverseFaceRemove( pre_num, post_num);
			}
#endif


#if 10
			//予想をはるかに超えるFace数に膨れ上がった（切断しているだけなのでそんなに増えるのはおかしい)
			bool dup_checkAll = false;

			//切断結果最大数変更がまだ可能で切断結果最大数を越えていたら
			if ( extend_numFacesMax_Num_count < extend_numFacesMax_Num_wrk && numFacesMax < post_num )
			{
				//重複みなしトレランス
				mlFloat* tolP = NULL;
				mlFloat tol = DUP_FACE_TOL2;
				
				////切断結果の重複完全除去回数が#回していたら「重複みなしトレランス」を緩めてみる
				//if ( extend_numFacesMax_Num_count2 > 5 )
				//{
				//	tol = DUP_FACE_TOL3;
				//	tolP = &tol;
				//}

				//printf("numFacesMax %d post_num %d  numFacesStart %d cuttingFaceNum %d\n", numFacesMax, post_num, numFacesStart, cuttingFaceNum);
#ifdef USE_QDOUBLE
				printf("[%d/%d][%d]Eliminating duplicates %g clear\n", extend_numFacesMax_Num_count, extend_numFacesMax_Num_wrk, extend_numFacesMax_Num_count2, to_double(tol));
#else
				printf("[%d/%d][%d]Eliminating duplicates %g clear\n", extend_numFacesMax_Num_count, extend_numFacesMax_Num_wrk, extend_numFacesMax_Num_count2, (double)tol);
#endif
				//重複Faceが存在する可能性があるのでそれを削除する
				//何度も切断されている可能性がある
				int remove_num = 0;
				
				if ( extend_numFacesMax_Num_count == extend_numFacesMax_Num_wrk-1 )
				{
					//もう規定回数トライして最後なので全部調べて重複Faceを削除する
					remove_num = DuplicateFaceRemove(0, 0, post_num, 0, post_num, tolP);
					extend_numFacesMax_Num_count2++;
					dup_checkAll = true;
				}else
				{
					//前回から増えたFaceを対象に重複Faceを削除する
					remove_num = DuplicateFaceRemove(0, 0, pre_num, pre_num, post_num, tolP);
				}
				if (remove_num < 0)
				{
					stat = -3;
					break;
				}
				int rest_num = RestFaceCount(i+1, post_num);

				printf("%s i=%d start=%d end=%d (rest %d)\n", (dup_checkAll)?"*":" ", i, startIndex, cuttingFaceNum-1, rest_num);
				fflush(stdout);


				//まだ切断待ちのFaceがあるのに規定回数トライしてしまった
				if ( rest_num && extend_numFacesMax_Num_count == extend_numFacesMax_Num_wrk-1 )
				{
					if ( rest_num )
					{
						//少し様子を見てみる(切断結果最大数変更を少し増やしておく)
						extend_numFacesMax_Num_wrk += 20;
					}
				}

				extend_numFacesMax_Num_count++;
				numFacesMax = post_num + NumStep;

				//extend_numFacesMax_Num_count2 > 1 は相当にモデルが悪い状態か接触ケースが頻発するような状態
				//全部調べて重複Faceを削除しても残りが（FACE_SPLIT_REST_NUM_ABNORMAL_VALUE）以上か既に「全部調べて重複Faceを削除」をEXTEND_NUMFACEMAX_NUM2回以上もやっているなら
				if ( extend_numFacesMax_Num_count2 > 1 && dup_checkAll && rest_num > FACE_SPLIT_REST_NUM_ABNORMAL_VALUE || extend_numFacesMax_Num_count2 > EXTEND_NUMFACEMAX_NUM2)
				{
					printf("pre_rest_num %d rest_num %d %.3f%%\n", pre_rest_num, rest_num, 100.0*(double)rest_num/(double)pre_rest_num); 
					
					//ただし、残りが減っているなら以下は行わないが残りが増えたか変化しないなら
					if ( dup_checkAll && pre_rest_num && (pre_rest_num <= rest_num) || extend_numFacesMax_Num_count2 > EXTEND_NUMFACEMAX_NUM2)
					{
						//Face交差判定のトレランスを緩めてみる
						extend_numFacesMax_Num_wrk += 20;
						SPLIT_TOL = SPLIT_TOL*10.0;
#ifdef USE_QDOUBLE
						printf("Changing Face of tolerance of intersection tests[%g=>%g]\n", to_double(org_SPLIT_TOL), to_double(SPLIT_TOL));
#else
						printf("Changing Face of tolerance of intersection tests[%g=>%g]\n", (double)org_SPLIT_TOL, (double)SPLIT_TOL);
#endif					
						extend_numFacesMax_Num_count2 = 0;				
						//{
						//	char b[256];
						//	printf("...");
						//	gets(b);
						//}

						//Face交差判定のトレランスをこれ以上緩められないので止めてリ上位からトライさせる
						if ( SPLIT_TOL > Object3D_SPLIT_TOLMAX )
						{
							stat= -1;
							break;
						}
					}
				}
				if ( dup_checkAll )
				{
					pre_rest_num = rest_num;
				}

			}
#endif

			//prevent from infinite loop (with a loss of faces...)

			//どうにもならずに膨大なFaceになってしまったので一旦終わる
			if( numFacesMax < post_num)
			{
				printf("possible infinite loop situation: terminating faces split.\n");
				printf("Split separation process is an infinite loop. %d => %d\n", numFacesStart, post_num);
				fflush(stdout);

				if ( !dup_checkAll )
				{
					//重複Faceが存在する可能性があるのでそれを削除する
					int remove_num = DuplicateFaceRemove(-1, 0, post_num, 0, post_num);
					int rest_num = RestFaceCount(i+1, post_num);
					printf("i=%d start=%d end=%d (rest %d)\n", i, startIndex, cuttingFaceNum-1, rest_num);
				}

#if 0
#pragma omp single
				{
					sprintf(dmpfile, "infinite_loop_error(%04d).obj", error_count);
					FILE* fp = fopen(dmpfile, "w");
					//fprintf(fp, "# %x\n", (void*)face1);
					int vnum = 0;
					for ( int k = pre_num; k < post_num; k++ )
					{
						if ( IS_DELETE_FACE(faces->GetFace(k))) continue;
						Vertex** vtx = 	faces->GetFace(k)->getVertex();
						for ( int i = 0; i < 3; i++ )
						{
							fprintf(fp, "v %s %s %s 255 100 0\n", 
								vtx[i]->xyz.x.to_string().c_str(),
								vtx[i]->xyz.y.to_string().c_str(),
								vtx[i]->xyz.z.to_string().c_str());
						}
						fprintf(fp, "f %d %d %d\n", vnum+1, vnum+2, vnum+3);
						fflush(stdout);
						vnum += 3;
					}
					fclose(fp);
					error_count++;
				}
#endif
				stat =  -1;
				break;
			}

			if ( splitObject_time_course_limit > 0 && timer.time_course() > splitObject_time_course_limit )
			{
				printf("\nSplit separation process time out.\n");
				fflush(stdout);

				stat =  -2;
				break;
			}
			
			//printf("                               \rSplit %d/%d", i+1, post_num);

			if ( post_num > 100000 )
			{
				DEBUG_PRINT(("                               \rSplit %d/%d", i+1, post_num));
			}
		}
		if ( stat < 0 ) break;

		//printf("%d -- %d -> %d -- %d\n", startIndex, cuttingFaceNum, cuttingFaceNum, getNumFaces());
		startIndex = cuttingFaceNum;
		cuttingFaceNum = getNumFaces();
	}while(startIndex != cuttingFaceNum);

	SPLIT_TOL = org_SPLIT_TOL;
	if ( cuttingFaceNum > 100000 )
	{
		DEBUG_PRINT(("\n"));
	}

#ifdef USE_OPENMP
	//最初のスレッドMAX数に戻しておく
	omp_limit.Undo();
#endif

	DEBUG_PRINT(("overlap_chk time %d ms\n", overlap_chk_time));
	DEBUG_PRINT(("split       time %d ms\n", GetTickCount() - ts - overlap_chk_time));

	//ReverseFaceRemove( 0, cuttingFaceNum); //削除したら駄目

	//削除されたファセットを実際に削除する
	int num = 0;
	std::vector<int> indexList;
	indexList.resize(cuttingFaceNum, -1);

	for ( int i = 0; i < cuttingFaceNum; i++ )
	{
		if ( IS_DELETE_FACE((faces->GetFace(i))) )
		{
			continue;
		}
		indexList[i] = num;
		num++;
	}
	printf("output Face %d\n", num);

	FaceSet* fset = new FaceSet();
	fset->Resize(num);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for ( int i = 0; i < cuttingFaceNum; i++ )
	{
		Face* f = faces->GetFace(i);

		if ( IS_DELETE_FACE(f) )
		{
			delete f;
		}else
		{
			fset->SetFace(indexList[i], f);
		}
		//後でthis->facesを差し替えるときにデストラクタが開放しないようにしておく
		faces->SetFace(i, NULL);
	}

	delete this->faces;
	this->faces = fset;

	//printf("split end [FaceNum %d]\n\n", getNumFaces());
	return stat;

}

int Object3D::RestFaceCount(int curindex, int num)
{
	int rest_num = 0;
	for ( int k = curindex; k < num; k++ )
	{
		Face* f = getFace(k);
		if ( IS_DELETE_FACE(f) )
		{
			continue;
		}
		rest_num++;
	}
	return rest_num;
}


int Object3D::DuplicateFaceRemove(const int call_marker, const int is, const int ie, const int js, const int je, mlFloat* chktol)
{
	int ts = GetTickCount();
	int remove_num = 0;

	printf("DuplicateFaceRemove start\n");
	fflush(stdout);

	mlFloat tol = DUP_FACE_TOL2;

	if (chktol)
	{
		tol = *chktol;
	}
	printf("is:%d  ie:%d %d\n", is, ie, ie - is);
	fflush(stdout);
	printf("js:%d  je:%d %d\n", js, je, je - js);
	printf("total loop:%d\n", (ie - is) * (je - js));
	printf("faces:%d\n", faces->GetSize());
	fflush(stdout);

	int max_ = 10000000;
	if ((ie - is) > max_ || (je - js) > max_ || (ie - is) * (je - js) < 0)
	{
		printf("total loop limit!!\n");
		return -1;
	}

	for ( int j = is; j < ie; j++ )
	{
		bool dup = false;
		Face* pre_face = faces->GetFace(j);

#ifdef USE_OPENMP
#pragma omp parallel for reduction(+: remove_num) 
#endif
		for ( int k = js; k < je; k++ )
		{
			if ( k == j ) continue;
			Face* post_face = faces->GetFace(k);
			if ( IS_DELETE_FACE(post_face)) continue;

			dup = pre_face->equals_geometry(post_face, tol) || pre_face->equals_geometry_invert(post_face, tol);
			if ( dup )
			{
				//printf("%g clear duplicates[%d] and [%d]\n",  DUP_FACE_TOL2, k, j);
				//fflush(stdout);
				remove_num++;

				DELETE_FACE(post_face);
			}
		}
	}
	//if ( remove_num )printf("[%d] remove_duplicates %d pre %d post %d->%d\n", call_marker, remove_num, pre_num, post_num, post_num-remove_num);
	printf("delete duplicate Face[%d]...[%dms]\n\n", remove_num, GetTickCount()-ts);
	fflush(stdout);
	printf("DuplicateFaceRemove end\n");
	fflush(stdout);

	return remove_num;
}

int Object3D::ReverseFaceRemove(const int is, const int ie)
{
	int ts = GetTickCount();
	int remove_num = 0;

#ifdef USE_OPENMP
#pragma omp parallel for reduction(+: remove_num) 
#endif
	for ( int j = is; j < ie; j++ )
	{
		Face* face = faces->GetFace(j);
		if ( IS_DELETE_FACE(face)) continue;
		if ( BIT_GET(face->attr, D_REVERS_FACE_BIT) )
		{
			DELETE_FACE(face);
			remove_num++;
		}
	}
	//if ( remove_num )printf("[%d] remove_duplicates %d pre %d post %d->%d\n", call_marker, remove_num, pre_num, post_num, post_num-remove_num);
	if ( remove_num )
	{
		printf("delete reverse Face[%d]...[%dms]\n\n", remove_num, GetTickCount()-ts);
		fflush(stdout);
	}
	return remove_num;
}

int Object3D::DuplicateFaceRemove2(const int call_marker, const int is, const int ie, const int js, const int je)
{
	int ts = GetTickCount();

	int remove_num = 0;
#ifdef USE_OPENMP
//#pragma omp parallel for reduction(+: remove_num)
#endif
	for ( int j = is; j < ie; j++ )
	{
		bool dup = false;
		Face* pre_face = faces->GetFace(j);

		for ( int k = js; k < je; k++ )
		{
			if ( k == j ) continue;
			Face* post_face = faces->GetFace(k);
			if ( IS_DELETE_FACE(post_face)) continue;

			dup = pre_face->equals_geometry(post_face, DUP_FACE_TOL) || pre_face->equals_geometry_invert(post_face, DUP_FACE_TOL);
			if ( (dup && (BIT_GET(post_face->attr, D_MICRO_FACE_BIT)) ))
			{
				//printf("Eliminating duplicates[%d] and [%d]\n", k, j);
				DELETE_FACE(post_face);
				remove_num++;
			}
		}
	}

	if ( remove_num)
	{
		printf("delete duplicate micro-Face[%d]...[%dms]\n\n", remove_num, GetTickCount()-ts);
		fflush(stdout);
	}
	return remove_num;
}

void Object3D::OverlapCheckInit(Object3D * object)
{
	// octreecellから近傍Faceを取得するためにスレッドによって
	// 間違った近傍を取らないようにするための初期化
	FaceSet* fset = object->faces;
	const int sz = fset->GetSize();

	int maxthread = 1;

#ifdef USE_OPENMP
	maxthread = omp_get_max_threads();
//#pragma omp parallel for
#endif
	for ( int i = 0; i < sz; i++ )
	{
		for ( int j = 0; j < maxthread; j++ )
		{
			fset->GetFace(i)->tmp[j] = 0;
		}
	}
}


int Object3D::OverlapCheck(Object3D * object, const Bound* object_bound, const int startIndex, const int endIndex, const int num)
{
	octreecell* occell = object->cell;

	const int endIndex_wk = (endIndex > num) ? num :endIndex;

	// octreecellからface1[startIndex...endIndex-1]の近傍Faceを取得する
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i = startIndex; i < endIndex_wk;i++)
	{
		int threadID = 0;
#ifdef USE_OPENMP
		threadID = omp_get_thread_num();
#endif
		Face * face1 = getFace(i);

		OVERLAP_FACE_LIST_BIT(face1);

		if ( IS_NO_OVERLAP_FACE(face1) || IS_DELETE_FACE(face1))
		{
			//ラップ無し
			OVERLAP_NO_SET(face1);
			continue;
		}

		Bound* face1_bound = face1->getBound();

		/* ここではじく時間だけ無駄なのでコメントにする
		if(!(IS_OVERLAP_FLAG(face1)) && !face1_bound->overlap(object_bound))
		{
			//ラップ無し
			OVERLAP_NO_SET(face1);
			continue;
		}
		*/


		std::vector<Face*> itfList_wrk;
		occell->getfaces(object, face1_bound, itfList_wrk, threadID);

		const int sz2 = itfList_wrk.size();
		int nn = 0;
		for( int j=0;j<sz2;j++)
		{
			Face* face2 = itfList_wrk[j];
			if( face1_bound->overlap(face2->getBound()) )
			{
				nn++;
			}else
			{
				itfList_wrk[j] = NULL;
			}
			//再度threadIDで探索された場合を考慮して初期化しておく
			face2->tmp[threadID] = 0;
		}
		if ( nn != 0 )
		{
			face1->itfList.swap(itfList_wrk);
			face1->split_max = nn*FACE_CUT_NUM_COEF;
			//ラップ有り
			OVERLAP_YES_SET(face1);
			continue;
		}
		//ラップ無し
		OVERLAP_NO_SET(face1);
	}

	return 1;
}




int Object3D::splitFaces0(Object3D * object, const int cuttingFaceIndex, Face* face1, const int numthread, const int pre_num, int& post_num)
{
	Bound* face1_bound = face1->getBound();
	mlVector3D& face1_normal = face1->getNormal();

	const mlFloat tol = SPLIT_TOL;
	
	std::vector<Face*> & itfList = face1->itfList;
	const int sz = itfList.size();

	SPLIT_MAX_BIT(face1);

	bool face1_not_cut = true;

	struct intersectvertexinf_type
	{
		Vertex** fVertex;
		mlFloat distFaceVert[3];
		int		signFaceVert[3];
	};

	//for each object2 face...
	int break_flg = 0;
#ifdef USE_OPENMP
#pragma omp parallel
#endif		
	{
		register struct intersectvertexinf_type f1;
		register struct intersectvertexinf_type f2;

		f1.fVertex = face1->getVertex();
		Line line(face1);	
	
		Thread_omp_LoopInBreak omp(numthread, sz, 0, &break_flg);
		//for each object2 face...
		for(int j = omp.Begin(); omp.Condition(j); j++ )
		{
			Face * face2 = face1->itfList[j];

			if ( face2 == NULL ) continue;
		
			if (/*j % 100 == 0 &&*/ omp.End() == sz)
			{
				printf("                       \rsplit %d/%d(%.3f%%)", j, sz, 100.0*j / (sz - 1.0));
			}
			//distance from the face1 vertices to the face2 plane
			mlVector3D& face2_normal = face2->getNormal();

			computeDistance(f1.fVertex, face2, &face2_normal, f1.distFaceVert);

			//distances signs from the face1 vertices to the face2 plane 
			f1.signFaceVert[0] = (f1.distFaceVert[0]>tol? 1 :(f1.distFaceVert[0]<-tol? -1 : 0)); 
			f1.signFaceVert[1] = (f1.distFaceVert[1]>tol? 1 :(f1.distFaceVert[1]<-tol? -1 : 0));
			f1.signFaceVert[2] = (f1.distFaceVert[2]>tol? 1 :(f1.distFaceVert[2]<-tol? -1 : 0));
		
			//if all the signs are zero, the planes are coplanar
			//if all the signs are positive or negative, the planes do not intersect
			//if the signs are not equal...
			if (!(f1.signFaceVert[0]==f1.signFaceVert[1] && f1.signFaceVert[1]==f1.signFaceVert[2]))
			{
				f2.fVertex = face2->getVertex();

				//distance from the face2 vertices to the face1 plane
				computeDistance(f2.fVertex, face1, &face1_normal, f2.distFaceVert);

				//distances signs from the face2 vertices to the face1 plane
				f2.signFaceVert[0] = (f2.distFaceVert[0]>tol? 1 :(f2.distFaceVert[0]<-tol? -1 : 0)); 
				f2.signFaceVert[1] = (f2.distFaceVert[1]>tol? 1 :(f2.distFaceVert[1]<-tol? -1 : 0));
				f2.signFaceVert[2] = (f2.distFaceVert[2]>tol? 1 :(f2.distFaceVert[2]<-tol? -1 : 0));
		
				//if the signs are not equal...
				if (!(f2.signFaceVert[0]==f2.signFaceVert[1] && f2.signFaceVert[1]==f2.signFaceVert[2]))
				{
					line.creaetCrossLine(face2);
				
					Segment segment[2];
		
					//intersection of the face1 and the plane of face2
					segment[0].Create(line, face1, f1.signFaceVert, f1.fVertex);
												
					//intersection of the face2 and the plane of face1
					segment[1].Create(line, face2, f2.signFaceVert, f2.fVertex);
											
					//if the two segments intersect...
					if(segment[0].intersect(segment[1]))
					{
						//PART II - SUBDIVIDING NON-COPLANAR POLYGONS
						#pragma omp critical
						{
							//スレッド処理されているので誰かが削除(face1->removed=1)しているかも知れない
							if ( !(IS_DELETE_FACE(face1)))
							{
								face1_not_cut = false;

								//PART II - SUBDIVIDING NON-COPLANAR POLYGONS
								//分割前のファセット数
								const int pre_div_num = getNumFaces();

								//分割処理
								splitFace(cuttingFaceIndex, segment);

								//分割後のファセット数
								const int pst_div_num  = getNumFaces();
#if 0
								int debug = 1;
								if ( debug && REMOVED_FACE(face1) )
								{
									if ( face1->micro )
									{
										FILE* fp = fopen("xxxx_in.obj", "w");
										for ( int i = 0; i < 3; i++ )
										{
											Vertex* vtx[3];
											face1->getVertex( vtx );
											fprintf(fp, "v %s %s %s 0 255 0\n", 
												vtx[i]->xyz.x.to_string().c_str(),
												vtx[i]->xyz.y.to_string().c_str(),
												vtx[i]->xyz.z.to_string().c_str());
										}
										fprintf(fp, "f 1 2 3\n");
										fclose(fp);

										int n = pst_div_num - pre_div_num;
										fp = fopen("xxxx_ot.obj", "w");
										for ( int j = 0; j < n; j++ )
										{
											Face* post_face = faces->GetFace(pre_div_num+j);
											Vertex* vtx[3];
											post_face->getVertex( vtx );
											for ( int i = 0; i < 3; i++ )
											{
												fprintf(fp, "v %s %s %s 255 0 0\n", 
													vtx[i]->xyz.x.to_string().c_str(),
													vtx[i]->xyz.y.to_string().c_str(),
													vtx[i]->xyz.z.to_string().c_str());
											}
										}
										int k = 1;
										for ( int j = 0; j < n; j++ )
										{
											fprintf(fp, "f %d %d %d\n", k, k+1, k+2);
											k += 3;
										}
										fclose(fp);
									}
								}
#endif
								//削除されて分割ファセットが追加されている場合
								if ( IS_DELETE_FACE(face1) && face1->equals(getFace(pst_div_num-1)))
								{
									if(cuttingFaceIndex != (pst_div_num-1))
									{
										faces->RemoveFace2(pst_div_num-1);
										DELETE_CANCEL_FACE(face1);
										face1->split_count = 0;
									}
								}
							}
						}
						if ( IS_DELETE_FACE(face1) ) 
						{
							MY_LOOP_BREAK(omp);
						}
					}
				}
			}
		}
	}

	if ( face1_not_cut )
	{
		OVERLAP_NO_SET(face1);
	}

	post_num = getNumFaces();

	for ( int i = pre_num; i < post_num; i++ )
	{
		Face* f = faces->GetFace(i);
		if ( BIT_GET(f->attr, D_REVERS_FACE_BIT) )
		{
			return D_REVERS_FACE_BIT;
		}
	}
	//printf("cuttingFaceNum %d->%d\n", pre_num, post_num);
	return 0;
}

/**
	* Computes closest distance from a vertex to a plane
	* 
	* @param vertex vertex used to compute the distance
	* @param face face representing the plane where it is contained
	* @return the closest distance from the vertex to the plane
	*/
mlFloat __fastcall Object3D::computeDistance(Vertex* vertex, Face* face, mlVector3D* normalP)
{

	if ( normalP == NULL )
	{
		normalP = &face->getNormal();
	}

	const Vertex* v = face->getVertex1();
	const mlFloat& a = normalP->x;
	const mlFloat& b = normalP->y;
	const mlFloat& c = normalP->z;
#if 0
	const mlFloat d = -(a*v->xyz.x + b*v->xyz.y + c*v->xyz.z);
	return a*vertex->xyz.x + b*vertex->xyz.y + c*vertex->xyz.z + d;
#else
	const mlVector3D* v1 = &vertex->xyz;
	const mlVector3D* v2 = &v->xyz;
	return a*(v1->x-v2->x) + b*(v1->y - v2->y)+ c*(v1->z - v2->z);
#endif

}

void __fastcall Object3D::computeDistance(Vertex* vertex[3], Face* face, mlVector3D* normalP, mlFloat dist[3])
{
	if ( normalP == NULL )
	{
		normalP = &face->getNormal();
	}

	const Vertex* v = face->getVertex1();
	const mlFloat& a = normalP->x;
	const mlFloat& b = normalP->y;
	const mlFloat& c = normalP->z;
#if 0
	const mlFloat d = -(a*v->xyz.x + b*v->xyz.y + c*v->xyz.z);

	dist[0] =  a*vertex[0]->xyz.x + b*vertex[0]->xyz.y + c*vertex[0]->xyz.z + d;
	dist[1] =  a*vertex[1]->xyz.x + b*vertex[1]->xyz.y + c*vertex[1]->xyz.z + d;
	dist[2] =  a*vertex[2]->xyz.x + b*vertex[2]->xyz.y + c*vertex[2]->xyz.z + d;
#else
	const mlVector3D* v1[3];
	const mlVector3D* v2 = &v->xyz;
	v1[0] = &vertex[0]->xyz;
	v1[1] = &vertex[1]->xyz;
	v1[2] = &vertex[2]->xyz;
	dist[0] =  a*(v1[0]->x-v2->x) + b*(v1[0]->y - v2->y)+ c*(v1[0]->z - v2->z);
	dist[1] =  a*(v1[1]->x-v2->x) + b*(v1[1]->y - v2->y)+ c*(v1[1]->z - v2->z);
	dist[2] =  a*(v1[2]->x-v2->x) + b*(v1[2]->y - v2->y)+ c*(v1[2]->z - v2->z);
#endif
}


/**
 * Split an individual face
 * 
 * @param facePos face position on the array of faces
 * @param segment1 segment representing the intersection of the face with the plane
 * of another face
 * @return segment2 segment representing the intersection of other face with the
 * plane of the current face plane
 */	  
void Object3D::splitFace(const int facePos, Segment segment[2])
{
	Vertex startPosVertex, endPosVertex;
	mlVector3D startPos, endPos;
	int startType, endType, middleType;
	mlFloat startDist, endDist;
	const mlFloat tol = TOL;
	
	Vertex** vrt = getFace(facePos)->getVertex();

	//Vertex * startVertex = segment[0].getStartVertex();
	//Vertex * endVertex = segment[0].getEndVertex();

	Vertex * startVertex;
	Vertex * endVertex;
	segment[0].getStartEnd(&startVertex, &endVertex);
	
	//starting point: deeper starting point 		
	if (segment[1].startDist > segment[0].startDist+tol)
	{
		startDist = segment[1].startDist;
		startType = segment[0].middleType;
		startPos  = segment[1].startPos;
	}
	else
	{
		startDist = segment[0].startDist;
		startType = segment[0].startType;
		startPos  = segment[0].startPos;
	}
	
	//ending point: deepest ending point
	if (segment[1].endDist < segment[0].endDist-tol)
	{
		endDist = segment[1].endDist;
		endType = segment[0].middleType;
		endPos  = segment[1].endPos;
	}
	else
	{
		endDist = segment[0].endDist;
		endType = segment[0].endType;
		endPos  = segment[0].endPos;
	}		
	middleType = segment[0].middleType;
	
	//set vertex to BOUNDARY if it is start type		
	if (startType == Segment_VERTEX)
	{
		startVertex->setStatus(Vertex_BOUNDARY);
	}
			
	//set vertex to BOUNDARY if it is end type
	if (endType == Segment_VERTEX)
	{
		endVertex->setStatus(Vertex_BOUNDARY);
	}
	
	//VERTEX-_______-VERTEX 
	if (startType == Segment_VERTEX && endType == Segment_VERTEX)
	{
		return;
	}
	
	//______-EDGE-______
	else if (middleType == Segment_EDGE)
	{
		//gets the edge 
		int splitEdge;
		if ((startVertex == vrt[0] && endVertex == vrt[1]) || (startVertex == vrt[1] && endVertex == vrt[0]))
		{
			splitEdge = 1;
		}
		else if ((startVertex == vrt[1] && endVertex == vrt[2]) || (startVertex == vrt[2] && endVertex == vrt[1]))
		{	  
			splitEdge = 2; 
		} 
		else
		{
			splitEdge = 3;
		} 
		
		//VERTEX-EDGE-EDGE
		if (startType == Segment_VERTEX)
		{
			breakFaceInTwo(facePos, endPos, splitEdge);
			return;
		}
		
		//EDGE-EDGE-VERTEX
		else if (endType == Segment_VERTEX)
		{
			breakFaceInTwo(facePos, startPos, splitEdge);
			return;
		}
    
		// EDGE-EDGE-EDGE
		else if (startDist == endDist)
		{
			breakFaceInTwo(facePos, endPos, splitEdge);
		}
		else
		{
			if((startVertex == vrt[0] && endVertex == vrt[1]) || (startVertex == vrt[1] && endVertex == vrt[2]) || (startVertex == vrt[2] && endVertex == vrt[0]))
			{
				breakFaceInThree(facePos, startPos, endPos, splitEdge);
			}
			else
			{
				breakFaceInThree(facePos, endPos, startPos, splitEdge);
			}
		}
		return;
	}
	
	//______-FACE-______
	
	//VERTEX-FACE-EDGE
	else if (startType == Segment_VERTEX && endType == Segment_EDGE)
	{
		breakFaceInTwo(facePos, endPos, *endVertex);
	}
	//EDGE-FACE-VERTEX
	else if (startType == Segment_EDGE && endType == Segment_VERTEX)
	{
		breakFaceInTwo(facePos, startPos, *startVertex);
	}
	//VERTEX-FACE-FACE
	else if (startType == Segment_VERTEX && endType == Segment_FACE)
	{
		breakFaceInThree(facePos, endPos, *startVertex);
	}
	//FACE-FACE-VERTEX
	else if (startType == Segment_FACE && endType == Segment_VERTEX)
	{
		breakFaceInThree(facePos, startPos, *endVertex);
	}
	//EDGE-FACE-EDGE
	else if (startType == Segment_EDGE && endType == Segment_EDGE)
	{
		breakFaceInThree(facePos, startPos, endPos, *startVertex, *endVertex);
	}
	//EDGE-FACE-FACE
	else if (startType == Segment_EDGE && endType == Segment_FACE)
	{
		breakFaceInFour(facePos, startPos, endPos, *startVertex);
	}
	//FACE-FACE-EDGE
	else if (startType == Segment_FACE && endType == Segment_EDGE)
	{
		breakFaceInFour(facePos, endPos, startPos, *endVertex);
	}
	//FACE-FACE-FACE
	else if (startType == Segment_FACE && endType == Segment_FACE)
	{
		mlVector3D segmentVector(startPos.x-endPos.x, startPos.y-endPos.y, startPos.z-endPos.z);
					
		//if the intersection segment is a point only...
		if (Absolute(segmentVector.x)<tol && Absolute(segmentVector.y)<tol && Absolute(segmentVector.z)<tol)
		{
			breakFaceInThree(facePos, startPos);
			return;
		}

		//gets the vertex more lined with the intersection segment
		int linedVertex;
		mlVector3D linedVertexPos;

		mlVector3D vertexVector1(endPos.x-vrt[0]->xyz.x, endPos.y-vrt[0]->xyz.y, endPos.z-vrt[0]->xyz.z);
		mlVector3D vertexVector2(endPos.x-vrt[1]->xyz.x, endPos.y-vrt[1]->xyz.y, endPos.z-vrt[1]->xyz.z);
		mlVector3D vertexVector3(endPos.x-vrt[2]->xyz.x, endPos.y-vrt[2]->xyz.y, endPos.z-vrt[2]->xyz.z);
		vertexVector1.Normalise();
		vertexVector2.Normalise();
		vertexVector3.Normalise();

		const mlFloat& dot1 = Absolute(VECTOR3D_DOT(segmentVector, vertexVector1));
		const mlFloat& dot2 = Absolute(VECTOR3D_DOT(segmentVector, vertexVector2));
		const mlFloat& dot3 = Absolute(VECTOR3D_DOT(segmentVector, vertexVector3));

		if (dot1 > dot2 && dot1 > dot3)
		{
		 	linedVertex = 1;
			linedVertexPos = vrt[0]->xyz;
		}
		else if (dot2 > dot3 && dot2 > dot1)
		{
			linedVertex = 2;
			linedVertexPos = vrt[1]->xyz;
		}
		else
		{
			linedVertex = 3;
			linedVertexPos = vrt[2]->xyz;
		}
    
		// Now find which of the intersection endpoints is nearest to that vertex.
//		if ((linedVertexPos - startPos).Magnitude() > (linedVertexPos - endPos).Magnitude())
		if ((linedVertexPos - startPos).MagnitudeSquared() > (linedVertexPos - endPos).MagnitudeSquared())
		{
			breakFaceInFive(facePos, startPos, endPos, linedVertex);
		}
		else
		{
			breakFaceInFive(facePos, endPos, startPos, linedVertex);
		}
	}
}

/**
 * Face breaker for VERTEX-EDGE-EDGE / EDGE-EDGE-VERTEX
 * 
 * @param facePos face position on the faces array
 * @param newPos new vertex position
 * @param edge that will be split 
 */		
void __fastcall Object3D::breakFaceInTwo(const int facePos, const mlVector3D & newPos, int splitEdge)
{
	Face* face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	Vertex * vertex = addVertex(newPos, v[0]->getColor(), Vertex_BOUNDARY, 1);
		
	face->split_count++;

	switch(splitEdge)
	{
	case 1:
		addFace(v[0], vertex, v[2], face, normal);
		addFace(vertex, v[1], v[2], face, normal);
		break;
	case 2:
		addFace(v[1], vertex, v[0], face, normal);
		addFace(vertex, v[2], v[0], face, normal);
		break;
	default:
		addFace(v[2], vertex, v[1], face, normal);
		addFace(vertex, v[0], v[1], face, normal);
		break;
	}

	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for VERTEX-FACE-EDGE / EDGE-FACE-VERTEX
 * 
 * @param facePos face position on the faces array
 * @param newPos new vertex position
 * @param endVertex vertex used for splitting 
 */		
void __fastcall Object3D::breakFaceInTwo(const int facePos, const mlVector3D & newPos, Vertex & endVertex)
{
	Face* face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();

	Vertex **v = face->getVertex();
	face->split_count++;

	
	Vertex * vertex = addVertex(newPos, v[0]->getColor(), Vertex_BOUNDARY, 1);
				
	if (endVertex.equals(v[0]))
	{
		addFace(v[0], vertex, v[2], face, normal);
		addFace(vertex, v[1], v[2], face, normal);
	}
	else if (endVertex.equals(v[1]))
	{
		addFace(v[1], vertex, v[0], face, normal);
		addFace(vertex, v[2], v[0], face, normal);
	}
	else
	{
		addFace(v[2], vertex, v[1], face, normal);
		addFace(vertex, v[0], v[1], face, normal);
	}

	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for EDGE-EDGE-EDGE
 * 
 * @param facePos face position on the faces array
 * @param newPos1 new vertex position
 * @param newPos2 new vertex position 
 * @param splitEdge edge that will be split
 */
void __fastcall Object3D::breakFaceInThree(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, int splitEdge)
{
	Face* face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	const color_attr_type& color = v[0]->getColor();

	Vertex * vertex1 = addVertex(newPos1, color, Vertex_BOUNDARY, 1);	
	Vertex * vertex2 = addVertex(newPos2, color, Vertex_BOUNDARY, 1);
					
	face->split_count++;

	switch(splitEdge)
	{
	case 1:
		addFace(v[0], vertex1, v[2], face, normal);
		addFace(vertex1, vertex2, v[2], face, normal);
		addFace(vertex2, v[1], v[2], face, normal);
		break;
	case 2:
		addFace(v[1], vertex1, v[0], face, normal);
		addFace(vertex1, vertex2, v[0], face, normal);
		addFace(vertex2, v[2], v[0], face, normal);
		break;
	default:
		addFace(v[2], vertex1, v[1], face, normal);
		addFace(vertex1, vertex2, v[1], face, normal);
		addFace(vertex2, v[0], v[1], face, normal);
		break;
	}

	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for VERTEX-FACE-FACE / FACE-FACE-VERTEX
 * 
 * @param facePos face position on the faces array
 * @param newPos new vertex position
 * @param endVertex vertex used for the split
 */
void __fastcall Object3D::breakFaceInThree(const int facePos, const mlVector3D & newPos, Vertex & endVertex)
{
	Face* face = faces->GetFace(facePos);
	
	mlVector3D* normal = &face->getNormal();

	Vertex **v = face->getVertex();
	Vertex * vertex = addVertex(newPos, v[0]->getColor(), Vertex_BOUNDARY, 1);
					
	face->split_count++;

	if (endVertex.equals(v[0]))
	{
		addFace(v[0], v[1], vertex, face, normal);
		addFace(v[1], v[2], vertex, face, normal);
		addFace(v[2], v[0], vertex, face, normal);
	}
	else if (endVertex.equals(v[1]))
	{
		addFace(v[1], v[2], vertex, face, normal);
		addFace(v[2], v[0], vertex, face, normal);
		addFace(v[0], v[1], vertex, face, normal);
	}
	else
	{
		addFace(v[2], v[0], vertex, face, normal);
		addFace(v[0], v[1], vertex, face, normal);
		addFace(v[1], v[2], vertex, face, normal);
	}

	faces->RemoveFace2(facePos);
}

// General translation rules:
// Use references in function parameters as opposed to pointers
// Would use pointers everywhere, but access is through dot operator
// 

/**
 * Face breaker for EDGE-FACE-EDGE
 * 
 * @param facePos face position on the faces array
 * @param newPos1 new vertex position
 * @param newPos2 new vertex position 
 * @param startVertex vertex used the new faces creation
 * @param endVertex vertex used for the new faces creation
 */
void __fastcall Object3D::breakFaceInThree(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, Vertex & startVertex, Vertex & endVertex)
{
	Face* face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	const color_attr_type& color = v[0]->getColor();

	Vertex * vertex1 = addVertex(newPos1, color, Vertex_BOUNDARY, 1);	
	Vertex * vertex2 = addVertex(newPos2, color, Vertex_BOUNDARY, 1);
					
	face->split_count++;

	if (startVertex.equals(v[0]) && endVertex.equals(v[1]))
	{
		addFace(v[0], vertex1, vertex2, face, normal);
		addFace(v[0], vertex2, v[2], face, normal);
		addFace(vertex1, v[1], vertex2, face, normal);
	}
	else if (startVertex.equals(v[1]) && endVertex.equals(v[0]))
	{
		addFace(v[0], vertex2, vertex1, face, normal);
		addFace(v[0], vertex1, v[2], face, normal);
		addFace(vertex2, v[1], vertex1, face, normal);
	}
	else if (startVertex.equals(v[1]) && endVertex.equals(v[2]))
	{
		addFace(v[1], vertex1, vertex2, face, normal);
		addFace(v[1], vertex2, v[0], face, normal);
		addFace(vertex1, v[2], vertex2, face, normal);
	}
	else if (startVertex.equals(v[2]) && endVertex.equals(v[1]))
	{
		addFace(v[1], vertex2, vertex1, face, normal);
		addFace(v[1], vertex1, v[0], face, normal);
		addFace(vertex2, v[2], vertex1, face, normal);
	}
	else if (startVertex.equals(v[2]) && endVertex.equals(v[0]))
	{
		addFace(v[2], vertex1, vertex2, face, normal);
		addFace(v[2], vertex2, v[1], face, normal);
		addFace(vertex1, v[0], vertex2, face, normal);
	}
	else
	{
		addFace(v[2], vertex2, vertex1, face, normal);
		addFace(v[2], vertex1, v[1], face, normal);
		addFace(vertex2, v[0], vertex1, face, normal);
	}
	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for FACE-FACE-FACE (a point only)
 * 
 * @param facePos face position on the faces array
 * @param newPos new vertex position
 */
void __fastcall Object3D::breakFaceInThree(const int facePos, const mlVector3D & newPos)
{
	Face * face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	Vertex * vertex = addVertex(newPos, v[0]->getColor(), Vertex_BOUNDARY, 1);

	face->split_count++;
			
	addFace(v[0], v[1], vertex, face, normal);
	addFace(v[1], v[2], vertex, face, normal);
	addFace(v[2], v[0], vertex, face, normal);

	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for EDGE-FACE-FACE / FACE-FACE-EDGE
 * 
 * @param facePos face position on the faces array
 * @param newPos1 new vertex position
 * @param newPos2 new vertex position 
 * @param endVertex vertex used for the split
 */	
void __fastcall Object3D::breakFaceInFour(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, Vertex & endVertex)
{
	Face* face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	const color_attr_type& color = v[0]->getColor();

	Vertex * vertex1 = addVertex(newPos1, color, Vertex_BOUNDARY, 1);	
	Vertex * vertex2 = addVertex(newPos2, color, Vertex_BOUNDARY, 1);
	face->split_count++;

	if (endVertex.equals(v[0]))
	{
		addFace(v[0], vertex1, vertex2, face, normal);
		addFace(vertex1, v[1], vertex2, face, normal);
		addFace(v[1], v[2], vertex2, face, normal);
		addFace(v[2], v[0], vertex2, face, normal);
	}
	else if (endVertex.equals(v[1]))
	{
		addFace(v[1], vertex1, vertex2, face, normal);
		addFace(vertex1, v[2], vertex2, face, normal);
		addFace(v[2], v[0], vertex2, face, normal);
		addFace(v[0], v[1], vertex2, face, normal);
	}
	else
	{
		addFace(v[2], vertex1, vertex2, face, normal);
		addFace(vertex1, v[0], vertex2, face, normal);
		addFace(v[0], v[1], vertex2, face, normal);
		addFace(v[1], v[2], vertex2, face, normal);
	}

	faces->RemoveFace2(facePos);
}

/**
 * Face breaker for FACE-FACE-FACE
 * 
 * @param facePos face position on the faces array
 * @param newPos1 new vertex position
 * @param newPos2 new vertex position 
 * @param linedVertex what vertex is more lined with the interersection found
 */		
void __fastcall Object3D::breakFaceInFive(const int facePos, const mlVector3D & newPos1, const mlVector3D & newPos2, int linedVertex)
{
	Face * face = faces->GetFace(facePos);
	mlVector3D* normal = &face->getNormal();
	
	Vertex **v = face->getVertex();
	const color_attr_type& color = v[0]->getColor();

	Vertex * vertex1 = addVertex(newPos1, color, Vertex_BOUNDARY, 1);	
	Vertex * vertex2 = addVertex(newPos2, color, Vertex_BOUNDARY, 1);

	face->split_count++;

	//mlFloat cont = 0;
	switch(linedVertex)
	{
	case 1:
		addFace(v[1], v[2], vertex1, face, normal);
		addFace(v[1], vertex1, vertex2, face, normal);
		addFace(v[2], vertex2, vertex1, face, normal);
		addFace(v[1], vertex2, v[0], face, normal);
		addFace(v[2], v[0], vertex2, face, normal);
		break;
	case 2:
		addFace(v[2], v[0], vertex1, face, normal);
		addFace(v[2], vertex1, vertex2, face, normal);
		addFace(v[0], vertex2, vertex1, face, normal);
		addFace(v[2], vertex2, v[1], face, normal);
		addFace(v[0], v[1], vertex2, face, normal);
		break;
	default:
		addFace(v[0], v[1], vertex1, face, normal);
		addFace(v[0], vertex1, vertex2, face, normal);
		addFace(v[1], vertex2, vertex1, face, normal);
		addFace(v[0], vertex2, v[2], face, normal);
		addFace(v[1], v[2], vertex2, face, normal);
		break;
	}

	faces->RemoveFace2(facePos);
}

//-----------------------------------OTHERS-------------------------------------//

int __fastcall Object3D::simple_classify(Object3D* object, const Bound* object_bound, Face* face, int& rayTrace_time)
{
	int stat = 0;

	//makes the ray trace classification
	int ts = GetTickCount();
	ClosestInfo_t closestInfo;
//#pragma omp critical
	{
		if (face->rayTraceClassify(object, object_bound, 0, &closestInfo) < 0)
		{
			stat = -1;
		}
	}
	if (stat < 0)
	{
		printf("rayTraceClassify error\n");
		//char tmp[32];
		//fgets(tmp, 32, stdin);
		return stat;
	}
	rayTrace_time += (GetTickCount() - ts);

	bool careful_inspection = false;

	if ( Absolute(closestInfo.dotProduct) < 0.32 || Absolute(closestInfo.closestDistance) < 1.0e-6 )
	{
		careful_inspection = true;
	}
	if (closestInfo.closestFace && BIT_GET(closestInfo.closestFace->attr, D_MICRO_FACE_BIT))
	{
		careful_inspection = true;
	}
	//careful_inspection = true;

	if ( careful_inspection )
	{
		int face_status_pre = face->status;
		const int chenum = 25;		//この回数微妙な方向でレイを飛ばしてみる
		int face_status_cnt[Face_STATUS_NUM]={0,0,0,0,0,0};
		face_status_cnt[face->status]++;

		int war = 0;
//#pragma omp critical
		{
		for (int j = 0; j < chenum; j++)
		{
			ClosestInfo_t clsinf;
			if (face->rayTraceClassify(object, object_bound, 1, NULL) < 0)
			{
				stat = -1;
				printf("------ray trace error---------\n");
			}
			else
			{
				//どう判定されたかをカウントする
				face_status_cnt[face->status]++;
			}
		}
		}
		if (stat < 0) return stat;

		rayTrace_time += (GetTickCount() - ts);

		if ( face_status_cnt[Face_INSIDE]   == chenum+1 ) face->status = Face_INSIDE;
		else if ( face_status_cnt[Face_OUTSIDE]  == chenum+1 ) face->status = Face_OUTSIDE;
		else if ( face_status_cnt[Face_SAME]     == chenum+1 ) face->status = Face_SAME;
		else if ( face_status_cnt[Face_OPPOSITE] == chenum+1 ) face->status = Face_OPPOSITE;
		else
		{
			int s = -1;
			int max_i = -1;
			for (int i = Face_UNKNOWN; i < Face_STATUS_NUM; ++i)
			{
				if (s < face_status_cnt[i])
				{
					s = face_status_cnt[i];
					max_i = i;
				}
			}
			if (max_i >= 0) face->status = max_i;
			else face->status = Face_UNKNOWN;

			war++;
		}

		if (  war )
		{
			//printf("###################\n");
		
			//face->getVertex1()->delete_mark();
			//face->getVertex2()->delete_mark();
			//face->getVertex3()->delete_mark();
			//printf("ang  %f [%f deg]\n",  (double)closestInfo.dotProduct, acos((double)closestInfo.dotProduct)*180.0/3.14159265);
			//printf("dist %f\n",  (double)closestInfo.closestDistance);
			printf("face->status %d --> %d [IN[2]:%d OUT[3]:%d OPP[4]:%d SAM[5]:%d]\n", face_status_pre, face->status, face_status_cnt[Face_INSIDE], face_status_cnt[Face_OUTSIDE], face_status_cnt[Face_SAME],face_status_cnt[Face_OPPOSITE] );
			//char tmp[32];
			//fgets(tmp, 32, stdin);
			return 1;
		}
	}

#ifdef USE_OPENMP
#pragma omp critical
#endif
	{
		Vertex** v = face->getVertex();
		const int face_stat = face->getStatus();

		//mark the vertices
		if(v[0]->getStatus()==Vertex_UNKNOWN) 
		{
			Vertex_mark_metod_call_stack_init();
			v[0]->mark(face_stat);
		}
		if(v[1]->getStatus()==Vertex_UNKNOWN) 
		{
			Vertex_mark_metod_call_stack_init();
			v[1]->mark(face_stat);
		}
		if(v[2]->getStatus()==Vertex_UNKNOWN) 
		{
			Vertex_mark_metod_call_stack_init();
			v[2]->mark(face_stat);
		}
	}
	return stat;
}

#define POSTPONE_RAYTRACE
/**
 * Classify faces as being inside, outside or on boundary of other object
 * 
 * @param object object 3d used for the comparison
 */
int __fastcall Object3D::classifyFaces(Object3D* object)
{
	//calculate adjacency information

	const int sz = getNumFaces();

	std::vector<Face*> facelist;
	facelist.resize(sz, 0);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i=0;i<sz;i++)
	{
		Face* face = getFace(i);
		Vertex** v = face->getVertex();
		if (v[0]->getStatus() == Vertex_BOUNDARY ||  v[1]->getStatus() == Vertex_BOUNDARY || v[2]->getStatus() == Vertex_BOUNDARY)
		{
			continue;
		}else{
			facelist[i] = face;
		}
	}

	for(int i=0;i<sz;i++)
	{
		if ( facelist[i] == NULL ) continue;
		
		Vertex** v = facelist[i]->getVertex();

		v[0]->addAdjacentVertex(v[1]);
		v[0]->addAdjacentVertex(v[2]);

		v[1]->addAdjacentVertex(v[0]);
		v[1]->addAdjacentVertex(v[2]);

		v[2]->addAdjacentVertex(v[0]);
		v[2]->addAdjacentVertex(v[1]);

	}

	int stat = 0;
	//for each face
	//sz = getNumFaces();

	int rayTrace_time = 0;

	//rayTraceClassify内部でoctree cellからcellボックスとレイの干渉をみてレイとの交点計算の対象とするFaceを絞り込むための初期化

	int sz2 = object->getNumFaces();
//#ifdef USE_OPENMP
//#pragma omp parallel for
//#endif
//	for ( int j = 0; j < sz2; j++ )
//	{
//		object->getFace(j)->tmp[0] = 0;
//	}
	//for ( int j = 0; j < sz2; j++ )
	//{
	//	if ( object.getFace(j)->tmp[0] != 0 )
	//	{
	//		printf("internal error.");
	//		throw "internal error.";
	//	}
	//}

	std::vector<Face*> postpone_face;	//判定をあとまわしにするFaceのリスト

	const Bound* object_bound = object->getBound();


	int nt = 1;
#ifdef USE_OPENMP
	nt = omp_get_max_threads();
#pragma omp parallel for
#endif
	for (int i = 0; i < sz2; ++i)
	{
		for (int j = 0; j < nt; ++j)
		{
			object->getFace(i)->tmp[j] = 0;
		}
	}
//#ifdef USE_OPENMP
//	nt = omp_get_max_threads();
//#pragma omp parallel for
//#endif
//	for (int i = 0; i < sz; ++i)
//	{
//		for (int j = 0; j < nt; ++j)
//		{
//			getFace(i)->tmp[j] = 0;
//		}
//	}
	int war = 0;

	int simple_classify_error = 0;
	int time_out = 0;
	StopWach timer;

	int break_flg = 0;
	int numthread = 1;
#ifdef USE_OPENMP
	numthread = omp_get_max_threads();
#pragma omp parallel num_threads(numthread)
#endif
	{
		Thread_omp_LoopInBreak omp(numthread, sz, 0, &break_flg);
		for (int i = omp.Begin(); omp.Condition(i); i++)
	{
		if (time_out) continue;
		if (simple_classify_error) continue;
		if (splitObject_time_course_limit > 0 && timer.time_course() > splitObject_time_course_limit)
		{
			printf("time out\n");
			//return -1;
			time_out = 1;
			continue;
		}
		if (i % 100 == 0 && omp.End() == sz)
		{
			printf("                  \r%d/%d(%.3f%%)", i, sz, 100.0*i / (sz - 1.0));
		}
		Face* face = getFace(i);
			//#ifdef POSTPONE_RAYTRACE 
			//		//微小なFaceは判定が微妙なので後回し
			//		if ( BIT_GET(face->attr, D_MICRO_FACE_BIT) )
			//		{
			//			postpone_face.push_back(face);
			//			continue;
			//		}
			//#endif
		//if the face vertices aren't classified to make the simple classify
		//if(face->simpleClassify()==false )
		{
			int stat = simple_classify(object, object_bound, face, rayTrace_time);
			if (stat < 0)
			{
				printf("error\n");
				simple_classify_error++;
				//abort();
				war = 1;
				//break;
			}
			if (stat == 1)
			{
				//printf("error\n");
				war = 1;
				//break;
				}
			}
		}
	}
	printf("rayTrace time %d ms\n", rayTrace_time);
	return (simple_classify_error==0)? 0: -1;


// ************************** 以下はもう使わない **************************************************
	if ( war )
	{
		postpone_face.clear();
		for(int i=0;i<sz;i++)
		{
			Face* face = getFace(i);
			face->status = Face_UNKNOWN;
			postpone_face.push_back(face);
		}
	}
//	for(int i=0;i<sz;i++)
//	{
//		Face* face = getFace(i);
//
//#ifdef POSTPONE_RAYTRACE 
//		//微小なFaceは判定が微妙なので後回し
//		if ( BIT_GET(face->attr, D_MICRO_FACE_BIT) )
//		{
//			postpone_face.push_back(face);
//			continue;
//		}
//#endif
//		//if the face vertices aren't classified to make the simple classify
//		if(face->simpleClassify()==false )
//		{
//			stat = simple_classify(object, object_bound, face, rayTrace_time);
//			if ( stat < 0 )
//			{
//				postpone_face.push_back(face);
//				//return stat;
//			}
//			if ( stat == 1 )
//			{
//				postpone_face.push_back(face);
//			}
//		}
//	}


	int retryMax = 10;
	int retryCnt = 0;

	//さらに判定を後回しにしたFaceのリスト
	std::vector<Face*> postpone_face2;

	int count = 0;
	do
	{
		//後回しになったFaceの判定
		const int sz3 = postpone_face.size();

		if ( sz3 )
		{
			printf("[%d]Judgment is required inside and outside of re-determination Face[%d]\n", count, sz3);
			count++;
		}
		for(int i=0;i<sz3;i++)
		{
			if (i % 100 == 0)
			{
				printf("                  \r%d/%d(%.3f%%)", i, sz3, 100.0*i / (sz3 - 1.0));
			}
			Face* face = postpone_face[i];
			face->tmp[0] = 0;
			int ts = GetTickCount();

			//他の隣接Faceから確定していたらもう判定は不要になっている。
			//if ( face->simpleClassify() )
			//{
			//	printf("(determined from adjacent Face)=>face->status %d\n", face->status ); 
			//	continue;
			//}

			int face_status_pre = face->status;
			int face_status_cnt_tot[Face_STATUS_NUM]={0,0,0,0,0,0};

			face->status = Face_UNKNOWN;
			int war = 0;
			for ( int j = 0; j < 10*(retryCnt+1); j++ )
			{
				const int chenum = 7;		//この回数微妙な方向でレイを飛ばしてみる
				int face_status_cnt[Face_STATUS_NUM]={0,0,0,0,0,0};

				for ( int k = 0; k < chenum; k++ )
				{
					if ( face->rayTraceClassify(object, object_bound, 1, NULL) < 0 )
					{
						stat = -1;
						printf("------ray trace error---------\n");
						return stat;
					}
					//どう判定されたかをカウントする
					face_status_cnt[face->status]++;
				}
				rayTrace_time += (GetTickCount() - ts);

				face_status_cnt_tot[Face_INSIDE]   += face_status_cnt[Face_INSIDE];
				face_status_cnt_tot[Face_OUTSIDE]  += face_status_cnt[Face_OUTSIDE];
				face_status_cnt_tot[Face_SAME]     += face_status_cnt[Face_SAME];
				face_status_cnt_tot[Face_OPPOSITE] += face_status_cnt[Face_OPPOSITE];

				if ( face_status_cnt[Face_INSIDE]   == chenum ) face->status = Face_INSIDE;
				else if ( face_status_cnt[Face_OUTSIDE]  == chenum ) face->status = Face_OUTSIDE;
				else if ( face_status_cnt[Face_SAME]     == chenum ) face->status = Face_SAME;
				else if ( face_status_cnt[Face_OPPOSITE] == chenum ) face->status = Face_OPPOSITE;
				else
				{
					war++;
				}
				//printf("[%d]face[%p]->status %d --> [IN[2]:%d OUT[3]:%d OPP[4]:%d SAM[5]:%d]\n", j, (void*)face, face_status_pre, face_status_cnt[Face_INSIDE], face_status_cnt[Face_OUTSIDE], face_status_cnt[Face_SAME],face_status_cnt[Face_OPPOSITE] ); 
				//fflush(stdout);

				if (  war )
				{
					face->status = Face_UNKNOWN;
					continue;
				}
				war = 0;
				//printf("==>face[%p]->status %d\n", (void*)face, face->status);
				break;
			}

			if ( war )
			{
				printf("face->status %d --> [IN[2]:%d OUT[3]:%d OPP[4]:%d SAM[5]:%d] ", face_status_pre, face_status_cnt_tot[Face_INSIDE], face_status_cnt_tot[Face_OUTSIDE], face_status_cnt_tot[Face_SAME],face_status_cnt_tot[Face_OPPOSITE] ); 
				//printf("--> [IN[2]:%d OUT[3]:%d OPP[4]:%d SAM[5]:%d]\n",face_status_cnt_tot[Face_INSIDE], face_status_cnt_tot[Face_OUTSIDE], face_status_cnt_tot[Face_SAME],face_status_cnt_tot[Face_OPPOSITE] ); 
				//fflush(stdout);
				int s = Face_INSIDE;
				if ( face_status_cnt_tot[s] < face_status_cnt_tot[Face_OUTSIDE] )
				{
					s = Face_OUTSIDE;
				}
				if ( face_status_cnt_tot[s] < face_status_cnt_tot[Face_SAME] )
				{
					s = Face_SAME;
				}
				if ( face_status_cnt_tot[s] < face_status_cnt_tot[Face_OPPOSITE] )
				{
					s = Face_OPPOSITE;
				}

				const double p1 = 0.65;
				int sum1 = face_status_cnt_tot[Face_INSIDE]+face_status_cnt_tot[Face_OUTSIDE]+face_status_cnt_tot[Face_SAME]+face_status_cnt_tot[Face_OPPOSITE];
				double pp1 =  (double)face_status_cnt_tot[s]/(double)sum1;

				if ( face_status_cnt_tot[Face_INSIDE] == 0 && face_status_cnt_tot[Face_OUTSIDE] >  0 ||
					 face_status_cnt_tot[Face_INSIDE] >  0 && face_status_cnt_tot[Face_OUTSIDE] == 0 )
				{
					war = 0;
				}

				face->status = s;
				printf("\n\tface->status %d [%.3f%%]\n", face->status, 100.0*pp1);
				fflush(stdout);
				if ( pp1 > p1 ) war = 0;
				//if ( war ) face->status = Face_UNKNOWN;
			}
			if ( war )
			{
				postpone_face2.push_back(face);
			}
		}

		if ( postpone_face2.size() )
		{
			postpone_face = postpone_face2;
			postpone_face2.clear();
			retryCnt++;
		}else
		{
			break;
		}
	}while ( retryCnt <  retryMax);

	printf("rayTrace time %d ms\n", rayTrace_time);
	return stat;
}

/** Inverts faces classified as INSIDE, making its normals point outside. Usually
 *  used into the second solid when the difference is applied. */
void Object3D::invertInsideFaces()
{
	//StopWach stopwatch("invertInsideFaces");
	const int sz = getNumFaces();

#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	for(int i=0;i<sz;i++)
	{
		Face * face = getFace(i);
		if(face->getStatus()==Face_INSIDE)
		{
			face->invert();
		}
	}
	//stopwatch.stop();
}



