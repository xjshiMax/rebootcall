//2018-11-29
//�Զ���
//pthread_mutex ��Mutex
#pragma once
#ifdef WIN32
	#include <Windows.h>
	#include "..\pthread\Pre-built.2\include\pthread.h"
//#pragma comment ���·�������ù��̵���·�������·��������xAutoLock.hpp����Ŀ¼�����·����
	#pragma comment(lib,"pthreadVC2.lib")
	#pragma comment(lib,"pthreadVCE2.lib")
	#pragma comment(lib,"pthreadVSE2.lib")
#else
	#include <pthread.h>
	#include<sys/time.h>
#endif
#include "xbaseclass.h"
namespace SAEBASE{
#define Mutex xMutex
#define CONST_NAO_PER_SECOND 	1000000000L
#define CONST_NAO_PER_MICRO 	1000L
static const struct timespec maxTimeWait={-1,-1};  //���øó�ʱʱ������޵ȴ���ֱ�����źŲ���
static const struct timespec zeroTimeWait={0,0};
inline static bool TimevalIsequal(const struct timespec&first,const struct timespec&second)
{
	if(first.tv_sec == second.tv_sec && first.tv_nsec==second.tv_nsec)
		return true;
	return false;
}

class xMutex
{
public:
	xMutex(){pthread_mutex_init(&m_lock,NULL);}
	~xMutex(){pthread_mutex_destroy(&m_lock);}
	void lock() const 
	{
		pthread_mutex_lock(&m_lock);
	}
	void unlock() const 
	{
		pthread_mutex_unlock(&m_lock);
	}
	bool tryLock() const 
	{
		pthread_mutex_trylock(&m_lock);
	}
private:
	mutable pthread_mutex_t m_lock;
	friend class xCondition;
};
class xAutoLock
{
public:
	//xAutoLock(const Mutex* mutex):m_mutex(mutex)
	//{
	//	//pthread_mutex_lock(&m_mutex);
	//	m_mutex->lock();
	//	printf("lock\n");
	//}
	xAutoLock(const Mutex & mutex):m_mutex(&mutex)
	{
		//pthread_mutex_lock(&m_mutex);
		m_mutex->lock();
		//printf("lock\n");
	}
	~xAutoLock()
	{
		//pthread_mutex_unlock(&m_mutex);
		if(m_mutex)
			m_mutex->unlock();
		m_mutex=NULL;
		//printf("unlock\n");
	}
private:
	const Mutex* m_mutex;
};

//�����ź�
class xCondition:protected Noncopyable
{
public:
	xCondition(void)throw(){pthread_cond_init(&m_cond,NULL);}
	~xCondition(){pthread_cond_destroy(&m_cond);}

	//����һ���ȴ��̣߳����ڶ���ȴ��߳�ʱ�����˳�򼤻�����һ����
	void signal(void)
	{
		int Ret = pthread_cond_signal(&m_cond);
			
	}
	//�������еȴ��߳�
	void broadCast(void)
	{
		int Ret = pthread_cond_broadcast(&m_cond);
	}
	//wait �ȴ������Ĵ���
	//�ڵ���wait�Լ�timewait�ӿ�֮ǰ��һ��Ҫ������������߳�ͬʱ���ʡ�
	void wait(xMutex &lock)
	{
		int Ret = pthread_cond_wait(&m_cond,&(lock.m_lock));
	}
	// timewait ʱ��ȴ�������ڸ���ʱ��ǰ����û�����㣬�򷵻�ETIMEOUT,�����ȴ���
	//���в���abstime����time()ϵͳ������ͬ����ľ���ʱ����ʽ���֣�0��ʾ1970��1��1��0ʱ0��0��
	bool timewait(xMutex &lock,const struct timespec&abstime)
	{
		if( abstime.tv_sec == maxTimeWait.tv_sec &&abstime.tv_nsec==maxTimeWait.tv_nsec)
		{
			wait(lock);
			return true;
		}
		//�������ʱ������������޵ȴ��������ó�ʱʱ��Ϊ����ʱ����ϴ����ʱ����
#if defined(WIN32)
		SYSTEMTIME	system_time;
		FILETIME	ft;
		GetLocalTime( &system_time );
		SystemTimeToFileTime(&system_time, &ft);  // converts to file time format

		LONGLONG nLL;
		ULARGE_INTEGER ui;
		ui.LowPart = ft.dwLowDateTime;
		ui.HighPart = ft.dwHighDateTime;
		nLL = (ft.dwHighDateTime << 32) + ft.dwLowDateTime;
		long  _nSec = (long)((LONGLONG)(ui.QuadPart - 116444736000000000)/10000000-28800);
		long _nNSec = (long)((LONGLONG)(ui.QuadPart - 116444736000000000)*100%1000000000);
#else
		timeval time_value;
		gettimeofday( &time_value, NULL );
		LONGLONG _nSec = time_value.tv_sec;
		LONGLONG _nNSec = time_value.tv_usec * CONST_NAO_PER_MICRO;
#endif // CMX_WIN32_VER
		struct timespec timeval_;
		timeval_.tv_sec=abstime.tv_sec;
		timeval_.tv_nsec=abstime.tv_nsec;
		timeval_.tv_sec+=_nSec;
		timeval_.tv_nsec+=_nNSec;
		if(timeval_.tv_nsec>=CONST_NAO_PER_SECOND)
		{
			timeval_.tv_nsec-=CONST_NAO_PER_SECOND;
			timeval_.tv_sec+=1;
		}

 		return timeWaitUntil(lock,timeval_);
	//	return true;
	}
	//�����ʱ���ͷ��أ�����ȵ������źţ�Ҳ���ء�
	bool timeWaitUntil(xMutex&lock,const struct timespec& waitTime)
	{
		if(waitTime.tv_sec == maxTimeWait.tv_sec &&waitTime.tv_nsec==maxTimeWait.tv_nsec)
		{
			wait(lock);
			return true;
		}
		struct timespec abstime;
		abstime.tv_sec = waitTime.tv_sec;
		abstime.tv_nsec = waitTime.tv_nsec;
		//RetΪ0��ȷ����������ʧ��
		int Ret = pthread_cond_timedwait(&m_cond,&(lock.m_lock),&abstime);
		return true;
	}
protected:
// 	void condUnblock(int nUnBlockAll);
// 	long m_WaitersBlocked;			//�������߳���
// 	long m_WaitersGone;				//��ʱ���߳���
// 	long m_WaitersUnblocked;		//δ�������߳���

	pthread_cond_t m_cond;			//pthread_cond_t ��ʾ���̵߳��������������ڿ����̵߳ȴ��;�����������
};

//���ԡ� waitfuc()��������wait�����ͻ��������ȴ��źš�
//xMutex mymutex;
//xCondition myCondition;
//static unsigned int __stdcall waitfuc(void *)
//{
//	xAutoLock L(mymutex);
//	printf("i am waittinf for the signal\n");
//	myCondition.wait(mymutex);
//	printf("---it coming!\n ");
//	return 0;
//}
//static unsigned int __stdcall siginalfuc(void *)
//{
//	xAutoLock L(mymutex);
//	printf("give the sigianl;\n");
//	myCondition.signal();
//	return 0;
//}
//void testcondition()
//{
//	xThread mythread[2];
//	mythread[0].start(waitfuc,"");
//	//xMutex mutex;
//	Sleep(2000);
//	mythread[1].start(siginalfuc,"");
//	mythread[0].join();
//	mythread[1].join();
//
//}

}