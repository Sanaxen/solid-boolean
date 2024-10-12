#ifndef _TOL_H

#define _TOL_H
//tolerance value

#if 0
#define Bound_TOL		1e-10
#define Face_TOL		1e-22
#define Line_TOL		1e-17
#define Object3D_TOL	1e-12
#define Segment_TOL		1e-18
#define Vertex_TOL		1e-10
#endif

//2011/11/13
/*
#define Bound_TOL		1e-6
#define Face_TOL		1e-12
#define Line_TOL		1e-12
#define Object3D_TOL	1e-12
#define Segment_TOL		1e-12
#define Vertex_TOL		1e-10

#define FaceArea_TOL	1e-12
#define FaceNorm_TOL	1e-16
*/

// 2012/10/01
/*
#define Bound_TOL		1e-6
#define Face_TOL		1e-12
#define Line_TOL		1e-12
#define Object3D_TOL	1e-12
#define Segment_TOL		1e-16
#define Vertex_TOL		1e-10

//#define FaceArea_TOL	1e-12
#define FaceNorm_TOL	1e-20

#define DUP_FACE_TOL	1e-16
#define DUP_FACE_TOL2	1e-18

//#define MERGE_VTX_TOL	1e-6
#define MERGE_VTX_TOL	1e-10
*/

// 2017/10/30
#if 0
#define Bound_TOL		1e-6
#define Face_TOL		1e-12
#define Line_TOL		1e-14
#define Object3D_TOL	1e-12
#define Segment_TOL		1e-16
#define Vertex_TOL		1e-14
#define Object3D_SPLIT_TOL		1e-12
#define Object3D_SPLIT_TOLMAX	1e-9

//#define FaceArea_TOL	1e-12
#define FaceNorm_TOL	1e-20

#define DUP_FACE_TOL	1e-16
#define DUP_FACE_TOL2	1e-18
#define DUP_FACE_TOL3	1e-17	/* > DUP_FACE_TOL2 */

//#define MERGE_VTX_TOL	1e-6
#define MERGE_VTX_TOL	1e-16
#endif



#define Bound_TOL		1e-6
#define Face_TOL		1e-10
#define Line_TOL		1e-12
#define Object3D_TOL	1e-10
#define Segment_TOL		1e-14
#define Vertex_TOL		1e-12
#define Object3D_SPLIT_TOL		1e-10
#define Object3D_SPLIT_TOLMAX	1e-7

//#define FaceArea_TOL	1e-12
#define FaceNorm_TOL	1e-18

#define DUP_FACE_TOL	1e-14
#define DUP_FACE_TOL2	1e-16
#define DUP_FACE_TOL3	1e-15	/* > DUP_FACE_TOL2 */

//#define MERGE_VTX_TOL	1e-6
#define MERGE_VTX_TOL	1e-14

#define SPLIT_FACSE_MAX	30
#endif