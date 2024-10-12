#ifndef _EXPRT_H
#define _EXPRT_H

#  pragma warning ( disable : 4996 ) /* use _CRT_SECURE_NO_WARNINGS */

#include "Solid.h"
class ExportSolid
{
	FILE* fp;
public:

	void SaveSTLOpen(char* filename)
	{
		fp = fopen(filename, "w");
		setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);
		fprintf(fp, "sold ascii\n");
	}
	void SaveSTLClose()
	{
		fprintf(fp, "endsolid\n");
		fclose(fp);
		fp = NULL;
	}

	void SaveSTLFace(mlVector3D vtx[3])
	{
		mlVector3D nromal;
		mlVector3D vec[3];

		vec[0] = vtx[1] - vtx[0];
		vec[1] = vtx[2] - vtx[1];
		vec[2] = vtx[0] - vtx[2];
		nromal = mlVectorCross(vec[0], vec[1]).Normalised();

		fprintf(fp, "facet normal %e %e %e\n", to_double(nromal.x), to_double(nromal.y), to_double(nromal.z));
		fprintf(fp, "   outer loop\n");
		fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[0].x), to_double(vtx[0].y), to_double(vtx[0].z));
		fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[1].x), to_double(vtx[1].y), to_double(vtx[1].z));
		fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[2].x), to_double(vtx[2].y), to_double(vtx[2].z));
		fprintf(fp, "   endloop\n");
		fprintf(fp, "endfacet\n");
	}

	void SaveSTLSolid(Solid* solid)
	{
		int index[3];
		mlVector3D vtx[3];
		mlVector3D nromal;
		mlVector3D vec[3];
		for ( int i = 0; i < solid->indices.GetSize()/3; i++ )
		{

			index[0] = solid->indices.GetInt(3*i);
			index[1] = solid->indices.GetInt(3*i+1);
			index[2] = solid->indices.GetInt(3*i+2);

			vtx[0] = solid->vertices.GetVector(index[0]);
			vtx[1] = solid->vertices.GetVector(index[1]);
			vtx[2] = solid->vertices.GetVector(index[2]);

			vec[0] = vtx[1] - vtx[0];
			vec[1] = vtx[2] - vtx[1];
			vec[2] = vtx[0] - vtx[2];
			nromal = mlVectorCross(vec[0], vec[1]).Normalised();

			fprintf(fp, "facet normal %e %e %e\n", to_double(nromal.x), to_double(nromal.y), to_double(nromal.z));
			fprintf(fp, "   outer loop\n");
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[0].x), to_double(vtx[0].y), to_double(vtx[0].z));
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[1].x), to_double(vtx[1].y), to_double(vtx[1].z));
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[2].x), to_double(vtx[2].y), to_double(vtx[2].z));
			fprintf(fp, "   endloop\n");
			fprintf(fp, "endfacet\n");
		}
	}

	void SaveSTL(char* filename, Solid* solid)
	{
		FILE* fp = fopen(filename, "w");
		setvbuf(fp, NULL, _IOFBF, IOBUFFER_SIZE_MAX);
		fprintf(fp, "sold ascii\n");

		int index[3];
		mlVector3D vtx[3];
		mlVector3D nromal;
		mlVector3D vec[3];
		for ( int i = 0; i < solid->indices.GetSize()/3; i++ )
		{

			index[0] = solid->indices.GetInt(3*i);
			index[1] = solid->indices.GetInt(3*i+1);
			index[2] = solid->indices.GetInt(3*i+2);

			vtx[0] = solid->vertices.GetVector(index[0]);
			vtx[1] = solid->vertices.GetVector(index[1]);
			vtx[2] = solid->vertices.GetVector(index[2]);

			vec[0] = vtx[1] - vtx[0];
			vec[1] = vtx[2] - vtx[1];
			vec[2] = vtx[0] - vtx[2];
			nromal = mlVectorCross(vec[0], vec[1]).Normalised();

			if ( nromal.MagnitudeSquared() < 0.0000001 )
			{
				nromal = mlVectorCross(vec[1], vec[2]).Normalised();
				if ( nromal.MagnitudeSquared() < 0.0000001 )
				{
					nromal = mlVectorCross(vec[2], vec[0]).Normalised();
				}
			}

			fprintf(fp, "facet normal %e %e %e\n", to_double(nromal.x), to_double(nromal.y), to_double(nromal.z));
			fprintf(fp, "   outer loop\n");
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[0].x), to_double(vtx[0].y), to_double(vtx[0].z));
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[1].x), to_double(vtx[1].y), to_double(vtx[1].z));
			fprintf(fp, "       vertex %f %f %f\n", to_double(vtx[2].x), to_double(vtx[2].y), to_double(vtx[2].z));
			fprintf(fp, "   endloop\n");
			fprintf(fp, "endfacet\n");
		}
		fprintf(fp, "endsolid\n");
		fclose(fp);
	}
};


#endif