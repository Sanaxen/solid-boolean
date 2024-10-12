#include "mesh_symplify.hpp"
#include <omp.h>

int create_mesh_obj(const char *filename, MeshV& mesh)
{
	int i, j, f;
	unsigned int i0, i1, i2;
	double v1[3], v2[3], n[3];
	char buf[256];

	FILE *pFile = fopen(filename, "r");
	if(!pFile)
	{
		fprintf(stderr, "cannot open file!\n");
		return -1;
	}
	setvbuf(pFile, NULL, _IOFBF, 256 * 1000);
	
	while( fgets(buf, 256, pFile) != NULL )
	{
		if ( buf[0] == 'v' && (buf[1] == 't' || buf[1] == 'n') ) continue;
		if ( buf[0] == 'v' ) mesh.iNumVertices++;
		if ( buf[0] == 'f' ) mesh.iNumIndices++;
	}
	fclose(pFile);

	pFile = fopen(filename, "r");
	setvbuf(pFile, NULL, _IOFBF, 256 * 1000);


	/* read vertex data */
	mesh.iNumVertices *= 3;
	mesh.iNumIndices *= 3;
	mesh.pVertices = new double[mesh.iNumVertices];
	mesh.pNormals = new double [mesh.iNumVertices];
	mesh.pIndices = new unsigned int[mesh.iNumIndices];
	mesh.pColors = new Color[mesh.iNumVertices/3];

	mesh.faceGroup = new std::set<unsigned int>[mesh.iNumVertices];

	bool use_color = true;
	for(i=0; i<mesh.iNumVertices/3; ++i)
	{
		double x, y, z;
		int rgb[3];
		float rgbf[3];
		fgets(buf, 256, pFile);
		while( buf[0] != 'v' || (buf[0] == 'v' && (buf[1] == 't' || buf[1] == 'n')))
		{
			fgets(buf, 256, pFile);
		}
		if ( sscanf(buf, "v %lf %lf %lf %d %d %d", &x, &y, &z, rgb, rgb+1, rgb+2) != 6 )
		{
			if ( sscanf(buf, "v %lf %lf %lf %f %f %f", &x, &y, &z, rgbf, rgbf+1, rgbf+2) != 6 )
			{
				sscanf(buf, "v %lf %lf %lf", &x, &y, &z);
				use_color = false;
			}else
			{
				rgb[0] = (int)(255.0*rgbf[0]);
				rgb[1] = (int)(255.0*rgbf[1]);
				rgb[2] = (int)(255.0*rgbf[2]);
			}
		}
		mesh.pVertices[3*i] = x;
		mesh.pVertices[3*i+1] = y;
		mesh.pVertices[3*i+2] = z;
		mesh.pNormals[3*i] = 0.0;
		mesh.pNormals[3*i+1] = 0.0;
		mesh.pNormals[3*i+2] = 0.0;
		if ( use_color )
		{
			mesh.pColors[i].rgb = 0;
			mesh.pColors[i].rgb = Color_RGBToInt( rgb );
		}
	}

	if ( !use_color )
	{
		delete mesh.pColors;
		mesh.pColors = NULL;
	}
	fclose(pFile);
	pFile = fopen(filename, "r");
	setvbuf(pFile, NULL, _IOFBF, 256 * 1000);

	bool normal_read = false;
	for(i=0; i<mesh.iNumVertices/3; ++i)
	{
		fgets(buf, 256, pFile);
		while (buf[0] != 'v' && buf[1] != 'n')
		{
			if ( fgets(buf, 256, pFile) == NULL ) goto exitFor;
		}
		if ( buf[0] == 'v' && buf[1] == 'n' )
		{
			double x, y, z;
			sscanf(buf, "vn %lf %lf %lf", &x, &y, &z);
			mesh.pNormals[3*i] = x;
			mesh.pNormals[3*i+1] = y;
			mesh.pNormals[3*i+2] = z;
			normal_read = true;
		}
	}
exitFor:;
	fclose(pFile);
	pFile = fopen(filename, "r");
	setvbuf(pFile, NULL, _IOFBF, 256 * 1000);

	int faceCount = 0;
	/* read index data and average face normals */
	for(i=0; i<mesh.iNumIndices; i+=3)
	{
		fgets(buf, 256, pFile);
		while (buf[0] != 'f')
		{
			if (fgets(buf, 256, pFile) == NULL) goto exitFor2;
		}

		bool s = (sscanf(buf, "f %d %d %d", mesh.pIndices+i, mesh.pIndices+i+1, mesh.pIndices+i+2) == 3 );
		if ( !s)
		{
			int nrm[3];
			s = (sscanf(buf, "f %d//%d %d//%d %d//%d", mesh.pIndices+i, nrm, mesh.pIndices+i+1, nrm+1, mesh.pIndices+i+2, nrm+2) == 6 );
		}
		if ( !s)
		{
			int nrm[3], uv[3];
			s = (sscanf(buf, "f %d/%d/%d %d/%d/%d %d/%d/%d", mesh.pIndices+i, uv, nrm, mesh.pIndices+i+1, uv+1, nrm+1, mesh.pIndices+i+2, uv+2, nrm+2) == 9 );
		}
		if ( !s)
		{
			int nrm[3], uv[3];
			s = (sscanf(buf, "f %d/%d %d/%d %d/%d", mesh.pIndices+i, uv, mesh.pIndices+i+1, uv+1, mesh.pIndices+i+2, uv+2) == 6 );
		}
		if ( !s )
		{
			printf("obj file read error.\n");
		}

		mesh.pIndices[i] += -1;
		mesh.pIndices[i+1] += -1;
		mesh.pIndices[i+2] += -1;

		mesh.faceGroup[mesh.pIndices[i]].insert(i/3);
		mesh.faceGroup[mesh.pIndices[i + 1]].insert(i/3);
		mesh.faceGroup[mesh.pIndices[i + 2]].insert(i/3);
		faceCount++;

		if (!normal_read)
		{
			i0 = 3 * mesh.pIndices[i];
			i1 = 3 * mesh.pIndices[i+1];
			i2 = 3 * mesh.pIndices[i+2];
			VEC3_SUB(v1, mesh.pVertices+i1, mesh.pVertices+i0);
			VEC3_SUB(v2, mesh.pVertices+i2, mesh.pVertices+i0);
			VEC3_CROSS(n, v1, v2);
			VEC3_ADD(mesh.pNormals+i0, mesh.pNormals+i0, n);
			VEC3_ADD(mesh.pNormals+i1, mesh.pNormals+i1, n);
			VEC3_ADD(mesh.pNormals+i2, mesh.pNormals+i2, n);
		}
	}
exitFor2:;
	fclose(pFile);
	pFile = fopen(filename, "r");
	setvbuf(pFile, NULL, _IOFBF, 256 * 1000);

	while( mesh.pColors && fgets(buf, 256, pFile))
	{
		if ( strcmp(buf, "#### VertexAttribute\n") == 0)
		{
			mesh.vertexAttr = true;
			int i = 0;
			while( fgets(buf, 256, pFile ))
			{
				int id = 0;
				int attr = 0;
				sscanf(buf, "#%d %d\n", &id, &attr);
				mesh.pColors[i].attr = (unsigned char)attr;
				i++;
			}
		}
	}
	fclose(pFile);

	if (!normal_read )
	{
		/* normalize vertex normals */
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for(i=0; i<mesh.iNumVertices; i+=3)
			VEC3_NORMALIZE(mesh.pNormals+i);
	}
	mesh.iNumIndices /= 3;
	mesh.iNumVertices /= 3;
	mesh.Box();
	printf("vertex num %d\n", mesh.iNumVertices);
	printf("face num %d==%d\n", mesh.iNumIndices, faceCount);

	//getc(stdin);
	return 0;
}


