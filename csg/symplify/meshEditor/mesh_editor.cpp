#include "mesh_editor.hpp"


void T_JunctionSeam::calcNormal(int id, double n[3])
{
	double v1[3], v2[3];
	int i0 = 3 * mesh_->pIndices[3*id];
	int i1 = 3 * mesh_->pIndices[3*id+1];
	int i2 = 3 * mesh_->pIndices[3*id+2];
	VEC3_SUB(v1, mesh_->pVertices+i1, mesh_->pVertices+i0);
	VEC3_SUB(v2, mesh_->pVertices+i2, mesh_->pVertices+i0);
	VEC3_CROSS(n, v1, v2);
}

void T_JunctionSeam::AddVertex(double* pnt, double* nor, int color, int attr)
{
	int pre_NumVertices = mesh_->iNumVertices;

	mesh_->iNumVertices++;

	double *new_vertex = new double[3*mesh_->iNumVertices];
	memcpy(new_vertex, mesh_->pVertices, 3*sizeof(double)*pre_NumVertices);
	delete [] mesh_->pVertices;

	double *new_normal = NULL;
	if ( mesh_->pNormals != NULL )
	{
		new_normal = new double[3*mesh_->iNumVertices];
		memcpy(new_normal, mesh_->pNormals, 3*sizeof(double)*pre_NumVertices);
		delete [] mesh_->pNormals;
	}

	Color *new_color = NULL;
	
	if ( mesh_->pColors != NULL )
	{
		new_color = new Color[mesh_->iNumVertices];
		memcpy(new_color, mesh_->pColors, sizeof(Color)*pre_NumVertices);
		delete [] mesh_->pColors;
	}

	mesh_->pVertices = new_vertex;
	mesh_->pColors = new_color;
	mesh_->pNormals = new_normal;

	new_vertex[3*pre_NumVertices  ] = pnt[0];
	new_vertex[3*pre_NumVertices+1] = pnt[1];
	new_vertex[3*pre_NumVertices+2] = pnt[2];
	if ( new_normal )
	{
		new_normal[3*pre_NumVertices  ] = nor[0];
		new_normal[3*pre_NumVertices+1] = nor[1];
		new_normal[3*pre_NumVertices+2] = nor[2];
	}
	if ( new_color != NULL )
	{
		new_color[pre_NumVertices].rgb = color;
		new_color[pre_NumVertices].attr = (unsigned char)attr;
	}
}


void T_JunctionSeam::AddFace()
{
	int pre_NumIndices = mesh_->iNumIndices;
	mesh_->iNumIndices++;

	unsigned int *new_Indices = new unsigned int[3*mesh_->iNumIndices];

	memcpy(new_Indices, mesh_->pIndices, 3*sizeof(unsigned int)*(pre_NumIndices));

	delete [] mesh_->pIndices;
	mesh_->pIndices = new_Indices;
}


bool T_JunctionSeam::findConnectFace(const unsigned int edge[2], std::vector<FaceS>& NeighborhoodFaces, int curid, std::vector<NGFace>& ngFaceList)
{
	bool connect = false;
	const int sz = NeighborhoodFaces.size();
	//printf("NeighborhoodFaces %d\n", sz);

	for ( int i = 0; i < sz; i++)
	{
		unsigned int myedge[3][2];

		int id = NeighborhoodFaces[i].id;
		unsigned int* index = &(mesh_->pIndices[3*id]);

		myedge[0][0] = index[0];
		myedge[0][1] = index[1];
		myedge[1][0] = index[1];
		myedge[1][1] = index[2];
		myedge[2][0] = index[2];
		myedge[2][1] = index[0];

		
		for ( int j = 0; j < 3; j++ )
		{
			if ( edge[0] == myedge[j][1] && edge[1] == myedge[j][0] )
			{
				ngFaceList[id].connect[j] = true;
				ngFaceList[id].faceid = j;
				connect = true;
				break;
			}
		}
		if ( connect )
		{
			break;
		}
	}

	return connect;
}


