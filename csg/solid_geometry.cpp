#include <math.h>
#include <float.h>

#include "Solid.h"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#define M_PI	3.1415926535897932384626433832795

void ogError(char* s)
{
	printf("%s\n", s);
}
void ogWarning(char* s)
{
	printf("%s\n", s);
}

class solid_vertexs
{
public:
	IntSet indices;
	VectorSet vertices;

	solid_vertexs()
	{
		indices.m_pInts.clear();
		vertices.m_pVectors.clear();
	}

	int AddVertex(mlFloat x, mlFloat y, mlFloat z)
	{

		const int sz = vertices.m_pVectors.size();
		for (int i = 0; i < sz; i++ )
		{
			float dx = to_double(x - vertices.m_pVectors[i].x);
			float dy = to_double(y - vertices.m_pVectors[i].y);
			float dz = to_double(z - vertices.m_pVectors[i].z);

			if ( fabs(dx) < 0.001f && fabs(dy) < 0.001f && fabs(dz) < 0.001f )
			{
				//printf("dup.\n");
				return i;
			}
		}

		vertices.AddVector( mlVector3D(x, y, z) );
		
		return vertices.m_pVectors.size()-1;
	}
	int AddVertex(mlFloat* v)
	{
		return AddVertex( v[0], v[1], v[2] );
	}

	void AddIndex(int i, int j, int k)
	{
		indices.AddInt(i);
		indices.AddInt(j);
		indices.AddInt(k);
	}		
};


static void ogCircleTable( mlFloat **sint, mlFloat **cost, const int n )
{
    int i;
    const int size = abs( n );
    mlFloat angle;

    assert( n );
    angle = 2 * M_PI / ( mlFloat )n;

    *sint = ( mlFloat * )calloc( sizeof( mlFloat ), size + 1 );
    *cost = ( mlFloat * )calloc( sizeof( mlFloat ), size + 1 );

    if( !( *sint ) || !( *cost ) )
    {
        free( *sint );
        free( *cost );
        ogError( "Failed to allocate memory in ogCircleTable" );
    }

    for( i = 0; i < size; i++ )
    {
        ( *sint )[ i ] = sin( angle * i );
        ( *cost )[ i ] = cos( angle * i );
    }

    /* Last sample is duplicate of the first */
    ( *sint )[ size ] = ( *sint )[ 0 ];
    ( *cost )[ size ] = ( *cost )[ 0 ];
}


Solid* SolidCube( mlFloat width )
{
    mlFloat size = width * 0.5;
	int id[4];
	solid_vertexs vert;

#   define V(a,b,c, id) id = vert.AddVertex( a size, b size, c size );
#   define N(a,b,c) glNormal3d( a, b, c );
    /* PWO: Again, I dared to convert the code to use macros... */
        V( 0+, -, 0+, id[0] ); V( 0+, -, -, id[1] ); V( 0+, 0+, -, id[2] ); V( 0+, 0+, 0+, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);

        V( 0+, 0+, 0+, id[0] ); V( 0+, 0+, -, id[1] ); V( -, 0+, -, id[2] ); V( -, 0+, 0+, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);

		V( 0+, 0+, 0+, id[0] ); V( -, 0+, 0+, id[1] ); V( -, -, 0+, id[2] ); V( 0+, -, 0+, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);

        V( -, -, 0+, id[0] ); V( -, 0+, 0+, id[1] ); V( -, 0+, -, id[2] ); V( -, -, -, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);

        V( -, -, 0+, id[0] ); V( -, -, -, id[1] ); V( 0+, -, -, id[2] ); V( 0+, -, 0+, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);

        V( -, -, -, id[0] ); V( -, 0+, -, id[1] ); V( 0+, 0+, -, id[2] ); V( 0+, -, -, id[3] );
		vert.AddIndex(id[0], id[1], id[2]);
		vert.AddIndex(id[2], id[3], id[0]);
#   undef V
#   undef N
	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}

Solid* SolidCube( mlFloat org[3], mlFloat width )
{
	Solid* solid = SolidCube( width );

	mlVector3D t(org[0]+width/2, org[1]+width/2, org[2]+width/2);
	solid->Translate(t);

	return solid;
}

Solid* SolidCube2( mlFloat minA[3], mlFloat maxA[3], int n )
{

	solid_vertexs vert;
	mlFloat step[3];

	step[0] = (maxA[0] - minA[0])/n;
	step[1] = (maxA[1] - minA[1])/n;
	step[2] = (maxA[2] - minA[2])/n;


	mlFloat x,  y,  z;
	mlFloat x1, y1, z1;
	mlFloat x2, y2, z2;

	int id[4];

	z = minA[2];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			x1 = minA[0] + step[0]*i;
			y1 = minA[1] + step[1]*j;

			x2 = minA[0] + step[0]*(i+1);
			y2 = minA[1] + step[1]*(j+1);
			id[0] = vert.AddVertex(x1, y1, z);
			id[1] = vert.AddVertex(x2, y1, z);
			id[2] = vert.AddVertex(x2, y2, z);
			id[3] = vert.AddVertex(x1, y2, z);
			vert.AddIndex(id[0], id[2], id[1]);
			vert.AddIndex(id[2], id[0], id[3]);
		}
	}

	z = maxA[2];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			x1 = minA[0] + step[0]*i;
			y1 = minA[1] + step[1]*j;

			x2 = minA[0] + step[0]*(i+1);
			y2 = minA[1] + step[1]*(j+1);
			id[0] = vert.AddVertex(x1, y1, z);
			id[1] = vert.AddVertex(x2, y1, z);
			id[2] = vert.AddVertex(x2, y2, z);
			id[3] = vert.AddVertex(x1, y2, z);
			vert.AddIndex(id[0], id[1], id[2]);
			vert.AddIndex(id[2], id[3], id[0]);
		}
	}

	x = minA[0];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			y1 = minA[1] + step[1]*i;
			z1 = minA[2] + step[2]*j;

			y2 = minA[1] + step[1]*(i+1);
			z2 = minA[2] + step[2]*(j+1);
			id[0] = vert.AddVertex(x, y1, z1);
			id[1] = vert.AddVertex(x, y2, z1);
			id[2] = vert.AddVertex(x, y2, z2);
			id[3] = vert.AddVertex(x, y1, z2);
			vert.AddIndex(id[0], id[2], id[1]);
			vert.AddIndex(id[2], id[0], id[3]);
		}
	}

	x = maxA[0];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			y1 = minA[1] + step[1]*i;
			z1 = minA[2] + step[2]*j;

			y2 = minA[1] + step[1]*(i+1);
			z2 = minA[2] + step[2]*(j+1);
			id[0] = vert.AddVertex(x, y1, z1);
			id[1] = vert.AddVertex(x, y2, z1);
			id[2] = vert.AddVertex(x, y2, z2);
			id[3] = vert.AddVertex(x, y1, z2);
			vert.AddIndex(id[0], id[1], id[2]);
			vert.AddIndex(id[2], id[3], id[0]);
		}
	}

	y = minA[1];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			x1 = minA[0] + step[0]*i;
			z1 = minA[2] + step[2]*j;

			x2 = minA[0] + step[0]*(i+1);
			z2 = minA[2] + step[2]*(j+1);
			id[0] = vert.AddVertex(x1, y, z1);
			id[1] = vert.AddVertex(x2, y, z1);
			id[2] = vert.AddVertex(x2, y, z2);
			id[3] = vert.AddVertex(x1, y, z2);
			vert.AddIndex(id[0], id[1], id[2]);
			vert.AddIndex(id[2], id[3], id[0]);
		}
	}

	y = maxA[1];
	for ( int i = 0; i < n; i++ )
	{
		for ( int j = 0; j < n; j++ )
		{
			x1 = minA[0] + step[0]*i;
			z1 = minA[2] + step[2]*j;

			x2 = minA[0] + step[0]*(i+1);
			z2 = minA[2] + step[2]*(j+1);
			id[0] = vert.AddVertex(x1, y, z1);
			id[1] = vert.AddVertex(x2, y, z1);
			id[2] = vert.AddVertex(x2, y, z2);
			id[3] = vert.AddVertex(x1, y, z2);
			vert.AddIndex(id[0], id[2], id[1]);
			vert.AddIndex(id[2], id[0], id[3]);
		}
	}

	Solid* solid = new Solid(&vert.vertices, &vert.indices);
	return solid;
}

