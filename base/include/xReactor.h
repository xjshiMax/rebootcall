//2018/12/10
//�򵥷�װreactor

#pragma  once 


#include <map>
#include "xbaseclass.h"
#include "xtimeheap.h"
#ifdef WIN32
	#include "xDemultiplexerSelect.h"
#else
	#include "xEventDemultiplexer.h"
#endif

namespace SAEBASE
{
class xReactorImplentation;

class xReactor
{
public:
	xReactor();

// 	{
// 		m_reactorimp = new xReactorImplentation();
// 	}
	virtual ~xReactor();
	int start() //�����¼�ѭ��
	{
		HandlerEvents();
		return 0;
	}
	//ע���¼����¼���Ӧ������¼����ͣ���д

	int RegisterHandler(xEventHandler*handler,event_t event_);

	int RemoveHandler(xEventHandler* handler);
	int	RemoveHandlerbyfd(handle_t handlefd);				//���tcp����Ҫ����fd��ע���¼��ĳ�����ӡ�
	void HandlerEvents();

	int RegisterTimeTask(xheaptimer* timerevent);
	private:
		xReactor(const xReactor&);
		xReactor & operator=(const xReactor&);
private:

	xReactorImplentation* m_reactorimp;

};

#define INITSIZE 100
class xReactorImplentation
{
public:
	//���������ʱ����С�ѵ�����СΪ INITSIZE=100.����������Զ�resize()
	xReactorImplentation()
	{
#ifdef WIN32
		m_demultiplexer = new xSelectDemultiplexer();
#else
		m_demultiplexer = new xEpollDemultiplexer();
#endif
		//m_demultiplexer = static_cast<xEventDemultiplexer*>(new xEpollDemultiplexer());
		m_eventtimer = new xtime_heap(INITSIZE);
	}
	~xReactorImplentation()
	{
		delete m_demultiplexer;
	}

	int RegisterHandler(xEventHandler*handler,event_t event_);
	int RemoveHandler(xEventHandler* handler);
	int RemoveHandlerbyfd(handle_t handlefd);
	void HandlerEvents();
	int RegisterTimeTask(xheaptimer* timerevent);
private:
	xEventDemultiplexer *		m_demultiplexer;
	std::map<handle_t,xEventHandler*> m_handlers;
	xtime_heap* m_eventtimer;
};


//reactor ����
class ReactorInstance
{
public:
	static xReactor* GetInstance()
	{
		if(m_reactor==NULL)
			m_reactor=new xReactor();
		return m_reactor;
	}
private:
	static xReactor* m_reactor;;
};

}