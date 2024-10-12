#include <stdlib.h>
#include <string>
#include <iostream>

#include "mesh_symplify.hpp"
#include "mesh_editor.hpp"

int main( int argc, char** argv)
{
    char drive[_MAX_DRIVE];	// ドライブ名
    char dir[_MAX_DIR];		// ディレクトリ名
    char fname[_MAX_FNAME];	// ファイル名
    char ext[_MAX_EXT];		// 拡張子

	//char filename[] = "solid_stock_0042.obj";
	char filename[256];
	char temp[256];
	char work[256];
	double tol = 1.0e-4;
	int vertexmergeonly = 0;
	int use_vertexcolor = 1;
	int no_vertexmergeonly = 0;
	double connect_tol =  1.0e-5;
						//0.000001;

	if ( argc < 2 )
	{
		printf("-tol トレランス :頂点マージトレランス\n");
		printf("-color         :頂点カラーを使う\n");
		printf("--color        :頂点カラーは参照しない\n");
		printf("-vertexmerge   :頂点マージのみ\n");
		printf("-novertexmerge  :頂点マージは行わない\n");
		printf("ファイル名\n");
		return 0;
	}

	filename[0] = '\0';
	for ( int i = 1; i < argc; i++ )
	{
		if ( std::string(argv[i]) == "-tol" )
		{
			tol = atof(argv[i+1]);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-connect_tol" )
		{
			connect_tol = atof(argv[i+1]);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-color" )
		{
			use_vertexcolor = 1;
			continue;
		}
		if ( std::string(argv[i]) == "--color" )
		{
			use_vertexcolor = 0;
			continue;
		}
		if ( std::string(argv[i]) == "-vertexmerge" )
		{
			vertexmergeonly = 1;
			continue;
		}
		if ( std::string(argv[i]) == "-novertexmerge" )
		{
			no_vertexmergeonly = 1;
			continue;
		}
		if ( argv[i][0] != '-' )
		{
			strcpy(filename, argv[i]);
		}
	}
    _splitpath(filename, drive, dir, fname, ext );
	std::cout << "Drive=" << drive << std::endl;
    std::cout << "Dir  =" << dir   << std::endl;
    std::cout << "Fname=" << fname << std::endl;
    std::cout << "Ext  =" << ext   << std::endl;

	if ( filename[0] == '\0' )
	{
		return 0;
	}

	MeshV mesh;
	int stat = create_mesh_obj(filename, mesh);
	if ( stat != 0 )
	{
		return stat;
	}

	//if (mesh.pNormals ) delete [] mesh.pNormals;
	//mesh.pNormals = NULL;
	if ( !use_vertexcolor )
	{
		if (mesh.pColors ) delete [] mesh.pColors;
		mesh.pColors = NULL;
		mesh.vertexAttr = false;
	}
	printf("vertex num %d\n", mesh.iNumVertices);
	printf("face num %d\n", mesh.iNumIndices);


	if (!no_vertexmergeonly)
	{
		VertexMerge(mesh, tol);
	}
	
	RemoveMinAreaFace(mesh, tol);
	
	remove_unreferenced_vertex(mesh);

	sprintf(temp, "%s%s%s_vertex_merge%s", drive, dir, fname,ext);
	save_mesh_obj(temp, mesh);
	if ( vertexmergeonly )
	{
		goto END;
	}
	//exit(0);

	mesh.Clear();
	stat = create_mesh_obj(temp, mesh);
	//if (mesh.pNormals ) delete [] mesh.pNormals;
	//mesh.pNormals = NULL;


	T_JunctionSeam Editor(mesh);
	
	Editor.Seam(connect_tol);

END:	;

	printf("vertex num %d\n", mesh.iNumVertices);
	printf("face num %d\n", mesh.iNumIndices);

	sprintf(work, "%s%s%s_edit_out%s", drive, dir, fname,ext);
	save_mesh_obj(work, mesh);

	remove(temp);
	return 0;
}
