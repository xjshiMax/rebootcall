//2018-12-5
//�������ڶ�����̳߳�
//      threadpool --------taskbase.run()
//
#include <stdio.h>
#include <string>
#include <list>
#include "xAutoLock.hpp"
#include "xbaseclass.hpp"
#include "xthreadbase.hpp"
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
	virtual int run() = 0;
protected:
	void* arg_;
	std::string taskName_;

};

//��Ҫ���óɲ��ɸ��Ƶ���
class xthreadPool:protected Noncopyable
{
public:
	typedef typename xTaskqueue<xtaskbase> xTaskqueue_type; 
	xthreadPool(const char* poolname);
	virtual ~xthreadPool(){stopPool();};
	//��ʼ���̳߳�
	//	LowThreadNumber ��С�ͳ�����
	//	HightThreadNumber ����߳�����
	//	IdleTimeout		�̳߳�ʱ�ȴ�ʱ��
	//	Tasklistsize	�������Ĭ�ϴ�С
	//	�̶߳�ջ��С  
// 	void initPool(size_t LowThreadNumber=4,size_t HightThreadNumber=16,
// 		size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024 );
//���
	void initsimplePool(size_t ThreadNumber=4,size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024);
	void startPool(bool defaultpools=false );
	void stopPool(bool defaultpools=true)  //Ĭ��defaultpoolsΪtrue������������߳�
	{

	}
	bool pushObj();
	bool trypushObj(xtaskbase*node,const struct timespec & Timeout=maxTimeWait)
	{
		//if(m_boStartPool && isBusy())/
		m_tasklist.pushTaskWithTimeOut(node,Timeout);
		xAutoLock L(m_lockForFinishJob);
		++m_nJobNotFinish;
		return true;

		
	}
	void waitforAllTaskDone(void) // �ȴ������������
	{
		xAutoLock L(m_lockForFinishJob);
		while(m_nJobNotFinish)
			m_CondForFinishJob.wait(m_lockForFinishJob);
	}

	bool isIdle(void) const {}    //�̳߳ض����Ƿ�Ϊ��
	bool isBusy(void) const {}		//�̳߳��й����̴߳��ڿ����߳�
	bool isFull(void) const {}		//�̳߳��Ƿ�����
	void dump();					//��ȡ�̳߳���Ϣ������һ��ʹ�÷ָ���
	virtual void onThreadBuild();
	virtual void onThreadEnd();
	bool waitforjob(xthreadPool pool,const struct timespec &timeout,OUT xtaskbase & tasknode);
	static unsigned int __stdcall proxythread(void*arg);

	void dealTask(xtaskbase* node)
	{
		if(node!=NULL);
		node->run();
	}
	void detachTask();
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
protected:
	const string	m_strPoolName;
	volatile bool	m_boStartPool;
// 	size_t			m_nThreadLowThreshold;
// 	size_t			m_nThreadHighThreshold;
	size_t			m_threadNum;
	volatile size_t	m_nFreeThreadCount;
//	size_t			m_nMaxThreadUsedCount;
	size_t			m_nIdleTimeout;
	size_t			m_nThreadStackSize;
	size_t			m_nJobNotFinish;
	xMutex			m_lockForThread,m_lockForFinishJob;
	xCondition		m_CondForThreadStop,m_CondForBuildThread,m_CondForFinishJob;
	xTaskqueue_type		m_tasklist;
	std::list<xThread> m_ThreadList;
	xMutex			m_waitforjob;
};

void xthreadPool::initPool(size_t ThreadNumber=4,size_t IdleTimeout=30,size_t Tasklistsize=3000,size_t threadStackSize=1024*1024)
{
	stopPool();
	m_nFreeThreadCount=0;
	m_nIdleTimeout=IdleTimeout;
	m_threadNum = ThreadNumber;
	m_nThreadStackSize = threadStackSize;
	m_tasklist.resizeQueue(Tasklistsize);

}
void xthreadPool::startPool(bool defaultpools=false )
{
	m_tasklist.setActive();// �����������Ϊ����״̬��������ӺͶ�ȡ����ʧ��
	//size_t threadnum = m_threadNum?m_threadNum:4
	for(size_t i=0;i<m_threadNum;i++)
	{
		xAutoLock L(m_lockForThread);
		xThread thread;
		m_ThreadList.push_back(thread);
		thread.start(proxythread,this);
		m_CondForBuildThread.wait(m_lockForThread);
	}

}
unsigned int xthreadPool::proxythread(void*arg)
{
	//xAutoLock antolock(m_lockForThread);
	
	//timeobj proxylife;
	xthreadPool* pthreadpool=reinterpret_cast<xthreadPool*> (arg);
	try
	{
		xtaskbase* xnode;
		do{
		if(pthreadpool->waitforjob(pthreadpool->m_tasklist,maxTimeWait,*xnode))
		{
			pthreadpool->dealTask(xnode);
			//pthreadpool->detachtask();
			pthreadpool->onFinishTask();
		}
		else
			break;
		}while(1);
	}
	catch (CMemoryException* e)
	{
		
	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}
}
bool xthreadPool::waitforjob(xthreadPool pool,const struct timespec &timeout,OUT xtaskbase & tasknode)
{
	//xtaskbase tasknode;
	if(!m_tasklist.waitForTask(tasknode,timeout))
		return false;
	return true;
	//xAutoLock _(m_waitforjob);
	//if()
}
}