/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "Face.h"

#include "Vertex.h"
#include "Bound.h"

#include "ML_Vector.h"

#include "Line.h"
#include "octreecell.h"

#include <math.h>
#include <stdio.h>

#include <Windows.h>
#include "def.h"


//#undef USE_OPENMP
//2012.01.30
#include "thread_omp_util.h"

float RAYBOX_BOX_EXT_TOL_value = RAYBOX_BOX_EXT_TOL;
float OCTREE_CELL_TOL_value = OCTREE_CELL_TOL;

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif


//const mlFloat Face::TOL = Face_TOL;

mlFloat Face::getArea()
{
#if 0
	//area = (a * c * sen(B))/2

	mlVector3D p1 = v1->xyz;
	mlVector3D p2 = v2->xyz;
	mlVector3D p3 = v3->xyz;
	mlVector3D xy = p2 - p1;
	mlVector3D xz = p3 - p1;
	
	mlFloat a = xy.Magnitude();
	mlFloat c = xz.Magnitude();
	//mlFloat B = xy.angle(xz);
	mlFloat B = 0.0f;
	{
		mlVector3D xyNorm = xy.Normalised();
		mlVector3D xzNorm = xz.Normalised();
		mlFloat fDot = xyNorm * xzNorm;
		mlFloat fAngle = acos(fDot);
		B = fAngle;
	}
	
	return (a * c * sin(B))/2.0f;
#else
	mlVector3D& normal = getNormal();
	//mlVector3D normal = mlVectorCross(
	//	mlVector3D(v[1]->xyz.x - v[0]->xyz.x,  v[1]->xyz.y - v[0]->xyz.y, v[1]->xyz.z - v[0]->xyz.z), 
	//	mlVector3D(v[2]->xyz.x - v[0]->xyz.x,  v[2]->xyz.y - v[0]->xyz.y, v[2]->xyz.z - v[0]->xyz.z));

	return sqrt(VECTOR3D_DOT(normal,normal))*0.5;
#endif
}





#ifdef USE_OPENMP
//#define RAY_THREAD_MAX		FACE_TMP_VAR_MAX
#define _BREAK_LOOP	continue
#else
//#define RAY_THREAD_MAX		(1)
#define _BREAK_LOOP	break
#endif

//#define USE_PARALLEL

