#include "csg_solid_modeller.h"

#include "Solid.h"
#include "export.h"
#include "solid_geometry.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#  pragma warning ( disable : 4996 ) /* use _CRT_SECURE_NO_WARNINGS */

int CSG_library;

int CSG_solid_slice_stack_ext_coef = 1;

int CSG_solid_CallID[2] = {-1, -1};

CSG_SOLD_EXPORT void SetOCTREE_CELL_TOL_value(float d)
{
	OCTREE_CELL_TOL_value = d;
}

CSG_SOLD_EXPORT void SetRAYBOX_BOX_EXT_TOL_value(float d)
{
	RAYBOX_BOX_EXT_TOL_value = d;
}

CSG_SOLD_EXPORT void ResetCSG_solid_CallID()
{
	CSG_solid_CallID[0] = -1;
	CSG_solid_CallID[1] = -1;
}
CSG_SOLD_EXPORT void SetCSG_solid_CallID(int id1, int id2)
{
	CSG_solid_CallID[0] = id1;
	CSG_solid_CallID[1] = id2;
}
CSG_SOLD_EXPORT void GetCSG_solid_CallID(int* id1, int* id2)
{
	*id1 = CSG_solid_CallID[0];
	*id2 = CSG_solid_CallID[1];
}


CSG_SOLD_EXPORT void csg_omp_set_num_threads(int num)
{
#ifdef _OPENMP
	omp_set_num_threads(num);
#endif
}
CSG_SOLD_EXPORT int csg_omp_get_max_threads(void)
{
#ifdef _OPENMP
	return omp_get_max_threads();
#endif
	return 1;
}

CSG_SOLD_EXPORT void csg_library(int use_flag)
{
	CSG_library = use_flag;
}

CSG_SOLD_EXPORT int is_csg_library()
{
	return CSG_library;
}

CSG_SOLD_EXPORT void* uad_double_library_Init()
{
	uad_double_library* uad_double_library_ = uad_double_library_init();

	return (void*)uad_double_library_;
}

CSG_SOLD_EXPORT void uad_double_library_Term(void* uad_double_library_)
{
	uad_double_library_term((uad_double_library*)uad_double_library_);
}

CSG_SOLD_EXPORT void csg_solid_slice_stack_ext_coef(int coef)
{
	if ( coef <= 0 ) coef = 1;
	CSG_solid_slice_stack_ext_coef = coef;
}

CSG_SOLD_EXPORT int get_csg_solid_slice_stack_ext_coef(void)
{
	return CSG_solid_slice_stack_ext_coef;
}

extern void set_splitObject_retry_flag(int flg);
extern int get_splitObject_retry_flag();
CSG_SOLD_EXPORT void SolidBooleanOperationTrySet(int flg )
{
	set_splitObject_retry_flag(flg);
}
CSG_SOLD_EXPORT int SolidBooleanOperationTryGet(void)
{
	return get_splitObject_retry_flag();
}


extern void set_splitObject_time_course_limit( int limit );
extern int get_splitObject_time_course_limit(void);
CSG_SOLD_EXPORT void SolidBooleanOperationTimeOutSet(int limit )
{
	set_splitObject_time_course_limit(limit);
}
CSG_SOLD_EXPORT int SolidBooleanOperationTimeOutGet(void)
{
	return get_splitObject_time_course_limit();
}

CSG_SOLD_EXPORT void SolidColor( csgSolid* solid_base, int r, int g, int b )
{
	Solid* solid = (Solid*)solid_base;

	int color = 0;
	color = r << 16;
	color |= (g << 8);
	color |= b;

	solid->setColor( color );
}


CSG_SOLD_EXPORT void SolidName( csgSolid* solid_base, int name )
{
	Solid* solid = (Solid*)solid_base;

	solid->name_id = name;
}

CSG_SOLD_EXPORT int GetSolidName( csgSolid* solid_base )
{
	Solid* solid = (Solid*)solid_base;

	return solid->name_id;
}

CSG_SOLD_EXPORT csgSolid* CreateSolid(int num, int* index, int vrtnum, float* vertex, int* color)
{

	Solid* solid = new Solid;
	IntSet& indices = solid->refIndices();
	VectorSet& vertices = solid->refVertices();
	ColorSet& colors = solid->refColors();

	if ( color == NULL ) colors.setDefaultColor();
	else colors.setUserColor();

	vertices.Reserve(vrtnum);
	for(int i=0; i<vrtnum; i++)
	{
		vertices.AddVector(mlVector3D(vertex[3*i+0], vertex[3*i+1], vertex[3*i+2]));
		if ( color != NULL )
		{
			colors.AddColor( color[i] );
		}
	}

	indices.Assgin(num, index);
	//for(int i=0; i<num; i++)
	//{
	//	indices.AddInt(index[i]);
	//}

	return solid;
}


