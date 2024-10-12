#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <vector>
#include <iostream>

#include "csg_solid_modeller.h"
#include <omp.h>
#include <ctype.h>
#include <Windows.h>
#include  <io.h>

#include <shlwapi.h>
#pragma comment( lib, "shlwapi.lib" )

#define NUM_PROCESS_MAX	65

void test()
{
	int nn = 200;
	float dirP[2][3];
	float ttP[2][3];

	ttP[0][0] = 0;
	ttP[0][1] = 0;
	ttP[0][2] = 70;

	ttP[1][0] = nn;
	ttP[1][1] = 0;
	ttP[1][2] = 70;

	dirP[0][0] = 1;
	dirP[0][1] = 1;
	dirP[0][2] = 1;

	dirP[1][0] = 1;
	dirP[1][1] = 1;
	dirP[1][2] = 1;

	csgSolid* solid = SolidCylinder(15, 90, 25, 15);
	csgSolid* sweept = SolidLinerSweepEx(solid, nn, ttP[0], dirP[0], ttP[1], dirP[1], 0, 1 );
}

void LinerSweep(std::vector<std::string>& files, csgSolid* solid, int num, float s_pos[3], float s_dir[3], float e_pos[3], float e_dir[3])
{
	files.clear();

	char fname[256];

	csgSolid* solid0 = SolidCopy(solid);;
	SolidRotate(solid0, s_dir);
	SolidTranslate(solid0, s_pos);

	sprintf(fname, "sweep_%03d.obj", 0);
	SaveSolid(solid0, fname);
	files.push_back(fname);
	DeleteSolid(solid0);

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

		csgSolid* solid1 = SolidCopy(solid);
		SolidRotate(solid1, dir);
		SolidTranslate(solid1, pos);
		sprintf(fname, "sweep_%03d.obj", i);
		SaveSolid(solid1, fname);
		files.push_back(fname);
		DeleteSolid(solid1);
	}

	
	
	csgSolid* solid1 = SolidCopy(solid);
	SolidRotate(solid1, e_dir);
	SolidTranslate(solid1, e_pos);
	sprintf(fname, "sweep_%03d.obj", num);
	SaveSolid(solid1, fname);
	files.push_back(fname);
	DeleteSolid(solid1);
}

int SystemCall(std::string command, PROCESS_INFORMATION* prInfo)
{
  STARTUPINFOA suInfo;
  memset(&suInfo, 0, sizeof(suInfo));
  suInfo.cb = sizeof(suInfo);

  suInfo.dwFlags = STARTF_USESHOWWINDOW;
  suInfo.wShowWindow = SW_SHOWMINNOACTIVE;//SW_SHOWMINIMIZED;

  char cmd[1024];
  strcpy(cmd, command.c_str());
  bool stat = CreateProcessA(NULL, cmd, NULL, NULL, FALSE,
                NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, NULL, NULL, &suInfo, prInfo);

  if ( stat ) return 0;
  return -1;
}


