//2018-12-6
//û�нṹ��Ķ��У�Ҳ������stl
//boost���ڷ�װreactor��

//2018-12-7
//�̰߳�ȫ��������У��ɽ���̳߳�ʹ�á�
//Ӧ��˵�ö��о���׼�������̵߳�������---������ģʽ�Ķ��С�
//�ڶ��߳��ﶨ��һ�����С��ɹ������߳��ǵĴ�ȡ�������
//��ʵ���߼������пյ�ʱ�򣬶������̵߳ȴ���������д����е��߳̿��������������
//           ��������ʱ�򣬶������߳̿��������д����е��̵߳ȴ���
//           �������������δ���Ҳ��գ������̶߳�ȡ�����ģ��������߳�д�루������
#pragma once
#include "xAutoLock.h"
#include <deque>
namespace SAEBASE{
template <typename Taskobj>
class xTaskqueue
{
public:
	typedef typename std::deque<Taskobj> Tasklist;
	typedef typename std::deque<Taskobj>::iterator iterator;
	typedef typename std::deque<Taskobj>::const_iterator const_iterator;
	xTaskqueue(size_t size=1000):m_IsActive(true),
		m_Maxsize(size),m_CurTaskCount(0),m_CurReadWaiter(0),
		m_CurWriteWaiter(0)
	{}
		
	virtual ~xTaskqueue(void) {clearAllTask();}

	//�����Ƿ�active
	bool isActive(){return m_IsActive;}
	//�����������Ϊactive״̬
	void setActive(void)
	{
		xAutoLock L(m_LockTask);
		m_IsActive=true;
	}
	//�����������Ϊ����Ծ״̬
	void setDeadstatus(void);
	//���������������
	void clearAllTask(void);
	//���Ѷ���
	void wakeup(void);

	void resizeQueue(size_t MaxSize);

	//��ȡ��������������
	size_t getTaskCount(void) const {return m_CurTaskCount;}
	//��ȡ�����еȴ���read waiter�̡߳�
	size_t getCurReadTaskWaiter(void) const {return m_CurReadWaiter;}
	//��ȡ��ǰд�ȴ� writer waiter
	size_t getCurWriteTaskWaiter(void) const {return m_CurWriteWaiter;}
 	size_t getMaxSize(void)		const {return m_Maxsize;}			//��ȡ�����������
 	bool queueIsFull(void)	const {return m_CurTaskCount >= m_Maxsize;}				//�����Ƿ�����
 	bool queueIsEmpty(void)	const {return m_CurTaskCount ==0;}				//�����Ƿ�Ϊ��

	//�ȴ����񣬲����볬ʱʱ�䣬Ĭ�������޵ȴ���ֱ���źŵ���
	//������ݴ�����ͨ������ node������
	bool waitForTask(Taskobj&node, const struct timespec& timeval_ = maxTimeWait);
	bool pushTask(const Taskobj& node);

	bool pushTaskWithTimeOut(const Taskobj& node,const struct timespec & timeval=maxTimeWait);
	xMutex getTaskLock(void) {return m_LockTask;}		//��ȡ������
	iterator begin(void) {return m_tasklist.begin();}	//��ȡ��һ��Ԫ�صĵ�����
	const_iterator begin(void) const {return m_tasklist.begin();}
	iterator end(void) {return m_tasklist.end();}
	const_iterator end(void) const {return m_tasklist.end();}
protected:
	bool m_IsActive;
	size_t m_Maxsize;		//�����������
	size_t m_CurTaskCount;	//��ǰ������
	size_t m_CurReadWaiter;	//��ǰ��
	size_t m_CurWriteWaiter;	//��ǰд�ȴ�

	std::deque<Taskobj> m_tasklist; //�������
	xMutex m_LockTask;				//�������������������һ��ʹ��
	xCondition m_CondTask;			//�������������������̣߳��ȴ�����
	xCondition m_CondTaskFree;
};
template <class Taskobj>
void xTaskqueue<Taskobj>::setDeadstatus(void)
{
	xAutoLock L(m_LockTask);
	m_IsActive = false;
	m_CondTask.broadCast();
	m_CondTaskFree.broadCast();
}
template <class Taskobj>
void xTaskqueue<Taskobj>::clearAllTask(void)
{
	xAutoLock L(m_LockTask);
	m_tasklist.clear();
	m_CurTaskCount=0;
	m_CondTask.broadCast();
	m_CondTaskFree.broadCast();
}
template <class Taskobj>
void xTaskqueue<Taskobj>::wakeup(void)
{
	xAutoLock L(m_LockTask);
	m_CondTask.broadCast();
	m_CondTaskFree.broadCast();
}
template <class Taskobj>
void xTaskqueue<Taskobj>::resizeQueue(size_t MaxSize)
{
	xAutoLock L(m_LockTask);
	m_Maxsize=MaxSize;
}
template <class Taskobj>
bool xTaskqueue<Taskobj>::waitForTask(Taskobj&node,const struct timespec& timeval_)
{
	xAutoLock L(m_LockTask);
	if(!m_IsActive)
	{
		return false;
	}
	if(m_tasklist.empty())
	{
		if(TimevalIsequal(timeval_ ,maxTimeWait))
		{
			++m_CurReadWaiter;
			m_CondTask.wait(m_LockTask);
			--m_CurReadWaiter;
		}
		else if(TimevalIsequal(timeval_ , zeroTimeWait))
		{
			return false;
		}
		else
		{
			++m_CurReadWaiter;
			m_CondTask.timewait(m_LockTask,timeval_);
			--m_CurReadWaiter;
		}
		if(!m_IsActive || m_tasklist.empty())
			return false;
	}
	node = m_tasklist.front();
	m_tasklist.pop_front();
	--m_CurTaskCount;
	if(m_CurWriteWaiter)
		m_CondTaskFree.signal();
	return true;
}
template <class Taskobj>
bool xTaskqueue<Taskobj>::pushTask(const Taskobj& node)
{
	xAutoLock L(m_LockTask);
	if(!m_IsActive)
		return false;
	if(queueIsFull())
	{
		++m_CurWriteWaiter; 
		m_CondTaskFree.wait(m_LockTask);
		--m_CurWriteWaiter;
	}
	m_tasklist.push_back(node);
	++m_CurTaskCount;
	if(m_CurReadWaiter)
		m_CondTask.signal();
	return true;
}
template <class Taskobj>
bool xTaskqueue<Taskobj>::pushTaskWithTimeOut(const Taskobj& node,const struct timespec & timeval)
{
	xAutoLock L(m_LockTask);
	if(!m_IsActive)
		return false;
	if(queueIsFull())
	{
		if(TimevalIsequal(timeval ,maxTimeWait))
		{
			++m_CurWriteWaiter;
			m_CondTaskFree.wait(m_LockTask);
			--m_CurWriteWaiter;
		}
		else if (TimevalIsequal(timeval ,zeroTimeWait))
			return false;
		else
		{
			++m_CurWriteWaiter;
			m_CondTaskFree.timewait(m_LockTask,timeval);
			--m_CurWriteWaiter;
		}
		if(!m_IsActive || queueIsFull())
			return false;
	}
	m_tasklist.push_back((Taskobj)node);
	++m_CurTaskCount;
	if(m_CurReadWaiter)
		m_CondTask.signal();
	return true;
}
}
