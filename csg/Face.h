/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef FACE_H
#define FACE_H

#include "Bound.h"
#include "Vertex.h"
#include "Object3D.h"
#include "RayVector.h"

#include <stdio.h>
#include "bitmacro.h"
#include "RayBox_def.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif

//tritri.cpp, rayrti.cpp
#if 10
#define double_ex	float
#define to_double__(x)	to_double(x)	
#else
#define double_ex	mlFloat
#define to_double__(x)	x	
#endif
//tritri.cpp
int tri_tri_intersect(double_ex V0[3],double_ex V1[3],double_ex V2[3],double_ex U0[3],double_ex U1[3],double_ex U2[3]);
// rayrti.cpp
int intersect_triangle3(double_ex orig[3], double_ex dir[3], double_ex vert0[3], double_ex vert1[3], double_ex vert2[3], double_ex *t, double_ex *u, double_ex *v);

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3]);

int RayBoxIntersection(const float minB[3], const float maxB[3], float origin[3], float dir[3]);


//各種属性ビット番号
#define D_DELETE_FACE_BIT		(1)
#define D_MICRO_FACE_BIT		(2)
#define D_REVERS_FACE_BIT		(3)
#define D_SPLIT_MAX_BIT			(4)


#define D_OVERLAP_FACE_LIST_BIT	(27)
#define D_OVERLAP_FLAG_BIT		(28)
#define D_OVERLAP_FACE_BIT		(29)


//Face削除情報
//削除されたか？
#define IS_DELETE_FACE(face)		( BIT_GET(face->attr, D_DELETE_FACE_BIT) )

//削除したという印を付ける
#define DELETE_FACE(face)			BIT_ON(face->attr,  D_DELETE_FACE_BIT)

//削除したという印を消す
#define DELETE_CANCEL_FACE(face)	BIT_OFF(face->attr ,D_DELETE_FACE_BIT)

//================== ラップ情報 ================================================
//ラップ情報が有効か
#define IS_OVERLAP_FLAG(face)		( BIT_GET(face->attr, D_OVERLAP_FLAG_BIT) )

//ラップ有り
#define OVERLAP_YES_SET(face)		BIT_ON(face->attr,  D_OVERLAP_FLAG_BIT);BIT_ON(face->attr,  D_OVERLAP_FACE_BIT)

//ラップ無し
#define OVERLAP_NO_SET(face)		BIT_ON(face->attr,  D_OVERLAP_FLAG_BIT);BIT_OFF(face->attr,  D_OVERLAP_FACE_BIT)

//ラップしているか
#define IS_OVERLAP_FACE(face)		( BIT_GET(face->attr, D_OVERLAP_FACE_BIT) )

//ラップしていない( D_OVERLAP_FACE_BITだけを見てもダメ -> ラップ情報を作っていないかも知れないから)
#define IS_NO_OVERLAP_FACE(face)		( IS_OVERLAP_FLAG(face) && !IS_OVERLAP_FACE(face) )
//=============================================================================

//干渉計算リストを付随させたか？
#define IS_OVERLAP_FACE_LIST_BITE(face)		( BIT_GET(face->attr, D_OVERLAP_FACE_LIST_BIT) )

//干渉計算リストを付随させたという印を付ける
#define OVERLAP_FACE_LIST_BIT(face)			BIT_ON(face->attr,  D_OVERLAP_FACE_LIST_BIT)


//Faceメンバのsplit_maxは有効か？
#define IS_SPLIT_MAX_BIT(face)		( BIT_GET(face->attr, D_SPLIT_MAX_BIT) )

//Faceメンバのsplit_maxを有効にする
#define SPLIT_MAX_BIT(face)			BIT_ON(face->attr,  D_SPLIT_MAX_BIT)

#define Face_STATUS_NUM   6

#define Face_UNKNOWN      1
#define Face_INSIDE       2		//Inside other solids.
#define Face_OUTSIDE      3		//Outside of another solid
#define Face_SAME         4		//Inside other solids.(in contact with a solid)
#define Face_OPPOSITE     5		//Outside of another solid(in contact with a solid)

#define Face_UP           6
#define Face_DOWN         7
#define Face_ON           8
#define Face_NONE         9

//class octreecellで使用するテンポラリ変数領域
//#define FACE_TMP_VAR_MAX		16

