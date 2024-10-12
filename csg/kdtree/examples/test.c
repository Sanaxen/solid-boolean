#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

/*! gcc -Wall -g -o test test.c libkdtree.a */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
//#include <sys/time.h>
#include <time.h>
#include "kdtree.h"

//unsigned int get_msec(void)
//{
//	static struct timeval timeval, first_timeval;
//
//	gettimeofday(&timeval, 0);
//
//	if(first_timeval.tv_sec == 0) {
//		first_timeval = timeval;
//		return 0;
//	}
//	return (timeval.tv_sec - first_timeval.tv_sec) * 1000 + (timeval.tv_usec - first_timeval.tv_usec) / 1000;
//}

double* pppp;
double getPnt(int id, int i)
{
	return pppp[3*id+i];
}

int main(int argc, char **argv)
{
	int i, vcount = 1000;
	void *kd, *set;
	unsigned int msec, start;
	double* pnts;
	double pt[3]={0,0,0};

#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if(argc > 1 && isdigit(argv[1][0])) {
		vcount = atoi(argv[1]);
	}
	printf("inserting %d random vectors... ", vcount);
	fflush(stdout);

	pnts = (double*)malloc(3*vcount*sizeof(double));
	pppp = pnts;
	kdGetPoint = getPnt;

	kd = kd_create(3);
#if _DEBUG
  {
		char* dmy = malloc(128);
	}
#endif
	start = clock();
	for(i=0; i<vcount; i++) {
		float x, y, z;
		x = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
		y = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
		z = ((float)rand() / RAND_MAX) * 200.0 - 100.0;
		pnts[3*i] =x;
		pnts[3*i+1] =y;
		pnts[3*i+2] =z;

		kd_insert(kd, i, 0);
	}
	msec = clock() - start;
	printf("%.3f sec\n", (float)msec / 1000.0);

	start = clock();
	set = kd_nearest_range(kd, pt, 40);
	msec = clock() - start;
	printf("range query returned %d items in %.5f sec\n", kd_res_size(set), (float)msec / 1000.0);
	kd_res_free(set);

	kd_free(kd);
	
	//_CrtDumpMemoryLeaks();
	return 0;
}
