//2018-12-5
//�������ڶ�����̳߳�
//      threadpool --------taskbase.run()
//2019-3-3 �޸��̳߳� by xjshi
/* 
xthreadPool ��Ϊһ�������࣬����Ҫwait������ͳһ�������
threadobj������߶�����̵߳Ķ��󣬵���û���Լ�������ء�
*/
#pragma once
#include <stdio.h>
#include <string>
#include <list>
#include <exception>
#include "xAutoLock.h"
#include "xbaseclass.h"
#include "xthreadbase.h"
#include "xTaskqueue.h"

#ifndef _OUT
	#define OUT 
#endif
#ifndef _IN
	#define IN 
#endif

using namespace std;
namespace SAEBASE{
//�������ļ����ࡣÿ��job����xtaskbase������Ķ���
class xtaskbase
{
public:
	xtaskbase(void *arg=NULL,const std::string taskName = "")
		:arg_(arg),taskName_(taskName)
	{

	}
	virtual ~xtaskbase(){}
	void setArg(void *arg){arg_=arg;};
	virtual int run() {return 0;} ; //��������дrun��������ʵ��ҵ���߼���
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
			xThread* workThread=new xThread;
			workThread->start(threadproxy,workThread);
			m_threadList.push_back(workThread);
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
			//delete *ite;
			//(*ite)->destory();
		}
		for( vector<xThread*>::iterator ite=m_threadList.begin();ite!=m_threadList.end();ite++)
		{
			//(*ite)->join();
			delete *ite;
			//(*ite)->destory();
		}
		m_threadList.clear();
	}	
    void pushObj(xtaskbase*task)
	{
		//xAutoLock L(m_threadLock);
		m_threadLock.lock();
		m_taskList.push_back(task);
		m_threadLock.unlock();
		m_threadCond.signal();
	}
	void waitforAllTaskDone(void)
	{
		for( vector<xThread*>::iterator ite=m_threadList.begin();ite!=m_threadList.end();ite++)
		{
			(*ite)->join();
		}
	}
	int getTaskSize(){return m_taskList.size();}
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

//��Ҫ���óɲ��ɸ��Ƶ���
class xthreadPool/*:protected Noncopyable*/
{
public:
	//typedef typename xTaskqueue<xtaskbase> xTaskqueue_type; 
	xthreadPool(const char* poolname):m_strPoolName(poolname)
	{
		//m_strPoolName=string(poolname);
	}
	virtual ~xthreadPool(){stopPool();};
	//��ʼ���̳߳�
	//	LowThreadNumber ��С�ͳ�����
	//	HightThreadNumber ����߳�����
	//	IdleTimeout		�̳߳�ʱ�ȴ�ʱ��
	//	Tasklistsize	�������Ĭ�ϴ�С
	//	�̶߳�ջ��С  
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
	//���
	void initsimplePool(size_t ThreadNumber=4,size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024);
	void startPool(bool defaultpools=false );
	void stopPool(bool defaultpools=true)  //Ĭ��defaultpoolsΪtrue������������߳�
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
	void waitforAllTaskDone(void) // �ȴ������������
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
// 	inline void exetask(); //ִ������
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

};