/* 逆行列を計算する	  	*/

#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"ML_Types.h"
#include	"sse_func.h"
#include	"openmp_config.h"

#include <Windows.h>

static void Matrix4x4f( mlFloat mat[16], float dst[16] )
{
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for ( int i = 0; i < 16; i++ )
	{
		dst[i] = to_double( mat[i] );
	}
}
static void Matrix4x4m( float mat[16], mlFloat dst[16] )
{
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for ( int i = 0; i < 16; i++ )
	{
		dst[i] = mat[i];
	}
}


int MatrixInverse3(float *mat, float *dst)
{
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for ( int i = 0; i < 16; i++ )
	{
		dst[i] = mat[i];
	}

	return Inverse_4x4_sse(dst);
}

int MatrixInverse3_4x4(mlFloat *mat, mlFloat *invmx_p)
{
	float fmat[16];
	float invfmat[16];

	Matrix4x4f( mat, fmat);

	int stat = MatrixInverse3( fmat, invfmat);
	Matrix4x4m(invfmat, invmx_p);

	return stat;
}
/************************************************************
5.2 C Code with Cramer's rule
The following C code performs 4x4-matrix inversion with Cramer's Rule (without Streaming
SIMD Extensions).
*************************************************************/

/************************************************************
*
* input:
* mat - pointer to array of 16 floats (source matrix)
* output:
* dst - pointer to array of 16 floats (invert matrix)
*
*************************************************************/
//Streaming SIMD Extensions - Inverse of 4x4 Matrix


int MatrixInverse2(float *mat, float *dst)
{
	float tmp[12];	/* temp array for pairs */
	float src[16];	/* array of transpose source matrix */
	float det;		/* determinant */

	/* transpose matrix */
#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for (int i = 0; i < 4; i++) {
		src[i] = mat[i*4];
		src[i + 4] = mat[i*4 + 1];
		src[i + 8] = mat[i*4 + 2];
		src[i + 12] = mat[i*4 + 3];
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];

	//Streaming SIMD Extensions - Inverse of 4x4 Matrix 

	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	/* calculate determinant */
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

	/* calculate matrix inverse */
	if ( Absolute(det) < 0.00000000001 )
	{
		printf("calculate matrix inverse ERROR.\n");
		return -1;
	}
	det = 1.0f/det;

#ifdef USE_OPENMP
//#pragma omp parallel for
#endif
	for (int j = 0; j < 16; j++)
		dst[j] *= det;

	return 0;
}

int MatrixInverse2_4x4(mlFloat *mat, mlFloat *invmx_p)
{
	float fmat[16];
	float invfmat[16];

	Matrix4x4f( mat, fmat);

	int stat = MatrixInverse2( fmat, invfmat);
	Matrix4x4m(invfmat, invmx_p);

	return stat;
}

#define TIMEMES	0

#if TIMEMES
#define TIMEMES_START(ts)	ts = GetTickCount()
#define TIMEMES_STOP(ts)	(GetTickCount() - ts)
#define TIMEMES_MSG( p )	p
#else
#define TIMEMES_START(ts)	/* */
#define TIMEMES_STOP(ts)	/* */
#define TIMEMES_MSG( p )	/* */
#endif