/*!
    \fn
    \ingroup  geometry
    \brief    Draw a solid sphere centered at the origin.
    \param    radius        Sphere radius.
    \param    slices        The number of divisions around the z axis.
                            (latitudal)
    \param    stacks        The number of divisions along the z axis.
                            (longitudal)

              The glutSolidSphere() function draws a shaded sphere centered at
              the origin.  The surface is created from quadrangles
              (except for triangles as degenerate quads at the poles) in a
              longitude/latitude pattern.  The equatorial great circle lies
              in the xy-plane and is centered on the origin.

    \note     The number of polygons representing the spherical surface is
              proportional to (slices*stacks).

    \see glutWireSphere()
*/
Solid* SolidSphere( mlFloat radius, int slices, int stacks )
{
	solid_vertexs vert;

    int i, j;

    /* Adjust z and radius as stacks are drawn. */
    mlFloat z0, z1;
    mlFloat r0, r1;

    /* Pre-computed circle */
    mlFloat *sint1 = NULL, *cost1 = NULL;
    mlFloat *sint2 = NULL, *cost2 = NULL;

    if( DBL_EPSILON > radius )
        ogWarning( "Small radius in glutSolidSphere" );

    if( 1 > slices )
        ogWarning( "Invalid slices in glutSolidSphere" );
    else
        ogCircleTable( &sint1, &cost1, slices );

    if( 1 > stacks )
        ogWarning( "Invalid stacks in glutSolidSphere" );
    else
        ogCircleTable( &sint2, &cost2, stacks * 2 );

    slices = abs( slices );
    stacks = abs( stacks );

	int id[4];
	int nn = 0;
    if( sint1 && cost1 && sint2 && cost2 )
    {
        /* The top stack is covered with a triangle fan */
        z0 = 1.0;
        z1 = cost2[ 1 ];
        r0 = 0.0;
        r1 = sint2[ 1 ];

		id[0] = vert.AddVertex( 0, 0, radius );

		nn = 1;
        for( j = slices; j >= 0; j-- )
        {
            id[nn] = vert.AddVertex(
                cost1[ j ] * r1 * radius, sint1[ j ] * r1 * radius, z1 * radius
            );
			nn++;
			if (nn == 3 )
			{
				vert.AddIndex(id[0], id[2], id[1]);
				nn = 2;
				id[1] = id[2];
			}
        }

        /* Cover each stack with a quad strip except the top/bottom stacks */
		for( i = 1; i < stacks - 1; i++ )
        {
			nn = 0;
            z0 = z1;
            z1 = cost2[ i + 1 ];
            r0 = r1;
            r1 = sint2[ i + 1];

            for( j = 0; j <= slices; j++ )
            {
                id[nn] = vert.AddVertex(
                    cost1[ j ] * r1 * radius,
                    sint1[ j ] * r1 * radius,
                    z1 * radius
                );
				nn++;
                id[nn] = vert.AddVertex(
                    cost1[ j ] * r0 * radius,
                    sint1[ j ] * r0 * radius,
                    z0 * radius
                );
				nn++;
				if ( nn == 4 )
				{
					vert.AddIndex(id[0], id[2], id[1]);
					vert.AddIndex(id[2], id[3], id[1]);
					nn = 2;
					id[0] = id[2];
					id[1] = id[3];
				}
            }
        }

        /* The bottom stack is covered with a triangle fan */
        z0 = z1;
        r0 = r1;

        id[0] = vert.AddVertex( 0, 0, -radius );

		nn = 1;
        for(j = 0; j <= slices; j++ )
        {
            id[nn] = vert.AddVertex(
                cost1[ j ] * r0 * radius, sint1[ j ] * r0 * radius, z0 * radius
            );
			nn++;
			if (nn == 3 )
			{
				vert.AddIndex(id[0], id[2], id[1]);
				id[1] = id[2];
				nn = 2;
			}
        }
    }

    free( sint1 );
    free( cost1 );
    free( sint2 );
    free( cost2 );

	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}



/*!
    \fn
    \brief    Draw a solid cone.
    \ingroup  geometry
    \param    base       Cone radius at the base in the xy plane.
    \param    height     Height of cone in positive z direction.
    \param    slices     The number of divisions around the z axis. (latitudal)
    \param    stacks     The number of divisions along the z axis. (longitudal)

              The glutSolidCone() function draws a shaded cone
              with a base in the xy-plane, oriented in the positive z
              direction.

    \note     The number of polygons representing the conical surface is
              proportional to (slices*stacks).

    \see      glutWireCone()
*/
Solid* SolidCone( mlFloat base, mlFloat height, int slices, int stacks )
{
 	solid_vertexs vert;
   int i, j;

    /* Step in z and radius as stacks are drawn. */
    mlFloat z0, z1;
    mlFloat r0, r1;

    /* Used for computing scaling factors for vertex normals */
    const mlFloat side_length = sqrt( height*height + base*base );

    /* Pre-computed circle */
    mlFloat *sint = NULL, *cost = NULL;

    if( DBL_EPSILON > base )
        ogWarning( "Small base in glutSolidCone" );
    if( DBL_EPSILON > height )
        ogWarning( "Small height in glutSolidCone" );

    if( 1 > slices )
        ogWarning( "Invalid slices in glutSolidCone" );
    else if( 1 > stacks )
        ogWarning( "Invalid stacks in glutSolidCone" );
    else
        ogCircleTable( &sint, &cost, -slices );

    slices = abs( slices );
    stacks = abs( stacks );

 	int id[4];
	int nn = 0;
   if( sint && cost && side_length>0.0 )
    {
        const mlFloat zStep = height/stacks;
        const mlFloat rStep = base/stacks;

        /* Scaling factors for vertex normals */
        const mlFloat cosn = height / side_length;
        const mlFloat sinn = base   / side_length;

        /* Cover the circular base with a triangle fan... */
        z0 = 0.0;
        z1 = zStep;

        r0 = base;
        r1 = r0 - rStep;

		id[0] = vert.AddVertex( 0.0, 0.0, z0 );

		nn = 1;
        for( j = 0; j <= slices; j++ )
		{
            id[nn] = vert.AddVertex( cost[ j ] * r0, sint[ j ] * r0, z0 );
			nn++;
			if ( nn == 3 )
			{
				vert.AddIndex( id[0], id[1], id[2]);
				id[1] = id[2];
				nn = 2;
			}
		}

        /* Cover each stack with a quad strip, except the top stack */
		for( i = 0; i < stacks - 1; i++ )
        {
			nn = 0;
            for( j = 0; j <= slices; j++ )
            {
                id[nn] = vert.AddVertex( cost[ j ] * r0,   sint[ j ] * r0,   z0   ); nn++;
                id[nn] = vert.AddVertex( cost[ j ] * r1,   sint[ j ] * r1,   z1   ); nn++;

				if ( nn == 4 )
				{
					vert.AddIndex(id[0], id[1], id[2]);
					vert.AddIndex(id[2], id[1], id[3]);
					id[0] = id[2];
					id[1] = id[3];
					nn = 2;
				}
            }

            z0 = z1;
            z1 += zStep;
            r0 = r1;
            r1 -= rStep;
        }

        /* The top stack is covered with individual triangles */

        for( j = 0; j < slices; j++ )
        {
            id[0] = vert.AddVertex( cost[ j + 0 ] * r0,   sint[ j + 0 ] * r0,   z0     );
            id[1] = vert.AddVertex( 0,                    0,                    height );
            id[2] = vert.AddVertex( cost[ j + 1 ] * r0,   sint[ j + 1 ] * r0,   z0     );
			vert.AddIndex(id[0], id[1], id[2]);
        }
    }

    free( sint );
    free( cost );

	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}