CSG_SOLD_EXPORT csgSolid* SolidSphere( float radius, int slices, int stacks )
{
	return (csgSolid*)SolidSphere( (mlFloat)radius,  slices*CSG_solid_slice_stack_ext_coef,  stacks*CSG_solid_slice_stack_ext_coef );
}

CSG_SOLD_EXPORT csgSolid* SolidCone( float base, float height, int slices, int stacks )
{
	return (csgSolid*)SolidCone( (mlFloat) base, (mlFloat) height, slices*CSG_solid_slice_stack_ext_coef,  stacks*CSG_solid_slice_stack_ext_coef );
}

CSG_SOLD_EXPORT csgSolid* SolidCube( float width )
{
	return (csgSolid*)SolidCube( (mlFloat) width );
}

CSG_SOLD_EXPORT csgSolid* SolidCube1( float org[3], float width )
{
	mlFloat dorg[3];

	dorg[0] = (mlFloat)org[0];
	dorg[1] = (mlFloat)org[1];
	dorg[2] = (mlFloat)org[2];

	return (csgSolid*)SolidCube( dorg, (mlFloat) width );
}

CSG_SOLD_EXPORT csgSolid* SolidCube2( float minA[3], float maxA[3], int n )
{
	mlFloat dminA[3];
	mlFloat dmaxA[3];

	dminA[0] = (mlFloat)minA[0];
	dminA[1] = (mlFloat)minA[1];
	dminA[2] = (mlFloat)minA[2];

	dmaxA[0] = (mlFloat)maxA[0];
	dmaxA[1] = (mlFloat)maxA[1];
	dmaxA[2] = (mlFloat)maxA[2];

	return (csgSolid*)SolidCube2( dminA, dmaxA, n );
}

CSG_SOLD_EXPORT csgSolid* SolidCube3( float minA[3], float length[3], int n )
{
	float maxA[3];

	maxA[0] = length[0] + minA[0];
	maxA[1] = length[1] + minA[1];
	maxA[2] = length[2] + minA[2];

	return SolidCube2( minA, maxA, n );
}

CSG_SOLD_EXPORT csgSolid* SolidCylinder2( float base, float base2, float height, int slices, int stacks )
{
	return (csgSolid*)SolidCylinder2( (mlFloat) base, (mlFloat) base2, (mlFloat) height,  slices*CSG_solid_slice_stack_ext_coef,  stacks*CSG_solid_slice_stack_ext_coef );
}

CSG_SOLD_EXPORT csgSolid* SolidCylinder( float base, float height, int slices, int stacks )
{
	return (csgSolid*)SolidCylinder( (mlFloat) base, (mlFloat) height,  slices*CSG_solid_slice_stack_ext_coef,  stacks*CSG_solid_slice_stack_ext_coef );
}

CSG_SOLD_EXPORT csgSolid* SolidTorus( float dInnerRadius, float dOuterRadius, int nSides, int nRings )
{
	return (csgSolid*)SolidTorus( (mlFloat) dInnerRadius, (mlFloat) dOuterRadius, nSides*CSG_solid_slice_stack_ext_coef, nRings*CSG_solid_slice_stack_ext_coef );
}

CSG_SOLD_EXPORT void SolidTranslate(csgSolid* solid, float t[3] )
{
	Solid* m_pSolid = (Solid*)solid;
	mlVector3D tt( t[0], t[1], t[2]);

	m_pSolid->Translate(tt);
}

CSG_SOLD_EXPORT void SolidScale(csgSolid* solid, float scale[3] )
{
	Solid* m_pSolid = (Solid*)solid;
	mlVector3D tt( scale[0], scale[1], scale[2]);

	m_pSolid->Scale(tt);
}

CSG_SOLD_EXPORT void SolidRotate(csgSolid* solid, float axis[3] )
{
	Solid* m_pSolid = (Solid*)solid;
	mlVector3D tt( axis[0], axis[1], axis[2]);

	m_pSolid->Rotate(tt);
}

CSG_SOLD_EXPORT void SolidRotateAnyAxis(csgSolid* solid, float p1[3], float p2[3], float angle )
{
	Solid* m_pSolid = (Solid*)solid;
	mlVector3D pnt1( p1[0], p1[1], p1[2]);
	mlVector3D pnt2( p2[0], p2[1], p2[2]);

	m_pSolid->Rotate(pnt1, pnt2, (mlFloat)angle);
}

CSG_SOLD_EXPORT csgSolid* SolidCopy( csgSolid* solid_base )
{
	if ( solid_base == NULL ) return NULL;
	Solid* solid = (Solid*)solid_base;

	Solid* solid0 = new Solid(&solid->vertices, &solid->indices, &solid->colors);
	solid0->name_id = solid->name_id;

	return solid0;
}

CSG_SOLD_EXPORT void DeleteSolid(csgSolid* solid )
{
	Solid* m_pSolid = (Solid*)solid;

	delete m_pSolid;
}