void save_mesh_obj(char* filename, MeshV& mesh)
{
	if ( mesh.iNumIndices == 0 )
	{
		return;
	}

	FILE* fp;

	if (filename != NULL )
	{
		fp = fopen( filename, "w");
	}else
	{
		return;
	}
	if ( fp == NULL )
	{
		return;
	}
	setvbuf(fp, NULL, _IOFBF, 256 * 1000);

	fprintf(fp, "# solid\n");
	fprintf(fp, "# Vertices: %d\n", mesh.iNumVertices);
	fprintf(fp, "# Face: %d\n", mesh.iNumIndices);
	if ( mesh.pColors )
	{
		int color[3];
		Color_IntToRGB(mesh.pColors[0].rgb, color);
		fprintf(fp, "# RGB: %d %d %d\n", color[0], color[1], color[2]);
	}

	//if ( fp ) fprintf(fp, "mtllib %s.mtl\n", filename);
	for ( int i = 0; i < mesh.iNumVertices; i++ )
	{
		fprintf(fp, "v %.16g %.16g %.16g", mesh.pVertices[3*i], mesh.pVertices[3*i+1], mesh.pVertices[3*i+2]);
		if ( mesh.pColors )
		{
			int rgb[3];
			Color_IntToRGB(mesh.pColors[i].rgb, rgb);
			fprintf(fp, " %d %d %d", rgb[0], rgb[1], rgb[2]);
		}
		fprintf(fp, "\n");
	}

	if ( mesh.pNormals )
	{
		for ( int i = 0; i < mesh.iNumVertices; i++ )
		{
			fprintf(fp, "vn %.16g %.16g %.16g\n", mesh.pNormals[3*i], mesh.pNormals[3*i+1], mesh.pNormals[3*i+2]);
		}
	}

	int num = 0;
	printf("%d Face ->", mesh.iNumIndices);
	//if ( fp ) fprintf(fp, "usemtl mat\n");
	for ( int i = 0; i < mesh.iNumIndices; i++ )
	{
		if ( mesh.pIndices[3*i] == mesh.pIndices[3*i+1] || mesh.pIndices[3*i] == mesh.pIndices[3*i+2] || mesh.pIndices[3*i+1] == mesh.pIndices[3*i+2] )
		{
			continue;
		}

		if (mesh.pNormals )
		{
			fprintf( fp, "f %d//%d",   mesh.pIndices[3*i]+1, mesh.pIndices[3*i]+1);
			fprintf( fp, "  %d//%d",   mesh.pIndices[3*i+1]+1, mesh.pIndices[3*i+1]+1);
			fprintf( fp, "  %d//%d", mesh.pIndices[3*i+2]+1, mesh.pIndices[3*i+2]+1);
		}else
		{
			fprintf( fp, "f %d %d %d",   mesh.pIndices[3*i]+1, mesh.pIndices[3*i+1]+1, mesh.pIndices[3*i+2]+1);
		}
		fprintf(fp, "\n");
		num++;
	}
	printf("%d Face\n", num);

	if ( mesh.vertexAttr && mesh.pColors)
	{
		fprintf(fp, "#### VertexAttribute\n");
		for ( int i = 0; i < mesh.iNumVertices; i++ )
		{
			fprintf(fp, "#%d %d\n", i, (int)mesh.pColors[i].attr);
		}
	}
	if ( fp ) fclose(fp);
	
	//{
	//	char f[256];
	//	strcpy(f, filename);
	//	strcat(f, ".mtl");

	//	fp = fopen(f, "w");
	//	if ( fp ) fprintf(fp, "newmtl mat\n"
	//				"map_Kd checker_1k.bmp\n"
	//				"Ka 0.25000 0.25000 0.25000\n"
	//				"Kd 1.00000 1.00000 1.00000\n"
	//				"Ks 1.00000 1.00000 1.00000\n"
	//				"Ns 5.00000\n");
	//	if ( fp ) fclose(fp);
	//}
}