Solid* SolidCylinder2( mlFloat base, mlFloat base2, mlFloat height, int slices, int stacks )
{
 	solid_vertexs vert;
   int i, j;

    /* Step in z and radius as stacks are drawn. */
    mlFloat z0, z1;
    mlFloat r0, r1;

    /* Used for computing scaling factors for vertex normals */
    const mlFloat side_length = sqrt( height*height + (base-base2)*(base-base2) );

    /* Pre-computed circle */
    mlFloat *sint = NULL, *cost = NULL;

    if( DBL_EPSILON > base )
        ogWarning( "Small base in glutSolidCone" );
    if( DBL_EPSILON > height )
        ogWarning( "Small height in glutSolidCone" );

    if( 1 > slices )
        ogWarning( "Invalid slices in glutSolidCone" );
    else if( 1 > stacks )
        ogWarning( "Invalid stacks in glutSolidCone" );
    else
        ogCircleTable( &sint, &cost, -slices );

    slices = abs( slices );
    stacks = abs( stacks );

 	int id[4];
	int nn = 0;
   if( sint && cost && side_length>0.0 )
    {
        const mlFloat zStep = height/stacks;
        const mlFloat rStep = (base-base2)/stacks;

        /* Scaling factors for vertex normals */
        const mlFloat cosn = height / side_length;
        const mlFloat sinn = (base-base2)   / side_length;

        /* Cover the circular base with a triangle fan... */
        z0 = 0.0;
        z1 = zStep;

        r0 = base;
        r1 = r0 - rStep;

		id[0] = vert.AddVertex( 0.0, 0.0, z0 );

		nn = 1;
        for( j = 0; j <= slices; j++ )
		{
            id[nn] = vert.AddVertex( cost[ j ] * r0, sint[ j ] * r0, z0 );
			nn++;
			if ( nn == 3 )
			{
				vert.AddIndex( id[0], id[1], id[2]);
				id[1] = id[2];
				nn = 2;
			}
		}

        /* Cover each stack with a quad strip, except the top stack */
		for( i = 0; i < stacks; i++ )
        {
			nn = 0;
            for( j = 0; j <= slices; j++ )
            {
                id[nn] = vert.AddVertex( cost[ j ] * r0,   sint[ j ] * r0,   z0   ); nn++;
                id[nn] = vert.AddVertex( cost[ j ] * r1,   sint[ j ] * r1,   z1   ); nn++;

				if ( nn == 4 )
				{
					vert.AddIndex(id[0], id[1], id[2]);
					vert.AddIndex(id[2], id[1], id[3]);
					id[0] = id[2];
					id[1] = id[3];
					nn = 2;
				}
            }

            z0 = z1;
            z1 += zStep;
            r0 = r1;
            r1 -= rStep;
        }

		//printf("r0 %f\n", r0);
		id[0] = vert.AddVertex( 0.0, 0.0, height );

		nn = 1;
        for( j = 0; j <= slices; j++ )
		{
            id[nn] = vert.AddVertex( cost[ j ] * r0, sint[ j ] * r0, height );
			nn++;
			if ( nn == 3 )
			{
				vert.AddIndex( id[0], id[2], id[1]);
				id[1] = id[2];
				nn = 2;
			}
		}
    }

    free( sint );
    free( cost );

	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}

Solid* SolidCylinder( mlFloat base, mlFloat height, int slices, int stacks )
{
 	solid_vertexs vert;
   int i, j;

    /* Step in z and radius as stacks are drawn. */
    mlFloat z0, z1;

    /* Used for computing scaling factors for vertex normals */
    const mlFloat side_length = sqrt( height*height + (base)*(base) );

    /* Pre-computed circle */
    mlFloat *sint = NULL, *cost = NULL;

    if( DBL_EPSILON > base )
        ogWarning( "Small base in glutSolidCone" );
    if( DBL_EPSILON > height )
        ogWarning( "Small height in glutSolidCone" );

    if( 1 > slices )
        ogWarning( "Invalid slices in glutSolidCone" );
    else if( 1 > stacks )
        ogWarning( "Invalid stacks in glutSolidCone" );
    else
        ogCircleTable( &sint, &cost, -slices );

    slices = abs( slices );
    stacks = abs( stacks );

 	int id[4];
	int nn = 0;
   if( sint && cost && side_length>0.0 )
    {
        const mlFloat zStep = height/stacks;
        const mlFloat rStep = (base)/stacks;

        /* Scaling factors for vertex normals */
        const mlFloat cosn = height / side_length;
        const mlFloat sinn = (base)   / side_length;

        /* Cover the circular base with a triangle fan... */
        z0 = 0.0;
        z1 = zStep;

		id[0] = vert.AddVertex( 0.0, 0.0, z0 );

		nn = 1;
        for( j = 0; j <= slices; j++ )
		{
            id[nn] = vert.AddVertex( cost[ j ] * base, sint[ j ] * base, z0 );
			nn++;
			if ( nn == 3 )
			{
				vert.AddIndex( id[0], id[1], id[2]);
				id[1] = id[2];
				nn = 2;
			}
		}

        /* Cover each stack with a quad strip, except the top stack */
		for( i = 0; i < stacks; i++ )
        {
			nn = 0;
            for( j = 0; j <= slices; j++ )
            {
                id[nn] = vert.AddVertex( cost[ j ] * base,   sint[ j ] * base,   z0   ); nn++;
                id[nn] = vert.AddVertex( cost[ j ] * base,   sint[ j ] * base,   z1   ); nn++;

				if ( nn == 4 )
				{
					vert.AddIndex(id[0], id[1], id[2]);
					vert.AddIndex(id[2], id[1], id[3]);
					id[0] = id[2];
					id[1] = id[3];
					nn = 2;
				}
            }

            z0 = z1;
            z1 += zStep;
        }

		//printf("r0 %f\n", r0);
		id[0] = vert.AddVertex( 0.0, 0.0, height );

		nn = 1;
        for( j = 0; j <= slices; j++ )
		{
            id[nn] = vert.AddVertex( cost[ j ] * base, sint[ j ] * base, height );
			nn++;
			if ( nn == 3 )
			{
				vert.AddIndex( id[0], id[2], id[1]);
				id[1] = id[2];
				nn = 2;
			}
		}

        /* The top stack is covered with individual triangles */

   //     for( j = 0; j < slices; j++ )
   //     {
   //         id[0] = vert.AddVertex( cost[ j + 0 ] * r0,   sint[ j + 0 ] * r0,   z0     );
   //         id[1] = vert.AddVertex( 0,                    0,                    height );
   //         id[2] = vert.AddVertex( cost[ j + 1 ] * r0,   sint[ j + 1 ] * r0,   z0     );
			//vert.AddIndex(id[0], id[1], id[2]);
   //     }
    }

    free( sint );
    free( cost );

	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}