void makeUnionList(char* filename, std::vector<std::string>& files)
{

	char drive[_MAX_DRIVE];	// ドライブ名
    char dir[_MAX_DIR];		// ディレクトリ名
    char fname[_MAX_FNAME];	// ファイル名
    char ext[_MAX_EXT];		// 拡張子

    _splitpath(filename, drive, dir, fname, ext );
	std::cout << "Drive=" << drive << std::endl;
    std::cout << "Dir  =" << dir   << std::endl;
    std::cout << "Fname=" << fname << std::endl;
    std::cout << "Ext  =" << ext   << std::endl;

	int n = 0;
	char base[256];
	strcpy(base, fname);
	char* p = &(base[strlen(base)-1]);
	if (*p == '\0' ) p--;
	while( isdigit(*p) ) p--;
	p++;
	//printf("%s\n", p);
	sscanf(p, "%d", &n);
	*p = '\0';
	//printf("[%s]   [%d]\n", base, n);

	int start_no = -1;
	int end_no = -1;
	int no_form = -1;
	n = 0;
	while( n < 1000 )
	{
		char f[256];

		no_form = 0;
		sprintf(f, "%s%s%s%d%s", drive, dir, base, n, ext);
		FILE* fp = fopen(f, "r");
		if ( fp == NULL )
		{
			no_form = 1;
			sprintf(f, "%s%s%s%01d%s", drive, dir, base, n, ext);
			fp = fopen(f, "r");
		}
		if ( fp == NULL )
		{
			no_form = 2;
			sprintf(f, "%s%s%s%02d%s", drive, dir, base, n, ext);
			fp = fopen(f, "r");
		}
		if ( fp == NULL )
		{
			no_form = 3;
			sprintf(f, "%s%s%s%03d%s", drive, dir, base, n, ext);
			fp = fopen(f, "r");
		}
		if ( fp == NULL )
		{
			no_form = 4;
			sprintf(f, "%s%s%s%04d%s", drive, dir, base, n, ext);
			fp = fopen(f, "r");
		}
		if ( fp == NULL )
		{
			no_form = 5;
			sprintf(f, "%s%s%s%05d%s", drive, dir, base, n, ext);
			fp = fopen(f, "r");
		}
		n++;
		if ( fp )
		{
			fclose(fp);
			start_no = n-1;
			//printf("===>%s\n", f);
			files.push_back(std::string(f));
			break;
		}
	}

	while( true )
	{
		char f[256];

		switch(no_form)
		{
		case 0:
		sprintf(f, "%s%s%s%d%s", drive, dir, base, n, ext);	break;
		case 1:
		sprintf(f, "%s%s%s%01d%s", drive, dir, base, n, ext);break;
		case 2:
		sprintf(f, "%s%s%s%02d%s", drive, dir, base, n, ext);break;
		case 3:
		sprintf(f, "%s%s%s%03d%s", drive, dir, base, n, ext);break;
		case 4:
		sprintf(f, "%s%s%s%04d%s", drive, dir, base, n, ext);break;
		case 5:
		sprintf(f, "%s%s%s%05d%s", drive, dir, base, n, ext);break;
		}
		FILE* fp = fopen(f, "r");
		if ( fp )
		{
			fclose(fp);
			files.push_back(std::string(f));
			end_no = n;
			n++;
		}
		if ( fp == NULL ) break;
	}

	printf("%d->%d (%d)\n", start_no, end_no, end_no-start_no+1);
	if ( start_no < 0 || end_no < 0 || end_no-start_no+1 <= 1 )
	{
		files.clear();
		return;
	}
}

struct command
{
	std::string cmd;
	std::string file1;
	std::string file2;
	std::string outfile;
	PROCESS_INFORMATION pi;
	int execute;
};


