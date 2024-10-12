#ifndef FACES_H__
#define FACES_H__

typedef struct boundBox
{
	float min[3];
	float max[3];
} boundaryBox;

inline void getFaceBox(MeshV* mesh, int faceid, boundaryBox* box)
{
	unsigned int* id = &(mesh->pIndices[3*faceid]);
	double* p = &(mesh->pVertices[3*id[0]]);

	box->min[0] = p[0];
	box->min[1] = p[1];
	box->min[2] = p[2];
	box->max[0] = box->min[0];
	box->max[1] = box->min[1];
	box->max[2] = box->min[2];

	p = &(mesh->pVertices[3*id[1]]);
	if ( p[0] < box->min[0] ) box->min[0] = p[0];
	if ( p[1] < box->min[1] ) box->min[1] = p[1];
	if ( p[2] < box->min[2] ) box->min[2] = p[2];
	if ( p[0] > box->max[0] ) box->max[0] = p[0];
	if ( p[1] > box->max[1] ) box->max[1] = p[1];
	if ( p[2] > box->max[2] ) box->max[2] = p[2];

	p = &(mesh->pVertices[3*id[2]]);
	if ( p[0] < box->min[0] ) box->min[0] = p[0];
	if ( p[1] < box->min[1] ) box->min[1] = p[1];
	if ( p[2] < box->min[2] ) box->min[2] = p[2];
	if ( p[0] > box->max[0] ) box->max[0] = p[0];
	if ( p[1] > box->max[1] ) box->max[1] = p[1];
	if ( p[2] > box->max[2] ) box->max[2] = p[2];
}


class FaceS
{
public:
	MeshV* mesh;
	int id;
	boundaryBox box;
	bool calcbox;
	
	inline FaceS()
	{
		mesh = NULL;
		id = 0;
		calcbox = false;
	}

	inline FaceS(int fid)
	{
		mesh = NULL;
		id = fid;
		calcbox = false;
	}

	inline void Mesh(MeshV* meshPtr)
	{
		mesh = meshPtr;
	}

	inline void getBox()
	{
		if ( calcbox )
		{
			return;
		}
		getFaceBox( mesh, id, &box);
		calcbox = true;
	}
};

inline bool isBoxOverlap(boundaryBox* box1, boundaryBox* box2)
{
	if ( box1->max[0]+0.01 < box2->min[0]-0.01 || box1->max[1]+0.01 < box2->min[1]-0.01 || box1->max[2]+0.01 < box2->min[2]-0.01 )
	{
		return false;
	}
	if ( box1->min[0]-0.01 > box2->max[0]+0.01 || box1->min[1]-0.01 > box2->max[1]+0.01 || box1->min[2]-0.01 > box2->max[2]+0.01 )
	{
		return false;
	}
	return true;
}
inline bool isBoxOverlap(float min[3], float max[3], boundaryBox* box2)
{
	if ( max[0]+0.01 < box2->min[0]-0.01 || max[1]+0.01 < box2->min[1]-0.01 || max[2]+0.01 < box2->min[2]-0.01 )
	{
		return false;
	}
	if ( min[0]-0.01 > box2->max[0]+0.01 || min[1]-0.01 > box2->max[1]+0.01 || min[2]-0.01 > box2->max[2]+0.01 )
	{
		return false;
	}
	return true;
}


#endif