/*!
    \fn
    \brief    Draw a solid torus.
    \ingroup  geometry
    \param    dInnerRadius    Radius of ``tube''
    \param    dOuterRadius    Radius of ``path''
    \param    nSides          Facets around ``tube''
    \param    nRings          Joints along ``path''

              This function effectively wraps a cylinder with \a nSides slats
              and bends it at \a nRings facets around a circular
              path, forming a torus, or ``donut''.  The center is
              at the origin and the ``path'' rings around the
              z axis.

              The torus parameters can be explored interactively
              with the OpenGLUT shapes demo.

    \note     \a dInnerRadius and \a dOuterRadius are <b>not</b>
              analogous to similar measurements of an anulus.

    \see      glutWireTorus()

*/
Solid* SolidTorus( mlFloat dInnerRadius, mlFloat dOuterRadius, int nSides, int nRings )
{
  	solid_vertexs vert;
    mlFloat iradius = dInnerRadius;
    mlFloat oradius = dOuterRadius;
    mlFloat phi, psi, dpsi, dphi;
    mlFloat *vertex = NULL;
    mlFloat spsi, cpsi, sphi, cphi;
    int i, j;

    if( DBL_EPSILON > dInnerRadius )
        ogWarning( "Small tube radius in glutSolidTorus" );
    if( DBL_EPSILON > dOuterRadius )
        ogWarning( "Small path radius in glutSolidTorus" );

    if( 1 > nSides )
        ogWarning( "Invalid sides in glutSolidTorus" );
    else if( 1 > nRings )
        ogWarning( "Invalid rings in glutSolidTorus" );
    else
    {

        /*
         * Increment the number of sides and rings to allow for one more point
         * than surface
         */

        nSides = abs(nSides)+1;
        nRings = abs(nRings)+1;

        vertex = ( mlFloat * )calloc( sizeof( mlFloat ), 3 * nSides * nRings );
    }

 	int id[4];
	int nn = 0;
	if( vertex )
    {
        dpsi =  2.0 * M_PI / ( mlFloat )( nRings - 1 );
        dphi = -2.0 * M_PI / ( mlFloat )( nSides - 1 );
        psi  = 0.0;

        for( j = 0; j < nRings; j++ )
        {
            cpsi = cos( psi );
            spsi = sin( psi );
            phi = 0.0;

            for( i = 0; i < nSides; i++ )
            {
                int offset = 3 * ( j * nSides + i );
                cphi = cos( phi );
                sphi = sin( phi );
                *( vertex + offset + 0 ) = cpsi * ( oradius + cphi * iradius );
                *( vertex + offset + 1 ) = spsi * ( oradius + cphi * iradius );
                *( vertex + offset + 2 ) =                    sphi * iradius;
                phi += dphi;
            }

            psi += dpsi;
        }

        for( i = 0; i < nSides - 1; i++ )
            for( j = 0; j < nRings - 1; j++ )
            {
                int offset = 3 * ( j * nSides + i );
                id[0] = vert.AddVertex( vertex + offset );
                id[1] = vert.AddVertex( vertex + offset + 3 );
                id[2] = vert.AddVertex( vertex + offset + 3 * nSides + 3 );
                id[3] = vert.AddVertex( vertex + offset + 3 * nSides );
				vert.AddIndex( id[0], id[1], id[2]);
				vert.AddIndex( id[2], id[3], id[0]);
            }
    }

    free( vertex );

	Solid* solid = new Solid(&vert.vertices, &vert.indices);

	return solid;
}

#if 0