void listUnion(int numProcMax, int threadNum, std::vector<std::string>& files, std::vector<std::string>& outfiles, bool deletefileFlg)
{
	if ( files.size() < 2 )
	{
		return;
	}

	char thisModulename[256];
	GetModuleFileNameA(NULL, thisModulename, 256);
	//printf("==%s\n", thisModulename);

	char drive[_MAX_DRIVE];	// ドライブ名
    char dir[_MAX_DIR];		// ディレクトリ名
    char fname[_MAX_FNAME];	// ファイル名
    char ext[_MAX_EXT];		// 拡張子

	_splitpath(files[0].c_str(), drive, dir, fname, ext );
	//std::cout << "Drive=" << drive << std::endl;
 //   std::cout << "Dir  =" << dir   << std::endl;
 //   std::cout << "Fname=" << fname << std::endl;
 //   std::cout << "Ext  =" << ext   << std::endl;

	int n = 0;
	char base[256];
	strcpy(base, fname);
	char* p = &(base[strlen(base)-1]);
	if (*p == '\0' ) p--;
	while( isdigit(*p) ) p--;
	p++;
	//printf("%s\n", p);
	sscanf(p, "%d", &n);
	*p = '\0';
	//printf("[%s]   [%d]\n", base, n);

	outfiles.clear();
	int k = 0;
	int sz = files.size();
	if ( sz%2 ) sz--;

	std::vector<int> run;
	run.resize(sz, 0);


	std::vector<struct command> cmds;
	for ( int i = 0; i < sz; i += 2 )
	{
		char out[256];
		sprintf(out,"%s%s%s_out%05d%s", drive, dir, base, k, ext);
		outfiles.push_back( std::string(out));
		k++;

		std::string cmd = "\"";
		cmd += thisModulename;
		cmd += "\"";

		cmd += " -u ";
		cmd += "\"";
		cmd += files[i];
		cmd += "\"";
		cmd += " ";
		cmd += "\"";
		cmd += files[i+1];
		cmd += "\"";
		cmd += " -o ";
		cmd += "\"";
		cmd += out;
		cmd += "\"";
		if ( deletefileFlg )
		{
			cmd += " -delete";
		}
		if ( threadNum > 0 )
		{
			char wrk[128];
			sprintf(wrk, " -thread %d", threadNum);
			cmd += wrk;
		}
		//printf("%s\n", cmd.c_str());

		struct command c;
		c.cmd = cmd;
		c.file1 = files[i];
		c.file2 = files[i+1];
		c.outfile = out;
		c.execute = 0;
		cmds.push_back( c );
	}
	if ( files.size() != sz ) outfiles.push_back( files[files.size()-1]);

	printf("Process=%d\n", cmds.size());
	bool s = false;
	int np = 0;
	do{
		const int sz = cmds.size();
		for ( int i = 0; i < sz; i ++ )
		{
			if ( np >= numProcMax )
			{
				for ( int k = 0; k < sz; k++ )
				{
					if ( cmds[k].execute == 0 || cmds[k].execute == 2 )
					{
						continue;
					}

					unsigned long rc = 0;
					GetExitCodeProcess(cmds[k].pi.hProcess, &rc);
					if ( rc == STILL_ACTIVE )
					{
						continue;
					}
					if ( _access(cmds[k].outfile.c_str(), 0) != -1)
					{
						cmds[k].execute = 2;
						np--;
					}
					if ( np == 0 || np < numProcMax) break;
				}
				if ( !(np == 0 || np < numProcMax) ) continue;
			}

			//printf("execute[%d]=>%d\n", i, (cmds[i].execute)? 1: 0);
			if ( cmds[i].execute )
			{
				continue;
			}
			if ( _access(cmds[i].file1.c_str(), 0) == -1 || _access(cmds[i].file2.c_str(), 0) == -1)
			{
				continue;
			}

			cmds[i].execute = 1;
			int stat = SystemCall(cmds[i].cmd, &(cmds[i].pi));
			if ( stat < 0 )
			{
				printf("CreateProcess error.\n");
				cmds[i].execute = 0;
				continue;
			}
			np++;
		}

		s = false;
		for ( int i = 0; i < sz; i ++ )
		{
			if ( cmds[i].execute )
			{
				continue;
			}
			s = true;
		}
	}while( s );


	do{
		const int sz = cmds.size();
		s = false;
		for ( int k = 0; k < sz; k++ )
		{
			if ( cmds[k].execute == 2 )
			{
				continue;
			}

			unsigned long rc = 0;
			GetExitCodeProcess(cmds[k].pi.hProcess, &rc);
			if ( rc == STILL_ACTIVE )
			{
				s = true;
				continue;
			}
			if ( _access(cmds[k].outfile.c_str(), 0) != -1)
			{
				cmds[k].execute = 2;
				continue;
			}
			s = true;
		}
	}while( s );

	printf("全プロセス完了\n");

}


void ListUnionMain(int numProcMax, int threadNum, char* filename, std::string& outfile)
{
	std::vector<std::string> files;
	std::vector<std::string> out;
	makeUnionList(filename, files);

	bool deletefileFlg = false;
	do{
		listUnion(numProcMax, threadNum, files, out, deletefileFlg);

		files = out;
		deletefileFlg = true;
	}while( out.size() != 1 );

	outfile = out[0];
}

