//2018/12/11
// �ַ���ʵ�֣�IO���÷���ʱ��Ļ��ƣ�
#pragma once
#include "xbaseclass.h"
#include "xtimeheap.h"
#ifndef WIN32
#include<sys/epoll.h>
#include <unistd.h>
#endif
#include <errno.h>
namespace SAEBASE{
//epoll IO����ʵ�ַ�����
///
/*
			epoll_create()
				|
			   \|/
	fd-----> epoll_ctl(epoll_fd,EPOLL_CTL_ADD,max_epoll_size,epll_event_set)
				|
				|
			 epoll_wait()
				|-------------> event_handler()
	
*/
///
class xEpollDemultiplexer:public xEventDemultiplexer
{
public:
	xEpollDemultiplexer();
	virtual ~xEpollDemultiplexer();
	virtual int WaitEvents(std::map<handle_t,xEventHandler*>*handlers,
		int timeout=1,xtime_heap* event_timer=NULL );
	//��ӻ����޸��ļ���������ע���¼���Ϣ
	virtual int RequestEvent(handle_t handle,event_t evt);


	virtual int UnrequestEvent(handle_t handle);
private:
	int m_epoll_fd;  //epoll_creat ���ص�������
	int m_fd_num;	  // ��ǰ���뼯�ϵ�����������
};

 }