void T_JunctionSeam::SeamFace(octreecell& octcell, std::vector<NGFace>& ngFaceList, int id, int edge_no, int pos, double tol, mesh_editting_info& edit_info)
{
	bool edit = false;
	
	const double zero_tol2 = ZERO_TOL*ZERO_TOL;
	const double tol2 = tol*tol;

	int IndicesTbl[3/*edge_no*/][2/*edge_no*/];

	//エッジ番号と点の位置（始点、終点)でどのインデックス（mesh.pIndices）なのかを示す
	IndicesTbl[0][0] = 0;
	IndicesTbl[0][1] = 1;
	IndicesTbl[1][0] = 1;
	IndicesTbl[1][1] = 2;
	IndicesTbl[2][0] = 2;
	IndicesTbl[2][1] = 0;

	//検査する孤立点
	int vertexid = ngFaceList[id].edge[edge_no][pos];
	double pnt[3];
	double nor[3] = {0.0, 0.0, 0.0};
	pnt[0] = mesh_->pVertices[3*vertexid];
	pnt[1] = mesh_->pVertices[3*vertexid+1];
	pnt[2] = mesh_->pVertices[3*vertexid+2];

	if ( mesh_->pNormals )
	{
		nor[0] = mesh_->pNormals[3*vertexid];
		nor[1] = mesh_->pNormals[3*vertexid+1];
		nor[2] = mesh_->pNormals[3*vertexid+2];
	}

	int color = 0;
	
	if ( mesh_->pColors ) color = mesh_->pColors[vertexid].rgb;

	int numface = mesh_->iNumIndices;
	for ( int i = 0; i < numface; i++ )
	{
		int target_face_id = i;
		unsigned int edge[3][2];

		if ( target_face_id == ngFaceList[id].faceid )
		{
			continue;
		}
		if ( ngFaceList[id].skipp )
		{
			continue;
		}

		unsigned int* index = &(mesh_->pIndices[3*i]);
		edge[0][0] = index[0];
		edge[0][1] = index[1];
		edge[1][0] = index[1];
		edge[1][1] = index[2];
		edge[2][0] = index[2];
		edge[2][1] = index[0];

		if ( vertexid == index[0] || vertexid == index[1] || vertexid == index[2] )
		{
			continue;
		}

		//隣接しているかチェック
		NGFace& targetFace = ngFaceList[target_face_id];
		if ( !targetFace.isOverlap( ngFaceList[id]))
		{
			continue;
		}

		//どのエッジに乗っているかをチェック
		double mindist = 99999999999.0;
		int edge_index = -1;

		for ( int j = 0; j < 3; j++ )
		{
			int color1 = 0;
			int color2 = 0;
			if ( mesh_->pColors )
			{
				color1 = mesh_->pColors[edge[j][0]].rgb;
				color2 = mesh_->pColors[edge[j][1]].rgb;
			}

			if ( color1 != color2 ) continue;
			double p1[3];
			double p2[3];
			p1[0] = mesh_->pVertices[3*edge[j][0]];
			p1[1] = mesh_->pVertices[3*edge[j][0]+1];
			p1[2] = mesh_->pVertices[3*edge[j][0]+2];
			p2[0] = mesh_->pVertices[3*edge[j][1]];
			p2[1] = mesh_->pVertices[3*edge[j][1]+1];
			p2[2] = mesh_->pVertices[3*edge[j][1]+2];


			double dir[3];

			dir[0] = p2[0] - p1[0]; 
			dir[1] = p2[1] - p1[1]; 
			dir[2] = p2[2] - p1[2];

			double ln = sqrt( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );
			if ( ln < 1.0e-16 )
			{
				//printf("normalize error\n");
				continue;
			}

			dir[0] /= ln;
			dir[1] /= ln;
			dir[2] /= ln;


			double vec[3];

			vec[0] = pnt[0] - p1[0];
			vec[1] = pnt[1] - p1[1];
			vec[2] = pnt[2] - p1[2];

			double t = dir[0]*vec[0] + dir[1]*vec[1] + dir[2]*vec[2];
			if ( t < 0.0 || t > ln )
			{
				continue;
			}
			double dist1 =	(p1[0] - pnt[0])*(p1[0] - pnt[0]) +
							(p1[1] - pnt[1])*(p1[1] - pnt[1]) +
							(p1[2] - pnt[2])*(p1[2] - pnt[2]);
			if ( dist1 < zero_tol2 )
			{
				continue;
				//edit = true;
				//break;
			}
			double dist2 =	(p2[0] - pnt[0])*(p2[0] - pnt[0]) +
							(p2[1] - pnt[1])*(p2[1] - pnt[1]) +
							(p2[2] - pnt[2])*(p2[2] - pnt[2]);
			if ( dist2 < zero_tol2 )
			{
				continue;
				//edit = true;
				//break;
			}

			double edge_on_p[3];

			edge_on_p[0] = p1[0] + dir[0]*t;
			edge_on_p[1] = p1[1] + dir[1]*t;
			edge_on_p[2] = p1[2] + dir[2]*t;
			double dist =	(edge_on_p[0] - pnt[0])*(edge_on_p[0] - pnt[0]) +
							(edge_on_p[1] - pnt[1])*(edge_on_p[1] - pnt[1]) +
							(edge_on_p[2] - pnt[2])*(edge_on_p[2] - pnt[2]);
			//if ( dist > tol2 )
			//{
			//	//他のエッジに乗っていないなら無関係
			//	continue;
			//}

			if ( dist < mindist )
			{
				mindist = dist;
				edge_index = j;
			}
		}

		if ( mindist > tol2 )
		{
			continue;
		}

		//このFaceのエッジには乗っていない
		if ( edge_index == -1 )
		{
			continue;
		}

		if ( edge_index == 0 )	//edge => vertex 0--> vertex 1
		{
			double org_normal[3];
			calcNormal( target_face_id, org_normal);

			unsigned int *v = &(mesh_->pIndices[3*target_face_id]);
			unsigned int vv = mesh_->pIndices[3*target_face_id+1];

			int target_face_color[3] = {0,0,0};
			if ( mesh_->pColors )
			{
				target_face_color[0] = mesh_->pColors[v[0]].rgb;
				target_face_color[1] = mesh_->pColors[v[1]].rgb;
				target_face_color[2] = mesh_->pColors[v[2]].rgb;
			}
			if ( target_face_color[0] != target_face_color[1] )
			{
				continue;
			}
			if ( target_face_color[0] != color )
			{
				AddVertex(pnt, nor, target_face_color[0], 0x1);
				vertexid = mesh_->iNumVertices-1;
			}

			//printf("%d %d %d ", v[0], v[1], v[2]);
			mesh_->pIndices[3*target_face_id+1] = vertexid;
			//printf("==>%d %d %d\n", v[0], v[1], v[2]);

			double new_normal[3];
			calcNormal( target_face_id, new_normal);

			double inner = new_normal[0]*org_normal[0]+new_normal[1]*org_normal[1]+new_normal[2]*org_normal[2];
			if ( inner < 0.0 )
			{
				printf("\n法線が反転 %.8f %.8f %.8f ==> %.8f %.8f %.8f\n", 
					org_normal[0], org_normal[1], org_normal[2],
					new_normal[0], new_normal[1], new_normal[2]);
				mesh_->pIndices[3*target_face_id+1] = v[1];
				continue;
			}

			//printf("Face分割\n");
			AddFace();
			v = &(mesh_->pIndices[3*target_face_id]);

			int fno = mesh_->iNumIndices-1;
			mesh_->pIndices[3*fno]   = vertexid;
			mesh_->pIndices[3*fno+1] = vv;
			mesh_->pIndices[3*fno+2] = v[2];

			NGFace f;
			f.faceid = fno;
			f.Mesh(mesh_);
			f.edge[0][0] = vertexid;
			f.edge[0][1] = vv;
			f.edge[1][0] = vv;
			f.edge[1][1] = v[2];
			f.edge[2][0] = v[2];
			f.edge[2][1] = vertexid;
			ngFaceList.push_back(f);
			numface = ngFaceList.size();

			FaceS fs;
			fs.id = fno;
			fs.Mesh(mesh_);
			octcell.addfaces(fs);

			//printf("add %d %d %d\n", mesh.pIndices[3*fno], mesh.pIndices[3*fno+1], mesh.pIndices[3*fno+2]);
			edit_info.split_num++;
			edit = true;
			break;
		}
		if ( edge_index == 1 )	//edge => vertex 1--> vertex 2
		{
			double org_normal[3];
			calcNormal( target_face_id, org_normal);
			unsigned int *v = &(mesh_->pIndices[3*target_face_id]);
			unsigned int vv = mesh_->pIndices[3*target_face_id+2];

			int target_face_color[3] = {0,0,0};
			if ( mesh_->pColors )
			{
				target_face_color[0] = mesh_->pColors[v[0]].rgb;
				target_face_color[1] = mesh_->pColors[v[1]].rgb;
				target_face_color[2] = mesh_->pColors[v[2]].rgb;
			}
			if ( target_face_color[1] != target_face_color[2] )
			{
				continue;
			}
			if ( target_face_color[1] != color )
			{
				AddVertex( pnt, nor, target_face_color[1], 0x1);
				vertexid = mesh_->iNumVertices-1;
			}

			//printf("%d %d %d ", v[0], v[1], v[2]);
			mesh_->pIndices[3*target_face_id+2] = vertexid;
			//printf("==>%d %d %d\n", v[0], v[1], v[2]);
			double new_normal[3];
			calcNormal( target_face_id, new_normal);

			double inner = new_normal[0]*org_normal[0]+new_normal[1]*org_normal[1]+new_normal[2]*org_normal[2];
			if ( inner < 0.0 )
			{
				printf("\n法線が反転 %.8f %.8f %.8f ==> %.8f %.8f %.8f\n", 
					org_normal[0], org_normal[1], org_normal[2],
					new_normal[0], new_normal[1], new_normal[2]);
				mesh_->pIndices[3*target_face_id+2] = vv;
				continue;
			}

			//printf("Face分割\n");
			AddFace();
			v = &(mesh_->pIndices[3*target_face_id]);

			int fno = mesh_->iNumIndices-1;
			mesh_->pIndices[3*fno]   = vertexid;
			mesh_->pIndices[3*fno+1] = vv;
			mesh_->pIndices[3*fno+2] = v[0];

			NGFace f;
			f.faceid = fno;
			f.Mesh(mesh_);
			f.edge[0][0] = vertexid;
			f.edge[0][1] = vv;
			f.edge[1][0] = vv;
			f.edge[1][1] = v[0];
			f.edge[2][0] = v[0];
			f.edge[2][1] = vertexid;
			ngFaceList.push_back(f);
			numface = ngFaceList.size();

			FaceS fs;
			fs.id = fno;
			fs.Mesh(mesh_);
			octcell.addfaces(fs);

			//printf("add %d %d %d\n", mesh.pIndices[3*fno], mesh.pIndices[3*fno+1], mesh.pIndices[3*fno+2]);
			edit_info.split_num++;
			edit = true;
			break;
		}
		if ( edge_index == 2 )	//edge => vertex 2--> vertex 0
		{
			double org_normal[3];
			calcNormal( target_face_id, org_normal);
			unsigned int *v = &(mesh_->pIndices[3*target_face_id]);
			unsigned int vv = mesh_->pIndices[3*target_face_id+2];

			int target_face_color[3] = {0,0,0};
			if ( mesh_->pColors )
			{
				target_face_color[0] = mesh_->pColors[v[0]].rgb;
				target_face_color[1] = mesh_->pColors[v[1]].rgb;
				target_face_color[2] = mesh_->pColors[v[2]].rgb;
			}
			if ( target_face_color[2] != target_face_color[0] )
			{
				continue;
			}
			if ( target_face_color[2] != color )
			{
				AddVertex( pnt, nor, target_face_color[2], 0x1);
				vertexid = mesh_->iNumVertices-1;
			}

			//printf("%d %d %d ", v[0], v[1], v[2]);
			mesh_->pIndices[3*target_face_id+2] = vertexid;
			//printf("==>%d %d %d\n", v[0], v[1], v[2]);
			double new_normal[3];
			calcNormal( target_face_id, new_normal);

			double inner = new_normal[0]*org_normal[0]+new_normal[1]*org_normal[1]+new_normal[2]*org_normal[2];
			if ( inner < 0.0 )
			{
				printf("\n法線が反転 %.8f %.8f %.8f ==> %.8f %.8f %.8f\n", 
					org_normal[0], org_normal[1], org_normal[2],
					new_normal[0], new_normal[1], new_normal[2]);
				mesh_->pIndices[3*target_face_id+2] = vv;
				continue;
			}

			//printf("Face分割\n");
			AddFace();
			v = &(mesh_->pIndices[3*target_face_id]);

			int fno = mesh_->iNumIndices-1;
			mesh_->pIndices[3*fno]   = vertexid;
			mesh_->pIndices[3*fno+1] = v[1];
			mesh_->pIndices[3*fno+2] = vv;

			NGFace f;
			f.faceid = fno;
			f.Mesh(mesh_);
			f.edge[0][0] = vertexid;
			f.edge[0][1] = v[1];
			f.edge[1][0] = v[1];
			f.edge[1][1] = vv;
			f.edge[2][0] = vv;
			f.edge[2][1] = vertexid;
			ngFaceList.push_back(f);
			numface = ngFaceList.size();

			FaceS fs;
			fs.id = fno;
			fs.Mesh(mesh_);
			octcell.addfaces(fs);

			//printf("add %d %d %d\n", mesh.pIndices[3*fno], mesh.pIndices[3*fno+1], mesh.pIndices[3*fno+2]);
			edit_info.split_num++;
			edit = true;
			break;
		}
	}
	//if (!edit) printf("編集不可\n");
}