int Face::rayTraceClassify(Object3D* object, const Bound* object_bound, const int dirchg, ClosestInfo_t* closestInfo)
{
	if ( closestInfo != NULL ) closestInfo->closestFace = NULL;

	//printf("rayTraceClassify\n");
		if ( !object_bound->overlap(getBoundFast()))
		{
			status = Face_OUTSIDE;
			return 0;
		}

	//creating a ray starting starting at the face baricenter going to the normal direction
#if 10
	const mlFloat t = 1.0/3.0;
	mlVector3D p0(	(v[0]->xyz.x + v[1]->xyz.x + v[2]->xyz.x)*t,
					(v[0]->xyz.y + v[1]->xyz.y + v[2]->xyz.y)*t,
					(v[0]->xyz.z + v[1]->xyz.z + v[2]->xyz.z)*t );
	Line ray(getNormal(),p0);
#else
	mlVector3D p0;
	p0.x = (v1->xyz.x + v2->xyz.x + v3->xyz.x)/3.0;
	p0.y = (v1->xyz.y + v2->xyz.y + v3->xyz.y)/3.0;
	p0.z = (v1->xyz.z + v2->xyz.z + v3->xyz.z)/3.0;
	Line ray(getNormal(),p0);
#endif


	//mlFloat TOL = 0.0001f;
	
	int stat = 0;
	int n = 0;
	const int loopmax = 1000;
	const mlFloat maxdist = 99999.9;
	const mlFloat zero = mlFloat(0.0);
	const mlFloat tol = Face_TOL;

	mlVector3D rayDir;
	Face * closestFace = 0;
	mlFloat closestDistance = maxdist;

	int numthread = 1;	//計算スレッド数初期値
	int ansID = 0;		//各スレッドが持ち帰る解のID
#ifdef USE_OPENMP
	#ifdef USE_PARALLEL
	//スレッド数制限
	Thread_omp_limit omp_limit(omp_get_max_threads());
	numthread = omp_limit.numthread;
	ansID = -1;
	#else
	//スレッド使わない（呼び出し側から並列的に呼び出される)
	Thread_omp_limit omp_limit;
	numthread = omp_limit.numthread;
	ansID = -1;
	#endif
#endif

	/*偽共有を回避する
	  closestInf[0]...closestInf[RAY_THREAD_MAX-1] だと隣り合った領域アクセスが起きる
	  これを偽共有という。これを回避するために以下のようにする
	   closestInf[0][0]...closestInf[RAY_THREAD_MAX-1][0]
	*/
	//ClosestInfo_t closestInf[RAY_THREAD_MAX][RAY_THREAD_MAX];
	std::vector<ClosestInfo_t> closestInf_tmp(numthread);
	std::vector<std::vector<ClosestInfo_t>> closestInf(numthread, closestInf_tmp);

#ifdef USE_OPENMP
	const int threadId = omp_get_thread_num();
#else
	const int threadId = 0;
#endif

	bool success = true;
	do
	{
		success = true;
		rayDir = ray.direction;
		if ( n > loopmax )
		{
			printf("rayTrace error. loopmax:%d\n", loopmax);
			//char tmp_buf[10];
			//gets_s(tmp_buf, 10);
			stat = -1;
			break;
		}
		//if (n >= 1) printf("re try %d\n", n);

		//リトライ処理（レイを少しだけ傾けてみる
		if ( n >= 1 || dirchg )
		{
			ray.perturbDirection2(rayDir);
		}

		n++;

		//for each face from the other solid...
		const int sz = object->getNumFaces();

		//各スレッドが持ち帰る解の初期化
//#ifdef USE_OPENMP
//#pragma omp parallel for
//#endif
		for ( int j = 0; j < numthread; j++ )
		{
			closestInf[j][0].success = 1;
			closestInf[j][0].closestFace = 0;
			closestInf[j][0].closestDistance = maxdist;
		}


		//octree cellボックスとレイが干渉するかをみて干渉する最深部cellに登録されたFaceを取得する
		rayVector rayVectorData(p0, rayDir);
		std::vector<Face*> octreeToitfList;

		//レイと干渉する可能性のあるFaceリストの数
		int itfsz = 0;

		//レイと干渉する可能性のあるFaceリストから個々のFaceのバウンディングBoxとレイの干渉をチェックして干渉する物だけを残す
		std::vector<Face*> itfList;
		int nn = 0;

#ifdef USE_OPENMP
#ifndef USE_PARALLEL
//#pragma omp critical
#endif
#endif
		{
			//printf("threadId:%d\n", threadId);
			object->cell->getfaces_itfRay( rayVectorData.orig, rayVectorData.dir, octreeToitfList, threadId);

			//レイと干渉する可能性のあるFaceリストの数
			itfsz = octreeToitfList.size();

			itfList.resize(itfsz, NULL);

#ifdef USE_OPENMP
#ifdef USE_PARALLEL
#pragma omp parallel for  reduction(+:nn)
#endif
#endif
			for ( int j = 0; j < itfsz; j++ )
			{
				Face* face = octreeToitfList[j];

				//元に戻しておく
				face->tmp[threadId] = 0;
				//ラフチェックで使用
				if (RayIntersection(rayVectorData, face))
				{
					itfList[j] = face;
					nn++;
				}
			}
		}

		//レイがどのFaceとも干渉しない場合
		if ( nn == 0 )
		{
			status = Face_OUTSIDE;
			return 0;
		}


		//干渉する可能性高いFaceとレイの交点計算を行う

		int break_flg = 0;

#ifdef USE_OPENMP
#ifdef USE_PARALLEL
#pragma omp parallel
#endif
#endif
		{
			Thread_omp_LoopInBreak omp(numthread, itfsz, 0, &break_flg);

#ifdef USE_OPENMP
#ifdef USE_PARALLEL
			const int ompID = omp.Id();
#else
			const int ompID = 0;
#endif
#else
			const int ompID = 0;
#endif

			for( int i = omp.Begin(); omp.Condition(i); i++ )
			{
				Face* face = itfList[i];

				if ( face == NULL )
				{
					continue;
				}

				mlFloat t0, u0, v0;
				mlVector3D 	intersectionPoint;
				mlFloat distance;

				if( RayIntersection(p0, rayDir, face, t0, u0, v0, intersectionPoint, distance) )
				{
					const mlVector3D& face_normal = face->getNormal();
					const mlFloat dotProduct = VECTOR3D_DOT(face_normal, rayDir); // dot product

#if 0
					bool bIntersectResult_tmp;
					mlVector3D 	tmpPnt = ray.computePlaneIntersection(face_normal, face->v[0]->xyz, bIntersectResult_tmp);
					mlFloat distance_tmp;
					ray.computePointToPointDistance(tmpPnt, distance_tmp);
				
					mlFloat s1 = (tmpPnt - intersectionPoint).Magnitude();
					mlFloat s2 = (distance_tmp - distance);
					mlFloat s3 = (distance_tmp - t0);

					if ( fabs(s1) > 1.0e-20 || fabs(s2) > 1.0e-20 || fabs(s3) > 1.0e-20 || bIntersectResult_tmp != bIntersectResult)
					{
						printf("point %s\n", s1.to_string().c_str());
						printf("dist  %s\n", s2.to_string().c_str());
						printf("dist2  %s\n", s3.to_string().c_str());
					}
					if ( distance < 0.0 && distance_tmp > 0.0 ) printf("aaa1\n");
					if ( distance < 0.0 && t0 > 0.0 ) printf("aaa2\n");
					if ( distance > 0.0 && distance_tmp < 0.0 ) printf("aaa3\n");
					if ( distance > 0.0 && t0 < 0.0 ) printf("aaa4\n");
					if ( t0 > 0.0 && distance_tmp < 0.0 ) printf("aaa5\n");
					if ( t0 < 0.0 && distance_tmp > 0.0 ) printf("aaa6\n");
#endif
					ClosestInfo_t* closestInfP = &(closestInf[ompID][0]);
					//if ray lies in plane...
					if( Absolute(distance) < tol )
					{
						if ( Absolute(dotProduct) < tol )
						{
							//disturb the ray in order to not lie into another plane 
							//ray.perturbDirection();
							//rayDir = ray.getDirection();

							closestInfP->success = 0;
						}else
							//if ray starts in plane...
						{
							//faces coincide
							closestInfP->closestFace = face;
							closestInfP->closestDistance = zero;
							//答えの確定
							closestInfP->success = 2;
						}
#ifdef USE_OPENMP
#ifdef USE_PARALLEL
						#pragma omp flush(break_flg)
#else
						break_flg = 1;
						break;
#endif
#else
						break_flg = 1;
						break;
#endif
					}				
					//if ray intersects plane... 
					else if(distance>tol)
					{
						if( distance < closestInfP->closestDistance )
						{
							//this face is the closest face untill now
							closestInfP->closestDistance = distance;
							closestInfP->closestFace = face;
						}
					}
				}
			}
		}

		success = false;
		//各スレッドが持ち帰った解を調べる
		for ( int j = 0; j < numthread; j++ )
		{
			//確定した答えがある
			if ( closestInf[j][0].success == 2 )
			{
				ansID = j;
				success = true;
				break;
			}
		}

		//確定した答えが無い
		if ( ansID == -1 )
		{
			bool ok = true;
			//各スレッドを調べて全てのRayで計算されたか調べる
			for ( int j = 0; j < numthread; j++ )
			{
				//中断した計算があった
				if ( closestInf[j][0].success == 0 )
				{
					//printf("n=%d\n", n);
					//char tmp_buf[32];
					//gets_s(tmp_buf, 32);
					ok = false;
					break;
				}
			}
			if ( ok ) success = true;
		}else
		{
			success = true;
		}
		//解無しで中断された場合はリトライする
	}while(success == false );
	

	status = Face_SAME;
	if ( stat < 0 )
	{
		//char tmp_buf[10];
		//gets_s(tmp_buf, 10);
		return stat;
	}


#ifdef USE_OPENMP
	if ( ansID >= 0 )
	{
		//確定した答えがあった場合
		closestDistance = closestInf[ansID][0].closestDistance;
		closestFace = closestInf[ansID][0].closestFace;
	}else
	{
		//全ての計算が正常に終わっている

		ansID = 0;		//Rayが何処に刺さらない場合もある。
		closestDistance = maxdist;
		closestFace = 0;

		//全てのスレッドが持ち帰った計算で一番手前でRayが刺さった所が解
		for ( int j = 0; j < numthread; j++ )
		{
			if ( closestDistance > closestInf[j][0].closestDistance )
			{
				closestDistance = closestInf[j][0].closestDistance;
				closestFace = closestInf[j][0].closestFace;
				ansID = j;
			}
		}
	}
#else
	closestDistance = closestInf[0][0].closestDistance;
	closestFace = closestInf[0][0].closestFace;
#endif


	//none face found: outside face
	if(closestFace==0)
	{
		status = Face_OUTSIDE;
	}
	//face found: test dot product
	else
	{
		const mlVector3D closestFaceNormal = closestFace->getNormal();
		const mlFloat dotProduct = VECTOR3D_DOT(closestFaceNormal, rayDir);
		
		//distance = 0: coplanar faces
		if(Absolute(closestDistance)<tol)
		{
			if(dotProduct>tol)
			{
				status = Face_SAME;
			}
			else if(dotProduct<-tol)
			{
				status = Face_OPPOSITE;
			}
		}
		
		//dot product > 0 (same direction): inside face
		else if(dotProduct>tol)
		{
			status = Face_INSIDE;
		}
		
		//dot product < 0 (opposite direction): outside face
		else if(dotProduct<-tol)
		{
			status = Face_OUTSIDE;
		}

		if ( closestInfo != NULL )
		{
			closestInfo->closestDistance = closestDistance;
			closestInfo->closestFace = closestFace;
			closestInfo->dotProduct = dotProduct;
		}
	}
	return stat;
}

