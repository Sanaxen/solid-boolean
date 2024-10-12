#include "EdgeCollapse.hpp"
#include <queue>
#include <random>
#include <chrono>

int Smoothing(Solid& solid, HEdge* he, std::vector<int>& vertexFlag, double costh, double dt, double probability);
struct target_face
{
	int id;
	double area;
	double quality;
};

bool operator<(const target_face& x, const target_face& y) {
	return x.area > y.area;
}

bool is_EdgeCollapseQueueIn(std::vector<target_face>& EdgeCollapseQueue, Solid& solid, int faceid, double minarea, const double diaglen)
{
	//無視できるFaceはスキップ
	if ( solid.faceAttributeList[faceid].alive == 0 || solid.faceAttributeList[faceid].skipp )
	{
		return false;
	}
	if ( (solid.faceAttributeList[faceid].area > 0.5*(diaglen) ) && solid.faceAttributeList[faceid].quality > 0.17 )
	{
		return false;
	}

	if (minarea > 0 && solid.faceAttributeList[faceid].area > minarea)
	{
		return false;
	}
#pragma omp critical
	{
		struct target_face t;
		t.id = faceid;
		t.area = solid.faceAttributeList[faceid].area;
		t.quality = solid.faceAttributeList[faceid].quality;
		EdgeCollapseQueue.push_back(t);
	}
	return true;
}

