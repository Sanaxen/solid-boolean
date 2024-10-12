#ifndef _UNIFORM_GRID_H
#define _UNIFORM_GRID_H

#include <vector>
#include "sse_func.h"

#define USE_SPATIAL_HASH_3D

#define  NEAR_PRIM_NUM010x010x010 (997)
#define  NEAR_PRIM_NUM020x020x020 (7993)
#define  NEAR_PRIM_NUM030x030x030 (26993)
#define  NEAR_PRIM_NUM040x040x040 (63997)
#define  NEAR_PRIM_NUM050x050x050 (125003)
#define  NEAR_PRIM_NUM060x060x060 (215983)
#define  NEAR_PRIM_NUM070x070x070 (342989)
#define  NEAR_PRIM_NUM080x080x080 (511997)	//<==  8Mb Over!!
#define  NEAR_PRIM_NUM090x090x090 (728993)	//<== 11Mb Over!!
#define  NEAR_PRIM_NUM100x100x100 (1000003)	//<== 15Mb Over!!
#define  NEAR_PRIM_NUM110x110x110 (1330997)
#define  NEAR_PRIM_NUM120x120x120 (1727989)
#define  NEAR_PRIM_NUM130x130x130 (2196979)
#define  NEAR_PRIM_NUM140x140x140 (2744003)
#define  NEAR_PRIM_NUM150x150x150 (3375007)
#define  NEAR_PRIM_NUM160x160x160 (4095991)
#define  NEAR_PRIM_NUM170x170x170 (4912991)
#define  NEAR_PRIM_NUM180x180x180 (5832007)
#define  NEAR_PRIM_NUM190x190x190 (6858997)
#define  NEAR_PRIM_NUM200x200x200 (7999993)
#define  NEAR_PRIM_NUM210x210x210 (9261013)
#define  NEAR_PRIM_NUM220x220x220 (10647983)
#define  NEAR_PRIM_NUM230x230x230 (12166997)
#define  NEAR_PRIM_NUM240x240x240 (13823987)
#define  NEAR_PRIM_NUM250x250x250 (15625007)
#define  NEAR_PRIM_NUM260x260x260 (17576017)
#define  NEAR_PRIM_NUM270x270x270 (19683007)
#define  NEAR_PRIM_NUM280x280x280 (21951997)
#define  NEAR_PRIM_NUM290x290x290 (24389003)
#define  NEAR_PRIM_NUM300x300x300 (27000011) //<== 400Mb Over!!

//グリッドサイズが大きいとメモリキャッシュミスなどによりコストが逆にかかる。
#define UNIFORM_GRID_CELL_MAX	(100)


#ifdef USE_SPATIAL_HASH_3D
//UNIFORM_GRID_CELL_MAX^3に近い素数
#define GRID_HASH_TABLE_SIZE	(NEAR_PRIM_NUM100x100x100)
#define GDIR_TABLE_SIZE			GRID_HASH_TABLE_SIZE
#else
#define UNIFORM_GRID_TABLE_SIZE	(UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX)
#define GDIR_TABLE_SIZE		UNIFORM_GRID_TABLE_SIZE
#endif

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


#define GRID_ROUND_OFFSET	(0.0005f)
/*
   (int)1.99999 => 1
   (int)2.00000 => 2

   (int)(1.99999+GRID_ROUND_OFFSET) => (int)(2.00049) => 2
   (int)(2.00000+GRID_ROUND_OFFSET) => (int)(2.00050) => 2
*/

//#define HASH3D(i,j,k) (i*(UNIFORM_GRID_CELL_MAX)*(UNIFORM_GRID_CELL_MAX) + j*(UNIFORM_GRID_CELL_MAX) + k)

inline unsigned int __fastcall Easyhash(int xi, int yi, int zi) 
{
	unsigned int h = (xi*(UNIFORM_GRID_CELL_MAX)*(UNIFORM_GRID_CELL_MAX) + yi*(UNIFORM_GRID_CELL_MAX) + zi);
	return h;
}


/*
M. Teschner, B. Heidelberger, M. Mueller, D. Pomeranets, M. Gross, 
"Optimized Spatial Hashing for Collision Detection of Deformable Objects," 
Proc. VMV, Munich, Germany, pp. 47-54, Nov. 19-21, 2003.
[http://cg.informatik.uni-freiburg.de/publications/collisionDetectionHashingVMV2003.pdf]

inline unsigned int hash(int xi, int yi, int zi) {

unsigned int h = (xi * 73856093)^(yi * 19349663)^(zi * 83492791) ; //same as in paper
return h % sizeOfHashTable} //sizeOfHashTable=big value
}

*/

