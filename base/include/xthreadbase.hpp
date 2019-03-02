#pragma once
#ifdef WIN32
	#include <process.h>
	#include <Windows.h>
#else
	#define __stdcall
	#include<pthread.h>
#endif
#include "stdio.h"
#include "timebase.hpp"
#include "xAutoLock.hpp"
//pthread_mutex_t zx;
namespace SAEBASE{
xMutex zx;
#pragma once
#ifdef  WIN32
	typedef unsigned int (__stdcall*pfunc)(void*);
#else
	typedef void* (__stdcall pfunc)(void*);
#endif
//基于对象的模式
class Threadbase
{
public:
	Threadbase(bool bDetach=true);
	virtual ~Threadbase(){};
	virtual void run()=0;		//业务接口
	int start();			//启动线程
	int join();				//等待线程结束
	void destory();			//销毁线程所申请的资源

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
	bool bExit_;			//线程是否要退出标志

};
//面向对象的模式
class xThread
{
public:
	xThread(bool bDetach=true):thr_id(0)
	{

	}
	virtual ~xThread(){};
	int start(pfunc func,void *arg);			//启动线程
	int join();				//等待线程结束
	void destory();			//销毁线程所申请的资源

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


#ifdef WIN32
	 unsigned int __stdcall Threadbase::thread_proxy(void* arg)
#else
	 void*  __stdcall Threadbase::thread_proxy(void* arg)
#endif
{

	xAutoLock antolock(zx);
	timeobj proxylife;
	Threadbase* pbase=reinterpret_cast<Threadbase*> (arg);
	pbase->run();
	//Sleep(3000);
	return 0;
}

Threadbase::Threadbase(bool bDetach)
{
	thr_id=0;
}
int Threadbase::start()
{

#ifdef WIN32
	unsigned int nval=_beginthreadex(0,0,thread_proxy,this,0,&thr_id);
	thr_id=nval;
#else
	pthread_attr_t attr;
	int arg=0;
	pthread_create(&thr_id,NULL,thread_proxy,this);
#endif
	return 0;
}
int Threadbase::join()
{
#ifdef WIN32
	//WAIT_OBJECT 表示执行结束
	if(WaitForSingleObject(reinterpret_cast<HANDLE>(thr_id),INFINITE)==WAIT_OBJECT_0)
	{
		printf("\n join thread %d finish\n",thr_id);
	}
#else
	pthread_join(thr_id,NULL);
#endif
	return 0;
}
void Threadbase::destory()
{
#ifdef WIN32
	CloseHandle(reinterpret_cast<HANDLE>(thr_id));
#else
	int thread_return;
	pthread_exit((void*)&thread_return);
#endif
}
//////////////////////////////////////////////////////////////////////////
//xThread 实现


int xThread::start(pfunc func,void *arg)
{
#ifdef WIN32
	unsigned int nval=_beginthreadex(0,0,func,arg,0,&thr_id);
	thr_id=nval;
#else
	pthread_create(&thr_id,NULL,func,arg);
#endif
	return 0;
}
int xThread::join()
{
#ifdef WIN32
	//WAIT_OBJECT 表示执行结束
	if(WaitForSingleObject(reinterpret_cast<HANDLE>(thr_id),INFINITE)==WAIT_OBJECT_0)
	{
		printf("\n join thread %d finish\n",thr_id);
	}
#else
	pthread_join(thr_id,NULL);
#endif
	return 0;
}
void xThread::destory()
{
#ifdef WIN32
	CloseHandle(reinterpret_cast<HANDLE>(thr_id));
#else
	int thread_return;
	pthread_exit((void*)&thread_return);
#endif
}
}