void CreateEdgeCollapseQueue( std::vector<target_face>& EdgeCollapseQueue, Solid& solid, double minarea, double pdf)
{
	clock_t start = clock();
	MeshV& mesh = *solid.getMesh();
	const int triNum = mesh.iNumIndices;

	std::random_device rnd;     // 非決定的な乱数生成器を生成
	std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
	std::uniform_real_distribution<double> rand1(0.0, 1.0);

	const double diaglen = solid.getMesh()->diagonal_length*0.1;
	const double diaglen2 = diaglen*diaglen;
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for ( int ii = 0; ii < triNum; ii++ )
	{
		if ((double)rand1(mt) < (1.0 - pdf) )
		{
			//fprintf(stderr, "-------------------------\n");
			continue;
		}
		if ( ii % 10000 == 0 ) printf("\r\r\r\r\r%d/%d", ii+1, triNum);

		//無視できるFaceはスキップ
		if ( !is_EdgeCollapseQueueIn(EdgeCollapseQueue, solid, ii, minarea, diaglen2) )
		{
			continue;
		}
	}
	std::sort(EdgeCollapseQueue.begin(), EdgeCollapseQueue.end());
	printf("\n%d ms -> %I64u/%d\n", clock() - start, EdgeCollapseQueue.size(), triNum);
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
int Simple_EdgeCollapse(int type, Solid& solid, double minarea, double costh, double pdf)
{
	MeshV& mesh = *solid.getMesh();
	const int triNum = mesh.iNumIndices;

	char fname[256];
	int stat = 0;

	fflush(stdout);
	printf("\nCreateEdgeCollapseQueue START\n"); fflush(stdout);
	std::vector<target_face> EdgeCollapseQueue;
	CreateEdgeCollapseQueue(EdgeCollapseQueue, solid, minarea, pdf);
	printf("->CreateEdgeCollapseQueue END\n");
	fflush(stdout);
	printf("sqrt %f -> %f\n", EdgeCollapseQueue[0].area, EdgeCollapseQueue[EdgeCollapseQueue.size() - 1].area);

	std::vector<int> vertexFlag;
	vertexFlag.resize( solid.getMesh()->iNumVertices, 0);

	std::vector<N_Cylinder> boundarysList;
	printf("\ntarget %I64u\n", EdgeCollapseQueue.size());

	int count = -1;
	const int sz = EdgeCollapseQueue.size();
	for( int jj = sz-1; jj >= 0; jj--)
	{
		//std::chrono::system_clock::time_point  start, end; // 型は auto で可
		clock_t start, end;
		START();

		count++;
		const target_face& tf = EdgeCollapseQueue[jj];
		const int ii = tf.id;
		if (count % 1000 == 0 && jj > 1000)
		{
			printf("                            \r\r\r\r\r%d %f %f", jj,  tf.area, tf.quality);
		}
		else
		{
			printf("                            \r\r\r\r\r%d %f %f", jj, tf.area, tf.quality);
		}
		END(1);

		//無視できるFaceはスキップ
		if ( solid.faceAttributeList[ii].alive == 0 )
		{
			//printf("\n無視できるFaceはスキップ\n");
			continue;
		}

		
		//Face(ii)に隣接するFace間の位相構造を構築して一つのパーツとする
		solid.Initial();
		START();
		Face* f = solid.SolidPart(ii);	//Face(ii)と隣接Faceの実態とエッジ・ループを生成
		END(2);

		if ( f == NULL  )
		{
			continue;
		}
		if ( f->error_ )
		{
			continue;
		}

		//sprintf(fname, "c:\\temp\\part%d.obj", ii);
		//solid.dump(fname);

		int boundary_error = 0;

		START();
		//パーツの境界を抽出する
		std::vector<std::set<HEdge*> > boundarys;
		if ( solid.boundaryLoop(boundarys) != 0)
		{
			//printf("\n境界を抽出エラー\n");
			boundary_error = true;
		}

		//境界が出来ない場合はその周辺の位相が不正なのでスキップする
		if ( boundarys.size() != 1 )
		{
			boundary_error = 2;
			//printf("\n%d-boundarys\n", boundarys.size());
		}

		if ( boundary_error )
		{
			//const int faceNum = solid.faceList.size();
			//printf("\n%d %d %d\n", f->index[0], f->index[1], f->index[2]);
			//printf("%d %d %d\n", f->loop->hedge->StartPos()->id_, f->loop->hedge->next->StartPos()->id_, f->loop->hedge->next->next->StartPos()->id_);
			//for ( int i = 0; i < faceNum; i++ )
			//{
			//	solid.faceAttributeList[solid.faceList[i]->id].alive = 0;
			//}
			//printf("境界エラー\n");
			//solid.dump("bound_error.obj");
			//N_Cylinder boundary;
			//for ( int i = 0; i < boundarys.size(); i++ )
			//{
			//	std::set<HEdge*>::iterator jt = boundarys[i].begin();
			//	for ( ; jt !=boundarys[i].end(); ++jt)
			//	{
			//		Cylinder cyl((*jt)->StartPos()->coord(),(*jt)->EndPos()->coord(), 0.015);
			//		boundary.Add(cyl);
			//	}
			//}
			//boundary.put("boundary.obj");
			//std::vector<int> ids;
			//ids.push_back(ii);
			//solid.dump("face.obj", ids);
			//ids.clear();
			//for ( int k = 0; k < solid.faceList.size(); k++ ) ids.push_back( solid.faceList[k]->id);
			//solid.dump("facelist.obj", ids);
			continue;
		}
		END(3);

		//境界を構成する頂点の境界マークを付ける
		const int faceNum = solid.faceList.size();
		const int boundary_num = boundarys.size();

		//fprintf(stderr, "faceNum %d boundary_num %d\n", faceNum, boundary_num);

		START();
		//N_Cylinder boundary;
		for ( int i = 0; i < boundary_num; i++ )
		{
			std::set<HEdge*>::iterator jt = boundarys[i].begin();
			for ( ; jt !=boundarys[i].end(); ++jt)
			{
				(*jt)->StartPos()->boundary_ = 1;
				(*jt)->EndPos()->boundary_ = 1;
				//Cylinder cyl((*jt)->StartPos()->coord(),(*jt)->EndPos()->coord(), 0.015);
				//boundary.Add(cyl);
			}
		}
		END(4);
		//boundarysList.push_back(boundary);

		//Faceのループの周囲が取り囲まれているかチェックする
		HEdge* he = f->loop->hedge;
		if ( he->edge_->HalfEdgeRefCount() == 2 && he->next->edge_->HalfEdgeRefCount() == 2 && he->next->next->edge_->HalfEdgeRefCount() == 2 )
		{
			/* ok */
		}else
		{
			//printf("\nFaceのループの周囲が取り囲まれていないのでスキップ\n");
			continue;
		}
		
		//if (! he )
		//{
		//	continue;
		//}

		//Faceの各エッジの長さを求め最小のエッジ（collapse_he）を見つける
		HEdge* he3[3];
		he3[0] = he;
		he3[1] = he->next;
		he3[2] = he->next->next;

		double len[3];
		
		len[0] = he3[0]->edge_->length_sqr();
		len[1] = he3[1]->edge_->length_sqr();
		len[2] = he3[2]->edge_->length_sqr();

		HEdge* collapse_he = he3[0];
		if ( len[1] < len[0] )
		{
			double ltmp = len[0];
			HEdge* tmp = he3[0];
			he3[0] = he3[1]; len[0] = len[1];
			he3[1] = tmp;	 len[1] = ltmp;
		}
		if ( len[2] < len[0] )
		{
			double ltmp = len[0];
			HEdge* tmp = he3[0];
			he3[0] = he3[2]; len[0] = len[2];
			he3[2] = tmp;	 len[2] = ltmp;
		}
		if ( len[2] < len[1] )
		{
			double ltmp = len[1];
			HEdge* tmp = he3[1];
			he3[1] = he3[2]; len[1] = len[2];
			he3[2] = tmp;	 len[2] = ltmp;
		}


		START();
		for ( int i = 0; i < 1/* 3 */; i++ )
		{
			collapse_he = he3[i];
			//printf("len %.16f\n", len);

			//そのエッジ（collapse_he）をはさむFaceが潰されていないかチェックする
			if ( !collapse_he->edge_->hedge[0]->loop_->face_->alive || !collapse_he->edge_->hedge[1]->loop_->face_->alive)
			{
				collapse_he = NULL;
				continue;
			}

			//そのエッジ（collapse_he）の両端が境界に無いかチェックする
			if (  collapse_he->EndPos()->boundary_ || collapse_he->StartPos()->boundary_ )
			{
				collapse_he = NULL;
				continue;
			}else
			{
				break;
			}
		}
		END(5);

		if ( collapse_he == NULL )
		{
			//printf("\nスキップ\n");
			continue;
		}

		
		//Smoothing(solid, collapse_he, vertexFlag,  costh, 0.3);
		
		//エッジ（collapse_he）を縮退化する
		double* p = collapse_he->EndPos()->coord();
		double* q = collapse_he->StartPos()->coord();
		double pp[3];
		double qq[3];
		pp[0] = p[0];
		pp[1] = p[1];
		pp[2] = p[2];
		qq[0] = q[0];
		qq[1] = q[1];
		qq[2] = q[2];
		q[0] = (pp[0] + qq[0])*0.5;
		q[1] = (pp[1] + qq[1])*0.5;
		q[2] = (pp[2] + qq[2])*0.5;
		p[0] = q[0];
		p[1] = q[1];
		p[2] = q[2];

		START();
		//QEM
		if ( type == 1 )
		{
			double qem_coord[3];
			int stat = calcVertexQEM(&solid, solid.faceList, collapse_he->StartPos(), collapse_he->EndPos(), qem_coord);
			if ( stat == 0 )
			{
				double s[3];
				VEC3_SUB(s, q, qem_coord);
				double d = DOTPRODUCT(s);

				//printf("==> %.16f\n", sqrt(d));
				if ( d <  collapse_he->edge_->length_sqr() )
				{
					q[0] = qem_coord[0];
					q[1] = qem_coord[1];
					q[2] = qem_coord[2];

					p[0] = q[0];
					p[1] = q[1];
					p[2] = q[2];
				}
			}
		}
		END(5);

		int retry = 0;
		bool check = true;
		do{
			//縮退化でつぶした
			collapse_he->edge_->hedge[0]->loop_->face_->alive = 0;
			collapse_he->edge_->hedge[1]->loop_->face_->alive = 0;
			collapse_he->edge_->alive = 0;

			std::vector<struct index_swap_inf> index_swapList;
			index_swapList.resize(3*faceNum);

			//頂点を無効にしたのでそれを参照している箇所をメンテナンスする
			const int cur_id = collapse_he->EndPos()->id_;
			const int new_id = collapse_he->StartPos()->id_;

			struct index_swap_inf
			{
				unsigned int* index;
				int org_id;
			};

			START();

#ifdef _OPENMP
#pragma omp parallel for
#endif
			for ( int jj = 0; jj < faceNum; jj++ )
			{
				index_swapList[3*jj].index = NULL;
				index_swapList[3*jj+1].index = NULL;
				index_swapList[3*jj+2].index = NULL;
				const int k = solid.faceList[jj]->id;
				for ( int i = 0; i < 3; i++ )
				{
					if ( mesh.pIndices[3*k+i] == cur_id )
					{
						index_swapList[3*jj+i].index = &(mesh.pIndices[3*k+i]);
						index_swapList[3*jj+i].org_id = cur_id;
						mesh.pIndices[3*k+i] = new_id;
					}
				}
			}
			END(6);


			//縮退化で周囲のFaceがひっくり返っていないかチェックする
			check = true;
			Face* f1 = collapse_he->edge_->hedge[0]->loop_->face_;
			Face* f2 = collapse_he->edge_->hedge[1]->loop_->face_;
			START();

#ifdef _OPENMP
#pragma omp parallel for
#endif
			for ( int jj = 0; jj < faceNum; jj++ )
			{
				if (!check) continue;
				Face* f = solid.faceList[jj];
				if (  f == f1 || f == f2 )
				{
					continue;
				}

				unsigned int* id =  &(mesh.pIndices[3*f->id]);
				if ( id[0] == id[1] || id[0] == id[2] || id[1] == id[2] )
				{
					continue;
				}

				double *n1 = f->Normal();
				double n2[3];
				f->CalcNormal(n2);

				double inner = DOTPRODUCT2(n2, n1);
				if ( inner  <= 0.6 )
				{
					//printf("N %.16f %.16f %.16f\nN %.16f %.16f %.16f\n", n2[0], n2[1], n2[2], n1[0], n1[1], n1[2] );
					check = false;
				}

				//double q = TriangleQuality(&mesh, f->id);
				//if ( q < solid.faceAttributeList[f->id].quality*0.3 )
				//{
				//	check = false;
				//	break;
				//}
				if ( inner  < costh )
				{
					//printf("N %.16f %.16f %.16f\nN %.16f %.16f %.16f\n", n2[0], n2[1], n2[2], n1[0], n1[1], n1[2] );
					check = false;
				}
			}

			END(7);

	#if 10
			//重なってしまっていないかチェックする
			if ( check )
			{
				for ( int jj = 0; jj < faceNum; jj++ )
				{
					if ( solid.faceList[jj] == collapse_he->edge_->hedge[0]->loop_->face_ )
					{
						continue;
					}
					if ( solid.faceList[jj] == collapse_he->edge_->hedge[1]->loop_->face_ )
					{
						continue;
					}
					for ( int kk = 0; kk < faceNum; kk++ )
					{
						if ( jj == kk ) continue;

						if ( solid.faceList[jj]->equal( solid.faceList[kk] ) )
						{
							printf("一致\n");
							check = false;
							break;
						}
					}
					if ( !check ) break;
				}
			}
	#endif

			//結果が不正になるならエッジの両端を元の位置に戻す
			if ( !check )
			{
				//printf("\n結果が不正になる　スキップ\n");
				p[0] = pp[0];
				p[1] = pp[1];
				p[2] = pp[2];
				q[0] = qq[0];
				q[1] = qq[1];
				q[2] = qq[2];
				f1->alive = 1;
				f2->alive = 1;
				collapse_he->edge_->alive = 1;

#ifdef _OPENMP
#pragma omp parallel for
#endif
				for ( int jj = 0; jj < faceNum; jj++ )
				{
					for ( int i = 0; i < 3; i++ )
					{
						if ( index_swapList[3*jj+i].index )
						{
							*(index_swapList[3*jj+i].index) = index_swapList[3*jj+i].org_id;
						}
					}
				}			
			}

			if (!check )
			{
				//リトライしない
				{
					retry = 0;
					break;
				}

				retry++;
				switch( retry )
				{
				case 1:
					p[0] = qq[0];
					p[1] = qq[1];
					p[2] = qq[2];
					break;
				case 2:
					q[0] = pp[0];
					q[1] = pp[1];
					q[2] = pp[2];
					break;
				default:
					retry = 0;
					break;
				}
			}else{
				stat++;
				solid.faceAttributeList[f1->id].alive = 0;
				solid.faceAttributeList[f2->id].alive = 0;
				retry = 0;
			}
		}while( retry );

		if ( !check )
		{
			continue;
		}
		//sprintf(fname, "c:\\temp\\part%d_out.obj", ii);
		//save_mesh_obj(fname, mesh);


#if 10
		//変形したので面積を再計算しておく
		std::vector<FaceAttribute>& faceAttributeList = solid.faceAttributeList;
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for ( int jj = 0; jj < faceNum; jj++ )
		{
			const unsigned int k = solid.faceList[jj]->id;
			unsigned int* pindex = &(mesh.pIndices[3 * k]);
			if (pindex[0] == pindex[1] || pindex[0] == pindex[2] || pindex[1] == pindex[2])
			{
				faceAttributeList[jj].alive = 0;
				faceAttributeList[jj].area = 0.0;
				continue;
			}
			faceAttributeList[jj].alive = 1;
			faceAttributeList[k].CalcArea(&mesh);
		}
#endif

		//パーツを削除
		solid.Clear();
	}

	//面積を再計算する。
	const int attrNum = solid.faceAttributeList.size();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for ( int jj = 0; jj < attrNum; jj++ )
	{
		unsigned int* pindex = &(mesh.pIndices[3 * jj]);

		if (pindex[0] == pindex[1] || pindex[0] == pindex[2] || pindex[1] == pindex[2])
		{
			solid.faceAttributeList[jj].alive = 0;
			solid.faceAttributeList[jj].area = 0.0;
			continue;
		}
		solid.faceAttributeList[jj].CalcArea(&mesh);
	}
	
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int jj = 0; jj < mesh.iNumVertices; jj++)
	{
		mesh.faceGroup[jj].clear();
	}

	for (int jj = 0; jj < mesh.iNumIndices; jj++)
	{
		unsigned int* pindex = &(mesh.pIndices[3 * jj]);

		if (pindex[0] == pindex[1] || pindex[0] == pindex[2] || pindex[1] == pindex[2])
		{
			solid.faceAttributeList[jj].alive = 0;
			continue;
		}
		solid.faceAttributeList[jj].alive = 1;
		mesh.faceGroup[pindex[0]].insert(jj);
		mesh.faceGroup[pindex[1]].insert(jj);
		mesh.faceGroup[pindex[2]].insert(jj);
	}

	//int non_manifolde = 0;
	//for (int jj = 0; jj < mesh.iNumIndices; jj++)
	//{
	//	if (solid.faceAttributeList[jj].alive == 0)
	//	{
	//		continue;
	//	}

	//	unsigned int* pindex = &(mesh.pIndices[3 * jj]);
	//	if (mesh.faceGroup[pindex[0]].size() < 2)
	//	{
	//		solid.faceAttributeList[jj].alive = 0;
	//		pindex[0] = pindex[1];
	//		non_manifolde++;
	//	}else
	//	if (mesh.faceGroup[pindex[1]].size() < 2)
	//	{
	//		solid.faceAttributeList[jj].alive = 0;
	//		pindex[1] = pindex[0];
	//		non_manifolde++;
	//	}
	//	else
	//	if (mesh.faceGroup[pindex[2]].size() < 2)
	//	{
	//		solid.faceAttributeList[jj].alive = 0;
	//		pindex[2] = pindex[1];
	//		non_manifolde++;
	//	}
	//}
	//printf("remove non manifold face[%d]\n", non_manifolde);
	return stat;
}