/*!
    \fn
    \brief      Draw a wireframe dodecahedron.
    \ingroup    geometry

                This function draws a regular, wireframe 12-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is
                sqrt(3).
                The facets are pentagons.
                No facet is normal any of the \a x, \a y, or \a z
                axes.

    \see        glutSolidDodecahedron(), glutWireRhombicDodecahedron(),
                glutSolidRhombicDodecahedron()

*/
void OGAPIENTRY glutWireDodecahedron( void )
{
    /*
     * Magic Numbers:  It is possible to create a dodecahedron by attaching
     * two pentagons to each face of of a cube.  The coordinates of the points
     * are:
     *   {(+/- x, 0, z), (+/- 1, 1, 1), (0, z, x )}
     * where x = (-1 + sqrt(5))/2 and z = (1 + sqrt(5))/2 or, approximately
     *       x = 0.61803398875    and z = 1.61803398875.
     */
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.0,  0.525731112119,  0.850650808354 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( -0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( 0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.0,  0.525731112119, -0.850650808354 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( -0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.0, -0.525731112119,  0.850650808354 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( -0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.0, -0.525731112119, -0.850650808354 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( -0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( 0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( 1.0, -1.0, -1.0 );
    glEnd( );

    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.850650808354,  0.0,  0.525731112119 );
        glVertex3d( 0.61803398875,  0.0,  1.61803398875 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( 1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( -0.850650808354,  0.0,  0.525731112119 );
        glVertex3d( -0.61803398875,  0.0,  1.61803398875 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( -1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( -1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.850650808354,  0.0, -0.525731112119 );
        glVertex3d( 0.61803398875,  0.0, -1.61803398875 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( 1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( 1.0, -1.0, -1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( -0.850650808354,  0.0, -0.525731112119 );
        glVertex3d( -0.61803398875,  0.0, -1.61803398875 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( -1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( -1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );

    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.525731112119,  0.850650808354,  0.0 );
        glVertex3d( 1.61803398875,  0.61803398875,  0.0 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.525731112119, -0.850650808354,  0.0 );
        glVertex3d( 1.61803398875, -0.61803398875,  0.0 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( 1.0, -1.0, -1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( -0.525731112119,  0.850650808354,  0.0 );
        glVertex3d( -1.61803398875,  0.61803398875,  0.0 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );
    glBegin( GL_LINE_LOOP );
        glNormal3d( -0.525731112119, -0.850650808354,  0.0 );
        glVertex3d( -1.61803398875, -0.61803398875,  0.0 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
}

/*!
    \fn
    \brief      Draw a solid dodecahedron.
    \ingroup    geometry

                This function draws a regular, solid, 12-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is
                sqrt(3).
                The facets are pentagons.

    \see        glutWireDodecahedron(), glutSolidRhombicDodecahedron(),
                glutWireRhombicDodecahedron()

*/
void OGAPIENTRY glutSolidDodecahedron( void )
{
    /* See glutWireDodecahedron() for info about the Magic Numbers. */
    glBegin( GL_POLYGON );
        glNormal3d( 0.0,  0.525731112119,  0.850650808354 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( -0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( 0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( 0.0,  0.525731112119, -0.850650808354 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( -0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( 0.0, -0.525731112119,  0.850650808354 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( -0.61803398875, 0.0,  1.61803398875 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( 0.0, -0.525731112119, -0.850650808354 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( -0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( 0.61803398875, 0.0, -1.61803398875 );
        glVertex3d( 1.0, -1.0, -1.0 );
     glEnd( );

    glBegin( GL_POLYGON );
        glNormal3d( 0.850650808354,  0.0,  0.525731112119 );
        glVertex3d( 0.61803398875,  0.0,  1.61803398875 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( 1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin ( GL_POLYGON );
        glNormal3d( -0.850650808354,  0.0,  0.525731112119 );
        glVertex3d( -0.61803398875,  0.0,  1.61803398875 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( -1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( -1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( 0.850650808354,  0.0, -0.525731112119 );
        glVertex3d( 0.61803398875,  0.0, -1.61803398875 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( 1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( 1.0, -1.0, -1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( -0.850650808354,  0.0, -0.525731112119 );
        glVertex3d( -0.61803398875,  0.0, -1.61803398875 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( -1.61803398875, -0.61803398875, 0.0 );
        glVertex3d( -1.61803398875,  0.61803398875, 0.0 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );

    glBegin( GL_POLYGON );
        glNormal3d( 0.525731112119,  0.850650808354,  0.0 );
        glVertex3d( 1.61803398875,  0.61803398875,  0.0 );
        glVertex3d( 1.0,  1.0, -1.0 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( 1.0,  1.0,  1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( 0.525731112119, -0.850650808354,  0.0 );
        glVertex3d( 1.61803398875, -0.61803398875,  0.0 );
        glVertex3d( 1.0, -1.0,  1.0 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( 1.0, -1.0, -1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( -0.525731112119,  0.850650808354,  0.0 );
        glVertex3d( -1.61803398875,  0.61803398875,  0.0 );
        glVertex3d( -1.0,  1.0,  1.0 );
        glVertex3d( 0.0,  1.61803398875,  0.61803398875 );
        glVertex3d( 0.0,  1.61803398875, -0.61803398875 );
        glVertex3d( -1.0,  1.0, -1.0 );
    glEnd( );
    glBegin( GL_POLYGON );
        glNormal3d( -0.525731112119, -0.850650808354,  0.0 );
        glVertex3d( -1.61803398875, -0.61803398875,  0.0 );
        glVertex3d( -1.0, -1.0, -1.0 );
        glVertex3d( 0.0, -1.61803398875, -0.61803398875 );
        glVertex3d( 0.0, -1.61803398875,  0.61803398875 );
        glVertex3d( -1.0, -1.0,  1.0 );
    glEnd( );
}



/*
 * Octahedron VERTices.  Cleans up the code a bit.  (^&
 */
static mlFloat overt[ 6 ][ 3 ] =
{
    {  1,  0,  0 },
    {  0,  1,  0 },
    {  0,  0,  1 },
    { -1,  0,  0 },
    {  0, -1,  0 },
    {  0,  0, -1 },
};


/*!
    \fn
    \brief      Draw a wireframe octahedron.
    \ingroup    geometry

                This function draws a regular wireframe 8-sided polyhedron
                centered at the origin.
                The vertices are at
                 (+/-1, 0, 0),
                 (0, +/-1, 0),
                 (0, 0, +/-1).

    \note       We visit the same vertices the same number of times
                as for the solid octahedron, but the order is different.
    \note       Draws every edge twice.
    \note       The lines have normals, but the normals are from the
                facets, rather than upon the edge.  If you squint too
                hard, the lighting on a wireframe octahedron does
                not look quite right.
    \todo       It may be faster (and look better) to draw each edge
                once, setting the Normal at each edge.  (I don't
                think that this matters all that much, but a lineloop
                was proposed for the wire Cube for speed.)
    \see        glutSolidOctahedron()

*/
void OGAPIENTRY glutWireOctahedron( void )
{
    glBegin( GL_LINE_LOOP );
        glNormal3d( 0.577350269189, 0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 2 ] );
        glNormal3d( 0.577350269189, 0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 5 ] );
        glNormal3d( 0.577350269189,-0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 2 ] );
        glVertex3dv( overt[ 4 ] );
        glNormal3d( 0.577350269189,-0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 5 ] );
        glNormal3d(-0.577350269189, 0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 2 ] );
        glVertex3dv( overt[ 3 ] );
        glNormal3d(-0.577350269189, 0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 3 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 5 ] );
        glNormal3d(-0.577350269189,-0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 3 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 2 ] );
        glNormal3d(-0.577350269189,-0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 3 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 5 ] );
    glEnd( );
}

/*!
    \fn
    \brief      Draw a solid octahedron.
    \ingroup    geometry

                This function draws a regular, solid 8-sided polyhedron
                centered at the origin.
                The vertices are at
                 (+/-1, 0, 0),
                 (0, +/-1, 0),
                 (0, 0, +/-1).

    \note       We visit the same vertices the same number of times
                as in the wire octahedron, but the order is different.

    \see        glutWireOctahedron()

*/
void OGAPIENTRY glutSolidOctahedron( void )
{
    glBegin( GL_TRIANGLES );
        glNormal3d( 0.577350269189, 0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 2 ] );
        glNormal3d( 0.577350269189, 0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 5 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 0 ] );
        glNormal3d( 0.577350269189,-0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 2 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 0 ] );
        glNormal3d( 0.577350269189,-0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 0 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 5 ] );
        glNormal3d(-0.577350269189, 0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 2 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 3 ] );
        glNormal3d(-0.577350269189, 0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 3 ] );
        glVertex3dv( overt[ 1 ] );
        glVertex3dv( overt[ 5 ] );
        glNormal3d(-0.577350269189,-0.577350269189, 0.577350269189 );
        glVertex3dv( overt[ 3 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 2 ] );
        glNormal3d(-0.577350269189,-0.577350269189,-0.577350269189 );
        glVertex3dv( overt[ 5 ] );
        glVertex3dv( overt[ 4 ] );
        glVertex3dv( overt[ 3 ] );
    glEnd();
}


/*!
    \fn
    \brief      Draw a wireframe tetrahedron.
    \ingroup    geometry

                This function draws a regular, wireframe 4-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is 1.

    \todo       Merge \a r0 \a r1 \a r2 and \a r3 into one array.
    \todo       Put the normals into the (or an) array.
    \todo       Make the array static const, with file scope, and share
                with glutSolidTetrahedron().
    \todo       Maybe consolidate with the SierpinskySponge?
    \see        glutSolidTetrahedron()

*/
void OGAPIENTRY glutWireTetrahedron( void )
{
    /*
     * Magic Numbers:  r0 = ( 1, 0, 0 )
     *                 r1 = ( -1/3, 2 sqrt(2) / 3, 0 )
     *                 r2 = ( -1/3, -sqrt(2) / 3, sqrt(6) / 3 )
     *                 r3 = ( -1/3, -sqrt(2) / 3, -sqrt(6) / 3 )
     * |r0| = |r1| = |r2| = |r3| = 1
     * Distance between any two points is 2 sqrt(6) / 3
     *
     * Normals:  The unit normals are simply the negative of the coordinates
     * of the point not on the surface.
     */

    mlFloat r0[3] = {             1.0,             0.0,             0.0 };
    mlFloat r1[3] = { -0.333333333333,  0.942809041582,             0.0 };
    mlFloat r2[3] = { -0.333333333333, -0.471404520791,  0.816496580928 };
    mlFloat r3[3] = { -0.333333333333, -0.471404520791, -0.816496580928 };

    glBegin( GL_LINE_LOOP );
        glNormal3d(           -1.0,             0.0,             0.0 );
        glVertex3dv( r1 );
        glVertex3dv( r3 );
        glVertex3dv( r2 );
        glNormal3d( 0.333333333333, -0.942809041582,             0.0 );
        glVertex3dv( r0 );
        glVertex3dv( r2 );
        glVertex3dv( r3 );
        glNormal3d( 0.333333333333,  0.471404520791, -0.816496580928 );
        glVertex3dv( r0 );
        glVertex3dv( r3 );
        glVertex3dv( r1 );
        glNormal3d( 0.333333333333,  0.471404520791,  0.816496580928 );
        glVertex3dv( r0 );
        glVertex3dv( r1 );
        glVertex3dv( r2 );
    glEnd();
}


/*!
    \fn
    \brief      Draw a solid tetrahedron.
    \ingroup    geometry

                This function draws a regular, solid 4-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is 1.

    \todo       See todo-list on glutWireTetrahedron().
    \see        glutWireTetrahedron()

*/
void OGAPIENTRY glutSolidTetrahedron( void )
{
    /*
     * Magic Numbers:  r0 = ( 1, 0, 0 )
     *                 r1 = ( -1/3, 2 sqrt(2) / 3, 0 )
     *                 r2 = ( -1/3, -sqrt(2) / 3, sqrt(6) / 3 )
     *                 r3 = ( -1/3, -sqrt(2) / 3, -sqrt(6) / 3 )
     * |r0| = |r1| = |r2| = |r3| = 1
     * Distance between any two points is 2 sqrt(6) / 3
     *
     * Normals:  The unit normals are simply the negative of the coordinates
     * of the point not on the surface.
     */

    mlFloat r0[3] = {             1.0,             0.0,             0.0 };
    mlFloat r1[3] = { -0.333333333333,  0.942809041582,             0.0 };
    mlFloat r2[3] = { -0.333333333333, -0.471404520791,  0.816496580928 };
    mlFloat r3[3] = { -0.333333333333, -0.471404520791, -0.816496580928 };

    glBegin( GL_TRIANGLES );
        glNormal3d(           -1.0,             0.0,             0.0 );
        glVertex3dv( r1 );
        glVertex3dv( r3 );
        glVertex3dv( r2 );
        glNormal3d( 0.333333333333, -0.942809041582,             0.0 );
        glVertex3dv( r0 );
        glVertex3dv( r2 );
        glVertex3dv( r3 );
        glNormal3d( 0.333333333333,  0.471404520791, -0.816496580928 );
        glVertex3dv( r0 );
        glVertex3dv( r3 );
        glVertex3dv( r1 );
        glNormal3d( 0.333333333333,  0.471404520791,  0.816496580928 );
        glVertex3dv( r0 );
        glVertex3dv( r1 );
        glVertex3dv( r2 );
    glEnd();
}

/*
 *
 */
mlFloat icos_r[ 12 ][ 3 ] =
{
    { 1.0, 0.0, 0.0 },
    {  0.447213595500,  0.894427191000, 0.0 },
    {  0.447213595500,  0.276393202252, 0.850650808354 },
    {  0.447213595500, -0.723606797748, 0.525731112119 },
    {  0.447213595500, -0.723606797748, -0.525731112119 },
    {  0.447213595500,  0.276393202252, -0.850650808354 },
    { -0.447213595500, -0.894427191000, 0.0 },
    { -0.447213595500, -0.276393202252, 0.850650808354 },
    { -0.447213595500,  0.723606797748, 0.525731112119 },
    { -0.447213595500,  0.723606797748, -0.525731112119 },
    { -0.447213595500, -0.276393202252, -0.850650808354 },
    { -1.0, 0.0, 0.0 }
};
int icos_v[ 20 ][ 3 ] =
{
    { 0, 1, 2 }, { 0, 2, 3 }, { 0, 3, 4 }, { 0, 4, 5 }, { 0, 5, 1 },
    { 1, 8, 2 }, { 2, 7, 3 }, { 3, 6, 4 }, { 4, 10, 5 }, { 5, 9, 1 },
    { 1, 9, 8 }, { 2, 8, 7 }, { 3, 7, 6 }, { 4, 6, 10 }, { 5, 10, 9 },
    { 11, 9, 10 }, { 11, 8, 9 }, { 11, 7, 8 }, { 11, 6, 7 }, { 11, 10, 6 }
};


/*!
    \fn
    \brief      Draw a wireframe icosahedron.
    \ingroup    geometry

                This function draws a regular, solid 20-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is 1.
                No facet is normal to any of the \a x, \a y, or \a z
                axes.

    \see        glutSolidIcosahedron()
*/
void OGAPIENTRY glutWireIcosahedron( void )
{
    int i;
    for( i = 0; i < 20; i++ )
    {
        mlFloat normal[ 3 ];
        normal[ 0 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] );
        normal[ 1 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] );
        normal[ 2 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] );
        glBegin( GL_LINE_LOOP );
            glNormal3dv( normal );
            glVertex3dv( icos_r[ icos_v[ i ][ 0 ] ] );
            glVertex3dv( icos_r[ icos_v[ i ][ 1 ] ] );
            glVertex3dv( icos_r[ icos_v[ i ][ 2 ] ] );
        glEnd( );
    }
}


/*!
    \fn
    \brief      Draw a solid icosahedron.
    \ingroup    geometry

                This function draws a regular, solid 20-sided polyhedron
                centered at the origin.
                The distance from the origin to the vertices is 1.

    \see        glutWireIcosahedron()
*/
void OGAPIENTRY glutSolidIcosahedron( void )
{
    int i;

    glBegin( GL_TRIANGLES );
    for( i = 0; i < 20; i++ )
    {
        mlFloat normal[ 3 ];
        normal[ 0 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] );
        normal[ 1 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 2 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 2 ] );
        normal[ 2 ] =
            ( icos_r[ icos_v[ i ][ 1 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) -
            ( icos_r[ icos_v[ i ][ 1 ] ][ 1 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 1 ] ) *
            ( icos_r[ icos_v[ i ][ 2 ] ][ 0 ] -
              icos_r[ icos_v[ i ][ 0 ] ][ 0 ] );
        glNormal3dv( normal );
        glVertex3dv( icos_r[ icos_v[ i ][ 0 ] ] );
        glVertex3dv( icos_r[ icos_v[ i ][ 1 ] ] );
        glVertex3dv( icos_r[ icos_v[ i ][ 2 ] ] );
    }
    glEnd( );
}

/*
 *
 */
mlFloat rdod_r[ 14 ][ 3 ] =
{
    { 0.0, 0.0, 1.0 },
    {  0.707106781187,  0.000000000000,  0.5 },
    {  0.000000000000,  0.707106781187,  0.5 },
    { -0.707106781187,  0.000000000000,  0.5 },
    {  0.000000000000, -0.707106781187,  0.5 },
    {  0.707106781187,  0.707106781187,  0.0 },
    { -0.707106781187,  0.707106781187,  0.0 },
    { -0.707106781187, -0.707106781187,  0.0 },
    {  0.707106781187, -0.707106781187,  0.0 },
    {  0.707106781187,  0.000000000000, -0.5 },
    {  0.000000000000,  0.707106781187, -0.5 },
    { -0.707106781187,  0.000000000000, -0.5 },
    {  0.000000000000, -0.707106781187, -0.5 },
    {  0.0, 0.0, -1.0 }
};
int rdod_v[ 12 ][ 4 ] =
{
    { 0,  1,  5,  2 },
    { 0,  2,  6,  3 },
    { 0,  3,  7,  4 },
    { 0,  4,  8,  1 },
    { 5, 10,  6,  2 },
    { 6, 11,  7,  3 },
    { 7, 12,  8,  4 },
    { 8,  9,  5,  1 },
    { 5,  9, 13, 10 },
    { 6, 10, 13, 11 },
    { 7, 11, 13, 12 },
    { 8, 12, 13,  9 }
};
mlFloat rdod_n[ 12 ][ 3 ] =
{
  {  0.353553390594,  0.353553390594,  0.5 },
  { -0.353553390594,  0.353553390594,  0.5 },
  { -0.353553390594, -0.353553390594,  0.5 },
  {  0.353553390594, -0.353553390594,  0.5 },
  {  0.000000000000,  1.000000000000,  0.0 },
  { -1.000000000000,  0.000000000000,  0.0 },
  {  0.000000000000, -1.000000000000,  0.0 },
  {  1.000000000000,  0.000000000000,  0.0 },
  {  0.353553390594,  0.353553390594, -0.5 },
  { -0.353553390594,  0.353553390594, -0.5 },
  { -0.353553390594, -0.353553390594, -0.5 },
  {  0.353553390594, -0.353553390594, -0.5 }
};

/*!
    \fn
    \brief    Draw a wireframe rhombic dodecahedron
    \ingroup  geometry

              This function draws a wireframe dodecahedron whose
              facets are rhombic and
              whose vertices are at unit radius.
              No facet lies normal to any coordinate axes.
              The polyhedron is centered at the origin.

    \see      glutSolidRhombicDodecahedron(), glutWireDodecahedron(),
              glutSolidDodecahedron()
*/
void OGAPIENTRY glutWireRhombicDodecahedron( void )
{
    int i;
    for( i = 0; i < 12; i++ )
    {
        glBegin( GL_LINE_LOOP );
            glNormal3dv( rdod_n[ i ] );
            glVertex3dv( rdod_r[ rdod_v[ i ][ 0 ] ] );
            glVertex3dv( rdod_r[ rdod_v[ i ][ 1 ] ] );
            glVertex3dv( rdod_r[ rdod_v[ i ][ 2 ] ] );
            glVertex3dv( rdod_r[ rdod_v[ i ][ 3 ] ] );
        glEnd( );
    }
}

/*!
    \fn
    \brief    Draw a solid rhombic dodecahedron
    \ingroup  geometry

              This function draws a solid-shaded dodecahedron
              whose facets are rhombic and
              whose vertices are at unit radius.
              No facet lies normal to any coordinate axes.
              The polyhedron is centered at the origin.

    \see      glutWireRhombicDodecahedron(), glutWireDodecahedron(),
              glutSolidDodecahedron()
*/
void OGAPIENTRY glutSolidRhombicDodecahedron( void )
{
    int i;

    glBegin( GL_QUADS );
    for( i = 0; i < 12; i++ )
    {
        glNormal3dv( rdod_n[ i ] );
        glVertex3dv( rdod_r[ rdod_v[ i ][ 0 ] ] );
        glVertex3dv( rdod_r[ rdod_v[ i ][ 1 ] ] );
        glVertex3dv( rdod_r[ rdod_v[ i ][ 2 ] ] );
        glVertex3dv( rdod_r[ rdod_v[ i ][ 3 ] ] );
    }
    glEnd( );
}



#define NUM_FACES     4

static mlFloat tetrahedron_v[ 4 ][ 3 ] =  /* Vertices */
{
    { -0.5, -0.288675134595, -0.144337567297 },
    {  0.5, -0.288675134595, -0.144337567297 },
    {  0.0,  0.577350269189, -0.144337567297 },
    {  0.0,  0.0,             0.672159013631 }
};

static int tetrahedron_i[ 4 ][ 3 ] =  /* Vertex indices */
{
    { 1, 0, 2 }, { 2, 0, 3 }, { 0, 1, 3 }, { 1, 2, 3 }
};

static mlFloat tetrahedron_n[ 4 ][ 3 ] =  /* Normals */
{
    {  0.0,             0.0,            -1.0 },
    { -0.816496580928,  0.471404520791,  0.333333333333 },
    {  0.0,            -0.942809041582,  0.333333333333 },
    {  0.816496580928,  0.471404520791,  0.333333333333 }
};

/*
 * The edges for the wireframe tetrahedron at the base of the Sponge object.
 *
 * This array is structured as: 6 edges.  Each edge has
 * the two vertices that define it, and the normal which
 * we determine as the 50/50 blend of the two faces that the
 * edge bounds.
 *
 * The array has been constructed with some care so that we can define
 * a GL_LINE_STRIP as:
 *
 *   edges[ 0 ].v1, edges[ 0 ].v2 == edges[ 1 ].v1,
 *                  edges[ 1 ].v2 == edges[ 2 ].v1,
 *                  edges[ 2 ].v2 == edges[ 3 ].v1,
 *   ...
 *
 * However, because an Euler Circuit can't be made on the vertices of a
 * tetrahedron, we either need to walk one edge twice or stop and
 * start at least once.  I chose to walk one edge twice, and use a secondary
 * array to tell me in what order to walk which edges.  (See below.)
 */
typedef struct tetrahedron_edge
{
    int v1;
    int v2;
    mlFloat normal[ 3 ];
} tetrahedron_edge;
static const tetrahedron_edge tetrahedron_edges[ 6 ] =
{
    { 0, 1, { -0.707106781186674,  0.408248290463772, -0.577350269189535 } },
    { 1, 2, { -0.707106781186674, -0.408248290463772,  0.577350269189535 } },
    { 2, 0, {  0.000000000000000, -0.816496580927708, -0.577350269189652 } },
    { 0, 3, {  0.707106781186674,  0.408248290463772, -0.577350269189535 } },
    { 3, 1, {  0.000000000000000,  0.816496580927708,  0.577350269189652 } },
    { 2, 3, {  0.707106781186674, -0.408248290463772,  0.577350269189535 } },
};

/*
 * This is the list of edges for a GL_LINE_STRIP to build the tetrahedron.
 * NOTE that this is not for a GL_LINE_LOOP.
 */
static const int tetrahedron_edge_list[ 7 ] = { 0, 1, 2, 3, 4, 1, 5 };

/*!
    \fn
    \brief    Draw a wireframe Spierspinski's sponge.
    \ingroup  geometry
    \param    num_levels    Recursive depth.
    \param    offset        Location vector.
    \param    scale         Relative size.

              This function recursively draws a few levels of
              Sierpinski's Sponge
              in wireframe.
              If \a num_levels is 0, draws 1 tetrahedron.
              The \a offset is a translation.
              The \a z axis is normal to the base.
              The sponge is centered at the origin.

    \note     Runtime is exponential in \a num_levels .

    \see      glutSolidSierpinskiSponge()
*/
void OGAPIENTRY glutWireSierpinskiSponge(
    int num_levels, const mlFloat offset[ 3 ], mlFloat scale
)
{
    int i;

    if( !num_levels )
    {
        int edge = tetrahedron_edge_list[ 0 ];
        int vert = tetrahedron_edges[ edge ].v1;
        mlFloat x = offset[ 0 ] + scale * tetrahedron_v[ vert ][ 0 ];
        mlFloat y = offset[ 1 ] + scale * tetrahedron_v[ vert ][ 1 ];
        mlFloat z = offset[ 2 ] + scale * tetrahedron_v[ vert ][ 2 ];

        glBegin( GL_LINE_STRIP );
        glVertex3d( x, y, z );
        for( i = 0; i < 5; ++i )
        {
            edge = tetrahedron_edge_list[ i ];
            vert = tetrahedron_edges[ edge ].v2;
            glNormal3dv( tetrahedron_edges[ edge ].normal );
            x = offset[ 0 ] + scale * tetrahedron_v[ vert ][ 0 ];
            y = offset[ 1 ] + scale * tetrahedron_v[ vert ][ 1 ];
            z = offset[ 2 ] + scale * tetrahedron_v[ vert ][ 2 ];
            glVertex3d( x, y, z );
        }
        glEnd( );
        glBegin( GL_LINES );
        for( i = 5; i < 7; ++i )
        {
            edge = tetrahedron_edge_list[ i ];
            vert = tetrahedron_edges[ edge ].v2;
            glNormal3dv( tetrahedron_edges[ edge ].normal );
            x = offset[ 0 ] + scale * tetrahedron_v[ vert ][ 0 ];
            y = offset[ 1 ] + scale * tetrahedron_v[ vert ][ 1 ];
            z = offset[ 2 ] + scale * tetrahedron_v[ vert ][ 2 ];
            glVertex3d( x, y, z );
        }
        glEnd( );
    }
    else
    {
        /* Use a local variable to avoid buildup of roundoff errors */
        mlFloat local_offset[ 3 ];
        num_levels--;
        scale /= 2.0;
        local_offset[ 0 ] = offset[ 0 ] + scale * tetrahedron_v[ 0 ][ 0 ];
        local_offset[ 1 ] = offset[ 1 ] + scale * tetrahedron_v[ 0 ][ 1 ];
        local_offset[ 2 ] = offset[ 2 ] + scale * tetrahedron_v[ 0 ][ 2 ];
        glutWireSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 0 ] += scale;
        glutWireSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 0 ] -= 0.5            * scale;
        local_offset[ 1 ] += 0.866025403784 * scale;
        glutWireSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 1 ] -= 0.577350269189 * scale;
        local_offset[ 2 ] += 0.816496580928 * scale;
        glutWireSierpinskiSponge( num_levels, local_offset, scale );
    }
}

/*!
    \fn
    \brief    Draw a solid Spierspinski's sponge.
    \ingroup  geometry
    \param    num_levels    Recursive depth.
    \param    offset        Location vector.
    \param    scale         Relative size.

              This function recursively draws a few levels of
              a solid-shaded Sierpinski's Sponge.
              If \a num_levels is 0, draws 1 tetrahedron.
              The \a offset is a translation.
              The \a z axis is normal to the base.
              The sponge is centered at the origin.

    \note     Runtime is exponential in \a num_levels .

    \todo     Consider removing the \a offset parameter from the
              API (use a helper function).

    \see      glutWireSierpinskiSponge()
*/
void OGAPIENTRY glutSolidSierpinskiSponge(
    int num_levels, const mlFloat offset[ 3 ], mlFloat scale
)
{
    int i, j;

    if( !num_levels )
    {
        /* Maybe the base-case should be a glutSolidTetrahedron() call? */
        glBegin( GL_TRIANGLES );
        for( i = 0; i < NUM_FACES; i++ )
        {
            glNormal3dv( tetrahedron_n[ i ] );
            for( j = 0; j < 3; j++ )
            {
                mlFloat x = offset[ 0 ] +
                    scale * tetrahedron_v[ tetrahedron_i[ i ][ j ] ][ 0 ];
                mlFloat y = offset[ 1 ] +
                    scale * tetrahedron_v[ tetrahedron_i[ i ][ j ] ][ 1 ];
                mlFloat z = offset[ 2 ] +
                    scale * tetrahedron_v[ tetrahedron_i[ i ][ j ] ][ 2 ];
                glVertex3d( x, y, z );
            }
        }
        glEnd( );
    }
    else
    {
        /* Use a local variable to avoid buildup of roundoff errors */
        mlFloat local_offset[ 3 ];
        num_levels--;
        scale /= 2.0;
        local_offset[ 0 ] = offset[ 0 ] + scale * tetrahedron_v[ 0 ][ 0 ];
        local_offset[ 1 ] = offset[ 1 ] + scale * tetrahedron_v[ 0 ][ 1 ];
        local_offset[ 2 ] = offset[ 2 ] + scale * tetrahedron_v[ 0 ][ 2 ];
        glutSolidSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 0 ] += scale;
        glutSolidSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 0 ] -= 0.5            * scale;
        local_offset[ 1 ] += 0.866025403784 * scale;
        glutSolidSierpinskiSponge( num_levels, local_offset, scale );
        local_offset[ 1 ] -= 0.577350269189 * scale;
        local_offset[ 2 ] += 0.816496580928 * scale;
        glutSolidSierpinskiSponge( num_levels, local_offset, scale );
    }
}

#endif
#undef NUM_FACES