CSG_SOLD_EXPORT void SimplifySolid(csgSolid* solid )
{
	Solid* m_pSolid = (Solid*)solid;

	m_pSolid->Simplify();
}
CSG_SOLD_EXPORT int SimplifySolid2(csgSolid* solid, const double collapse_tol, const double angle, const double tol )
{
	Solid* m_pSolid = (Solid*)solid;

	EdgeCollapse edge_collapse( m_pSolid );

	return edge_collapse.Simplify(collapse_tol, angle, tol);
}

CSG_SOLD_EXPORT csgBooleanModeller* CreateSolidModeller(csgSolid* solid1, csgSolid* solid2, int* status)
{
	BooleanModeller* Modeller;
	*status = 0;

	Modeller = new BooleanModeller((Solid*)solid1, (Solid*)solid2);
	if ( Modeller == NULL || (Modeller != NULL && !Modeller->create_status) )
	{
		*status = -1;
		if ( Modeller )
		{
			if ( Modeller->time_out && status ) *status = -2;
			delete Modeller;
		}
		Modeller = NULL;
	}

	return (csgBooleanModeller*)Modeller;
}

CSG_SOLD_EXPORT void DeleteSolidModeller(csgBooleanModeller* Modeller)
{
	delete (BooleanModeller*)Modeller;
}

CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_get1(csgBooleanModeller* Modeller)
{
	BooleanModeller* modeller = (BooleanModeller*)Modeller;

	return (void*)modeller->get1();
}
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_get2(csgBooleanModeller* Modeller)
{
	BooleanModeller* modeller = (BooleanModeller*)Modeller;

	return (void*)modeller->get2();
}

CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Union(csgBooleanModeller* Modeller)
{
	BooleanModeller* modeller = (BooleanModeller*)Modeller;

	return (void*)modeller->getUnion();

}
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Intersection(csgBooleanModeller* Modeller)
{
	BooleanModeller* modeller = (BooleanModeller*)Modeller;

	return (void*)modeller->getIntersection();
}

CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Difference(csgBooleanModeller* Modeller)
{
	BooleanModeller* modeller = (BooleanModeller*)Modeller;

	return (void*)modeller->getDifference();
}

CSG_SOLD_EXPORT void SolidTriangleList(csgSolid* solid , int* num, float** vertex, float** normal)
{
	Solid* m_pSolid = (Solid*)solid;

	*num = 0;
	*normal = NULL;
	*vertex = NULL;

	int index[3];
	mlVector3D vtx[3];
	mlVector3D vnromal;

	*num = m_pSolid->indices.GetSize()/3;
	if ( *num == 0 )
	{
		return;
	}

	*vertex = new float [9*(*num)+1];
	*normal = new float [3*(*num)+1];

	mlVector3D vec[3];
	for ( int i = 0; i < *num; i++ )
	{

		index[0] = m_pSolid->indices.GetInt(3*i);
		index[1] = m_pSolid->indices.GetInt(3*i+1);
		index[2] = m_pSolid->indices.GetInt(3*i+2);

		vtx[0] = m_pSolid->vertices.GetVector(index[0]);
		vtx[1] = m_pSolid->vertices.GetVector(index[1]);
		vtx[2] = m_pSolid->vertices.GetVector(index[2]);

		vec[0] = vtx[1] - vtx[0];
		vec[1] = vtx[2] - vtx[1];
		vec[2] = vtx[0] - vtx[2];
		vnromal = mlVectorCross(vec[0], vec[1]).Normalised();

		if ( vnromal.MagnitudeSquared() < 0.0000001 )
		{
			vnromal = mlVectorCross(vec[1], vec[2]).Normalised();
			if ( vnromal.MagnitudeSquared() < 0.0000001 )
			{
				vnromal = mlVectorCross(vec[2], vec[0]).Normalised();
			}
		}

		(*vertex)[9*i+0] = to_double(vtx[0].x);
		(*vertex)[9*i+1] = to_double(vtx[0].y);
		(*vertex)[9*i+2] = to_double(vtx[0].z);

		(*vertex)[9*i+3] = to_double(vtx[1].x);
		(*vertex)[9*i+4] = to_double(vtx[1].y);
		(*vertex)[9*i+5] = to_double(vtx[1].z);

		(*vertex)[9*i+6] = to_double(vtx[2].x);
		(*vertex)[9*i+7] = to_double(vtx[2].y);
		(*vertex)[9*i+8] = to_double(vtx[2].z);

		(*normal)[3*i+0] = to_double(vnromal.x);
		(*normal)[3*i+1] = to_double(vnromal.y);
		(*normal)[3*i+2] = to_double(vnromal.z);
	}
}
CSG_SOLD_EXPORT void DeleteSolidTriangleList(float* vertex, float* normal)
{
	delete [] normal;
	delete [] vertex;
}


