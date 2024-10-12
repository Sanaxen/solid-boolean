#include "mesh_symplify.hpp"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <Windows.h>
#include <time.h>

int main( int argc, char** argv)
{
	//getCPUNum();
	//DWORD cpuinfo;
	//GetThreadInformation( &cpuinfo );
	//exit(0);

    char drive[_MAX_DRIVE];	// �h���C�u��
    char dir[_MAX_DIR];		// �f�B���N�g����
    char fname[_MAX_FNAME];	// �t�@�C����
    char ext[_MAX_EXT];		// �g���q

	int quality_dump = 0;
	int runtype = 1;
	int useqem = 1;
	double a = -1.0;
	int iter = 2;
	double th = cos(6.0*M_PI/180.0);
	int th_op = 0;
	double dt = 0.65;
	int edit_flag = 0;
	char filename[256];
	char work[256];

	double probability = 1.0;
	if ( argc < 2 )
	{
		printf("-simple          :�ȈՉ�\n");
		printf("-smooth          :������\n");
		printf("-qem             :QEM�@���g��\n");
		printf("--qem            :QEM�@�͎g��Ȃ�\n");
		printf("-a �ʐϒl        :�����Ώۂ̍ő�ʐ� (environmental variables[MESHSIMPLIFY_OPT_A]\n");
		printf("-th �p�x(�x)     :�@�����m�̌X�������iFace�܂���������Ȃ�)(environmental variables[MESHSIMPLIFY_OPT_TH]\n");
		printf("-iter �J��Ԃ��� :�������w��񐔌J��Ԃ�(environmental variables[MESHSIMPLIFY_OPT_ITER]\n");
		printf("-dt 0.0�`1.0�@�@ :�������x����(0.0�`1.0(�ő�))\n");
		printf("-qdump           :quality dump�@�̂�\n");
		printf("--qdump           :quality dump�@����\n");
		printf("-flag            :�ҏW�ӏ��t���O���c��\n");
		printf("-prob            :(0.0�`1.0) target face => FaceNum*prog%%\n");
		printf("�t�@�C����\n");
		return 0;
	}

	char* env = NULL;
	if ( (env = getenv("MESHSIMPLIFY_OPT_A")))
	{
		a = atof(env);
		printf("MESHSIMPLIFY_OPT_A=%f\n", a);
	}
	if ( (env = getenv("MESHSIMPLIFY_OPT_TH")))
	{
		th = atof(env);
		printf("MESHSIMPLIFY_OPT_TH=%f\n", th);
		th_op = 1;
	}
	if ( (env = getenv("MESHSIMPLIFY_OPT_ITER")))
	{
		iter = atoi(env);
		printf("MESHSIMPLIFY_OPT_ITER=%d\n", iter);
	}

	filename[0] = '\0';
	for ( int i = 1; i < argc; i++ )
	{
		if ( std::string(argv[i]) == "-simple" )
		{
			runtype = 1;
			continue;
		}
		if ( std::string(argv[i]) == "-smooth" )
		{
			runtype = 2;
			continue;
		}
		if ( std::string(argv[i]) == "--qem" )
		{
			useqem = 0;
			continue;
		}
		if ( std::string(argv[i]) == "-qem" )
		{
			useqem = 1;
			printf("QEM\n");
			continue;
		}
		if (std::string(argv[i]) == "-prob")
		{
			probability = atof(argv[i + 1]);
			printf("prob=%.8f\n", probability);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-a" )
		{
			a = atof(argv[i+1]);
			printf("a=%f\n", a);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-iter" )
		{
			iter = atoi(argv[i+1]);
			printf("iter=%d\n", iter);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-th" )
		{
			th = atof(argv[i+1]);
			printf("th=%f\n", th);
			th = cos(th*M_PI/180.0);
			i++;
			th_op = 1;
			continue;
		}
		if ( std::string(argv[i]) == "-dt" )
		{
			dt = atof(argv[i+1]);
			printf("dt=%f\n", dt);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-qdump" )
		{
			quality_dump = 1;
			continue;
		}
		if ( std::string(argv[i]) == "--qdump" )
		{
			quality_dump = -1;
			continue;
		}
		if ( std::string(argv[i]) == "-flag" )
		{
			edit_flag = 1;
			continue;
		}
		if ( argv[i][0] != '-' )
		{
			strcpy(filename, argv[i]);
		}
	}
    _splitpath(filename, drive, dir, fname, ext );
	std::cout << "Drive=" << drive << std::endl;
    std::cout << "Dir  =" << dir   << std::endl;
    std::cout << "Fname=" << fname << std::endl;
    std::cout << "Ext  =" << ext   << std::endl;

	if ( filename[0] == '\0' )
	{
		return 0;
	}

	printf("loading ... [%s]\n", filename);
	clock_t start = clock();
	MeshV mesh;
	create_mesh_obj(filename, mesh);

	Solid solid(&mesh);
	//solid.ColorVertexMerge();
	solid.CreateFaceAttri();

	sprintf(work, "%s%s%s_in%s", drive, dir, fname,ext);
	printf("[%s]\n", work);
	printf("input data copy ... [%s]\n", work);
	save_mesh_obj(work, mesh);
	
	if ( runtype == 1 && quality_dump >= 0)
	{
		ColorPalette color;
		int* faceColor = new int[mesh.iNumIndices];
		for ( int i = 0; i < mesh.iNumIndices; i++ )
		{
			int rgb[3];
			color.getColor(255, (int)(solid.faceAttributeList[i].quality*255.0), rgb);
			faceColor[i] = Color_RGBToInt( rgb );

			//rgb[0] = rgb[1] = rgb[2] = 222;
			//if ( solid.faceAttributeList[i].quality > 0.35 ) faceColor[i]= Color_RGBToInt( rgb );
		}
		sprintf(work, "%s%s%s_in_quality%s", drive, dir, fname,ext);
		dump_mesh_obj_faceColor(work, mesh, faceColor);
		delete [] faceColor;
		if ( quality_dump == 1 ) return 0;
	}

	//getc(stdin);

	int stat = -1;
	if ( runtype == 1 )
	{
		if (probability < 0.0 )
		{ 
			if (solid.getMesh()->iNumIndices > 100000.0)
			{
				probability = 100000.0 / (double)solid.getMesh()->iNumIndices;
				printf("auto probability=%.8f\n", probability);
			}
			else
			{
				probability = 1.0;
			}
		}
		stat = Simple_EdgeCollapseMain(solid, useqem, a, iter, th, probability);
	}else
	{
		if ( th_op == 0 )
		{
			th = 170.0;
			printf("th=%f\n", th);
			th = cos(th*M_PI/180.0);
		}

		if (probability < 0.0)
		{
			if (solid.getMesh()->iNumIndices > 100000.0)
			{
				probability = 100000.0 / (double)solid.getMesh()->iNumIndices;
				printf("auto probability=%.8f\n", probability);
			}
			else
			{
				probability = 1.0;
			}
		}
		stat = Simple_LaplacianSmoothing(solid,dt, a, iter, th, probability);
	}

	printf("STATUS %d\n", stat);
	if ( stat >= 0 )
	{
		solid.VertexSplit();
		remove_unreferenced_vertex(mesh);
		if ( runtype == 1 )
		{
			sprintf(work, "%s%s%s_simple_out%s", drive, dir, fname,ext);
		}else
		{
			sprintf(work, "%s%s%s_smooth_out%s", drive, dir, fname,ext);
		}
		printf("[%s]\n", work);
		if ( mesh.vertexAttr )
		{
			//for ( int i = 0; i < mesh.iNumVertices; i++ )
			//{
			//	mesh.pColors[i].attr = 0;
			//}
			if (! edit_flag ) mesh.vertexAttr = false;
		}
		save_mesh_obj(work, mesh);
	}

#if USE_TriangleQuality
	if ( runtype == 1 && quality_dump == 0)
	{
		ColorPalette color;
		int* faceColor = new int[mesh.iNumIndices];
		for ( int i = 0; i < mesh.iNumIndices; i++ )
		{
			int rgb[3];
			color.getColor(255, (int)(solid.faceAttributeList[i].quality*255.0), rgb);
			faceColor[i] = Color_RGBToInt( rgb );
		}
		sprintf(work, "%s%s%s_out_quality%s", drive, dir, fname,ext);
		dump_mesh_obj_faceColor(work, mesh, faceColor);
		delete [] faceColor;
	}
#endif
	printf("--------------\n");
	printf("time: %.3fsec\n", (double)(clock() - start) / (double)CLOCKS_PER_SEC);
	return 0;
}