void dump_mesh_obj_faceColor(char* filename, MeshV& mesh, int* faceColor)
{
	if ( mesh.iNumIndices == 0 )
	{
		return;
	}

	FILE* fp;

	if (filename != NULL )
	{
		fp = fopen( filename, "w");
	}else
	{
		return;
	}
	if ( fp == NULL )
	{
		return;
	}

	int vertex_count = 0;
	FILE* tmpfp1 = fopen("$$face.fct", "w");
	FILE* tmpfp2 = fopen("$$vertex.vrt", "w");
	for ( int i = 0; i < mesh.iNumIndices; i++ )
	{
		if ( mesh.pIndices[3*i] == mesh.pIndices[3*i+1] || mesh.pIndices[3*i] == mesh.pIndices[3*i+2] || mesh.pIndices[3*i+1] == mesh.pIndices[3*i+2] )
		{
			continue;
		}
		
		int rgb[3];

		Color_IntToRGB(faceColor[i], rgb);
		unsigned int* id = &(mesh.pIndices[3*i]);

		for ( int j = 0; j < 3; j++ )
		{
			fprintf(tmpfp2, "v %g %g %g %d %d %d\n", 
				mesh.pVertices[3*id[j]], mesh.pVertices[3*id[j]+1], mesh.pVertices[3*id[j]+2], rgb[0], rgb[1], rgb[2]);
		}
		fprintf( tmpfp1, "f %d %d %d\n",   vertex_count+1, vertex_count+2, vertex_count+3);
		vertex_count += 3;
	}

	fclose(tmpfp1);
	fclose(tmpfp2);
	tmpfp1 = fopen("$$face.fct", "r");
	tmpfp2 = fopen("$$vertex.vrt", "r");
	char buf[256];
	while( fgets(buf, 256, tmpfp2) != NULL )
	{
		fprintf(fp, "%s", buf);
	}
	while( fgets(buf, 256, tmpfp1) != NULL )
	{
		fprintf(fp, "%s", buf);
	}
	fclose(tmpfp1);
	fclose(tmpfp2);

	remove("$$face.fct");
	remove("$$vertex.vrt");
	if ( fp ) fclose(fp);
}


void* create_mesh_obj_p( char *filename)
{
	MeshV* p = new MeshV;
	MeshV& m = (*p);

	int stat = create_mesh_obj(filename, m);
	if ( stat == 0 )
	{
		return (void*)p;
	}
	return NULL;
}

void save_mesh_obj_p(char* filename, void* mesh)
{
	MeshV* p = (MeshV*)mesh;
	MeshV& m = (*p);

	return save_mesh_obj(filename, m);
}