inline unsigned int __fastcall Spatialhash(int xi, int yi, int zi) 
{
	unsigned int h = (xi * 73856093)^(yi * 19349663)^(zi * 83492791) ; //same as in paper
	return h % GDIR_TABLE_SIZE;
}

#ifdef USE_SPATIAL_HASH_3D
#define HASH3D(i,j,k)	Spatialhash(i,j,k)
#else
#define HASH3D(i,j,k)	Easyhash(i,j,k)
#endif


template<class T>
class UniformGrid
{
	std::vector<std::vector<T>> cell;

	ALIGN16 float org[4];
	ALIGN16 float length[4];
	ALIGN16 float invlength[4];

public:
	inline void Init()
	{
		const int sz = GDIR_TABLE_SIZE;
		cell = std::vector<std::vector<T>>(sz);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
	}

	inline UniformGrid(){Init();}
	inline UniformGrid(const float xmin, const float ymin, const float zmin, const float xmax, const float ymax, const float zmax)
	{
		Init();
		//printf("UNIFORM_GRID_CELL_MAX %d => near_prime %d\n", UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX, near_prime(UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX*UNIFORM_GRID_CELL_MAX));
		//printf("UniformGrid memory %.2fMbyts\n", (sizeof(cell)/1024.0)/1000.0f);
		
		org[0] = xmin;
		org[1] = ymin;
		org[2] = zmin;
		org[3] = 0.0f;

		length[0] = xmax - org[0];
		length[1] = ymax - org[1];
		length[2] = zmax - org[2];
		length[3] = 0.0f;

		invlength[0] = invlength[1] = invlength[2] = invlength[3] =0.0f;
		if ( length[0] != 0.0f ) invlength[0] = 1.0f/length[0];
		if ( length[1] != 0.0f ) invlength[1] = 1.0f/length[1];
		if ( length[2] != 0.0f ) invlength[2] = 1.0f/length[2];

		invlength[0] *= (UNIFORM_GRID_CELL_MAX-1);
		invlength[1] *= (UNIFORM_GRID_CELL_MAX-1);
		invlength[2] *= (UNIFORM_GRID_CELL_MAX-1);

		//Clear();
	}

	~UniformGrid()
	{
		Clear();
	}

	void Clear()
	{
		const int sz = GDIR_TABLE_SIZE;
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for ( int i = 0; i < sz; i++ )
			if ( cell[i].size() > 0 ) cell[i].shrink_to_fit();
		cell.shrink_to_fit();
	}


	inline void __fastcall Add(const float* xyz, const T t)
	{
		int ix = (int)(( xyz[0] - org[0])*invlength[0] + GRID_ROUND_OFFSET);
		int iy = (int)(( xyz[1] - org[1])*invlength[1] + GRID_ROUND_OFFSET);
		int iz = (int)(( xyz[2] - org[2])*invlength[2] + GRID_ROUND_OFFSET);

		if ( ix < 0 ) ix = 0;
		if ( iy < 0 ) iy = 0;
		if ( iz < 0 ) iz = 0;
		if ( ix >= UNIFORM_GRID_CELL_MAX ) ix = UNIFORM_GRID_CELL_MAX-1;
		if ( iy >= UNIFORM_GRID_CELL_MAX ) iy = UNIFORM_GRID_CELL_MAX-1;
		if ( iz >= UNIFORM_GRID_CELL_MAX ) iz = UNIFORM_GRID_CELL_MAX-1;

		const unsigned int index = HASH3D(ix, iy, iz);
		cell[index].push_back(t);
	}

	inline std::vector<T>& __fastcall LockUp(const float* xyz)
	{
		int ix = (int)(( xyz[0] - org[0])*invlength[0] + GRID_ROUND_OFFSET);
		int iy = (int)(( xyz[1] - org[1])*invlength[1] + GRID_ROUND_OFFSET);
		int iz = (int)(( xyz[2] - org[2])*invlength[2] + GRID_ROUND_OFFSET);

		if ( ix < 0 ) ix = 0;
		if ( iy < 0 ) iy = 0;
		if ( iz < 0 ) iz = 0;
		if ( ix >= UNIFORM_GRID_CELL_MAX ) ix = UNIFORM_GRID_CELL_MAX-1;
		if ( iy >= UNIFORM_GRID_CELL_MAX ) iy = UNIFORM_GRID_CELL_MAX-1;
		if ( iz >= UNIFORM_GRID_CELL_MAX ) iz = UNIFORM_GRID_CELL_MAX-1;
	
		const unsigned int index = HASH3D(ix, iy, iz);
		return  cell[index];
	}
};

#endif