int main(int argc, char** argv)
{
#if _DEBUG
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif


	char drive[_MAX_DRIVE];	// ドライブ名
    char dir[_MAX_DIR];		// ディレクトリ名
    char fname[_MAX_FNAME];	// ファイル名
    char ext[_MAX_EXT];		// 拡張子

	if ( argc < 2 )
	{
		printf("-u                  :(Union)       Merger of two objects into one\n");
		printf("-d                  :(Difference)  Subtraction of one object from another\n");
		printf("-i                  :(Intersection)Portion common to both objects\n");
		printf("-union_auto         :連番ファイル(objfilename1を元に自動決定)のソリッド全ての和(Union)を行う\n");
		printf("-np                 :-union_auto,-sweep で処理する同時プロセス数(%d以下)\n", NUM_PROCESS_MAX-1);
		printf("-thread 並列処理数   :OpenMP スレッド数の指定\n");
		printf("-delete             : File delete (objfilename1 objfilename2)\n");
		printf("-sweep              :objfilename1を元に掃引する\n");
		printf("-start_p x y z i j k   :掃引開始点(x,y,zに平行移動してZ軸を(i,j,k)に向ける回転\n");
		printf("-end_p x y z i j k     :掃引終了点(x,y,zに平行移動してZ軸を(i,j,k)に向ける回転\n");
		printf("-num                :掃引開始位置から終了位置までの分割数\n");
		printf("-time_out  msec        :計算時間タイムアウト設定\n");
		printf("-exb  拡張量           :ray-box干渉 box拡張量\n");
		printf("objfilename1 objfilename2\n");
		printf("-o outputfilename\n");
		return -1;
	}

	int time_out = -1;
	int threadNum = 0;
	int op = -1;
	char filename1[256];
	char filename2[256];
	char outputname[256];
	bool union_auto = false;
	bool sweep = false;
	bool deleteflg = false;
	int numProcMax = 3;

	float sp[3], ep[3];
	float sdir[3], edir[3];
	int num = 3;

	sp[0] = sp[1] = sp[2] = 0.0;
	ep[0] = ep[1] = ep[2] = 1.0;
	sdir[0] = sdir[1] = 0.0; sdir[2] = 1.0;
	edir[0] = edir[1] = 0.0; edir[2] = 1.0;

	filename1[0] = '\0';
	filename2[0] = '\0';
	outputname[0] = '\0';
	for ( int i = 1; i < argc; i++ )
	{
		if (std::string(argv[i]) == "-exb")
		{
			SetRAYBOX_BOX_EXT_TOL_value (-atof(argv[i + 1]));
			i++;
			continue;
		}
		if (std::string(argv[i]) == "-time_out")
		{
			time_out = atoi(argv[i + 1]);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-u" )
		{
			op = 1;
			continue;
		}
		if ( std::string(argv[i]) == "-d" )
		{
			op = 2;
			continue;
		}
		if ( std::string(argv[i]) == "-i" )
		{
			op = 3;
			continue;
		}
		if ( std::string(argv[i]) == "-thread" )
		{
			threadNum = atoi(argv[i+1]);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-o" )
		{
			strcpy(outputname, argv[i+1]);
			i++;
			continue;
		}
		if ( std::string(argv[i]) == "-union_auto" )
		{
			union_auto = true;
			continue;
		}
		if ( std::string(argv[i]) == "-sweep" )
		{
			sweep = true;
			continue;
		}
		if ( std::string(argv[i]) == "-start_p" )
		{
			i++;
			sp[0] = atof( argv[i] );
			i++;
			sp[1] = atof( argv[i] );
			i++;
			sp[2] = atof( argv[i] );
			i++;
			sdir[0] = atof( argv[i] );
			i++;
			sdir[1] = atof( argv[i] );
			i++;
			sdir[2] = atof( argv[i] );
			continue;
		}
		if ( std::string(argv[i]) == "-end_p" )
		{
			i++;
			ep[0] = atof( argv[i] );
			i++;
			ep[1] = atof( argv[i] );
			i++;
			ep[2] = atof( argv[i] );
			i++;
			edir[0] = atof( argv[i] );
			i++;
			edir[1] = atof( argv[i] );
			i++;
			edir[2] = atof( argv[i] );
			continue;
		}
		if ( std::string(argv[i]) == "-num" )
		{
			i++;
			num = atoi( argv[i] );
			continue;
		}

		if ( std::string(argv[i]) == "-delete" )
		{
			deleteflg = true;
			continue;
		}
		if ( std::string(argv[i]) == "-np" )
		{
			numProcMax = atoi(argv[i+1]);
			i++;
			continue;
		}
		
		if ( filename1[0] == '\0' )
		{
			strcpy(filename1, argv[i]);
			continue;
		}
		if ( filename1[1] != '\0' )
		{
			strcpy(filename2, argv[i]);
			continue;
		}
	}

#ifdef _OPENMP
	if ( threadNum > 0 )
	{
		omp_set_num_threads(threadNum);
	}
	printf("OpenMP : Enabled (Max # of threads = %d\n", omp_get_max_threads());
#endif

    _splitpath(filename1, drive, dir, fname, ext );
	std::cout << "Drive=" << drive << std::endl;
    std::cout << "Dir  =" << dir   << std::endl;
    std::cout << "Fname=" << fname << std::endl;
    std::cout << "Ext  =" << ext   << std::endl;

	printf("file[%s]\n", filename1);
	printf("file[%s]\n", filename2);

	char output[256];
	sprintf(output, "%s%s%s%s", drive, dir, "output", ext);

	if ( outputname[0] == '\0' ) strcpy(outputname, output);
	printf("==>[%s]\n", outputname);

	if ( numProcMax <= 0 )
	{
		numProcMax = 1;
	}
	if ( numProcMax >= NUM_PROCESS_MAX )
	{
		numProcMax = NUM_PROCESS_MAX-1;
	}

	if (time_out >= 0)
	{
		SolidBooleanOperationTimeOutSet(time_out);
	}
	csgSolid* solid1 = NULL;
	csgSolid* solid2 = NULL;
	csgBooleanModeller* Modeller = NULL;

	std::vector<std::string> sweep_files;
	if ( sweep )
	{
		csgSolid* solid =  LoadSolid(filename1);
		if ( solid == NULL )
		{
			goto err;
		}
		LinerSweep(sweep_files, solid, num, sp, sdir, ep, edir);
		if ( sweep_files.size() < 2 )
		{
			goto err;
		}
		strcpy(filename1, sweep_files[0].c_str());
		union_auto = true;
	}

	if ( union_auto )
	{
		int ts = clock();
		//test();
		std::string out;
		ListUnionMain(numProcMax, threadNum, filename1, out);

		printf("%s\n", out.c_str());
		for ( int i = 0; i < 10; i++ )
		{
			Sleep(500);
			if ( !CopyFileA( out.c_str(), outputname, FALSE))
			{
				//printf("copy error.\n");
				continue;
			}
			if ( !DeleteFileA(out.c_str()))
			{
				//printf("delete error.\n");
				continue;
			}
			break;
		}
		printf("time %d ms\n", clock() - ts);

		const int sz = sweep_files.size();
		for ( int i = 0; i < sz; i++ )
		{
			DeleteFileA(sweep_files[i].c_str());
		}
		return 0;
	}

	if ( filename1[0] == '\0' || filename2[0] == '\0' )
	{
		return -1;
	}

	void* env = uad_double_library_Init();

	if ( threadNum > 0 )
	{
		csg_omp_set_num_threads(threadNum);
	}

	solid1 = LoadSolid(filename1);
	solid2 = LoadSolid(filename2);
	Modeller = NULL;

	if ( solid1 == NULL || solid2 == NULL )
	{
		goto err;
	}

	int stat = 0;
	Modeller = CreateSolidModeller(solid1, solid2, &stat);
	if ( Modeller == NULL || stat != 0 )
	{
		printf("CreateSolidModeller error.\n");
		goto err;
	}

	csgSolid* solid3 = NULL;
	switch(op)
	{
	case 1:
		solid3 = CreateSolidModeller_Union(Modeller);
		break;
	case 2:
		solid3 = CreateSolidModeller_Difference(Modeller);
		break;
	case 3:
		solid3 = CreateSolidModeller_Intersection(Modeller);
		break;
	}

	if ( solid3 )
	{
		SaveSolid(solid3, outputname);
		if ( solid3 ) DeleteSolid(solid3);
		solid3 = NULL;

		if ( deleteflg )
		{
			DeleteFileA(filename1);
			DeleteFileA(filename2);
		}
	}

err:	;
	goto end;


end:	;
	if ( solid1 ) DeleteSolid(solid1);
	if ( solid2 ) DeleteSolid(solid2);
	if ( solid3 ) DeleteSolid(solid3);
	if ( Modeller ) DeleteSolidModeller(Modeller);

	uad_double_library_Term(env);
	return 0;
}


