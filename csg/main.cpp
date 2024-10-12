#include "Solid.h"
#include "export.h"
#include "solid_geometry.h"


void test0();
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();

int main( int argc, char** argv)
{
	test0();
	//test1();
	//test2();
	//test3();
	test4();
	test5();
	test6();
	test7();
	//test8();
	//test9();

	return 0;
}

void test0()
{
	printf("test0\n");
	ExportSolid export_solid;
	Solid* m_pSolid1 = new Solid("sphere.txt");
	Solid* m_pSolid2 = new Solid("cone.txt");

	m_pSolid1->Translate(mlVector3D(0.2, -0.2, 0));
	m_pSolid1->Scale(mlVector3D(0.4, 0.3, 2.8));

	BooleanModeller* m_pModeller = new BooleanModeller(m_pSolid2, m_pSolid1);

	Solid* m_pUnionResult = m_pModeller->getUnion();
	Solid* m_pIntersectionResult = m_pModeller->getIntersection();
	Solid* m_pDifferenceResult = m_pModeller->getDifference();

	printf("              %10s %10s %10s\n", "Union", "Intersection", "Difference");
	printf("Num vertices  %10d %10d %10d\n", m_pUnionResult->vertices.GetSize(), m_pIntersectionResult->vertices.GetSize(), m_pDifferenceResult->vertices.GetSize());
	printf("Num indices   %10d %10d %10d\n", m_pUnionResult->indices.GetSize(), m_pIntersectionResult->indices.GetSize(), m_pDifferenceResult->indices.GetSize());

	export_solid.SaveSTL("test0_sphere0.stl", m_pSolid1);
	export_solid.SaveSTL("test0_cone0.stl", m_pSolid2);
	export_solid.SaveSTL("test0_Difference0.stl", m_pDifferenceResult);
	export_solid.SaveSTL("test0_Union0.stl", m_pUnionResult);
	export_solid.SaveSTL("test0_Intersection0.stl", m_pIntersectionResult);




	Solid* solid1 = SolidSphere(10, 25, 25);
	Solid* solid2 = SolidCone( 20, 30, 25, 25 );

	{
		Solid* solid5 = SolidCone( 20, 30, 25, 25 );
		solid5->Rotate(mlVector3D(0,0,-1));
		solid5->Translate(mlVector3D(0,0,30));
		export_solid.SaveSTL("test0_SolidCone3.stl", solid5);
		delete solid5;
	}

#if 0
	solid1->Scale(mlVector3D(0.1, 0.1, 0.1));
	solid2->Scale(mlVector3D(0.1, 0.1, 0.1));
	solid1->Translate(mlVector3D(0,0,0.2));
#else
	solid1->Translate(mlVector3D(0,0,0.2));
#endif
	export_solid.SaveSTL("test0_sphere2.stl", solid1);
	export_solid.SaveSTL("test0_cone2.stl", solid2);

	BooleanModeller* Modeller1 = new BooleanModeller(solid2, solid1);

	m_pUnionResult = Modeller1->getUnion();
	export_solid.SaveSTL("test0_Union.stl", m_pUnionResult);

	m_pIntersectionResult = Modeller1->getIntersection();
	export_solid.SaveSTL("test0_Intersection1.stl", m_pIntersectionResult);

	Solid* solid3 = Modeller1->getDifference();
	export_solid.SaveSTL("test0_Difference1.stl", solid3);




	Solid* solid4 = SolidCube(1000.0);
	Solid* solid5 = SolidSphere(10, 25, 25);
	solid5->Translate(mlVector3D(0,0,500));
	export_solid.SaveSTL("test0_Cube.stl", solid4);
	export_solid.SaveSTL("test0_Sphere.stl", solid5);
	BooleanModeller* Modeller2 = new BooleanModeller(solid4, solid5);
	Solid* solid6 = Modeller2->getDifference();
	export_solid.SaveSTL("test0_Difference2.stl", solid6);


	delete Modeller2;
	delete solid4;
	delete solid5;
	delete solid6;
}

void test1()
{
	printf("test1\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid5;
	Solid* solid7 = SolidCube(1000.0);

	for ( int i = 0; i < 30; i++ )
	{
		printf("[%d]\n", i);
		solid5 = SolidSphere(10, 25, 25);
		solid5->Translate(mlVector3D(i,i,500));
		
		Modeller2 = new BooleanModeller(solid7, solid5);
		if ( !Modeller2->split_status)
		{
			delete Modeller2;
			delete solid5;
			continue;
		}

		Solid* solid8 = Modeller2->getDifference();
		delete Modeller2;
		delete solid7;
		delete solid5;
		solid7 = solid8;
	}
	export_solid.SaveSTL("test1_Difference.stl", solid7);

	for ( int i = 0; i < 30; i++ )
	{
		printf("[%d]\n", i);
		solid5 = SolidSphere(5, 25, 25);
		solid5->Translate(mlVector3D(i,i,490));
		
		Modeller2 = new BooleanModeller(solid7, solid5);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", i);
			delete Modeller2;
			delete solid5;
			continue;
		}

		Solid* solid8 = Modeller2->getDifference();
		delete Modeller2;
		delete solid7;
		delete solid5;
		solid7 = solid8;
	}
	export_solid.SaveSTL("test1_Difference2.stl", solid7);
	delete solid7;
}