CSG_SOLD_EXPORT void SolidVertexBuffer(csgSolid* solid , int* num, int** index, int* vrtnum, float** vertex, int** vertex_color)
{
	Solid* m_pSolid = (Solid*)solid;
	bool vertex_color_use = false;

	*num = 0;
	*vrtnum = 0;
	*index = NULL;
	*vertex = NULL;

	//色を取得して欲しい指定
	if ( vertex_color != NULL ) 
	{
		vertex_color_use = true;
		*vertex_color = NULL;
	}

	*num = (int)m_pSolid->indices.m_pInts.size();
	*vrtnum = (int)m_pSolid->vertices.m_pVectors.size();

	*index = new int[*num];
	*vertex = new float[*vrtnum*3];

	bool default_color_use = m_pSolid->colors.isDefaultColor();

	//色を取得して欲しい指定があって、色が設定されているなら
	if ( vertex_color_use && !default_color_use)
	{
		*vertex_color =  new int[*vrtnum*3];
	}else{
		vertex_color_use = false;
	}

	for ( int i = 0; i < *num; i++ )
	{
		(*index)[i] = m_pSolid->indices.m_pInts[i];
	}

	for ( int i = 0; i < *vrtnum; i++ )
	{
		(*vertex)[3*i+0] = to_double(m_pSolid->vertices.m_pVectors[i].x);
		(*vertex)[3*i+1] = to_double(m_pSolid->vertices.m_pVectors[i].y);
		(*vertex)[3*i+2] = to_double(m_pSolid->vertices.m_pVectors[i].z);
		if ( vertex_color_use )
		{
			(*vertex_color)[i] = m_pSolid->colors.m_pColors[i].color;
		}
	}
}

CSG_SOLD_EXPORT void DeleteSolidVertexBuffer(int* index, float* vertex, int* vertex_color)
{
	delete [] index;
	delete [] vertex;
	if ( vertex_color ) delete [] vertex_color;
}

CSG_SOLD_EXPORT void SolidNormalVectors(csgSolid* solid, float** normal)
{
	if ( solid == NULL )
	{
		return;
	}
	Solid* solid_org = (Solid*)solid;

	int sz =  solid_org->indices.m_pInts.size()/3;

	*normal = new float[sz*3];
	for (int i = 0; i < sz; i++ )
	{
		const mlVector3D& p1 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+0]];
		const mlVector3D& p2 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+1]];
		const mlVector3D& p3 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+2]];

		color_attr_type color_dmy;
		Vertex v1(p1, &(color_dmy));
		Vertex v2(p2, &(color_dmy));
		Vertex v3(p3, &(color_dmy));

		Face f(&v1, &v2, &v3);
		mlVector3D normalv = f.getNormal();

		(*normal)[3*i+0] = to_double(normalv.x);
		(*normal)[3*i+1] = to_double(normalv.y);
		(*normal)[3*i+2] = to_double(normalv.z);
	}
}

CSG_SOLD_EXPORT void DeleteSolidNormalVectors(float* normal)
{
	if ( normal ) delete [] normal;
}

CSG_SOLD_EXPORT void ExportSolidSTL(char* filename, csgSolid* solid)
{
	ExportSolid export_solid;

	export_solid.SaveSTL(filename, (Solid*)solid);
}

CSG_SOLD_EXPORT csgSolid* SolidSweep(csgSolid* solid, int num, float* pos, float* dir)
{
	Solid* solid_org = (Solid*)solid;

	Solid* solid0 = (Solid*)SolidCopy(solid_org);
	SolidRotate(solid0, dir);
	SolidTranslate(solid0, pos);

	for ( int i = 1; i < num; i++ )
	{
		//printf("[%d]\n", i);
		Solid* solid1 = (Solid*)SolidCopy(solid_org);
		SolidRotate(solid1, &(dir[3*i]));
		SolidTranslate(solid1, &(pos[3*i]));

		BooleanModeller* modeller = new BooleanModeller(solid0, solid1);
		if ( modeller == NULL || (modeller != NULL && !modeller->create_status) )
		{
			printf("%d error skipp.\n", i);	fflush(stdout);

			delete solid1;
			delete modeller;
			continue;
		}

		Solid* solid2 = modeller->getUnion();
		delete solid0;
		delete modeller;
		delete solid1;
		solid0 = solid2;
	}

	return solid0;

}

CSG_SOLD_EXPORT csgSolid* SolidLinerSweep(csgSolid* solid, int num, float s_pos[3], float s_dir[3], float e_pos[3], float e_dir[3])
{
	return SolidLinerSweepEx( solid, num, s_pos,  s_dir,  e_pos,  e_dir, 0, 0);
}