#include "openmp_config.h"

struct ClosestInfo_t;
class Object3D;
class Line;
class Face
{
private:
	Vertex * v[3];

	Bound bound;

public:
	std::vector<Face*> itfList;
	unsigned int attr;
	int split_count;
	int split_max;
	inline Face():calc_normal_vec(false),status(Face_UNKNOWN),split_count(0),split_max(0),attr(0)
	{
		v[0] = NULL;
		v[1] = NULL;
		v[2] = NULL;
		itfList.clear();
#ifdef USE_OPENMP
		tmp.resize(omp_get_max_threads(), 0);
#else
		tmp.resize(1, 0);
		
#endif
		tmp[0] = 0;
	}

	inline Face(Vertex * v1i, Vertex * v2i, Vertex * v3i):calc_normal_vec(false),status(Face_UNKNOWN),split_count(0),split_max(0),attr(0)
	{
		v[0] = v1i;
		v[1] = v2i;
		v[2] = v3i;
		itfList.clear();
		bound.set(v[0]->xyz.x, v[0]->xyz.y, v[0]->xyz.z, v[1]->xyz.x, v[1]->xyz.y, v[1]->xyz.z, v[2]->xyz.x, v[2]->xyz.y, v[2]->xyz.z);
#ifdef USE_OPENMP
		tmp.resize(omp_get_max_threads(), 0);
#else
		tmp.resize(1, 0);
		
#endif
		tmp[0] = 0;
	}

	inline void clearItfList()
	{
		//std::vector<Face*>().swap(itfList);
		itfList.clear();
		itfList.shrink_to_fit();
	}
	inline  ~Face()
	{
		clearItfList();
	}

	inline Vertex** __fastcall getVertex() const { return (Vertex**)v;}
	inline Vertex* __fastcall getVertex1() const { return v[0];}
	inline Vertex* __fastcall getVertex2() const { return v[1];}
	inline Vertex* __fastcall getVertex3() const { return v[2];}

	inline void __fastcall getPosition(mlVector3D pos[3]) const 
	{
		pos[0] = v[0]->xyz;
		pos[1] = v[1]->xyz;
		pos[2] = v[2]->xyz;
	}

	inline bool __fastcall equals_pointersmatch(const Face * pFace) const
	{
		return (v[0] == pFace->v[0] && v[1] == pFace->v[1] && v[2] == pFace->v[2]);
	}
	inline bool __fastcall equals_pointersmatch(Face& pFace) const
	{
		return (v[0] == pFace.v[0] && v[1] == pFace.v[1] && v[2] == pFace.v[2]);
	}

	inline bool __fastcall equals( Face * pFace) const
	{
		Vertex** pv = pFace->v;

		return	(v[0]->equals(pv[0]) && v[1]->equals(pv[1]) && v[2]->equals(pv[2])) ||
				(v[0]->equals(pv[1]) && v[1]->equals(pv[2]) && v[2]->equals(pv[0])) ||
				(v[0]->equals(pv[2]) && v[1]->equals(pv[0]) && v[2]->equals(pv[1]));
	}

	inline bool __fastcall equals_geometry( Face * pFace, const mlFloat& tol) const
	{
		Vertex** pv = pFace->v;

		return	(v[0]->equals_geometry(pv[0], tol) && v[1]->equals_geometry(pv[1], tol) && v[2]->equals_geometry(pv[2], tol)) ||
				(v[0]->equals_geometry(pv[1], tol) && v[1]->equals_geometry(pv[2], tol) && v[2]->equals_geometry(pv[0], tol)) ||
				(v[0]->equals_geometry(pv[2], tol) && v[1]->equals_geometry(pv[0], tol) && v[2]->equals_geometry(pv[1], tol));
	}
	inline bool __fastcall equals_geometry_invert( Face * pFace, const mlFloat& tol) const
	{
		Vertex** pv = pFace->v;

		return	(v[0]->equals_geometry(pv[0], tol) && v[1]->equals_geometry(pv[2], tol) && v[2]->equals_geometry(pv[1], tol)) ||
				(v[0]->equals_geometry(pv[1], tol) && v[1]->equals_geometry(pv[0], tol) && v[2]->equals_geometry(pv[2], tol)) ||
				(v[0]->equals_geometry(pv[2], tol) && v[1]->equals_geometry(pv[1], tol) && v[2]->equals_geometry(pv[0], tol));
	}