void T_JunctionSeam::SeamNGFace( octreecell& octcell, std::vector<NGFace>& ngFaceList, int id, double tol, mesh_editting_info& edit_info )
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( ngFaceList[id].connect[i] )
		{
			continue;
		}

		SeamFace( octcell, ngFaceList, id, i, 0, tol, edit_info);
		SeamFace( octcell, ngFaceList, id, i, 1, tol, edit_info);
	}
}

void T_JunctionSeam::mesh_min_max()
{
	const int sz = mesh_->iNumVertices;

	bbox.min[0] = bbox.max[0] = mesh_->pVertices[0];
	bbox.min[1] = bbox.max[1] = mesh_->pVertices[1];
	bbox.min[2] = bbox.max[2] = mesh_->pVertices[2];
	for ( int i = 1; i < sz; i++ )
	{
		const double* p = &(mesh_->pVertices[3*i]);
		if ( p[0] < bbox.min[0] ) bbox.min[0] = p[0];
		if ( p[1] < bbox.min[1] ) bbox.min[1] = p[1];
		if ( p[2] < bbox.min[2] ) bbox.min[2] = p[2];
		if ( p[0] > bbox.max[0] ) bbox.max[0] = p[0];
		if ( p[1] > bbox.max[1] ) bbox.max[1] = p[1];
		if ( p[2] > bbox.max[2] ) bbox.max[2] = p[2];
	}
	bbox.min[0] -= 0.001;
	bbox.min[1] -= 0.001;
	bbox.min[2] -= 0.001;
	bbox.max[0] += 0.001;
	bbox.max[1] += 0.001;
	bbox.max[2] += 0.001;
}