CSG_SOLD_EXPORT csgSolid* SolidLinerSweepEx(csgSolid* solid, int num, float s_pos[3], float s_dir[3], float e_pos[3], float e_dir[3], int Simplify, int debugExprt)
{
	Solid* solid_org = (Solid*)solid;

	Solid* solid0 = (Solid*)SolidCopy(solid_org);;
	SolidRotate(solid0, s_dir);
	SolidTranslate(solid0, s_pos);

	float dt = 1.0f/(float)num;

	float t = 0.0f;
	float pos[3], dir[3];
	for ( int i = 1; i < num; i++ )
	{
		t += dt;
		pos[0] = e_pos[0]*t + s_pos[0]*(1.0f - t);
		pos[1] = e_pos[1]*t + s_pos[1]*(1.0f - t);
		pos[2] = e_pos[2]*t + s_pos[2]*(1.0f - t);

		dir[0] = e_dir[0]*t + s_dir[0]*(1.0f - t);
		dir[1] = e_dir[1]*t + s_dir[1]*(1.0f - t);
		dir[2] = e_dir[2]*t + s_dir[2]*(1.0f - t);

		//printf("[%d]\n", i);
		Solid* solid1 = (Solid*)SolidCopy(solid_org);;
		SolidRotate(solid1, dir);
		SolidTranslate(solid1, pos);

		BooleanModeller* modeller = new BooleanModeller(solid0, solid1);
		if ( modeller == NULL || (modeller != NULL && !modeller->create_status) )
		{
			printf("%d error skipp.\n", i);	fflush(stdout);

			delete solid1;
			delete modeller;
			continue;
		}

		Solid* solid2 = modeller->getUnion();
		if ( Simplify )
		{
			solid2->Simplify();
		}
		if ( debugExprt )
		{
			char fname[256];
			sprintf(fname, "debug_sweep_%03d.obj", i);
			SaveSolid(solid2, fname);
		}
		delete solid0;
		delete modeller;
		delete solid1;
		solid0 = solid2;
	}

	Solid* solid1 = (Solid*)SolidCopy(solid_org);;
	SolidRotate(solid1, e_dir);
	SolidTranslate(solid1, e_pos);

	BooleanModeller* modeller = new BooleanModeller(solid0, solid1);
	if ( modeller == NULL )
	{
		printf("error skipp.\n");	fflush(stdout);

		delete solid1;
		delete modeller;
		return solid0;
	}

	Solid* solid2 = modeller->getUnion();
	delete solid0;
	delete solid1;
	delete modeller;
	solid0 = solid2;

	return solid0;
}


CSG_SOLD_EXPORT void SaveSolid(csgSolid* solid, char* filename)
{
	if ( solid == NULL )
	{
		return;
	}
	Solid* solid_org = (Solid*)solid;

	FILE* fp = fopen(filename, "w");
	if ( fp == NULL )
	{
		return;
	}
	setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);
	fprintf(fp, "# solid\n");
	fprintf(fp, "# Vertices: %d\n", (int)solid_org->vertices.m_pVectors.size());
	fprintf(fp, "# Face: %d\n", (int)solid_org->indices.m_pInts.size()/3);

	bool is_default_color = solid_org->colors.isDefaultColor();

	if (!is_default_color)
	{
		int color = solid_org->colors.m_pColors[0].color;
		int r, g, b;
		solid_org->colors.ToRGB( color, &r, &g, &b);
		fprintf(fp, "# RGB: %d %d %d\n", r, g, b);
	}

	int sz = solid_org->vertices.m_pVectors.size();
	for (int i = 0; i < sz; i++ )
	{
		if (!is_default_color)
		{
			int color = solid_org->colors.m_pColors[i].color;
			unsigned char attr = solid_org->colors.m_pColors[i].attr;
			int r, g, b;
			solid_org->colors.ToRGB( color, &r, &g, &b);

			fprintf(fp, "v %.16f %.16f %.16f %d %d %d\n",  
				to_double(solid_org->vertices.m_pVectors[i].x), 
				to_double(solid_org->vertices.m_pVectors[i].y), 
				to_double(solid_org->vertices.m_pVectors[i].z),
				r, g, b);
		}else{
			fprintf(fp, "v %.16f %.16f %.16f\n",  
				to_double(solid_org->vertices.m_pVectors[i].x), 
				to_double(solid_org->vertices.m_pVectors[i].y), 
				to_double(solid_org->vertices.m_pVectors[i].z));
		}

	}

	bool use_color = solid_org->colors.m_pColors.size() != 0;

	sz =  solid_org->indices.m_pInts.size()/3;
	for (int i = 0; i < sz; i++ )
	{
		const mlVector3D& p1 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+0]];
		const mlVector3D& p2 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+1]];
		const mlVector3D& p3 = solid_org->vertices.m_pVectors[solid_org->indices.m_pInts[3*i+2]];

		color_attr_type color1;
		color_attr_type color2;
		color_attr_type color3;
		if ( use_color )
		{
			color1 = solid_org->colors.m_pColors[solid_org->indices.m_pInts[3*i+0]];
			color2 = solid_org->colors.m_pColors[solid_org->indices.m_pInts[3*i+1]];
			color3 = solid_org->colors.m_pColors[solid_org->indices.m_pInts[3*i+2]];
		}

		Vertex v1(p1, &(color1));
		Vertex v2(p2, &(color2));
		Vertex v3(p3, &(color3));

		Face f(&v1, &v2, &v3);
		mlVector3D normal = f.getNormal();
		fprintf(fp, "vn %.16f %.16f %.16f\n",  to_double(normal.x), to_double(normal.y), to_double(normal.z));
	}

	for (int i = 0; i < sz; i++ )
	{
		fprintf(fp, "f %d//%d %d//%d %d//%d\n",  solid_org->indices.m_pInts[3*i+0]+1, i+1, solid_org->indices.m_pInts[3*i+1]+1, i+1, solid_org->indices.m_pInts[3*i+2]+1, i+1);
	}

	if (!is_default_color)
	{
		fprintf(fp, "#### VertexAttribute\n");
		int sz = solid_org->vertices.m_pVectors.size();
		for (int i = 0; i < sz; i++ )
		{
			unsigned char attr = solid_org->colors.m_pColors[i].attr;
			fprintf(fp, "#%d %d\n", i, (int)attr);
		}
	}
	fclose(fp);
}

