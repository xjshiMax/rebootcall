//2018-12-5
//创建基于对象的线程池
//      threadpool --------taskbase.run()
//2019-3-3 修改线程池 by xjshi
/* 
xthreadPool 作为一个管理类，不需要wait操作。统一的任务池
threadobj在类外边定义带线程的对象，但是没有自己的任务池。
*/
#include <stdio.h>
#include <string>
#include <list>
#include <exception>
#include "xAutoLock.hpp"
#include "xbaseclass.hpp"
#include "xthreadbase.hpp"
#include "xTaskqueue.h"

#ifndef _OUT
	#define OUT 
#endif
#ifndef _IN
	#define IN 
#endif

using namespace std;
namespace SAEBASE{
//任务对象的即基类。每个job都是xtaskbase派生类的对象
class xtaskbase
{
public:
	xtaskbase(void *arg=NULL,const std::string taskName = "")
		:arg_(arg),taskName_(taskName)
	{

	}
	virtual ~xtaskbase(){}
	void setArg(void *arg){arg_=arg;};
	virtual int run() {return 0;} ; //任务类重写run，在里面实现业务逻辑。
protected:
	void* arg_;
	std::string taskName_;

};
//class xthreadPool;

class xsimpleThreadPool
{
public:
	xsimpleThreadPool(){}
	xsimpleThreadPool(const char*poolname):m_PoolName(poolname){}
	void initPool(size_t LowThreadNumber=4,size_t HightThreadNumber=16)
	{
		m_threadNum=LowThreadNumber;
	}
	void startPool(bool defaultpools=false )
	{
		for(int i=0;i<m_threadNum;i++)
		{
			xThread workThread;
			workThread.start(threadproxy,this);
			m_threadList.push_back(&workThread);
			//workThread.start(threadproxy,this);
		}
	}
	void stopPool(bool defaultpools=true)
	{
		if(shutdown)
		{
			return  ;
		}
		shutdown = true;
		m_threadCond.broadCast();
		for( vector<xThread*>::iterator ite=m_threadList.begin();ite!=m_threadList.end();ite++)
		{
			(*ite)->join();
			(*ite)->destory();
		}
	}	
    void pushObj(xtaskbase*task)
	{
		xAutoLock L(m_threadLock);
		m_taskList.push_back(task);
		m_threadCond.signal();
	}
	void waitforAllTaskDone(void)
	{
		for( vector<xThread*>::iterator ite=m_threadList.begin();ite!=m_threadList.end();ite++)
		{
			(*ite)->join();
		}
	}
#ifdef WIN32
	 static unsigned int __stdcall threadproxy(void* arg);
#else
	 static void*  __stdcall threadproxy(void* arg);
#endif

private:
	static std::deque<xtaskbase*> m_taskList;
	std::vector<xThread*> m_threadList;
	int m_threadNum;
	static xMutex m_threadLock;
	static xCondition m_threadCond;
	static bool shutdown;
	string m_PoolName;
};
bool xsimpleThreadPool::shutdown=false;
xMutex xsimpleThreadPool::m_threadLock;
xCondition xsimpleThreadPool::m_threadCond;
std::deque<xtaskbase*> xsimpleThreadPool::m_taskList;
#ifdef WIN32
	 unsigned int __stdcall xsimpleThreadPool::threadproxy(void* arg)
#else
	 void*  __stdcall xsimpleThreadPool::threadproxy(void* arg)
#endif
{
	pthread_t tid = pthread_self();
	xThread* pthread=(xThread*)arg;
	while(1)
	{
		xAutoLock L(m_threadLock);
		while(m_taskList.size()==0&&!shutdown)
		{
			m_threadCond.wait(m_threadLock);
		}
		if(shutdown)
		{
			pthread->destory();
		}
		xtaskbase* task=m_taskList.front();
		m_taskList.pop_front();
		task->run();
	}
	return 0;
}
//需要设置成不可复制的类
class xthreadPool/*:protected Noncopyable*/
{
public:
	//typedef typename xTaskqueue<xtaskbase> xTaskqueue_type; 
	xthreadPool(const char* poolname):m_strPoolName(poolname)
	{
		//m_strPoolName=string(poolname);
	}
	virtual ~xthreadPool(){stopPool();};
	//初始化线程池
	//	LowThreadNumber 最小贤臣数量
	//	HightThreadNumber 最大线程数量
	//	IdleTimeout		线程超时等待时间
	//	Tasklistsize	任务队列默认大小
	//	线程堆栈大小  
	void initPool(size_t LowThreadNumber=4,size_t HightThreadNumber=16,
		size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024 )
		{
			stopPool();
			m_nFreeThreadCount=0;
			m_nIdleTimeout=IdleTimeout;
			m_threadNum = LowThreadNumber;
			m_nThreadStackSize = threadStackSize;
			m_tasklist.resizeQueue(Tasklistsize);
		}
	//简版
	void initsimplePool(size_t ThreadNumber=4,size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024);
	void startPool(bool defaultpools=false );
	void stopPool(bool defaultpools=true)  //默认defaultpools为true，清除掉所有线程
	{
		xAutoLock lock(m_lockForThread);
		m_tasklist.setDeadstatus();
		std::list<threadobj*>::iterator ite = m_ThreadList.begin();
		while(ite!=m_ThreadList.end())
		{
			(*ite)->destory();
			ite++;
		}
		m_tasklist.clearAllTask();
	}
	bool pushObj();
	bool trypushObj(xtaskbase&node,const struct timespec & Timeout=maxTimeWait)
	{
		//if(m_boStartPool && isBusy())/
		m_tasklist.pushTaskWithTimeOut(node,Timeout);
		//xAutoLock L(m_lockForFinishJob);
		//++m_nJobNotFinish;
		return true;
	}
	void waitforAllTaskDone(void) // 等待所有任务结束
	{
		//xAutoLock L(m_lockForFinishJob);
		//while(m_nJobNotFinish)
			//m_CondForFinishJob.wait(m_lockForFinishJob);
	}
class threadobj:public Threadbase
{
public:
 	void beginthreadobj(xthreadPool* threadpool)
 	{
 		m_parentPool=threadpool;
 		start();
 	}
 	virtual void run()
 	{	
  		xtaskbase node;
  		while(1)
  		{
   			if(waitforjob(m_parentPool->m_tasklist,maxTimeWait,node))
   			{
   				(node).run();
  				//m_parentPool->onFinishTask();
   			}
  		}
 	}
// 	inline void exetask(); //执行任务
  	bool waitforjob(xTaskqueue<xtaskbase>&tasklist,const struct timespec &timeout,OUT xtaskbase & tasknode)
  	{
		if(!tasklist.waitForTask(tasknode,timeout))
			return false;
 		//xAutoLock L(m_mutex);
 		//m_task = &tasknode;
  		return true;
  	}
  	xtaskbase* m_task;
  	xMutex m_mutex;
  	xthreadPool* m_parentPool;
  	friend class xthreadPool;
};

