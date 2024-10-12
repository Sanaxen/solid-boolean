#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

/*! gcc -std=c89 -pedantic -Wall -g -o test2 test2.c libkdtree.a -lm */
/* Extended test program, contributed by David Underhill */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "kdtree.hpp"

#define DEF_NUM_PTS 10000000

/* returns the distance squared between two dims-dimensional double arrays */
static double dist_sq( double *a1, double *a2, int dims );

/* get a random double between -10 and 10 */
static double rd( void );

class Tmp
{
public:
	float x;
	float y;
	float z;
	int index;
};

Tmp* pppp;
double getPnt(int id, int i)
{
	if (i == 0) return  pppp[id].x;
	if (i == 1) return  pppp[id].y;
	if (i == 2) return  pppp[id].z;
}

int main(int argc, char **argv) {
  int i, num_pts = DEF_NUM_PTS;
  char *data, *pch;
  struct kdres *presults;
  double pos[3], dist;
  double pt[3] = { 0, 0, 1 };
  double radius = 0.1;

#if _DEBUG
 _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
 //_CrtSetBreakAlloc( 2000157 );
 //_CrtSetBreakAlloc( 2000160 );
#endif

 if(argc > 1 && isdigit(argv[1][0])) {
    num_pts = atoi(argv[1]);
  }


  srand( time(0) );

  /* create a k-d tree for 3-dimensional points */
  KdTree<Tmp>* ptree = new KdTree<Tmp>;

  kdGetPoint = getPnt;


  printf("make points\n");
  Tmp* pp = new Tmp[DEF_NUM_PTS];
  pppp = pp;

  /* add some random nodes to the tree (assert nodes are successfully inserted) */
  for( i=0; i<num_pts; i++ ) {
	  float x,y,z;
 	 x = rd();
	 y = rd();
	 z = rd();
	 pp[i].x = x;
	 pp[i].y = y;
	 pp[i].z = z;
	 pp[i].index = i;
	//printf("(%d) %f %f %f\n", i, x, y, z);
	 ptree->insert(i, pp+i);
  }
  printf("make points end.\n");

  int ts = clock();
  std::vector<int> pp_list;
  ptree->nearest_find(pt, radius, pp_list);
  printf("time %d ms\n", clock() - ts);

#if _DEBUG
  char* dmy = new char[8];
#endif
  /* print out all the points found in results */
  printf( "found %d results:\n", pp_list.size() );

  for ( int i = 0; i < pp_list.size(); i++ )
  {
	  int aa = pp_list[i];
	  pos[0] = kdGetPoint(aa,0);
	  pos[1] = kdGetPoint(aa,1);
	  pos[2] = kdGetPoint(aa,2);

    /* compute the distance of the current result from the pt */
    dist = sqrt( dist_sq( pt, pos, 3 ) );

    /* print out the retrieved data */
    printf( "node at (%.3f, %.3f, %.3f) is %.3f index=%d dist %f\n", 
		pos[0], pos[1], pos[2], dist, pp[aa].index, dist );
  }

  pp_list.clear();
   ptree->nearest_find(pt, radius*3.0, pp_list);
  printf( "found %d results:\n", pp_list.size() );

  std::vector<int>().swap(pp_list);
  delete [] pp;
  delete ptree;

  //この時点ではまだフリーされていない場合があるので自動（終了時に_CrtDumpMemoryLeaks）で出すようにする。
 	//_CrtDumpMemoryLeaks();
 return 0;
}

static double dist_sq( double *a1, double *a2, int dims ) {
  double dist_sq = 0, diff;
  while( --dims >= 0 ) {
    diff = (a1[dims] - a2[dims]);
    dist_sq += diff*diff;
  }
  return dist_sq;
}

static double rd( void ) {
  return (double)rand()/RAND_MAX * 20.0 - 10.0;
}