	inline Bound* __fastcall getBound()
	{
		if ( bound.isBound() ) return &bound;
		
		bound.set(	v[0]->xyz.x, v[0]->xyz.y, v[0]->xyz.z,
					v[1]->xyz.x, v[1]->xyz.y, v[1]->xyz.z,
					v[2]->xyz.x, v[2]->xyz.y, v[2]->xyz.z); 
		return &bound;
	}
	inline Bound& __fastcall getBoundFast()
	{
		//if (!bound.isBound())
		//{
		//	printf("!bound.isBound()\n");
		//	abort();
		//}
		return bound;
	}

	mlVector3D normal_vec;
	bool calc_normal_vec;

	
private:
	inline void __fastcall calsNormal(mlVector3D& normal)
	{
		calc_normal_vec = true;

#if 0
		mlVector3D p1 = v1->xyz;
		mlVector3D p2 = v2->xyz;
		mlVector3D p3 = v3->xyz;
	
		mlVector3D xy = p2 - p1;
		mlVector3D xz = p3 - p1;

		//xy.Normalise();
		//xz.Normalise();
	
		normal = mlVectorCross(xy, xz);
		normal.Normalise();
#else
		normal = mlVectorCross(
			mlVector3D(v[1]->xyz.x - v[0]->xyz.x,  v[1]->xyz.y - v[0]->xyz.y, v[1]->xyz.z - v[0]->xyz.z), 
			mlVector3D(v[2]->xyz.x - v[0]->xyz.x,  v[2]->xyz.y - v[0]->xyz.y, v[2]->xyz.z - v[0]->xyz.z));
		normal.Normalise();
#endif
	}

public:
	// lengは二乗の値がセットされて返る
	inline bool __fastcall chkNormal(const mlFloat& tol, mlFloat& leng, mlVector3D& normal)
	{

		normal = mlVectorCross(
			mlVector3D(v[1]->xyz.x - v[0]->xyz.x,  v[1]->xyz.y - v[0]->xyz.y, v[1]->xyz.z - v[0]->xyz.z), 
			mlVector3D(v[2]->xyz.x - v[0]->xyz.x,  v[2]->xyz.y - v[0]->xyz.y, v[2]->xyz.z - v[0]->xyz.z));
	
		bool s = normal.NormaliseChk(tol, leng);
		//if ( leng != 0.0 )
		//{
		//	mlFloat invMag = 1.0 / leng;
		//	normal.x *= invMag;
		//	normal.y *= invMag;
		//	normal.z *= invMag;
		//}
		return s;
	}

	inline mlVector3D& __fastcall getNormal()
	{
		if ( !calc_normal_vec )  calsNormal(normal_vec);

		return normal_vec;
	}

	inline void __fastcall setNormal( mlVector3D& nvec)
	{
		normal_vec = nvec;
		calc_normal_vec = true;
	}

	mlFloat getArea();
	
	inline void __fastcall invert()
	{
		Vertex * vertexTemp = v[1];
		v[1] = v[0];
		v[0] = vertexTemp;
		if (calc_normal_vec) normal_vec = -normal_vec;
	}

	inline int __fastcall simpleClassify()
	{
		const int status1 = v[0]->getStatus();
		const int status2 = v[1]->getStatus();
		const int status3 = v[2]->getStatus();
	
#if 10
		//if(status1==Vertex_INSIDE && status2==Vertex_INSIDE && status3==Vertex_INSIDE)
		//{
		//	status = Face_INSIDE;
		//	return 1; 
		//}
		//if(status1==Vertex_OUTSIDE && status2==Vertex_OUTSIDE && status3==Vertex_OUTSIDE)
		//{
		//	status = Face_OUTSIDE;
		//	return 1; 
		//}

		if(status1==Vertex_INSIDE || status2==Vertex_INSIDE || status3==Vertex_INSIDE)
		{
			if(status1==Vertex_OUTSIDE || status2==Vertex_OUTSIDE || status3==Vertex_OUTSIDE)
			{
				return 0;
			}
			status = Face_INSIDE;
			return 1; 
		}
		if(status1==Vertex_OUTSIDE || status2==Vertex_OUTSIDE || status3==Vertex_OUTSIDE)
		{
			if(status1==Vertex_INSIDE || status2==Vertex_INSIDE || status3==Vertex_INSIDE)
			{
				return 0;
			}
			status = Face_OUTSIDE;
			return 1; 
		}


		return 0;
#else
		if(status1==Vertex::INSIDE)
		{
			status = INSIDE;
			return true; 
		}

		if(status1==Vertex::OUTSIDE)
		{
			status = OUTSIDE;
			return true; 
		}

		if(status2==Vertex::INSIDE)
		{
			status = INSIDE;
			return true;
		}

		if(status2==Vertex::OUTSIDE)
		{
			status = OUTSIDE;
			return true;
		}

		if(status3==Vertex::INSIDE)
		{
			status = INSIDE;
			return true;
		}

		if(status3==Vertex::OUTSIDE)
		{
			status = OUTSIDE;
			return true;
		}
		return false;
#endif
	}

