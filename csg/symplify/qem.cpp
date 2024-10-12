#include "qem.hpp"

void calcQEM(Vertex* v, Face* f, double qem[3][3], double& dd)
{
	const double* n = f->Normal();
	const double* p = v->coord();
	const double a = n[0];
	const double b = n[1];
	const double c = n[2];
	const double d = -(a*p[0] + b*p[1] + c*p[2]);

	qem[0][0] = a*a;
	qem[0][1] = a*b;
	qem[0][2] = a*c;
	qem[1][0] = a*b;
	qem[1][1] = b*b;
	qem[1][2] = b*c;
	qem[2][0] = a*c;
	qem[2][1] = c*b;
	qem[2][2] = c*c;
	dd = d;
}

int invers3x3(double mat[3][3], double inv[3][3])
{
	const double a0 = mat[0][0];
	const double a1 = mat[1][0];
	const double a2 = mat[2][0];
	const double b0 = mat[0][1];
	const double b1 = mat[1][1];
	const double b2 = mat[2][1];
	const double c0 = mat[0][2];
	const double c1 = mat[1][2];
	const double c2 = mat[2][2];

	const double det = (a0*b1-a1*b0)*c2+(a2*b0-a0*b2)*c1+(a1*b2-a2*b1)*c0;
	if (fabs(det) < 1.0e-14)
	{
		return -1;
	}

	const double invdet = 1.0/det;


	inv[0][0] = (b1*c2-b2*c1)*invdet;
	inv[0][1] = -(b0*c2-b2*c0)*invdet;
	inv[0][2] = (b0*c1-b1*c0)*invdet;

	inv[1][0] = -(a1*c2-a2*c1)*invdet;
	inv[1][1] = (a0*c2-a2*c0)*invdet;
	inv[1][2] = -(a0*c1-a1*c0)*invdet;

	inv[2][0] = (a1*b2-a2*b1)*invdet;
	inv[2][1] = -(a0*b2-a2*b0)*invdet;
	inv[2][2] = (a0*b1-a1*b0)*invdet;

	return 0;
}

int calcQEM(Solid* solid, Vertex* v, std::set<Face*>& faces, double qem[3][3], double normal[3])
{
	qem[0][0] = 0.0;
	qem[0][1] = 0.0;
	qem[0][2] = 0.0;
	qem[1][0] = 0.0;
	qem[1][1] = 0.0;
	qem[1][2] = 0.0;
	qem[2][0] = 0.0;
	qem[2][1] = 0.0;
	qem[2][2] = 0.0;

	normal[0] = 0.0;
	normal[1] = 0.0;
	normal[2] = 0.0;

	std::set<Face*>::iterator it;
	for ( it = faces.begin(); it != faces.end(); ++it)
	{
		double mat[3][3];
		double d;
		const double area = solid->faceAttributeList[(*it)->id].area;

		calcQEM( v, *it, mat, d);
		qem[0][0] += area*mat[0][0];
		qem[0][1] += area*mat[0][1];
		qem[0][2] += area*mat[0][2];
		qem[1][0] += area*mat[1][0];
		qem[1][1] += area*mat[1][1];
		qem[1][2] += area*mat[1][2];
		qem[2][0] += area*mat[2][0];
		qem[2][1] += area*mat[2][1];
		qem[2][2] += area*mat[2][2];

		double* n = (*it)->Normal();
		normal[0] += area*d*n[0];
		normal[1] += area*d*n[1];
		normal[2] += area*d*n[2];
	}
	return 0;
}

int calcVertexQEM(Solid* solid, std::vector<Face*>& faceList, Vertex* v1, Vertex* v2, double new_coord[3])
{
	//’¸“_Žü‚è‚ÌFace‚ðŽæ“¾
	std::set<Face*> faces1;
	std::set<Face*> faces2;

	//’¸“_‚ÌQEM‚ðŒvŽZ‚·‚é
	double qem1[3][3], Bn1[3];
	double qem2[3][3], Bn2[3];

#pragma omp parallel
#pragma omp sections
{
#pragma omp section
	{
		VertexRoundFaces(v1, faceList, faces1);
		calcQEM(solid, v1, faces1, qem1, Bn1);
	}
#pragma omp section
	{
		VertexRoundFaces(v2, faceList, faces2);
		calcQEM(solid, v2, faces2, qem2, Bn2);
	}
}



	double qem[3][3], B[3];
	qem[0][0] = qem1[0][0] + qem2[0][0];
	qem[0][1] = qem1[0][1] + qem2[0][1];
	qem[0][2] = qem1[0][2] + qem2[0][2];
	qem[1][0] = qem1[1][0] + qem2[1][0];
	qem[1][1] = qem1[1][1] + qem2[1][1];
	qem[1][2] = qem1[1][2] + qem2[1][2];
	qem[2][0] = qem1[2][0] + qem2[2][0];
	qem[2][1] = qem1[2][1] + qem2[2][1];
	qem[2][2] = qem1[2][2] + qem2[2][2];

	B[0] = Bn1[0] + Bn2[0];
	B[1] = Bn1[1] + Bn2[1];
	B[2] = Bn1[2] + Bn2[2];

	double mat[3][3];
	if ( invers3x3(qem, mat) != 0 )
	{
		return -2;
	}

	const double a0 = mat[0][0];
	const double a1 = mat[1][0];
	const double a2 = mat[2][0];
	const double b0 = mat[0][1];
	const double b1 = mat[1][1];
	const double b2 = mat[2][1];
	const double c0 = mat[0][2];
	const double c1 = mat[1][2];
	const double c2 = mat[2][2];
	const double B0 = B[0];
	const double B1 = B[1];
	const double B2 = B[2];

	new_coord[0] =-(c0*B2+b0*B1+a0*B0);
	new_coord[1] =-(c1*B2+b1*B1+a1*B0);
	new_coord[2] =-(c2*B2+b2*B1+a2*B0);

	return 0;
}

