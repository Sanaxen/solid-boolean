#ifndef thread_omp_util_h

#define thread_omp_util_h

/*

forループの中にbreakがある場合
OpenMPで並列化しようとしてもコンパイルエラーとなって並列化出来ない。

	#pragma omp parallel　for
	for ( int i = 0; i < 1000; i++ )
	{
		.........
		if ( xxxx )
		{
			break;
		}
		......
	}

以下のようにしてThread_omp_LoopInBreakクラスを使うことでbreakがforループにあっても並列化出きる

#include "openmp_config.h"

#pragma omp parallel
{
	Thread_omp_LoopInBreak omp(並列化数, 1000);
	
	for ( int i = omp.Begin(); omp.Condition(i); i++ )
	{
		.........
		if ( xxxx )
		{
			MY_LOOP_BREAK(omp);
		}
		......
	}
}
*/
	
class Thread_omp_LoopInBreak
{
	int threadNum_;			//スレッド数設定
	int	loopNum_;			//ループ数 ( 0...loopNum-1)
	int threadId;			//スレッドID
	
	//並列化されている今のタスクが行うべきループ処理の開始と終了値
	int i_st;
	int i_ed;
	
	int* break_flag_;
	int start_i_;
	
public:
	Thread_omp_LoopInBreak( int threadNum, int loopNum, int start_i, int* break_flag):threadNum_(threadNum),loopNum_(loopNum),start_i_(start_i), break_flag_(break_flag)
	{

#ifdef USE_OPENMP	
		threadNum_ = threadNum;
		loopNum_ = loopNum;

		//並列化されているときの今のタスクのスレッドID
		if ( threadNum_ > 1 ) threadId = omp_get_thread_num();
		else threadId = 0;
		
		//並列化の分割ステップ
		int n = ((loopNum_ - start_i_)  + threadNum_ -1 )/threadNum_;

		//並列化されている今のタスクが行うべきループ処理の開始と終了値
		i_st = start_i_ + n*threadId;
		i_ed = ( i_st + n < loopNum_ ) ? i_st + n: loopNum_;
#else
		threadNum_ = 1;
		loopNum_ = loopNum;
		threadId = 0;
		i_st = start_i_;
		i_ed = loopNum_;
#endif
		//printf("threadNum(%d) loop %d ～ %d\n", threadId, i_st, i_ed );

	}
	
	inline int __fastcall Id()
	{
		//並列化されているときの今のタスクのスレッドID
		return threadId;
	}
	

	inline int __fastcall Begin()
	{
		return i_st;
	}
	inline int __fastcall End()
	{
		return i_ed;
	}
	
	inline bool __fastcall Condition(const int loopCounter)
	{
		return (!(*break_flag_) && loopCounter < i_ed);
	}
	
	inline void __fastcall LoopBreak()
	{
		*break_flag_ = true;
	}
};

#ifdef USE_OPENMP
#define MY_LOOP_BREAK( omp )	omp.LoopBreak();continue
#else
#define MY_LOOP_BREAK(omp)		break
#endif


class Thread_omp_limit
{
	int numthreadsv;

public:
	int numthread;
	inline Thread_omp_limit(int threadNum)
	{
#ifdef USE_OPENMP
		numthread = omp_get_max_threads();	//指定されている利用可能なスレッド数
		numthreadsv = numthread;			//現在の指定されている利用可能なスレッド数の保存

		//スレッド数制限threadNumを超えていたら調整する
		if ( numthread >= threadNum )
		{
			omp_set_num_threads(threadNum);
		}
		numthread = omp_get_max_threads ();
#endif
	}
	inline Thread_omp_limit()
	{
#ifdef USE_OPENMP
		numthread = omp_get_max_threads();	//指定されている利用可能なスレッド数
		numthreadsv = numthread;			//現在の指定されている利用可能なスレッド数の保存

		numthread = 1;
#endif
	}

	inline void __fastcall Undo()
	{
#ifdef USE_OPENMP
		//最初のスレッドMAX数に戻しておく
		omp_set_num_threads(numthreadsv);
#endif
	}

	inline ~Thread_omp_limit()
	{
		Undo();
	}
};

class Thread_omp_lock
{
#ifdef USE_OPENMP
	omp_lock_t my_lock;
#endif

public:
	inline Thread_omp_lock()
	{
#ifdef USE_OPENMP
		omp_init_lock(&my_lock);
#endif
	}

	inline ~Thread_omp_lock()
	{
#ifdef USE_OPENMP
		omp_destroy_lock(&my_lock);
#endif
	}

	inline void Lock()
	{
#ifdef USE_OPENMP
		omp_set_lock(&my_lock);
#endif
	}

	inline void UnLock()
	{
#ifdef USE_OPENMP
		omp_unset_lock(&my_lock);
#endif
	}

};

#endif