	int rayTraceClassify(Object3D* object, const Bound* object_bound = NULL, const int dirchg = 0, ClosestInfo_t* closestInfo = NULL);

//private:

	bool hasPoint(const mlVector3D & point);

	//enum Status
	//{
	//	UNKNOWN,
	//	INSIDE,
	//	OUTSIDE,
	//	SAME,
	//	OPPOSITE
	//};
	//
	//Status status;

	//Status getStatus();

	//enum LinePosition
	//{
	//	UP,
	//	DOWN,
	//	ON,
	//	NONE
	//};

	int status;

	inline int __fastcall getStatus() const
	{
		return status;
	}
	int linePositionInX(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2);
	int linePositionInY(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2);
	int linePositionInZ(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2);

	int boxOverlap(Face* face)
	{
		float boxcenter[3];
		float boxhalfsize[3];
		float triverts[3][3];

		Bound* bound = getBound();

		boxhalfsize[0] = ((bound->xyz_Max[0]) - (bound->xyz_Min[0]))*0.5f;
		boxhalfsize[1] = ((bound->xyz_Max[1]) - (bound->xyz_Min[1]))*0.5f;
		boxhalfsize[2] = ((bound->xyz_Max[2]) - (bound->xyz_Min[2]))*0.5f;

		boxcenter[0] = ((bound->xyz_Max[0]) + (bound->xyz_Min[0]))*0.5f;
		boxcenter[1] = ((bound->xyz_Max[1]) + (bound->xyz_Min[1]))*0.5f;
		boxcenter[2] = ((bound->xyz_Max[2]) + (bound->xyz_Min[2]))*0.5f;

		Vertex** vrt = face->getVertex();
		
		triverts[0][0] = to_double(vrt[0]->xyz.x);
		triverts[0][1] = to_double(vrt[0]->xyz.y);
		triverts[0][2] = to_double(vrt[0]->xyz.z);

		triverts[1][0] = to_double(vrt[1]->xyz.x);
		triverts[1][1] = to_double(vrt[1]->xyz.y);
		triverts[1][2] = to_double(vrt[1]->xyz.z);

		triverts[2][0] = to_double(vrt[2]->xyz.x);
		triverts[2][1] = to_double(vrt[2]->xyz.y);
		triverts[2][2] = to_double(vrt[2]->xyz.z);
		
		return triBoxOverlap(boxcenter, boxhalfsize, triverts);
	}

public:

	std::vector<unsigned char> tmp;
};

inline int RayIntersection(rayVector& rayVectorData, Face* face)
{
	Bound* bound = face->getBound();
	const double_ex minB[3] = {(bound->xyz_Min[0])-RAYBOX_BOX_EXT_TOL_value, (bound->xyz_Min[1])-RAYBOX_BOX_EXT_TOL_value, (bound->xyz_Min[2])-RAYBOX_BOX_EXT_TOL_value};
	const double_ex maxB[3] = {(bound->xyz_Max[0])+RAYBOX_BOX_EXT_TOL_value, (bound->xyz_Max[1])+RAYBOX_BOX_EXT_TOL_value, (bound->xyz_Max[2])+RAYBOX_BOX_EXT_TOL_value};

	return RayBoxIntersection( minB,  maxB, rayVectorData.orig, rayVectorData.dir);

}

