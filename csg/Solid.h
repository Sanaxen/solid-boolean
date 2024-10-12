/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef SOLID_H
#define SOLID_H

#include "IntSet.h"
#include "VectorSet.h"
#include "ColorSet.h"
#include "ML_Transform.h"
#include "BooleanModeller.h"
#include "def.h"

#include <string>

class uad_double_library
{
  unsigned int old_cw;
public:

	uad_double_library()
	{
	  fpu_fix_start(&old_cw);
	}

	~uad_double_library()
	{
	  fpu_fix_end(&old_cw);
	}
};
uad_double_library* uad_double_library_init(void);
void uad_double_library_term(uad_double_library* uad_double_library_);


class Solid
{
public:

	int	name_id;
	IntSet indices;
	VectorSet vertices;
	ColorSet colors;
	
	Solid()
	{
		indices.m_pInts.clear();
		vertices.m_pVectors.clear();
		colors.m_pColors.clear();
		name_id = 0;
	}
	Solid(const std::string & sFileName);
	Solid(VectorSet * vertices, IntSet * indices);
	Solid(VectorSet * vertices, IntSet * indices, ColorSet* colors);
	Solid(VectorSet * vertices, IntSet * indices, int color);

	~Solid();
	
	VectorSet * getVertices();
	IntSet * getIndices();
	ColorSet * getColors();

	inline VectorSet& refVertices(){ return vertices;}
	inline IntSet& refIndices(){ return indices;}
	inline ColorSet& refColors(){ return colors;}

	bool isEmpty();

	void setColor(int color);
	void setData(VectorSet & vertices, IntSet & indices);
	
	void Translate(const mlVector3D & t);
	void Rotate(const mlVector3D & a, const mlVector3D & b, mlFloat angle);
	void Rotate(const mlVector3D & n);
	void Scale(const mlVector3D & s);

	//void Render();

	void loadCoordinateFile(const std::string & sFileName);

	mlVector3D getMean();

	mlTransform		m_correctionalTransform;


	void Simplify();
};





//  Edge Collapse Simplify
class simEdge
{
public:
	int start_vtx;
	int end_vtx;
	int side_face_id;
	double length2;

	simEdge()
	{
		side_face_id = -1;
	}
	void Flush(VectorSet& vertex_coord_list, IntSet& indices)
	{
		mlFloat d = (vertex_coord_list[indices[end_vtx]] - vertex_coord_list[indices[start_vtx]]).MagnitudeSquared();
		length2 = to_double(d);
	}
};


class simFace
{
public:
	simEdge e[3];
	bool collapse_edge;
	simFace()
	{
		collapse_edge = false;
	}
	mlVector3D nromal_vec;
	double area;
	double qes;	//メッシュ品質 EquiAngleSkew (QEAS)

	void Flush(VectorSet& vertex_coord_list, IntSet& indices)
	{
		nromal_vec = calcNormal(vertex_coord_list, indices);
		area = calcArea(vertex_coord_list, indices);
		qes = calcEquiAnglSkew(vertex_coord_list, indices);
	}

	mlVector3D getNormal() const
	{
		return nromal_vec;
	}
	double getArea() const
	{
		return area;
	}
	double getQes() const
	{
		return qes;
	}

	void setNormal(mlVector3D& vec)
	{
		nromal_vec = vec;
	}

	mlVector3D calcNormal(VectorSet& vertex_coord_list, IntSet& indices)
	{
		mlVector3D v[3];

		v[0] = vertex_coord_list[indices[e[0].start_vtx]];
		v[1] = vertex_coord_list[indices[e[1].start_vtx]];
		v[2] = vertex_coord_list[indices[e[2].start_vtx]];
		mlVector3D normal = mlVectorCross(
			mlVector3D(v[1].x - v[0].x,  v[1].y - v[0].y, v[1].z - v[0].z), 
			mlVector3D(v[2].x - v[0].x,  v[2].y - v[0].y, v[2].z - v[0].z));
		normal.Normalise();

		return normal;
	}

