#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>


int main( int argc, char** argv)
{
	if ( argc < 4 )
	{
		return -1;
	}

	char path[256];
	char baseName[256];
	strcpy(path, argv[1]);
	strcpy(baseName, argv[2]);

	int startnum = atoi(argv[3]);
	int num = startnum;
	int step = 1;

	if ( argc == 5 )
	{
		step = atoi(argv[4]);
	}

	while(1)
	{
		int tmp_n = 0;
		char filename[256];
		sprintf(filename, "%s\\%s_%04d.obj", path, baseName, num);
		FILE* fp = fopen(filename, "r");
		if ( fp == NULL )
		{
			break;
		}

		printf("obj2pov.exe \"%s\" %d_tmp\n", filename, tmp_n);
		tmp_n++;
		fclose(fp);

		char toolname[256];
			
		int toolPartNum = 0;
		for (int i = 0; i < 32; i++ )
		{
			sprintf(toolname, "%s\\%s_%04d_tool_%d_%d.obj", path, baseName, num, i, 0);
			FILE* fp = fopen(toolname, "r");
			if ( fp == NULL )
			{
				continue;
			}
			fclose(fp);
			toolPartNum = i;
			break;
		}
		if ( toolPartNum == 0 )
		{
			break;
		}
		for (int i = 0; i < toolPartNum; i++ )
		{
			sprintf(toolname, "%s\\%s_%04d_tool_%d_%d.obj", path, baseName, num, toolPartNum, i);
			printf("obj2pov.exe \"%s\" %d_tmp\n", toolname, tmp_n);
			tmp_n++;
		}

		printf("type %d_tmp > tmp\n", 0);
		printf("del %d_tmp\n", 0);
		for ( int i = 1; i < tmp_n; i++ )
		{
			printf("type %d_tmp >> tmp\n", i);
			printf("del %d_tmp\n", i);
		}

		printf("type mesh_model__begin.txt > pov\\%04d_main.pov\n", num);
		printf("type tmp >> pov\\%04d_main.pov\n", num);
		printf("type mesh_model__end.txt >> pov\\%04d_main.pov\n", num);
		printf("del tmp\n");
		num += step;
	}

	int count = 1;
	printf("echo call povray.bat %04d_main.pov %d> RenderExecution.bat\n", startnum, count);
	count++;
	for ( int i = startnum+step; i < num; i += step )
	{
		printf("echo call povray.bat %04d_main.pov %d>> RenderExecution.bat\n", i, count);
		count++;
	}

}
