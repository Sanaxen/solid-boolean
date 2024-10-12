#ifndef _csg_solid_modeller_h
#define _csg_solid_modeller_h

#if defined(WIN32) && defined(_USRDLL)
# define CSG_SOLD_EXPORT extern "C" __declspec( dllexport )
#else
# define CSG_SOLD_EXPORT extern "C" __declspec( dllimport )
#endif

extern int CSG_library;
extern float RAYBOX_BOX_EXT_TOL_value;
extern float OCTREE_CELL_TOL_value;

#define csgSolid void
#define csgBooleanModeller void

CSG_SOLD_EXPORT void SetOCTREE_CELL_TOL_value(float d);
CSG_SOLD_EXPORT void SetRAYBOX_BOX_EXT_TOL_value(float d);

CSG_SOLD_EXPORT void ResetCSG_solid_CallID();
CSG_SOLD_EXPORT void SetCSG_solid_CallID(int id1, int id2);
CSG_SOLD_EXPORT void GetCSG_solid_CallID(int* id1, int* id2);

CSG_SOLD_EXPORT int getCPUNum();
CSG_SOLD_EXPORT int getCPUCoreNum();

CSG_SOLD_EXPORT void csg_omp_set_num_threads(int num);
CSG_SOLD_EXPORT int csg_omp_get_max_threads(void);

CSG_SOLD_EXPORT void csg_library(int use_flag);
CSG_SOLD_EXPORT int is_csg_library();

CSG_SOLD_EXPORT void* uad_double_library_Init();
CSG_SOLD_EXPORT void uad_double_library_Term(void* p);

CSG_SOLD_EXPORT void csg_solid_slice_stack_ext_coef(int coef);
CSG_SOLD_EXPORT int get_csg_solid_slice_stack_ext_coef(void);

CSG_SOLD_EXPORT void SolidBooleanOperationTrySet(int flg );
CSG_SOLD_EXPORT int SolidBooleanOperationTryGet(void);

CSG_SOLD_EXPORT void SolidBooleanOperationTimeOutSet( int limit );
CSG_SOLD_EXPORT int SolidBooleanOperationTimeOutGet(void);


CSG_SOLD_EXPORT void SolidColor( csgSolid* solid_base, int r, int g, int b );

CSG_SOLD_EXPORT void SolidName( csgSolid* solid_base, int name );
CSG_SOLD_EXPORT int GetSolidName( csgSolid* solid_base );

CSG_SOLD_EXPORT csgSolid* CreateSolid(int num, int* index, int vrtnum, float* vertex, int* color);

CSG_SOLD_EXPORT csgSolid* SolidSphere( float radius, int slices, int stacks );
CSG_SOLD_EXPORT csgSolid* SolidCone( float base, float height, int slices, int stacks );
CSG_SOLD_EXPORT csgSolid* SolidCube( float width );
CSG_SOLD_EXPORT csgSolid* SolidCube1( float org[3], float width );
CSG_SOLD_EXPORT csgSolid* SolidCube2( float minA[3], float maxA[3], int n );
CSG_SOLD_EXPORT csgSolid* SolidCube3( float minA[3], float lenth[3], int n );
CSG_SOLD_EXPORT csgSolid* SolidCylinder2( float base, float base2, float height, int slices, int stacks );
CSG_SOLD_EXPORT csgSolid* SolidCylinder( float base, float height, int slices, int stacks );
CSG_SOLD_EXPORT csgSolid* SolidTorus( float dInnerRadius, float dOuterRadius, int nSides, int nRings );

CSG_SOLD_EXPORT void SolidTranslate(csgSolid* solid, float t[3] );
CSG_SOLD_EXPORT void SolidScale(csgSolid* solid, float scale[3] );
CSG_SOLD_EXPORT void SolidRotate(csgSolid* solid, float axis[3] );
CSG_SOLD_EXPORT void SolidRotateAnyAxis(csgSolid* solid, float p1[3], float p2[3], float angle );

CSG_SOLD_EXPORT csgSolid* SolidCopy( csgSolid* solid_base );
CSG_SOLD_EXPORT void DeleteSolid(csgSolid* solid );
CSG_SOLD_EXPORT void SimplifySolid(csgSolid* solid);
CSG_SOLD_EXPORT int SimplifySolid2(csgSolid* solid, const double collapse_tol, const double angle, const double tol);

CSG_SOLD_EXPORT csgBooleanModeller* CreateSolidModeller(csgSolid* solid1, csgSolid* solid2, int* time_out); 
CSG_SOLD_EXPORT void DeleteSolidModeller(csgBooleanModeller* Modeller); 

CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_get1(csgBooleanModeller* Modeller); 
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_get2(csgBooleanModeller* Modeller); 
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Union(csgBooleanModeller* Modeller); 
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Intersection(csgBooleanModeller* Modeller); 
CSG_SOLD_EXPORT csgSolid* CreateSolidModeller_Difference(csgBooleanModeller* Modeller); 

CSG_SOLD_EXPORT void SolidTriangleList(csgSolid* solid , int* num, float** vertex, float** normal);
CSG_SOLD_EXPORT void DeleteSolidTriangleList(float* vertex, float* normal);

CSG_SOLD_EXPORT void SolidVertexBuffer(csgSolid* solid , int* num, int** index, int* vrtnum, float** vertex, int** vertex_color);
CSG_SOLD_EXPORT void SolidNormalizeSize(csgSolid* solid, float radius, int* num, int** index, int* vrtnum, float** resize_vertex);
CSG_SOLD_EXPORT void DeleteSolidVertexBuffer(int* index, float* vertex, int* vertex_color);

CSG_SOLD_EXPORT void SolidNormalVectors(csgSolid* solid, float** normal);
CSG_SOLD_EXPORT void DeleteSolidNormalVectors(float* normal);

CSG_SOLD_EXPORT void ExportSolidSTL(char* filename, csgSolid* solid);
CSG_SOLD_EXPORT void ExportSolidPov( char* filename, csgSolid* solid);

CSG_SOLD_EXPORT void SaveSolid(csgSolid* solid, char* filename);
CSG_SOLD_EXPORT void SaveSolidBin(csgSolid* solid, char* filename);
CSG_SOLD_EXPORT csgSolid* LoadSolid(char* filename);
CSG_SOLD_EXPORT csgSolid* LoadSolidBin(char* filename);

CSG_SOLD_EXPORT csgSolid* SolidSweep(csgSolid* solid, int num, float* pos, float* dir);
CSG_SOLD_EXPORT csgSolid* SolidLinerSweep(csgSolid* solid, int num, float s_pos[3], float s_dir[3], float e_pos[3], float e_dir[3]);
CSG_SOLD_EXPORT csgSolid* SolidLinerSweepEx(csgSolid* solid, int num, float s_pos[3], float s_dir[3], float e_pos[3], float e_dir[3], int Simplify, int debugExprt);

CSG_SOLD_EXPORT int RGBToSolidColor(csgSolid* solid, int r, int g, int b);
CSG_SOLD_EXPORT void SolidColorToRGB(csgSolid* solid, int color, int* r, int* g, int* b);

#endif