struct _color_value
{
	float r;
	float g;
	float b;
};

CSG_SOLD_EXPORT csgSolid* LoadSolid(char* filename)
{
	int index[3];
	double vertex[3];

	FILE* fp = fopen(filename, "r");
	if ( fp == NULL )
	{
		printf("open error[%s]\n", filename);
		return NULL;
	}
	setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);

	printf("Load start [%s]", filename);
	VectorSet vertices;
	IntSet indices;
	ColorSet colors;

	char buf[1024];
	while(1)
	{
		fgets(buf, 1024, fp);
		if ( strncmp(buf, "v ", 2) == 0) break;
	}

	std::vector<struct _color_value> tmp_color;
	bool file_end = false;
	while(1)
	{
		double rr, gg, bb;
		int r, g, b;
		int attr = 0;
		color_attr_type color;
		int n = sscanf(buf, "v %lf %lf %lf %d %d %d", &(vertex[0]), &(vertex[1]), &(vertex[2]), &r, &g, &b);
		if (n != 6)
		{
			n = sscanf(buf, "v %lf %lf %lf %lf %lf %lf", &(vertex[0]), &(vertex[1]), &(vertex[2]), &rr, &gg, &bb);
			if (n == 6)
			{
				struct _color_value tmp;
				tmp.r = rr;
				tmp.g = gg;
				tmp.b = bb;
				tmp_color.push_back(tmp);
			}
		}else
		{
				struct _color_value tmp;
				tmp.r = r;
				tmp.g = g;
				tmp.b = b;
				tmp_color.push_back(tmp);
		}

		if (n != 6)
		{
			n = sscanf(buf, "v %lf %lf %lf",  &(vertex[0]), &(vertex[1]), &(vertex[2]));
		}
		//printf("n = %d\n", n);

		vertices.m_pVectors.push_back(mlVector3D(mlFloat(vertex[0]), mlFloat(vertex[1]), mlFloat(vertex[2])));
		do{
			if (fgets(buf, 1024, fp) == NULL)
			{
				file_end = true;
				break;
			}
		} while (buf[0] != 'v' || buf[0] == 'v' && (buf[1] == 'n' || buf[1] == 't'));
		if ( strncmp(buf, "v ", 2) != 0) break;
		if (file_end) break;
	}

	const int sz = tmp_color.size();
	bool color_float = false;
	for (int i = 0; i < sz; i++)
	{
		if (tmp_color[i].r < 1.0 && tmp_color[i].r > 0.0)
		{
			color_float = true;
			break;
		}
		if (tmp_color[i].g < 1.0 && tmp_color[i].g > 0.0)
		{
			color_float = true;
			break;
		}
		if (tmp_color[i].b < 1.0 && tmp_color[i].b > 0.0)
		{
			color_float = true;
			break;
		}
	}
	if (color_float)
	{
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		for (int i = 0; i < sz; i++)
		{
			tmp_color[i].r *= 255.0;
			tmp_color[i].g *= 255.0;
			tmp_color[i].b *= 255.0;
		}
	}
	for (int i = 0; i < sz; i++)
	{
		color_attr_type color;
		color.color = colors.ToInt(tmp_color[i].r, tmp_color[i].g, tmp_color[i].b);
		colors.m_pColors.push_back(color);
	}

	//printf("colors %d\n", colors.GetSize());
	if ( colors.GetSize() == 0 ) colors.setDefaultColor();
	else colors.setUserColor();

	fclose(fp);
	fp = fopen(filename, "r");

	file_end = false;
	buf[0] = '\0';
	while(1)
	{
		if ( buf[0] == 'f' ) break;
		fgets(buf, 1024, fp);
	}
	//printf("%s\n", buf);

	while(1)
	{
		int dmy1, dmy2, dmy3, dmy4, dmy5, dmy6;
		int n = sscanf(buf, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",   &(index[0]), &dmy1, &dmy2, &(index[1]), &dmy3, &dmy4, &(index[2]), &dmy5, &dmy6);
		if ( n != 9 )
		{
			n = sscanf(buf, "f %d//%d %d//%d %d//%d\n",   &(index[0]), &dmy1, &(index[1]), &dmy2, &(index[2]), &dmy3);
			if ( n != 6 )
			{
				sscanf(buf, "f %d %d %d\n",   &(index[0]), &(index[1]), &(index[2]));
			}
		}
		//printf("n = %d\n", n);
		index[0] -= 1;
		index[1] -= 1;
		index[2] -= 1;
		indices.m_pInts.push_back(index[0]);
		indices.m_pInts.push_back(index[1]);
		indices.m_pInts.push_back(index[2]);
		//printf("%d\n", indices.m_pInts.size());

		do
		{
			if (fgets(buf, 1024, fp) == NULL)
			{
				file_end = true;
				break;
			}
			//printf("%s\n", buf);
		} while (buf[0] != 'f');

		if ( buf[0] != 'f' ) break;
		if (file_end) break;
	}

	while( fgets(buf, 1024, fp) )
	{
		if ( strcmp(buf, "#### VertexAttribute\n") )
		{
			const int sz = colors.GetSize();
			for ( int i = 0; i < sz; i++ )
			{
				fgets(buf, 1024, fp);
				int id = i;
				int attr = 0;
				sscanf(buf, "#%d %d\n",  &id, &attr);
				colors.m_pColors[i].attr = (unsigned char)attr;
			}
		}
	}

	fclose(fp);

	Solid* m_solid = new Solid(&vertices, &indices, &colors);
	printf("=>Load end.\n");	fflush(stdout);


	return (csgSolid*)m_solid;
}

