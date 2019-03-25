//2018-12-18��Ӷ�ʱ������
/*
1����ʱ��������һ���̣߳�
2���̳ж�ʱ�����࣬Ȼ�����һֱִ�С�
3) ȷ�������̻߳�ȴ������������˳�����Ȼ��ʱ���߳�Ҳû���㹻��ʱ��ִ�С�
*/
#include "xAutoLock.h"
#include "xthreadbase.h"
namespace SAEBASE{
	//��ʱ�����뼶��
	class OnTimerBase:public Threadbase
	{
	public:	
		OnTimerBase(int timeout):m_TimeOut(timeout),m_bIsstop(false){}
		int startTimer()
		{
			return start();
		}
		void stopTimer()
		{
			//join();
			destory();
			m_bIsstop=true;
		}
		virtual void timeout()=0;
	private:
		virtual void run()
		{
			while(!m_bIsstop){
				xAutoLock L(m_mutex);
				struct timespec tc;
				tc.tv_sec=m_TimeOut;
				tc.tv_nsec=0;
				//m_SleepCondition.timewait(m_mutex,tc);
				m_SleepCondition.timewait(m_mutex,tc);
				this->timeout();
			}

		}
	private:
		int m_TimeOut;
		xCondition m_SleepCondition;
		xMutex m_mutex;
		bool m_bIsstop;
	};
}