
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_COND_H_
#define _W_COND_H_

#include <pthread.h>

#include "wNoncopyable.h"
#include "wMutex.h"

class wCond : private wNoncopyable
{
	public:
		wCond(int pshared = PTHREAD_PROCESS_PRIVATE)
		{
			pthread_condattr_init(&mAttr);
			pthread_condattr_setpshared(&mAttr, pshared);
			pthread_cond_init(&mCond, &mAttr);
		}
		
		~wCond()
		{
			pthread_condattr_destroy(&mAttr);
			pthread_cond_destroy(&mCond);
		}
		
		int Broadcast()
		{
			return pthread_cond_broadcast(&mCond);
		}
		
		/**
		 *  ʹ��pthread_cond_signal�����С���Ⱥ���󡱲����������ֻ��һ���̷߳��ź�
		 */
		int Signal()
		{
			return pthread_cond_signal(&mCond);
		}

		/**
		 * �ȴ��ض���������������
		 * @param stMutex ��Ҫ�ȴ��Ļ�����
		 */
		int Wait(wMutex &stMutex)
		{
			return pthread_cond_wait(&mCond, &stMutex.mMutex);
		}
		
		int TimeWait(wMutex &stMutex, struct timespec *tsptr)
		{
			return pthread_cond_timewait(&mCond, &stMutex.mMutex, tsptr);
		}
		
	private:
		pthread_cond_t mCond;		//ϵͳ��������
		pthread_condattr_t mAttr;
};


#endif