//Tomas Mollerの交差判定(Practical Analysis of Optimized Ray-Triangle Intersection )
inline bool RayIntersection(int n, const mlVector3D& Orig, const mlVector3D& dir, const Face* face, mlFloat& pRetT, mlFloat& pRetU, mlFloat& pRetV, mlVector3D& itfPnt, mlFloat& distance)
{
	const mlFloat zero = 1.0e-18;
	mlVector3D  qvec;
	mlFloat t0, u0, v0;

	Vertex** v = face->getVertex();
	mlVector3D e1;
	mlVector3D e2;
	if (n == 0)
	{
		e1 = v[1]->xyz - v[0]->xyz;
		e2 = v[2]->xyz - v[0]->xyz;
	}
	if (n == 1)
	{
		e1 = v[2]->xyz - v[1]->xyz;
		e2 = v[0]->xyz - v[1]->xyz;
	}
	if (n == 2)
	{
		e1 = v[1]->xyz - v[2]->xyz;
		e2 = v[0]->xyz - v[2]->xyz;
	}
	const mlVector3D pvec = VECTOR3D_CROSS(dir, e2);
	const mlFloat det = VECTOR3D_DOT(e1, pvec);

	if (det > zero) {

		const mlVector3D tvec = Orig - v[n]->xyz;
		u0 = VECTOR3D_DOT(tvec, pvec);
		if (u0 < 0.0 || u0 > det) return false;

		qvec = VECTOR3D_CROSS(tvec, e1);

		v0 = VECTOR3D_DOT(dir, qvec);
		if (v0 < 0.0 || u0 + v0 > det) return false;
	} else if (det < -zero) {
		const mlVector3D tvec = Orig - v[n]->xyz;

		u0 = VECTOR3D_DOT(tvec, pvec);
		if (u0 > 0.0 || u0 < det) return false;

		qvec = VECTOR3D_CROSS(tvec, e1);

		v0 = VECTOR3D_DOT(dir, qvec);
		if (v0 > 0.0 || u0 + v0 < det) return false;

	} else {
		return false;
	}

	const mlFloat inv_det = 1.0 / det;

	t0 = VECTOR3D_DOT(e2, qvec);
	t0 *= inv_det;
	u0 *= inv_det;
	v0 *= inv_det;

	pRetT = t0;
	pRetU = u0;
	pRetV = v0;
	itfPnt = dir * t0 + Orig;

	/*
		distance = sqrt((itfPnt - Orig)*(itfPnt - Orig))
		distance = sqrt((dir * t0 + Orig - Orig)*(dir * t0 + Orig - Orig))
		distance = sqrt((dir * t0)*(dir * t0))
		distance = sqrt(t0^2*dir*dir) = sqrt(t0^2)*sqrt(|dir|)= t0
	*/
#if 0
	mlFloat x = itfPnt.x - Orig.x;
	mlFloat y = itfPnt.y - Orig.y;
	mlFloat z = itfPnt.z - Orig.z;

	distance = sqrt(x*x + y*y + z*z);
	if ( distance != 0.0 )
	{
		mlFloat w = 1.0/distance;
		x = x*w;
		y = y*w;
		z = z*w;
	}
	if (( x*dir.x + y*dir.y + z*dir.z ) < 0 )
	{
		distance = -distance;			
	}
#else
	distance = t0;
#endif
	return true;
}

//Tomas Mollerの交差判定(Practical Analysis of Optimized Ray-Triangle Intersection )
inline bool RayIntersection(const mlVector3D& Orig, const mlVector3D& dir, const Face* face, mlFloat& pRetT, mlFloat& pRetU, mlFloat& pRetV, mlVector3D& itfPnt, mlFloat& distance)
{
	bool s = RayIntersection(0, Orig, dir, face, pRetT, pRetU, pRetV, itfPnt, distance);
	if (!s)
	{
		s = RayIntersection(1, Orig, dir, face, pRetT, pRetU, pRetV, itfPnt, distance);
	}
	if (!s)
	{
		s = RayIntersection(2, Orig, dir, face, pRetT, pRetU, pRetV, itfPnt, distance);
	}

	return s;
}



struct ClosestInfo_t
{
	int success;
	Face * closestFace;
	mlFloat closestDistance;
	mlFloat dotProduct;
};


#endif // FACE_H