int	MatrixInverse(int dij, mlFloat *inmx_p, mlFloat *invmx_p)
{
	int			*ip;
	mlFloat		*wk;
	mlFloat		*wk2;
	int			in_ip[10];
	mlFloat		in_wk[10];
	mlFloat		in_wk2[10];
	mlFloat		eps;
	int			i1,kp;
	int			tmp;
	mlFloat		pivot,s,aw;
	mlFloat		amax,diag;
	int			status;
	register int	i,j,k;
	int			dwi,dwj,dwk,dwi1;

#if 0
	int ts = 0;
	if ( dij == 4 )
	{
#if 10
		TIMEMES_START(ts);
		status = MatrixInverse2_4x4(inmx_p, invmx_p);
		TIMEMES_MSG(printf("MatrixInverse2_4x4 status %d time:%d\n", status, TIMEMES_STOP(ts)));
#else
		TIMEMES_START(ts);
		status = MatrixInverse3_4x4(inmx_p, invmx_p);
		TIMEMES_MSG(printf("MatrixInverse3_4x4 status %d time:%d\n", status, TIMEMES_STOP(ts)));
#endif		
		if ( status == 0 ) return 0;
	}
#endif

	TIMEMES_START(ts);

	status = 0;
	eps = 0.00000000001;	/* 特異性の判定値		*/

	/* 
	 *	作業用領域の確保
	 */
	if (dij <= 10) {
		ip = in_ip;
		wk = in_wk;
		wk2 = in_wk2;
	}
	else {
		ip = new int[ dij*( sizeof(int) + 2 * sizeof(mlFloat) ) ];
		if (ip == NULL){
			return -10;
		}
		wk = (mlFloat *)(ip + dij);
		wk2 = wk + dij;
	}

	tmp = dij * dij;
	for (i=0; i<tmp; i++) {
		invmx_p[i] = inmx_p[i];
	}

	for (i=0; i<dij; i++) {
		wk[i] = 0.0;
	}

	for (j=0; j<dij; j++) {
		for (i=0, dwi=0; i<dij; i++, dwi+=dij) {
			if (Absolute(invmx_p[dwi+j]) > wk[i]) {
				wk[i] = Absolute(invmx_p[dwi+j]);
			}
		}
	}

	for (i=0; i<dij; i++) {
		if(wk[i] != 0.0) {
			wk[i] = 1.0/wk[i];
		}
	}

	for (k=0, dwk=0; k<dij; k++,dwk+=dij) {
		kp = k;
		amax = Absolute(invmx_p[dwk+k]) * wk[k];
		if (k != dij-1) {
			for(i=k+1,dwi=(k+1)*dij; i<dij; i++, dwi+=dij) {
				aw = Absolute(invmx_p[dwi+k]) * wk[i];
				if (aw > amax) {
					kp = i;
					amax = aw;
				}
			}
		}

		if (amax <= eps) {		
			/* 逆行列が求まらない	*/
			if (dij > 10) {
				delete [] ip;
			}
			return -2;
		}

		ip[k] = kp;
		if (kp != k) {
			aw = wk[kp];
			wk[kp] = wk[k];
			wk[k] = aw;
			dwi = dij*kp;
			for (j=0; j<dij; j++) {
				aw = invmx_p[dwi+j];
				invmx_p[dwi+j] = invmx_p[dwk+j];
				invmx_p[dwk+j] = aw;
			}
		}
		invmx_p[dwk+k] = 1.0 / invmx_p[dwk+k];
		if (k != dij-1) {
			pivot = invmx_p[dwk+k];
			for (i= k+1,dwi=(k+1)*dij; i<dij; i++,dwi+=dij) {
				invmx_p[dwi+k] = invmx_p[dwi+k] * pivot;
			}
			if (k != dij-1) {
				for (j=k+1; j<dij; j++) {
					aw = -invmx_p[dwk+j];
					for (i=k+1,dwi=(k+1)*dij; i<dij; i++,dwi+=dij) { 
						invmx_p[dwi+j] = 
							invmx_p[dwi+j]
							+ aw * invmx_p[dwi+k];

					}
				}
			}
		}
	}

	for (k=dij-2,dwk=(dij-2)*dij; k>= 0; k--,dwk -= dij) {
		for (j=k+1; j<dij; j++) {
			wk[j] = 0.0;
		}
		for (i=k+1,dwi=(k+1)*dij; i<dij; i++,dwi+=dij) {
			for (j=k+1,dwj=(k+1)*dij; j<dij; j++,dwj+=dij) {
				wk[j] = wk[j] - 
					invmx_p[dwi+k] * invmx_p[dwj+i];
			}
			wk2[i] = invmx_p[dwk+i];
		}
		for (j=k+1,dwj=(k+1)*dij; j<dij; j++,dwj+=dij) {
			invmx_p[dwj+k] = wk[j];
		}
		s = 0.0;
		for (i=k+1; i<dij; i++) {
			s = s + wk[i] * wk2[i];
		}
		diag = invmx_p[dwk+k];
		invmx_p[dwk+k] = (1.0 - s) * diag;
		for (j=k+1; j<dij; j++) {
			s = 0.0;
			for(i = k+1,dwi=(k+1)*dij; i<dij; i++,dwi+=dij) {
				s = s + wk2[i] * invmx_p[dwi+j];
			}
			invmx_p[dwk+j] = -s * diag;
		}
	}

	for (k=dij-2; k>=0; k--) {
		if (ip[k] != k) {
			for (i1=0,dwi1=0; i1<dij; i1++,dwi1+=dij) {
				aw = invmx_p[dwi1+ip[k]];
				invmx_p[dwi1+ip[k]] = invmx_p[dwi1+k];
				invmx_p[dwi1+k] = aw;
			}
		}
	}

	if (dij > 10) {
		delete [] ip;
	}
	TIMEMES_MSG(printf("MatrixInverse status %d time:%d\n", status, TIMEMES_STOP(ts)));

	return(status);
}	