CSG_SOLD_EXPORT void SolidNormalizeSize(csgSolid* solid, float radius, int* num, int** index, int* vrtnum, float** resize_vertex)
{
	float* vertex = NULL; 

	SolidVertexBuffer(solid , num, index, vrtnum, &vertex, NULL);

	*resize_vertex = new float[3*(*vrtnum)];

	float* re_vertex = *resize_vertex;
	memcpy(re_vertex, vertex, 3*(*vrtnum)*sizeof(float));

	// Compute the bounding box
	float max[3], min[3];
	float size[3], mid[3];
	for(int i=0; i<3; i++){
		max[i] = re_vertex[3*0+i];
		min[i] = re_vertex[3*0+i];
		for(int j=1; j<(*vrtnum); j++){
			float v = re_vertex[3*j+i];
			if(v > max[i])			max[i] = v;
			else if(v < min[i])		min[i] = v;
		}
		size[i] = max[i] - min[i];
		mid[i] = 0.5f*(max[i] + min[i]);
	}

	float diagonal = (float)sqrt(size[0]*size[0] + size[1]*size[1] + size[2]*size[2]);
	float scale = radius/(0.5f*diagonal);

	for(int i=0; i<3; i++)
		for(int j=0; j<(*vrtnum); j++)
			re_vertex[3*j+i] = re_vertex[3*j+i] = scale*(re_vertex[3*j+i] - mid[i]);

	delete [] vertex;
}


CSG_SOLD_EXPORT void ExportSolidPov( char* filename, csgSolid* solid)
{
	FILE* fp = fopen(filename, "w");

	if ( fp == NULL ) return;
	setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);

	float* re_vertex = NULL;

	int vertexNum = 0;
	int indexNum = 0;
	int* index = NULL;

	SolidNormalizeSize(solid, 3.0, &indexNum, &index, &vertexNum, &re_vertex);
	if ( re_vertex == NULL )
	{
		fclose(fp);
		return;
	}

	fprintf(fp, "#declare Mesh_A=\n");
	fprintf(fp, "mesh2{\n");

	fprintf(fp, "  vertex_vectors{\n");
	fprintf(fp, "     %d,\n", vertexNum);
	for ( int i = 0; i < vertexNum; i++ )
	{
		fprintf(fp, "     < %f, %f, %f>", re_vertex[3*i], re_vertex[3*i+1], re_vertex[3*i+2]);
		if ( i != vertexNum-1 )
		{
			fprintf(fp, ",");
		}
		fprintf(fp,"\n");
	}
	fprintf(fp, "  }\n");
	//fprintf(fp, "  normal_vectors{\n");
	//fprintf(fp, "     %d,\n", vertexNum);
	//for ( int i = 0; i < vertexNum; i++ )
	//{
	//	fprintf(fp, "     < %f, %f, %f>", normal[3*i], normal[3*i+1], normal[3*i+2]);
	//	if ( i != vertexNum-1 )
	//	{
	//		fprintf(fp, ",");
	//	}
	//	fprintf(fp,"\n");
	//}
	//fprintf(fp, "  }\n");
	fprintf(fp, "\n");
	fprintf(fp, "  face_indices{\n");
	fprintf(fp, "     %d,\n", indexNum/3);
	for ( int i = 0; i < indexNum/3; i++ )
	{
		fprintf(fp, "<%d, %d, %d>", index[3*i], index[3*i+1], index[3*i+2]);
		if ( i != indexNum-1 )
		{
			fprintf(fp, ",");
		}
		fprintf(fp,"\n");
	}
	fprintf(fp, "  }\n");
	fprintf(fp, "}\n");
	fclose(fp);

	delete [] re_vertex;
	delete [] index;
}