void test2()
{
	printf("test2\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid5;
	Solid* solid7 = SolidCube(1000.0);

	for ( int i = 0; i < 30; i++ )
	{
		printf("[%d]\n", i);
		solid5 = SolidSphere(10, 25, 25);
		solid5->Translate(mlVector3D(i,0,500));
		
		Modeller2 = new BooleanModeller(solid7, solid5);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", i);
			delete Modeller2;
			delete solid5;
			continue;
		}

		Solid* solid8 = Modeller2->getDifference();
		delete Modeller2;
		delete solid7;
		delete solid5;
		solid7 = solid8;
	}

	for ( int i = 29; i != 0; i-- )
	{
		printf("[%d]\n", i);
		solid5 = SolidSphere(10, 25, 25);
		solid5->Translate(mlVector3D(i,5,500));
		
		Modeller2 = new BooleanModeller(solid7, solid5);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", i);
			delete Modeller2;
			delete solid5;
			continue;
		}

		Solid* solid8 = Modeller2->getDifference();
		delete Modeller2;
		delete solid7;
		delete solid5;
		solid7 = solid8;
	}
	export_solid.SaveSTL("test2_Difference.stl", solid7);
	delete solid7;
}

void test3()
{
	printf("test3\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid5;
	Solid* solid7[5][5][5];
	
	for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 5; j++ )
		{
			for ( int k = 0; k < 5; k++ )
			{
				solid7[i][j][k]= SolidCube(200.0);
				solid7[i][j][k]->Translate(mlVector3D(-200+200*i,-200+200*j,-200+200*k));
			}
		}
	}

	for ( int ii = 0; ii < 60; ii++ )
	{
		printf("[%d]\n", ii);
		for ( int i = 0; i < 5; i++ )
		{
			for ( int j = 0; j < 5; j++ )
			{
				for ( int k = 0; k < 5; k++ )
				{
					solid5 = SolidSphere(50, 25, 25);
					solid5->Translate(mlVector3D(5*ii,0,700));
					
					Modeller2 = new BooleanModeller(solid7[i][j][k], solid5);
					if ( !Modeller2->split_status)
					{
						printf("%d error skipp.\n", ii);
						delete Modeller2;
						delete solid5;
						continue;
					}

					Solid* solid8 = Modeller2->getDifference();
					delete Modeller2;
					delete solid7[i][j][k];
					delete solid5;
					solid7[i][j][k] = solid8;
				}
			}
		}
	}

#if 10
	for ( int ii = 59; ii != 0; ii-- )
	{
		printf("[%d]\n", ii);
		for ( int i = 0; i < 5; i++ )
		{
			for ( int j = 0; j < 5; j++ )
			{
				for ( int k = 0; k < 5; k++ )
				{
					solid5 = SolidSphere(50, 25, 25);
					solid5->Translate(mlVector3D(5*ii,95,700));
					
					Modeller2 = new BooleanModeller(solid7[i][j][k], solid5);
					if ( !Modeller2->split_status)
					{
						printf("%d error skipp.\n", ii);
						delete Modeller2;
						delete solid5;
						continue;
					}

					Solid* solid8 = Modeller2->getDifference();
					delete Modeller2;
					delete solid7[i][j][k];
					delete solid5;
					solid7[i][j][k] = solid8;
				}
			}
		}
	}
#endif

	export_solid.SaveSTLOpen("test3_Difference.stl");
	for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 5; j++ )
		{
			for ( int k = 0; k < 5; k++ )
			{
				export_solid.SaveSTLSolid(solid7[i][j][k]);
			}
		}
	}
	export_solid.SaveSTLClose();

	for ( int i = 0; i < 5; i++ )
	{
		for ( int j = 0; j < 5; j++ )
		{
			for ( int k = 0; k < 5; k++ )
			{
				delete solid7[i][j][k];
			}
		}
	}
	export_solid.SaveSTLClose();
}

void test4()
{
	printf("test4\n");
	ExportSolid export_solid;

	Solid* solid = SolidCylinder2( 100, 30, 150, 30, 10 );

	export_solid.SaveSTL("SolidCylinder2.stl", solid);

	mlVector3D n(0,0,-1);
	solid->Rotate(n);
	mlVector3D t(0,0,150);
	solid->Translate(t);
	export_solid.SaveSTL("SolidCylinder2_2.stl", solid);

	delete solid;
}

