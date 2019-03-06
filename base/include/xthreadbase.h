#pragma once
#ifdef WIN32
	#include <process.h>
	#include <Windows.h>
#else
	#define __stdcall
	#include<pthread.h>
#endif
#include "stdio.h"
#include "timebase.h"
#include "xAutoLock.h"
//pthread_mutex_t zx;
namespace SAEBASE{
//xMutex zx;
#pragma once
#ifdef  WIN32
	typedef unsigned int (__stdcall*pfunc)(void*);
#else
	typedef void* (__stdcall pfunc)(void*);
#endif
//���ڶ����ģʽ
class Threadbase
{
public:
	Threadbase(bool bDetach=true);
	virtual ~Threadbase(){};
	virtual void run()=0;		//ҵ��ӿ�
	int start();			//�����߳�
	int join();				//�ȴ��߳̽���
	void destory();			//�����߳����������Դ

	int get_thread_id(){return thr_id;}
	int set_thread_id(unsigned long thrID){thr_id=thrID;}

protected:
#ifdef WIN32
	static unsigned int __stdcall thread_proxy(void* arg);
#else
	static void*  __stdcall thread_proxy(void* arg);
#endif
private:
#ifdef WIN32
	size_t thr_id;
#else
	pthread_t thr_id;
#endif
	bool bExit_;			//�߳��Ƿ�Ҫ�˳���־
	xCondition m_ConditionState;
	xMutex m_LockState;
};
//��������ģʽ
class xThread
{
public:
	xThread(bool bDetach=true):thr_id(0)
	{

	}
	virtual ~xThread(){};
	int start(pfunc func,void *arg);			//�����߳�
	int join();				//�ȴ��߳̽���
	void destory();			//�����߳����������Դ

	int get_thread_id(){return thr_id;}
	int set_thread_id(unsigned long thrID){thr_id=thrID;}
public:
#ifdef WIN32
	size_t thr_id;
#else
	pthread_t thr_id;
#endif
	bool bExit_;
};





}