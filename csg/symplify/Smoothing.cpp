#include "Smoothing.hpp"
#include <random>

int Smoothing(Solid& solid, HEdge* he, std::vector<int>& vertexFlag, double costh, double dt, double probability);

int LaplacianSmoothing(Solid& solid, double dt, double minarea, double costh, double probability)
{
	MeshV& mesh = *solid.getMesh();
	const int triNum = mesh.iNumIndices;

	char fname[256];
	int stat = 0;

	std::random_device rnd;     // �񌈒�I�ȗ���������𐶐�
	std::mt19937 mt(rnd());     //  �����Z���k�E�c�C�X�^��32�r�b�g�ŁA�����͏����V�[�h�l
	std::uniform_real_distribution<double> rand1(0.0, 1.0);

	std::vector<int> vertexFlag;
	vertexFlag.resize( solid.getMesh()->iNumVertices, 0);

	std::vector<N_Cylinder> boundarysList;
	for ( int ii = 0; ii < triNum; ii++ )
	{
		if ((double)rand1(mt) < (1.0 - probability))
		{
			//fprintf(stderr, "-------------------------\n");
			continue;
		}

		//�����ł���Face�̓X�L�b�v
		if ( solid.faceAttributeList[ii].skipp )
		{
			continue;
		}
		if (minarea > 0 && solid.faceAttributeList[ii].area > minarea)
		{
			continue;
		}

		//Face(ii)�ɗאڂ���Face�Ԃ̈ʑ��\�����\�z���Ĉ�̃p�[�c�Ƃ���
		solid.Initial();
		Face* f = solid.SolidPart(ii);
		if ( f == NULL  )
		{
			continue;
		}

		//sprintf(fname, "c:\\temp\\part%d.obj", ii);
		//solid.dump(fname);


		//�p�[�c�̋��E�𒊏o����
		std::vector<std::set<HEdge*> > boundarys;
		if ( solid.boundaryLoop(boundarys) != 0)
		{
			continue;
		}

		//���E���o���Ȃ��ꍇ�͂��̎��ӂ̈ʑ����s���Ȃ̂ŃX�L�b�v����
		if ( boundarys.size() != 1 )
		{
			continue;
		}

		//���E���\�����钸�_�̋��E�}�[�N��t����
		const int faceNum = solid.faceList.size();
		const int boundary_num = boundarys.size();
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
		//boundarysList.push_back(boundary);

		HEdge* he = f->loop->hedge;
		int ist = Smoothing(solid, he, vertexFlag, costh, dt, probability);
		if ( ist == 0 )
		{
			stat++;
			//vertexFlag[vc[kk]->id_] = 1;
			if (ii % 1000 == 0 && ii > triNum-1000)
			{
				printf("                            \r\r\r[%d/%d]", ii + 1, triNum);
			}
			else
			{
				printf("                            \r\r\r[%d/%d]", ii + 1, triNum);
			}
			//�ό`�����̂Ŗʐς��Čv�Z���Ă���
			//for ( int jj = 0; jj < faceNum; jj++ )
			//{
			//	solid.faceAttributeList[solid.faceList[jj]->id].CalcArea(&mesh);
			//}
		}

		//�p�[�c���폜
		solid.Clear();
	}
	return stat;
}

int Simple_LaplacianSmoothing(Solid& solid, double dt, double alp, int iter, double th, double probability)
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
		if ( workattr[i].area < alp || alp < 0.0) n++;
	}
	avarea /= (double) workattr.size();

	printf("target %.3f/%.3f (%.3f)%%\n", (double)n, (double)workattr.size(), 100.0*(double)n/(double)workattr.size());
	printf("min area %.16f av %.16f\n", minarea, avarea);


	for ( int i = 0; i < iter; i++ )
	{
		solid.getMesh()->laplacianSmoothing_flag_clear();

		int stat = LaplacianSmoothing( solid, dt, alp, th, probability);
		printf("[%d/%d] --%d-- END\n", i, iter-1, stat);
		if ( stat == 0 )
		{
			break;
		}
	}
	return 0;
}