void test5()
{
	printf("test5\n");
	ExportSolid export_solid;

	Solid* solid = SolidCylinder( 100,150, 30, 10 );

	export_solid.SaveSTL("SolidCylinder.stl", solid);
	delete solid;
}

void test6()
{
	printf("test6\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid = SolidTorus( 20,100, 30, 30 );

	mlFloat org[3]={0, 0, 0};

	Solid* solid2 = SolidCube( org, 100 );

	export_solid.SaveSTL("SolidTorus.stl", solid);
	export_solid.SaveSTL("SolidCube.stl", solid2);

	solid->Translate(mlVector3D(0,0,100));
	Modeller2 = new BooleanModeller(solid2, solid);
	Solid* solid3 = Modeller2->getDifference();

	export_solid.SaveSTL("Cube-Torus.stl", solid3);

	delete solid;
	delete solid2;
	delete solid3;
}

void test7()
{
	printf("test7\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid = SolidTorus( 20,100, 50, 50 );

	mlFloat minA[3]={0, 0, 0};
	mlFloat maxA[3]={100, 100, 100};

	Solid* solid2 = SolidCube2( minA, maxA, 30 );

	export_solid.SaveSTL("SolidTorus.stl", solid);
	export_solid.SaveSTL("SolidCube2.stl", solid2);

	solid->Translate(mlVector3D(0,0,100));
	Modeller2 = new BooleanModeller(solid2, solid);
	Solid* solid3 = Modeller2->getDifference();

	export_solid.SaveSTL("Cube2-Torus.stl", solid3);

	delete solid;
	delete solid2;
	delete solid3;
}

void test8()
{
	printf("test8\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid1;
	Solid* solid2;
	Solid* solid3;
	
	mlFloat minA[3]={0, 0, 0};
	mlFloat maxA[3]={100, 100, 100};

	solid1 = SolidCube2( minA, maxA, 5 );

	for ( int ii = 0; ii < 20; ii++ )
	{
		printf("[%d]\n", ii);
		solid2 = SolidCylinder(15, 60, 25, 15);
		solid2->Translate(mlVector3D(ii,0,70));
		
		solid3 = SolidSphere(15, 25, 15);
		solid3->Translate(mlVector3D(ii,0,70));
		
		Modeller2 = new BooleanModeller(solid1, solid2);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", ii);
			delete Modeller2;
			delete solid2;
			continue;
		}

		Solid* solid4 = Modeller2->getDifference();
		delete Modeller2;
		delete solid1;
		delete solid2;
		solid1 = solid4;

		Modeller2 = new BooleanModeller(solid1, solid3);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", ii);
			delete Modeller2;
			delete solid3;
			continue;
		}
		solid4 = Modeller2->getDifference();

		delete Modeller2;
		delete solid1;
		delete solid3;
		solid1 = solid4;
	}


	export_solid.SaveSTL("test8_Difference.stl", solid1);
	delete solid1;

}

void test9()
{
	printf("test9\n");
	ExportSolid export_solid;

	BooleanModeller* Modeller2;
	Solid* solid1;
	Solid* solid2;
	Solid* solid3;
	
	mlFloat minA[3]={0, 0, 0};
	mlFloat maxA[3]={100, 100, 100};

	solid1 = SolidCube2( minA, maxA, 5 );

	for ( int ii = 0; ii < 20; ii++ )
	{
		printf("[%d]\n", ii);
		solid2 = SolidCylinder(15, 90, 25, 15);
		solid2->Rotate(mlVector3D(1,1,1));
		solid2->Translate(mlVector3D(ii,0,70));

		char fname[256];
		sprintf(fname, "test9cy_%02d.stl", ii);
		export_solid.SaveSTL(fname, solid2);
		
		solid3 = SolidSphere(15, 25, 15);
		solid3->Translate(mlVector3D(ii,0,70));
		sprintf(fname, "test9sh_%02d.stl", ii);
		export_solid.SaveSTL(fname, solid3);
		
		Modeller2 = new BooleanModeller(solid1, solid2);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", ii);
			delete Modeller2;
			delete solid2;
			continue;
		}

		Solid* solid4 = Modeller2->getDifference();
		delete Modeller2;
		delete solid1;
		delete solid2;

		solid1 = solid4;

		Modeller2 = new BooleanModeller(solid1, solid3);
		if ( !Modeller2->split_status)
		{
			printf("%d error skipp.\n", ii);
			delete Modeller2;
			delete solid3;
			continue;
		}
		solid4 = Modeller2->getDifference();

		delete Modeller2;
		delete solid1;
		delete solid3;
		solid1 = solid4;
	}


	export_solid.SaveSTL("test9_Difference.stl", solid1);
	delete solid1;

}
