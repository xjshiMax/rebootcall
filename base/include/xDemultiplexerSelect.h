//2018-12-17
// ����select io ����ģʽ��ʵ�ַ�����
/*
1���̳�	xEventDemultiplexer�� ʵ����Ӧ�Ľӿ�
2) Ĭ�϶���д������ ����ע��64��fd.
*/
#pragma once
#include "xbaseclass.h"
#include "basesock.h"
namespace SAEBASE{
class xSelectDemultiplexer:public xEventDemultiplexer
{
public:
	xSelectDemultiplexer();
	virtual ~xSelectDemultiplexer();
	virtual int WaitEvents(std::map<handle_t,xEventHandler*>*handlers,
		int timeout=0,xtime_heap* event_timer=NULL );
	//��ӻ����޸��ļ���������ע���¼���Ϣ
	virtual int RequestEvent(handle_t handle,event_t evt);


	virtual int UnrequestEvent(handle_t handle);
private:
	int m_epoll_fd;  //epoll_creat ���ص�������
	int m_fd_num;	  // ��ǰ���뼯�ϵ�����������
	int m_maxfdID;	  //���fdֵ��1.��selectģ���г����������������
	//fd_set m_fdread;
	//fd_set m_fdError;
	//fd_set m_fdwrite;
	fd_set m_fdReadSave;
	std::vector<handle_t> m_Readevents;
};
xSelectDemultiplexer::xSelectDemultiplexer()
{
	m_fd_num=0;
	FD_ZERO(&m_fdReadSave);
}
xSelectDemultiplexer::~xSelectDemultiplexer()
{

}

int xSelectDemultiplexer::WaitEvents(std::map<handle_t,xEventHandler*>*handlers,
	int timeout,xtime_heap* event_timer )
{
	//std::vector<handle_t> m_Readevents;
	fd_set fdread;
	FD_ZERO(&fdread);
	memcpy(&fdread,&m_fdReadSave,sizeof(m_fdReadSave));
	timeval timev_;
	timev_.tv_sec=timeout/1000;
	timev_.tv_usec=0;
	int res = select( 0,  &fdread , NULL,NULL, &timev_);
	if(res>0)
	{

			for(int j=0;j<m_fdReadSave.fd_count;j++)
			{
				int fd=m_fdReadSave.fd_array[j];
				if(FD_ISSET(fd,&fdread))
				{
					(*handlers)[fd]->HandleRead(fd);
				}
			}

	}
	if(event_timer !=NULL)
	{
		event_timer->tick();
	}
	return 0;
}
int xSelectDemultiplexer::RequestEvent(handle_t handle,event_t evt)
{
	if(evt & xReadEvent)
		FD_SET((SOCKET)handle,&m_fdReadSave);
	if((int)handle > m_maxfdID)
		m_maxfdID = (int)handle +1;
	++m_fd_num;
// 	if(evt & xWriteEvent)
// 		FD_SET(handle,&m_fdread);
// 	if(evt &xErrorEvent)
// 		FD_SET(handle,&m_fdError);
	return 0;
}
int xSelectDemultiplexer::UnrequestEvent(handle_t handle)
{
	if((SOCKET)handle!=INVALID_SOCKET)
	{
		FD_CLR((SOCKET)handle,&m_fdReadSave);
		--m_fd_num;
	}
	return 0;
}
}