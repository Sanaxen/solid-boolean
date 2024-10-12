#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "csg_solid_modeller.h"
#include <omp.h>
#include "stopwatch.h"

void test_bunny(int n);
void test9();
void test10();
void test11();
void testdebug(int);
void test00(int flg);


inline int is_prime(int n)
{
	int i;

	if(!(n & 1)) return 0;
	for(i = 3; i * i <= n; i += 2){
	if(!(n % i)) return 0;
	}
	return 1;
}

inline int near_prime(int n)
{
	int p1 = n + !(n & 1), p2 = n - !(n & 1);

	if(n <= 2) return 2;
	while(!is_prime(p2)){
	if(is_prime(p1)) return p1;
	p1 += 2;
	p2 -= 2;
	}
	return p2;
}
int main( int argc, char** argv)
{
#if _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	//for ( int i = 10; i < 1024; i += 10 )
	//{
	//	printf("#define  NEAR_PRIM_NUM%03dx%03dx%03d (%d)\n", i, i, i, near_prime(i*i*i));
	//}
	//exit(0);

//printf("cpu core %d\n", getCPUCoreNum());
//#ifdef _OPENMP
omp_set_num_threads(26);
printf("OpenMP : Enabled (Max # of threads = %d\n", omp_get_max_threads());
//#endif
	void* p = uad_double_library_Init();
	
//omp_set_num_threads(32);
	//test9();
	//test10();
	

	char buf[32];

	//test_bunny(100);exit(0);

	//test_bunny(2);
	//printf("========. enter!!.\n");	
	//gets(buf);

	//testdebug(8);/*exit(0);*/
	//testdebug(9);/*exit(0);*/
	//testdebug(10);/*exit(0);*/
	//testdebug(11);/*exit(0);*/
	//testdebug(12);/*exit(0);*/
	//testdebug(13);/*exit(0);*/
	//testdebug(14);/*exit(0);*/
	//exit(0);
	//test00(0);
	//exit(0);

	//gets(buf);
	//test11();
	//printf("========. enter!!.\n");
	//gets(buf);
	//test00(0);
	//test00(1);
	testdebug(14);/*exit(0);*/
	testdebug(9);/*exit(0);*/
	testdebug(0);
	testdebug(1);
	testdebug(2);
	testdebug(3);
	testdebug(4);
	testdebug(5);
	testdebug(6);
	testdebug(7);
	testdebug(8);
	testdebug(10);/*exit(0);*/
	testdebug(11);/*exit(0);*/
	testdebug(12);/*exit(0);*/
	testdebug(13);/*exit(0);*/

	uad_double_library_Term(p);

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

void test_bunny(int n)
{
	printf("============== test_bunny start =================\n");
	clock_t ts = clock();

	csgSolid* solid1;

	solid1 = LoadSolid( "bunny.obj" );
	//ExportSolidSTL("bunny_solid1.stl", solid1);

	int nn = n;
	float dirP[2][3];
	float ttP[2][3];

	ttP[0][0] = 0;
	ttP[0][1] = 0;
	ttP[0][2] = 70;

	ttP[1][0] = 1;
	ttP[1][1] = 0;
	ttP[1][2] = 70;

	dirP[0][0] = 1;
	dirP[0][1] = 1;
	dirP[0][2] = 1;

	dirP[1][0] = 1;
	dirP[1][1] = 1;
	dirP[1][2] = -0.5;

	csgSolid* sweept = SolidLinerSweepEx(solid1, nn, ttP[0], dirP[0], ttP[1], dirP[1], 1, 1 );
	SolidColor(sweept, 205, 200, 10);

	SimplifySolid(sweept);
	char fname[256];
	sprintf(fname, "bunny_sweep_%d.obj", n);
	SaveSolid(sweept, fname);

	DeleteSolid(solid1);
	DeleteSolid(sweept);

	printf("time %d ms\n", clock()-ts);
	printf("============== test_bunny end =================\n\n");
}



void test9()
{

	csgBooleanModeller* Modeller2;
	csgSolid* solid1;
	csgSolid* solid2;
	csgSolid* solid3;
	
	float minA[3]={0, 0, 0};
	float maxA[3]={100, 100, 100};

	solid1 = SolidCube2( minA, maxA, 5 );

	float dir[]={1,1,1};
	float tt[3];
	for ( int ii = 0; ii < 20; ii++ )
	{
		printf("[%d]\n", ii);
		solid2 = SolidCylinder(15, 90, 25, 15);
		SolidRotate(solid2, dir);
		tt[0] = ii;
		tt[1] = 0;
		tt[2] = 70;
		SolidTranslate(solid2, tt);

		char fname[256];
		//sprintf(fname, "test9cy_%02d.stl", ii);
		//ExportSolidSTL(fname, solid2);
		
		solid3 = SolidSphere(15, 25, 15);
		SolidTranslate(solid3, tt);
		//sprintf(fname, "test9sh_%02d.stl", ii);
		//ExportSolidSTL(fname, solid3);
		int mstat = 0;
		Modeller2 = CreateSolidModeller(solid1, solid2, &mstat);
		if ( Modeller2 == NULL)
		{
			printf("%d error skipp.\n", ii);
			DeleteSolid(solid2);
			continue;
		}

		csgSolid* solid4 = CreateSolidModeller_Difference(Modeller2);
		DeleteSolidModeller(Modeller2);
		DeleteSolid(solid1);
		DeleteSolid(solid2);

		solid1 = solid4;

		Modeller2 = CreateSolidModeller(solid1, solid3, &mstat);
		if ( Modeller2 == NULL)
		{
			printf("%d error skipp.\n", ii);
			DeleteSolid(solid3);
			continue;
		}
		solid4 = CreateSolidModeller_Difference(Modeller2);

		DeleteSolidModeller(Modeller2);
		DeleteSolid(solid1);
		DeleteSolid(solid3);
		solid1 = solid4;
	}


	ExportSolidSTL("test9.stl", solid1);
	DeleteSolid(solid1);

}

void test10()
{

	csgBooleanModeller* Modeller2;
	csgSolid* solid1;
	csgSolid* solid2;
	
	float minA[3]={0, 0, 0};
	float maxA[3]={100, 100, 100};

	solid1 = SolidCube2( minA, maxA, 5 );

	float dir[]={1,1,1};
	float tt[3];

	int nn = 20;
	float* dirP = new float[3*nn];
	float* ttP = new float[3*nn];

	for ( int ii = 0; ii < nn; ii++ )
	{
		tt[0] = ii;
		tt[1] = 0;
		tt[2] = 70;

		ttP[3*ii+0] = ii;
		ttP[3*ii+1] = 0;
		ttP[3*ii+2] = 70;

		dirP[3*ii+0] = 1;
		dirP[3*ii+1] = 1;
		dirP[3*ii+2] = 1;
	}

	solid2 = SolidCylinder(15, 90, 25, 15);
	DeleteSolid(solid2);

	csgSolid* sweept = SolidSweep(solid2, nn, ttP, dirP);
	DeleteSolid(solid2);
	delete [] ttP;
	delete [] dirP;

	ExportSolidSTL("test10sweep.stl", sweept);

	int mstat = 0;
	Modeller2 = CreateSolidModeller(solid1, sweept, &mstat);
	if ( Modeller2 == NULL)
	{
		printf("error.\n");
		DeleteSolid(solid1);
		DeleteSolid(sweept);
		return;
	}
	csgSolid* solid3 = CreateSolidModeller_Difference(Modeller2);

	DeleteSolidModeller(Modeller2);
	DeleteSolid(solid1);
	DeleteSolid(sweept);

	ExportSolidSTL("test10.stl", solid3);
	DeleteSolid(solid3);
}

void test11()
{
	printf("============== test11 start =================\n");
	clock_t ts = clock();
#if 10
	csgBooleanModeller* Modeller2;
	csgSolid* solid1;
	csgSolid* solid2;
	
	float minA[3]={0, 0, 0};
	float maxA[3]={100, 100, 100};

	solid1 = SolidCube2( minA, maxA, 5 );
	ExportSolidSTL("test11solid1.stl", solid1);
	ExportSolidPov("test11solid1.pov", solid1);
	
	{
		int num;
		float* vertex;
		float* normal;

		SolidTriangleList(solid1 , &num, &vertex, &normal);

		FILE* fp = fopen("SolidTriangleList.stl", "w");
		fprintf(fp, "sold ascii\n");

		for ( int i = 0; i < num; i++ )
		{
			fprintf(fp, "facet normal %e %e %e\n", normal[3*i],normal[3*i+1],normal[3*i+2]);
			fprintf(fp, "   outer loop\n");
			fprintf(fp, "       vertex %f %f %f\n", vertex[9*i+0],vertex[9*i+1],vertex[9*i+2]);
			fprintf(fp, "       vertex %f %f %f\n", vertex[9*i+3],vertex[9*i+4],vertex[9*i+5]);
			fprintf(fp, "       vertex %f %f %f\n", vertex[9*i+6],vertex[9*i+7],vertex[9*i+8]);
			fprintf(fp, "   endloop\n");
			fprintf(fp, "endfacet\n");
		}
		fprintf(fp, "endsolid\n");
		fclose(fp);
		DeleteSolidTriangleList(vertex, normal);
	}

	int nn = 20;
	float dirP[2][3];
	float ttP[2][3];

	ttP[0][0] = 0;
	ttP[0][1] = 0;
	ttP[0][2] = 70;

	ttP[1][0] = nn;
	ttP[1][1] = 0;
	ttP[1][2] = 70;

	dirP[0][0] = 1;
	dirP[0][1] = 1;
	dirP[0][2] = 1;

	dirP[1][0] = 1;
	dirP[1][1] = 1;
	dirP[1][2] = 1;

	solid2 = SolidCylinder(15, 90, 25, 15);
	csgSolid* sweept = SolidLinerSweep(solid2, nn, ttP[0], dirP[0], ttP[1], dirP[1] );
	DeleteSolid(solid2);
	SolidColor(sweept, 255, 0, 0);

	ExportSolidSTL("test11sweep.stl", sweept);
	SaveSolid(sweept, "test11sweep.obj");
	SimplifySolid(sweept);
	SaveSolid(sweept, "test11sweep_b.obj");

	int mstat = 0;
	Modeller2 = CreateSolidModeller(solid1, sweept, &mstat);
	if ( Modeller2 == NULL)
	{
		printf("error.\n");
		DeleteSolid(solid1);
		DeleteSolid(sweept);
		return;
	}
	csgSolid* solid3 = CreateSolidModeller_Difference(Modeller2);

	DeleteSolidModeller(Modeller2);
	DeleteSolid(solid1);
	DeleteSolid(sweept);
	ExportSolidSTL("test11.stl", solid3);


	solid1 = solid3;

	solid2 = SolidSphere(15, 25, 15);
	sweept = SolidLinerSweep(solid2, nn, ttP[0], dirP[0], ttP[1], dirP[1] );
	DeleteSolid(solid2);
	ExportSolidSTL("test11sweep_2.stl", sweept);
	SolidColor(sweept, 0, 255, 0);
	SaveSolid(sweept, "test11sweep_2.obj");

	SimplifySolid(sweept);
	SaveSolid(sweept, "test11sweep_2b.obj");

	Modeller2 = CreateSolidModeller(solid1, sweept, &mstat);
	if ( Modeller2 == NULL)
	{
		printf("error.\n");
		DeleteSolid(solid1);
		DeleteSolid(sweept);
		return;
	}
	solid3 = CreateSolidModeller_Difference(Modeller2);

	DeleteSolidModeller(Modeller2);
	DeleteSolid(solid1);
	DeleteSolid(sweept);


	ExportSolidSTL("test11_2.stl", solid3);
	SaveSolid(solid3, "test11_2.obj");

	//SimplifySolid(solid3);
	SaveSolid(solid3, "test11_2b.obj");
	DeleteSolid(solid3);

#endif
	csgSolid* solid4 = LoadSolid("test11_2.obj");
	SimplifySolid(solid4);
	SaveSolid(solid4, "test11_3.obj");
	DeleteSolid(solid4);

	printf("time %d ms\n", clock()-ts);
	printf("============== test11 end =================\n\n");
}


void testdebug(int no)
{

	printf("============== testdebug(%d) start =================\n", no);
	StopWach stopwatch("testdebug");
	csgBooleanModeller* Modeller2;
	csgSolid* solid1;
	csgSolid* solid2;
	csgSolid* solid3;
	csgSolid* solid4;
	csgSolid* solid5;
	
	char filename[256];
	switch(no)
	{
	case 0:
		solid1 = LoadSolid( "testcase\\001\\debug_solid_stock_0050.obj" );
		solid2 = LoadSolid( "testcase\\001\\debug_solid_tool_0000.obj" );
		solid3 = LoadSolid( "testcase\\001\\debug_solid_tool_0001.obj" );
		SolidColor(solid2, 255, 0, 0);
		SolidColor(solid3, 0, 255, 0);
		break;
	case 1:
		solid1 = LoadSolid( "testcase\\002\\solid_stock_0619(8).obj" );
		solid2 = LoadSolid( "testcase\\002\\debug_solid_tool_0000_619(0008).obj" );
		solid3 = LoadSolid( "testcase\\002\\debug_solid_tool_0001_619(0008).obj" );
		SolidColor(solid2, 255, 0, 0);
		SolidColor(solid3, 0, 255, 0);
		break;
	case 2:
		solid1 = LoadSolid( "testcase\\003\\solid_stock_0101.obj" );
		solid2 = LoadSolid( "testcase\\003\\debug_solid_tool_0000_101(0000).obj" );
		solid3 = LoadSolid( "testcase\\003\\debug_solid_tool_0001_101(0000).obj" );
		SolidColor(solid2, 255, 0, 0);
		SolidColor(solid3, 0, 255, 0);
		break;
	case 3:
		solid1 = LoadSolid( "testcase\\004\\solid_stock_0197(17).obj" );
		solid2 = LoadSolid( "testcase\\004\\debug_solid_tool_0000_197(0017).obj" );
		solid3 = LoadSolid( "testcase\\004\\debug_solid_tool_0001_197(0017).obj" );
		SolidColor(solid2, 255, 0, 0);
		SolidColor(solid3, 0, 255, 0);
		break;
	case 4:
		solid1 = LoadSolid( "testcase\\005\\solid_stock_0160(1).obj" );
		solid2 = LoadSolid( "testcase\\005\\debug_solid_tool_0000_160(0001).obj" );
		SolidColor(solid2, 255, 0, 0);
		solid3 = NULL;
		break;
	case 5:
		solid1 = LoadSolid( "testcase\\006\\aaa2.obj" );
		solid2 = LoadSolid( "testcase\\006\\aaa1.obj" );
		SolidColor(solid2, 0, 255, 0);
		solid3 = NULL;
		break;
	case 6:
		solid1 = LoadSolid( "testcase\\007\\aaa2.obj" );
		solid2 = LoadSolid( "testcase\\007\\aaa1.obj" );
		SolidColor(solid2, 0, 255, 0);
		solid3 = NULL;
		break;
	case 7:
		solid1 = LoadSolid( "testcase\\008\\solid_stock_0445(1).obj" );
		solid2 = LoadSolid( "testcase\\008\\debug_solid_tool_0000_445(0001).obj" );
		//SolidColor(solid2, 0, 255, 0);
		solid3 = NULL;
		break;
	case 8:
		solid1 = LoadSolid( "testcase\\009\\debug_solid_stock_0240.obj" );
		solid2 = LoadSolid( "testcase\\009\\debug_solid_tool_0001_240(0000).obj" );
		solid3 = LoadSolid( "testcase\\009\\debug_solid_tool_0000_240(0000).obj" );
		SolidColor(solid2, 255, 0, 0);
		SolidColor(solid3, 0, 255, 0);
		//solid3 = NULL;
		break;
	case 9:
		solid1 = LoadSolid( "testcase\\010\\solid_stock_0455(43).obj" );
		solid2 = LoadSolid( "testcase\\010\\debug_solid_tool_0000_455(0043).obj" );
		SolidColor(solid2, 181, 130, 77);
		solid3 = NULL;
		//SolidColor(solid3, 3, 23, 92);
		break;
	case 10:
		solid1 = LoadSolid( "testcase\\011\\solid_stock_0094(60).obj" );
		solid2 = LoadSolid( "testcase\\011\\debug_solid_tool_0000_94(0060).obj" );
		solid3 = LoadSolid( "testcase\\011\\debug_solid_tool_0001_94(0060).obj" );
		SolidColor(solid2, 181, 130, 77);
		SolidColor(solid3, 3, 23, 92);
		break;
	case 11:
		solid1 = LoadSolid( "testcase\\012\\solid_stock_0043(1).obj" );
		solid2 = LoadSolid( "testcase\\012\\debug_solid_tool_0000_43(0001).obj" );
		solid3 = LoadSolid( "testcase\\012\\debug_solid_tool_0001_43(0001).obj" );
		SolidColor(solid2, 181, 130, 77);
		SolidColor(solid3, 3, 23, 92);
		break;
	case 12:
		solid1 = LoadSolid( "testcase\\013\\solid_stock_0042(20).obj" );
		solid2 = LoadSolid( "testcase\\013\\debug_solid_tool_0000_42(0020).obj" );
		solid3 = LoadSolid( "testcase\\013\\debug_solid_tool_0001_42(0020).obj" );
		SolidColor(solid2, 181, 130, 77);
		SolidColor(solid3, 3, 23, 92);
		break;
	case 13:
		solid1 = LoadSolid( "testcase\\014\\solid_stock_0042(34).obj" );
		solid2 = LoadSolid( "testcase\\014\\debug_solid_tool_0000_42(0034).obj" );
		//SolidColor(solid2, 0, 255, 0);
		solid3 = NULL;
		break;
	case 14:
		solid1 = LoadSolid( "testcase\\015\\solid_stock_0042(38).obj" );
		solid2 = LoadSolid( "testcase\\015\\debug_solid_tool_0001_42(0038).obj" );
		//SolidColor(solid2, 0, 255, 0);
		solid3 = NULL;
		break;
	default:
		return;
	}
	if ( solid3 != NULL ) SolidColor(solid3, 0, 255, 0);

	sprintf(filename, "debug(%d)_1.obj", no);
	SaveSolid(solid1, filename);
	sprintf(filename, "debug(%d)_2.obj", no);
	SaveSolid(solid2, filename);
	sprintf(filename, "debug(%d)_3.obj", no);
	if ( solid3 != NULL ) SaveSolid(solid3, filename);

	int mstat = 0;
	Modeller2 = CreateSolidModeller(solid1, solid2, &mstat);
	if ( Modeller2 == NULL)
	{
		printf("error.\n");
		return;
	}
	//{
	//	csgSolid* arg1 = CreateSolidModeller_get1(Modeller2);
	//	sprintf(filename, "debug(%d)_arg1.obj", no);
	//	SaveSolid(arg1, filename);
	//	DeleteSolid(arg1);

	//	csgSolid* arg2 = CreateSolidModeller_get2(Modeller2);
	//	sprintf(filename, "debug(%d)_arg2.obj", no);
	//	SaveSolid(arg2, filename);
	//	DeleteSolid(arg2);
	//}

	solid4 = CreateSolidModeller_Difference(Modeller2);
	sprintf(filename, "debug(%d)_4.obj", no);
	SaveSolid(solid4, filename);
	DeleteSolidModeller(Modeller2);


	if ( solid3 != NULL )
	{
		Modeller2 = CreateSolidModeller(solid4, solid3, &mstat);
		if ( Modeller2 == NULL)
		{
			printf("error.\n");
			return;
		}
		solid5 = CreateSolidModeller_Difference(Modeller2);
		SimplifySolid(solid5);
		sprintf(filename, "debug(%d)_5.obj", no);
		SaveSolid(solid5, filename);
		DeleteSolidModeller(Modeller2);
	}
	DeleteSolid(solid1);
	DeleteSolid(solid2);

	if ( solid3 != NULL )	DeleteSolid(solid3);
	DeleteSolid(solid4);
	if ( solid3 != NULL )	DeleteSolid(solid5);

	stopwatch.stop();
	printf("============== testdebug(%d) end =================\n", no);
}

#define MAXCUBENUM 1000
void test00(int flg)
{
	printf("============== test00(%d) start =================\n", flg);

	StopWach stopwatch("test00");
	csgBooleanModeller* Modeller;
	csgSolid** solid =new csgSolid*[MAXCUBENUM];
	
	float org[3];
	float w;
	float axis[3];
	for ( int i = 0; i < MAXCUBENUM; i++ )
	{
		org[0] = 100.0f*rand()/(float)RAND_MAX;
		org[1] = 100.0f*rand()/(float)RAND_MAX;
		org[2] = 100.0f*rand()/(float)RAND_MAX;
		w = 100.0f*rand()/(float)RAND_MAX;
		if ( flg )
		{
			axis[0] = rand()/(float)RAND_MAX;
			axis[1] = rand()/(float)RAND_MAX;
			axis[2] = rand()/(float)RAND_MAX;
		}
		solid[i] = SolidCube1(org, w);
		if ( flg ) SolidRotate(solid[i], axis);
	}

	int mstat = 0;
	Modeller = CreateSolidModeller(solid[0], solid[1], &mstat);
	if ( Modeller == NULL)
	{
		printf("error.\n");
		return;
	}
	csgSolid* solid2 = CreateSolidModeller_Union(Modeller);
	DeleteSolidModeller(Modeller);

	csgSolid* solid3;
	for ( int i = 2; i < MAXCUBENUM; i++ )
	{
		Modeller = CreateSolidModeller(solid2, solid[i], &mstat);
		if ( Modeller == NULL)
		{
			printf("error.\n");
			return;
		}
		solid3 = CreateSolidModeller_Union(Modeller);
		DeleteSolidModeller(Modeller);
		DeleteSolid(solid2);
		solid2 = solid3;
	}

	for ( int i = 0; i < MAXCUBENUM; i++ )
	{
		DeleteSolid(solid[i]);
	}
	SimplifySolid(solid3);
	if ( flg ) 
	{
		SaveSolid(solid3, "rand_cube100(1).obj");
		ExportSolidPov("rand_cube100(1).pov", solid3);
	}
	else
	{
		SaveSolid(solid3, "rand_cube100(0).obj");
		ExportSolidPov("rand_cube100(0).pov", solid3);
	}
	DeleteSolid(solid3);

	stopwatch.stop();
	printf("============== test00(%d) end =================\n\n", flg);
}