//------------------------------------PRIVATES----------------------------------//

/**
 * Checks if the the face contains a point
 * 
 * @param point to be tested
 * @param true if the face contains the point, false otherwise 
 */	
bool Face::hasPoint(const mlVector3D &  point)
{
	int result1, result2, result3;
	const mlVector3D& normal = getNormal(); 

	mlVector3D pos[3];

	getPosition(pos);

	int axis = 0;
	mlFloat maxval = Absolute(normal.x);

	if ( Absolute(normal.y) > maxval )
	{
		axis = 1;
		maxval = Absolute(normal.y);
	}
	if ( Absolute(normal.z) > maxval )
	{
		axis = 2;
		maxval = Absolute(normal.z);
	}

	switch(axis)
	{
	case 0:
		result1 = linePositionInX(point, pos[0], pos[1]);
		result2 = linePositionInX(point, pos[1], pos[2]);
		result3 = linePositionInX(point, pos[2], pos[0]);
		break;
	case 1:
		result1 = linePositionInY(point, pos[0], pos[1]);
		result2 = linePositionInY(point, pos[1], pos[2]);
		result3 = linePositionInY(point, pos[2], pos[0]);
		break;
	case 2:
		result1 = linePositionInZ(point, pos[0], pos[1]);
		result2 = linePositionInZ(point, pos[1], pos[2]);
		result3 = linePositionInZ(point, pos[2], pos[0]);
	}

	//if the point is up and down two lines...		
	if(((result1==Face_UP)||(result2==Face_UP)||(result3==Face_UP))&&((result1==Face_DOWN)||(result2==Face_DOWN)||(result3==Face_DOWN)))
	{
		return true;
	}
	//if the point is on of the lines...
	else if ((result1==Face_ON)||(result2==Face_ON)||(result3==Face_ON))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/** 
 * Gets the position of a point relative to a line in the x plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */
int Face::linePositionInX(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	if((Absolute(pointLine1.y-pointLine2.y)>Face_TOL)&&(((point.y>=pointLine1.y)&&(point.y<=pointLine2.y))||((point.y<=pointLine1.y)&&(point.y>=pointLine2.y))))
	{
		const mlFloat a = (pointLine2.z-pointLine1.z)/(pointLine2.y-pointLine1.y);
		const mlFloat b = pointLine1.z - a*pointLine1.y;
		const mlFloat z = a*point.y + b;
		if(z>point.z+Face_TOL)
		{
			return Face_UP;			
		}
		else if(z<point.z-Face_TOL)
		{
			return Face_DOWN;
		}
		else
		{
			return Face_ON;
		}
	}
	else
	{
		return Face_NONE;
	}
}

/** 
 * Gets the position of a point relative to a line in the y plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */

int Face::linePositionInY(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	if((Absolute(pointLine1.x-pointLine2.x)>Face_TOL)&&(((point.x>=pointLine1.x)&&(point.x<=pointLine2.x))||((point.x<=pointLine1.x)&&(point.x>=pointLine2.x))))
	{
		const mlFloat a = (pointLine2.z-pointLine1.z)/(pointLine2.x-pointLine1.x);
		const mlFloat b = pointLine1.z - a*pointLine1.x;
		const mlFloat z = a*point.x + b;
		if(z>point.z+Face_TOL)
		{
			return Face_UP;			
		}
		else if(z<point.z-Face_TOL)
		{
			return Face_DOWN;
		}
		else
		{
			return Face_ON;
		}
	}
	else
	{
		return Face_NONE;
	}
}

/** 
 * Gets the position of a point relative to a line in the z plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */

int Face::linePositionInZ(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	if((Absolute(pointLine1.x-pointLine2.x)>Face_TOL)&&(((point.x>=pointLine1.x)&&(point.x<=pointLine2.x))||((point.x<=pointLine1.x)&&(point.x>=pointLine2.x))))
	{
		const mlFloat a = (pointLine2.y-pointLine1.y)/(pointLine2.x-pointLine1.x);
		const mlFloat b = pointLine1.y - a*pointLine1.x;
		const mlFloat y = a*point.x + b;
		if(y>point.y+Face_TOL)
		{
			return Face_UP;			
		}
		else if(y<point.y-Face_TOL)
		{
			return Face_DOWN;
		}
		else
		{
			return Face_ON;
		}
	}
	else
	{
		return Face_NONE;
	}
}
