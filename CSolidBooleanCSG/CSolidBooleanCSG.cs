using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace CSolidBooleanCSGClassLibrary
{
    using csgSolid = System.IntPtr;
    using csgBooleanModeller = System.IntPtr;

    unsafe public class CSolidBoolean
    {

        //CSG利用開始
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern System.IntPtr uad_double_library_Init();
   
        //CSG利用終了
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void uad_double_library_Term( System.IntPtr p);

		//OpenMP並列処理スレッド数の設定
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void csg_omp_set_num_threads(int num);
		
		//設定されているOpenMP並列処理スレッド最大数取得
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int csg_omp_get_max_threads();

        //プリミティブソリッドの精度設定(2<= coef <= n)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void csg_solid_slice_stack_ext_coef(int coef);

        //プリミティブソリッドの精度取得
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csg_solid_slice_stack_ext_coef();

        //集合演算をリトライするか設定する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidBooleanOperationTrySet(int flg );

        //集合演算をリトライするかを取得する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SolidBooleanOperationTryGet();

        //集合演算時間制限を設定する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidBooleanOperationTimeOutSet(int limit);

        //集合演算時間制限を取得する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SolidBooleanOperationTimeOutGet();

        //ソリッドに色を付加する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidColor(csgSolid solid_base, int r, int g, int b);

        //ソリッドに固有の名前(ID)を付加する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidName(csgSolid solid, int name );

        //ソリッドに固有の名前(ID)を取得する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetSolidName(csgSolid solid);

        //ソリッド生成
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid CreateSolid(int num, int* index, int vrtnum, float* vertex, int* color);

        //球
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidSphere(float radius, int slices, int stacks);

        //円錐
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCone(float base1, float height, int slices, int stacks);

        //角柱
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCube(float width);

        //角柱1
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCube1(float* org, float width);
        //角柱2
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCube2(float* minA, float* maxA, int n);
        //角柱3
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCube3(float* minA, float* lenth, int n);

        //円柱
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCylinder2(float base1, float base2, float height, int slices, int stacks);
        //円柱
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCylinder(float base1, float height, int slices, int stacks);

        //トーラス
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidTorus(float dInnerRadius, float dOuterRadius, int nSides, int nRings);

        //平行移動
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidTranslate(csgSolid solid, float* t);
        //拡大縮小
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidScale(csgSolid solid, float* scale);
        //回転
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidRotate(csgSolid solid, float* axis);

        //回転
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidRotateAnyAxis(csgSolid solid, float* p1, float* p2, float angle);

        //コピー
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidCopy(csgSolid solid_base);
        //削除
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteSolid(csgSolid solid);

        //圧縮(重複頂点削除)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SimplifySolid(csgSolid solid);

        //csgSolidのEdgeCollapse
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SimplifySolid2(csgSolid solid, double collapse_tol, double angle, double tol);

        //集合演算準備
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgBooleanModeller CreateSolidModeller(csgSolid solid1, csgSolid solid2, int* status);       
        //集合演算解除
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteSolidModeller(csgBooleanModeller modeller); 
        //集合演算（和）
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid CreateSolidModeller_Union(csgBooleanModeller modeller);
        //集合演算（積）
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid CreateSolidModeller_Intersection(csgBooleanModeller modeller);
        //集合演算（差）
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid CreateSolidModeller_Difference(csgBooleanModeller modeller); 

        //三角形リスト取得
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidTriangleList(csgSolid solid, int* num, float** vertex, float** normal);
        //三角形リスト破棄
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteSolidTriangleList(float* vertex, float* normal);

        //頂点配列取得
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidVertexBuffer(csgSolid solid, int* num, int** index, int* vrtnum, float** vertex, int** vertex_color);
        //サイズの正規化＋頂点配列取得
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidNormalizeSize(System.IntPtr solid, float radius, int* num, int** index, int* vrtnum, float** resize_vertex);
        //頂点配列破棄
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DeleteSolidVertexBuffer(int* index, float* vertex, int* vertex_color);
        

        //STL出力
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ExportSolidSTL(char* filename, csgSolid solid);
       //Pov出力
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void ExportSolidPov(char* filename, csgSolid solid);

        //ソリッド保存(OBJ)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SaveSolid(csgSolid solid, string filename);
        
        //ソリッド保存(内部形式（バイナリー）)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SaveSolidBin(csgSolid solid, string filename);

        //ソリッド読み込み(OBJ)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid LoadSolid(string filename);
        //ソリッド読み込み(内部形式（バイナリー）)
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid LoadSolidBin(string filename);


        //ソリッド掃引1
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidSweep(csgSolid solid, int num, float* pos, float* dir);
        //ソリッド掃引2
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern csgSolid SolidLinerSweep(csgSolid solid, int num, float* s_pos, float* s_dir, float* e_pos, float* e_dir);

        //RGBからcsgSolidのカラー型に変換する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int RGBToSolidColor(csgSolid solid, int r, int g, int b);

        //csgSolidのカラー型からRGBに変換する
        [System.Runtime.InteropServices.DllImport("csg_solid_modeller.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SolidColorToRGB(csgSolid solid, int color, int* r, int* g, int* b);

        //C#の配列がそのまま使えないためのラップメソッド
        static public csgSolid CreateSolid(int n, int[] index, int npnt, float[] vertex, int [] color)
        {
            csgSolid solid2 = (csgSolid)0;
            //頂点バッファからソリッドを作成
            fixed (int* indexArray = index)
            {
                fixed (float* vertexArray = vertex)
                {
                    fixed (int* colorArray = color)
                    {
                        //頂点インデックスと頂点からソリッドを生成
                        solid2 = CSolidBoolean.CreateSolid(n, indexArray, npnt, vertexArray, colorArray);
                    }
                }
            }
            return solid2;
        }
        static public csgSolid CreateSolid(int n, int[] index, int npnt, float[] vertex)
        {
            csgSolid solid2 = (csgSolid)0;
            //頂点バッファからソリッドを作成
            fixed (int* indexArray = index)
            {
                fixed (float* vertexArray = vertex)
                {
                    //頂点インデックスと頂点からソリッドを生成
                    solid2 = CSolidBoolean.CreateSolid(n, indexArray, npnt, vertexArray, (int*)0);
                }
            }
            return solid2;
        }
        static public csgBooleanModeller CreateSolidModeller(csgSolid solid1, csgSolid solid2, out int status)
        {
            csgBooleanModeller modeller = (csgBooleanModeller)0;
            fixed (int* stat = &status)
            {
                modeller = CSolidBoolean.CreateSolidModeller(solid1, solid2, stat);
            }
            return modeller;
        }      

        static public void SolidTranslate(csgSolid solid, float[] t)
        {
            fixed (float* sift = t)
            {
                CSolidBoolean.SolidTranslate(solid, sift);
            }
        }
        static public void SolidScale(csgSolid solid, float[] t)
        {
            fixed (float* scale = t)
            {
                CSolidBoolean.SolidScale(solid, scale);
            }
        }
        static public void SolidRotate(csgSolid solid, float[] t)
        {
            fixed (float* axis = t)
            {
                CSolidBoolean.SolidRotate(solid, axis);
            }
        }
        static public void SolidRotateAnyAxis(csgSolid solid, float[] p1, float [] p2, float angle)
        {
            fixed (float* pnt1 = p1)
            {
                fixed (float* pnt2 = p2)
                {
                    CSolidBoolean.SolidRotateAnyAxis(solid, pnt1, pnt2, angle);
                }
            }
        }
    }
}