	double calcArea(VectorSet& vertex_coord_list, IntSet& indices)
	{
		mlVector3D v[3];

		v[0] = vertex_coord_list[indices[e[0].start_vtx]];
		v[1] = vertex_coord_list[indices[e[1].start_vtx]];
		v[2] = vertex_coord_list[indices[e[2].start_vtx]];
		mlVector3D normal = mlVectorCross(
			mlVector3D(v[1].x - v[0].x,  v[1].y - v[0].y, v[1].z - v[0].z), 
			mlVector3D(v[2].x - v[0].x,  v[2].y - v[0].y, v[2].z - v[0].z));

		return to_double( VECTOR3D_DOT(normal, normal) )*0.5;
	}

	double computeDistance(mlVector3D& p, VectorSet& vertex_coord_list, IntSet& indices)
	{
		const mlVector3D& v = vertex_coord_list[indices[e[0].start_vtx]];
		return fabs( to_double(nromal_vec.x*(v.x-p.x) + nromal_vec.y*(v.y - p.y)+ nromal_vec.z*(v.z - p.z)) );
	}

	double calcEquiAnglSkew(VectorSet& vertex_coord_list, IntSet& indices)
	{
		mlVector3D v[3];

		v[0] = vertex_coord_list[indices[e[0].start_vtx]];
		v[1] = vertex_coord_list[indices[e[1].start_vtx]];
		v[2] = vertex_coord_list[indices[e[2].start_vtx]];

		mlVector3D vec[3];

		vec[0] = v[1] - v[0];
		vec[1] = v[2] - v[1];
		vec[2] = v[0] - v[2];

		vec[0].Normalise();
		vec[1].Normalise();
		vec[2].Normalise();

		double a[3];

		a[0] = to_double(VECTOR3D_DOT(vec[0], vec[2]));
		a[1] = to_double(VECTOR3D_DOT(vec[0], vec[1]));
		a[2] = to_double(VECTOR3D_DOT(vec[1], vec[2]));

		if ( a[0] > 1.0 ) a[0] = 1.0;
		if ( a[0] < -1.0 ) a[0] = -1.0;
		if ( a[1] > 1.0 ) a[1] = 1.0;
		if ( a[1] < -1.0 ) a[1] = -1.0;
		if ( a[2] > 1.0 ) a[2] = 1.0;
		if ( a[2] < -1.0 ) a[2] = -1.0;

		double th[3];
		const double c = 180.0/3.14159265358979323846;

		th[0] = acos( a[0] )*c;
		th[1] = acos( a[1] )*c;
		th[2] = acos( a[2] )*c;

		double tmax, tmin;

		tmax = th[0];
		if ( tmax < th[1] ) tmax = th[1];
		if ( tmax < th[2] ) tmax = th[2];

		tmin = th[0];
		if ( tmin > th[1] ) tmin = th[1];
		if ( tmin > th[2] ) tmin = th[2];

		double s = (tmax - 60.0)/(180.0 - 60.0);
		double t = (60.0 - tmin)/60.0;

		if ( t > s ) return t;
		return s;
	}
};

class flistS
{
public:
	int face_id;
	int edge;
	bool start;
	bool chk[2];	//ファセットの２エッジが隣接関係にあるかのチェックフラグ
};


class edge_collapse_target
{
public:
	int f1;
	int f2;
	int e1;
	int e2;
	double cost;
	double c;
	bool x;
	
	edge_collapse_target()
	{
		x = false;
	}

	bool operator < (const edge_collapse_target &a)const{
		return cost < a.cost;
	}
	std::vector<flistS> flist;
};


class EdgeCollapse
{
private:
	Solid* solid_;
	std::vector<simFace> faceList;
	int FaceNum;

	void Construction();
	void Adjacency();

	bool isSurrounded(int face);

	void AroundFacet(simEdge& edge, std::vector<flistS>& flist);
	bool ChecksAdjacency(std::vector<flistS>& flist);

	bool CheckFaceEdge(int face, double tol2, double angle, edge_collapse_target& target);

	double CalcCost(mlVector3D& vtx1, mlVector3D& vtx2, mlVector3D& midVtx, edge_collapse_target& target);
	bool EdgeCollapse::Collapse(double angle, edge_collapse_target& target, int& collapse);

public:
	EdgeCollapse(Solid* solid)
	{
		solid_ = solid;

		const int sz = solid_->indices.GetSize();
		FaceNum = sz/3;
	}

	int LoockUpTarget( int face );
	int Simplify(const double collapse_tol=0.5, const double angle = 0.96, const double tol=0.001);

};


#endif // SOLID_H
