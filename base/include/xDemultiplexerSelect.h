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

}