	void onFinishTask()
	{
		xAutoLock L(m_lockForFinishJob);
		--m_nJobNotFinish;
		m_CondForFinishJob.signal();
	}
	bool addFreeThread()
	{
		xAutoLock L(m_lockForThread);
		++m_nFreeThreadCount;
		return true;
	}
	void destoryThread(){}
	void joinAllThread()
	{
		//xAutoLock L(m_lockForThread);
		std::list<threadobj*>::iterator ite = m_ThreadList.begin();
		while(ite!=m_ThreadList.end())
		{
			(*ite)->join();
			ite++;
		}
		
	}
protected:
	const string	m_strPoolName;
	volatile bool	m_boStartPool;
	size_t			m_threadNum;
	volatile size_t	m_nFreeThreadCount;
	size_t			m_nIdleTimeout;
	size_t			m_nThreadStackSize;
	size_t			m_nJobNotFinish;
	xMutex			m_lockForThread,m_lockForFinishJob;
	xCondition		m_CondForThreadStop,m_CondForBuildThread,m_CondForFinishJob;
	xTaskqueue<xtaskbase>		m_tasklist;
	std::list<threadobj*> m_ThreadList;
	xMutex			m_waitforjob;
	static xMutex   m_poollock;
};
xMutex  xthreadPool::m_poollock;
void xthreadPool::initsimplePool(size_t ThreadNumber,size_t IdleTimeout,size_t Tasklistsize,size_t threadStackSize)
{
	stopPool();
	m_nFreeThreadCount=0;
	m_nIdleTimeout=IdleTimeout;
	m_threadNum = ThreadNumber;
	m_nThreadStackSize = threadStackSize;
	m_tasklist.resizeQueue(Tasklistsize);

}
void xthreadPool::startPool(bool defaultpools )
{
	m_tasklist.setActive();// 设置任务队列为开启状态，否则添加和读取都会失败
	//size_t threadnum = m_threadNum?m_threadNum:4
	for(size_t i=0;i<m_threadNum;i++)
	{
		//xAutoLock L(m_lockForThread);
		threadobj thread;
		//m_ThreadList.push_back(&thread);
		thread.beginthreadobj(this);
		m_ThreadList.push_back(&thread);
	}

}
};