extern void save_mesh_obj(char* filename, MeshV& mesh);
int Simple_EdgeCollapseMain(Solid& solid, int type, double alp, int iter, double th, double pdf)
{
	std::vector<FaceAttribute> workattr = solid.faceAttributeList;
	std::sort(workattr.begin(), workattr.end());
	double minarea = workattr[0].area;
	for ( int i = 0; i < workattr.size(); i++ )
	{
		if ( minarea > 1.0e-16 ) break;
		minarea = workattr[i].area;
	}

	int n = 0;
	double avarea = 0.0;
	for ( int i = 0; i < workattr.size(); i++ )
	{
		avarea += workattr[i].area;
		if ( workattr[i].area < alp || alp < 0) n++;
	}
	avarea /= (double) workattr.size();

	printf("target %.3f/%.3f (%.3f)%%\n", (double)n, (double)workattr.size(), 100.0*(double)n/(double)workattr.size());
	printf("min area %.16f av %.16f\n", minarea, avarea);

	for (int i = 0; i < iter; i++)
	{
		int stat = Simple_EdgeCollapse(type, solid, alp, th, pdf);
		printf("[%d/%d] --%d-- END\n", i, iter - 1, stat);
		if (stat == 0)
		{
			break;
		}

		if (1){
			const MeshV* mesh = solid.getMesh();
			int num = 0;
			const int sz = mesh->iNumIndices;

			printf("%d Face ->", sz);
			for (int i = 0; i < sz; i++)
			{
				unsigned int* index = &(mesh->pIndices[3 * i]);

				if (index[0] == index[1] || index[0] == index[2] || index[1] == index[2])
				{
					continue;
				}
				num++;
			}
			printf("%d Face  remove %.2f%% [%d/%d]\n", num, ((double)(sz - num) / (double)sz)*100.0, num, sz);
		}
	}
	return 0;
}

int Simple_EdgeCollapse_p(int type, void* solid, double minarea, double costh, double pdf)
{
	Solid* p = (Solid*)solid;
	Solid& s = *p;

	return Simple_EdgeCollapse(type, s, minarea, costh, pdf);
}

int Simple_EdgeCollapseMain_p(void* solid, int type, double alp, int iter, double th, double pdf)
{
	Solid* p = (Solid*)solid;
	Solid& s = *p;

	return Simple_EdgeCollapseMain(s, type, alp, iter, th, pdf);
}

double getFaceAreaMin(void* solid)
{
	Solid* p = (Solid*)solid;
	std::vector<FaceAttribute> workattr = p->faceAttributeList;
	std::sort(workattr.begin(), workattr.end());
	double minarea = workattr[0].area;
	for ( int i = 0; i < workattr.size(); i++ )
	{
		if ( minarea > 1.0e-16 ) break;
		minarea = workattr[i].area;
	}
	return minarea;
}

double getFaceAreaMax(void* solid)
{
	Solid* p = (Solid*)solid;
	std::vector<FaceAttribute> workattr = p->faceAttributeList;
	std::sort(workattr.begin(), workattr.end());
	return workattr[workattr.size()-1].area;
}