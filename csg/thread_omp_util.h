#ifndef thread_omp_util_h

#define thread_omp_util_h

/*

for���[�v�̒���break������ꍇ
OpenMP�ŕ��񉻂��悤�Ƃ��Ă��R���p�C���G���[�ƂȂ��ĕ��񉻏o���Ȃ��B

	#pragma omp parallel�@for
	for ( int i = 0; i < 1000; i++ )
	{
		.........
		if ( xxxx )
		{
			break;
		}
		......
	}

�ȉ��̂悤�ɂ���Thread_omp_LoopInBreak�N���X���g�����Ƃ�break��for���[�v�ɂ����Ă����񉻏o����

#include "openmp_config.h"

#pragma omp parallel
{
	Thread_omp_LoopInBreak omp(���񉻐�, 1000);
	
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
	int threadNum_;			//�X���b�h���ݒ�
	int	loopNum_;			//���[�v�� ( 0...loopNum-1)
	int threadId;			//�X���b�hID
	
	//���񉻂���Ă��鍡�̃^�X�N���s���ׂ����[�v�����̊J�n�ƏI���l
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

		//���񉻂���Ă���Ƃ��̍��̃^�X�N�̃X���b�hID
		if ( threadNum_ > 1 ) threadId = omp_get_thread_num();
		else threadId = 0;
		
		//���񉻂̕����X�e�b�v
		int n = ((loopNum_ - start_i_)  + threadNum_ -1 )/threadNum_;

		//���񉻂���Ă��鍡�̃^�X�N���s���ׂ����[�v�����̊J�n�ƏI���l
		i_st = start_i_ + n*threadId;
		i_ed = ( i_st + n < loopNum_ ) ? i_st + n: loopNum_;
#else
		threadNum_ = 1;
		loopNum_ = loopNum;
		threadId = 0;
		i_st = start_i_;
		i_ed = loopNum_;
#endif
		//printf("threadNum(%d) loop %d �` %d\n", threadId, i_st, i_ed );

	}
	
	inline int __fastcall Id()
	{
		//���񉻂���Ă���Ƃ��̍��̃^�X�N�̃X���b�hID
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
		numthread = omp_get_max_threads();	//�w�肳��Ă��闘�p�\�ȃX���b�h��
		numthreadsv = numthread;			//���݂̎w�肳��Ă��闘�p�\�ȃX���b�h���̕ۑ�

		//�X���b�h������threadNum�𒴂��Ă����璲������
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
		numthread = omp_get_max_threads();	//�w�肳��Ă��闘�p�\�ȃX���b�h��
		numthreadsv = numthread;			//���݂̎w�肳��Ă��闘�p�\�ȃX���b�h���̕ۑ�

		numthread = 1;
#endif
	}

	inline void __fastcall Undo()
	{
#ifdef USE_OPENMP
		//�ŏ��̃X���b�hMAX���ɖ߂��Ă���
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