CSG_SOLD_EXPORT void SaveSolidBin(csgSolid* solid, char* filename)
{
	if ( solid == NULL )
	{
		return;
	}
	Solid* solid_org = (Solid*)solid;

	FILE* fp = fopen(filename, "wb");
	if ( fp == NULL )
	{
		return;
	}
	setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);

	int sz = solid_org->vertices.m_pVectors.size();
	fwrite(&sz, sizeof(int), 1, fp);
	for (int i = 0; i < sz; i++ )
	{
		fwrite(&(solid_org->vertices.m_pVectors[i].x), sizeof(mlFloat), 1, fp);
		fwrite(&(solid_org->vertices.m_pVectors[i].y), sizeof(mlFloat), 1, fp);
		fwrite(&(solid_org->vertices.m_pVectors[i].z), sizeof(mlFloat), 1, fp);
	}

	int default_color = 0;
	if ( solid_org->colors.isDefaultColor())
	{
		default_color = 1;
		fwrite(&(default_color), sizeof(int), 1, fp);
	}else{
		fwrite(&(default_color), sizeof(int), 1, fp);
		for (int i = 0; i < sz; i++ )
		{
			fwrite(&(solid_org->colors.m_pColors[i]), sizeof(int), 1, fp);
		}
	}

	sz =  solid_org->indices.m_pInts.size()/3;
	fwrite(&sz, sizeof(int), 1, fp);
	for (int i = 0; i < sz; i++ )
	{
		fwrite(&(solid_org->indices.m_pInts[3*i+0]), sizeof(int), 1, fp);
		fwrite(&(solid_org->indices.m_pInts[3*i+1]), sizeof(mlFloat), 1, fp);
		fwrite(&(solid_org->indices.m_pInts[3*i+2]), sizeof(mlFloat), 1, fp);
	}
	fclose(fp);
}

CSG_SOLD_EXPORT csgSolid* LoadSolidBin(char* filename)
{
	int index[3];
	mlVector3D vertex;

	FILE* fp = fopen(filename, "rb");
	if ( fp == NULL )
	{
		return NULL;
	}
	setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);

	VectorSet vertices;
	IntSet indices;
	ColorSet colors;

	int sz = 0;
	fread(&sz, sizeof(int), 1, fp);
	for (int i = 0; i < sz; i++ )
	{
		fread(&(vertex.x), sizeof(mlFloat), 1, fp);
		fread(&(vertex.y), sizeof(mlFloat), 1, fp);
		fread(&(vertex.z), sizeof(mlFloat), 1, fp);
		vertices.m_pVectors.push_back(vertex);
	}

	int default_color = 0;
	fread(&default_color, sizeof(int), 1, fp);
	if ( default_color )
	{
		colors.setDefaultColor();
	}else{
		colors.setUserColor();
		for (int i = 0; i < sz; i++ )
		{
			int color;
			fread(&color, sizeof(int), 1, fp);
			colors.m_pColors.push_back(color);
		}
	}

	fread(&sz, sizeof(int), 1, fp);
	for (int i = 0; i < sz; i++ )
	{
		fread(&(index[0]), sizeof(int), 1, fp);
		fread(&(index[1]), sizeof(int), 1, fp);
		fread(&(index[2]), sizeof(int), 1, fp);
		indices.m_pInts.push_back(index[0]);
		indices.m_pInts.push_back(index[1]);
		indices.m_pInts.push_back(index[2]);
	}

	fclose(fp);

	Solid* m_solid = new Solid(&vertices, &indices, &colors);

	return (csgSolid*)m_solid;
}

CSG_SOLD_EXPORT int RGBToSolidColor(csgSolid* solid, int r, int g, int b)
{
	Solid* solid_org = (Solid*)solid;

	return solid_org->colors.ToInt(r, g, b);
}
CSG_SOLD_EXPORT void SolidColorToRGB(csgSolid* solid, int color, int* r, int* g, int* b)
{
	Solid* solid_org = (Solid*)solid;

	solid_org->colors.ToRGB(color, r, g, b);
}