void T_JunctionSeam::Seam(double connect_tol)
{
	const int sz = mesh_->iNumIndices;

	FaceList* faceList = new FaceList;

	faceList->face.resize(sz);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for ( int i = 0; i < sz; i++ )
	{
		faceList->face[i].id = i;
		faceList->face[i].Mesh(mesh_);
	}

	octreecell octcell;

	octcell.buildoctree(faceList, &bbox, 0);
	delete faceList;



	std::vector<NGFace> ngFaceList;
	std::vector<int> fids;

	ngFaceList.resize(sz);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for ( int i = 0; i < sz; i++)
	{
		printf("[%d]    \r\r\r\r\r", sz-i-1);
		ngFaceList[i].faceid = i;
		ngFaceList[i].Mesh(mesh_);
		unsigned int* index = &(mesh_->pIndices[3*i]);
		ngFaceList[i].edge[0][0] = index[0];
		ngFaceList[i].edge[0][1] = index[1];
		ngFaceList[i].edge[1][0] = index[1];
		ngFaceList[i].edge[1][1] = index[2];
		ngFaceList[i].edge[2][0] = index[2];
		ngFaceList[i].edge[2][1] = index[0];
	}

	int editnum = 0;
	int facenum = sz;
	printf("edge connect check\n");
	mesh_editting_info edit_info;
	for ( int i = 0; i < facenum; i++)
	{
		printf("[%d]      \r\r\r\r\r", sz-i-1);
		if ( ngFaceList[i].connect[0] && ngFaceList[i].connect[1] && ngFaceList[i].connect[2] )
		{			
			continue;
		}
		unsigned int vertexid[3];

		vertexid[0] = mesh_->pIndices[3*ngFaceList[i].faceid];
		vertexid[1] = mesh_->pIndices[3*ngFaceList[i].faceid+1];
		vertexid[2] = mesh_->pIndices[3*ngFaceList[i].faceid+2];

		if ( mesh_->vertexAttr &&  mesh_->pColors[vertexid[0]].attr == 0 && mesh_->pColors[vertexid[1]].attr == 0 && mesh_->pColors[vertexid[2]].attr == 0 )
		{
			ngFaceList[i].skipp = true;
			continue;
		}

		std::vector<FaceS> NeighborhoodFaces;
		ngFaceList[i].getBox();
		octcell.getfaces(&ngFaceList[i].box, NeighborhoodFaces, 0);
		if ( !ngFaceList[i].connect[0] ) ngFaceList[i].connect[0] = findConnectFace( ngFaceList[i].edge[0], NeighborhoodFaces, i, ngFaceList);
		if ( !ngFaceList[i].connect[1] ) ngFaceList[i].connect[1] = findConnectFace( ngFaceList[i].edge[1], NeighborhoodFaces, i, ngFaceList);
		if ( !ngFaceList[i].connect[2] ) ngFaceList[i].connect[2] = findConnectFace( ngFaceList[i].edge[2], NeighborhoodFaces, i, ngFaceList);
		
		if ( ngFaceList[i].connect[0] && ngFaceList[i].connect[1] && ngFaceList[i].connect[2] )
		{			
			continue;
		}

		editnum++;
		SeamNGFace( octcell, ngFaceList, i, connect_tol, edit_info );
		//facenum = ngFaceList.size();
	}
	printf("編集Face :%d\n", editnum);
	printf("頂点マージ1:%d\n", edit_info.merge_num1);
	printf("頂点マージ2:%d\n", edit_info.merge_num2);
	printf("Face分割:%d\n", edit_info.split_num);
	printf("--------------\n");
}

