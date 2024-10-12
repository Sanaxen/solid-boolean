/*  ベクトルの外積を計算する				*/

#include "ML_Types.h"


void VectorOuterProduct(mlFloat invt1[3], mlFloat invt2[3], mlFloat outer[3])
{
	outer[0] = invt1[1] * invt2[2] - invt1[2] * invt2[1];
	outer[1] = invt1[2] * invt2[0] - invt1[0] * invt2[2];
	outer[2] = invt1[0] * invt2[1] - invt1[1] * invt2[0];
}