int LaplacianSmoothing_p(void* solid, double dt, double minarea, double costh, double probability)
{
	Solid* p = (Solid*)solid;
	Solid& s = *p;

	return LaplacianSmoothing(s, dt, minarea, costh, probability);
}


int Simple_LaplacianSmoothing_p(void* solid, double dt, double alp, int iter, double th, double probability)
{
	Solid* p = (Solid*)solid;
	Solid& s = *p;

	return Simple_LaplacianSmoothing(s, dt, alp, iter, th, probability);
}


int Smoothing(Solid& solid, HEdge* he, std::vector<int>& vertexFlag, double costh, double dt, double probability)
{
	const int faceNum = solid.faceList.size();
	
	Vertex* vc[3];

	vc[0] = he->StartPos();
	vc[1] = he->next->StartPos();
	vc[2] = he->next->next->StartPos();
		
	for ( int kk = 0; kk < 3; kk++ )
	{
		if ( solid.getMesh()->laplacianSmoothing_flag[vc[kk]->id_] )
		{
			continue;
		}
		if ( vc[kk]->boundary_ )
		{
			continue;
		}
		if ( vertexFlag[vc[kk]->id_] )
		{
			continue;
		}

		bool skipp = false;
		std::set<Vertex*> vlist;
		//���_�����͂ޒ��_�𒊏o����
		std::set<HEdge*>::iterator it = vc[kk]->outhedge.begin();
		for ( ; it != vc[kk]->outhedge.end(); ++it)
		{
			if ( (*it)->edge_->HalfEdgeRefCount() != 2 )
			{
				skipp = true;
				break;
			}
			if ( (*it)->EndPos() != vc[kk] )
			{
				vlist.insert((*it)->EndPos());
			}
		}
		if ( skipp )
		{
			continue;
		}


		double Laplacian[3];
		Laplacian[0] = 0.0;
		Laplacian[1] = 0.0;
		Laplacian[2] = 0.0;
			
		double* p = vc[kk]->coord();
		double pp[3];

		pp[0] = p[0];
		pp[1] = p[1];
		pp[2] = p[2];

		std::set<Vertex*>::iterator jt = vlist.begin();
		for ( ; jt != vlist.end(); ++jt )
		{
			double* q = (*jt)->coord();
			VEC3_ADD(Laplacian, Laplacian, q);
		}
		double w = 1.0/(double)vlist.size();

		Laplacian[0] *= w;
		Laplacian[1] *= w;
		Laplacian[2] *= w;

		p[0] = p[0] + dt*(Laplacian[0]-p[0]);
		p[1] = p[1] + dt*(Laplacian[1]-p[1]);
		p[2] = p[2] + dt*(Laplacian[2]-p[2]);

		//�k�މ��Ŏ��͂�Face���Ђ�����Ԃ��Ă��Ȃ����`�F�b�N����
		bool check = true;
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for ( int jj = 0; jj < faceNum; jj++ )
		{
			if (!check) continue;
			double *n1 = solid.faceList[jj]->Normal();
			double n2[3];
			solid.faceList[jj]->CalcNormal(n2);

			if ( DOTPRODUCT(n2) < 1.0e-16 )
			{
				continue;
			}
			if ( DOTPRODUCT2(n2, n1) < costh )
			{
				//printf("N %.16f %.16f %.16f\nN %.16f %.16f %.16f\n", n2[0], n2[1], n2[2], n1[0], n1[1], n1[2] );
				check = false;
			}
		}

#if 0
		//�d�Ȃ��Ă��܂��Ă��Ȃ����`�F�b�N����
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
						printf("��v\n");
						check = false;
						break;
					}
				}
				if ( !check ) break;
			}
		}
#endif

		//���ʂ��s���ɂȂ�Ȃ�G�b�W�̗��[�����̈ʒu�ɖ߂�
		if ( !check )
		{
			p[0] = pp[0];
			p[1] = pp[1];
			p[2] = pp[2];
			continue;
		}
		solid.getMesh()->laplacianSmoothing_flag[vc[kk]->id_] = 1;
		return 0;
	}
	return 1;
}