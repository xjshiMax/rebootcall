//2018-12-17
/*
��װ����tcpserver base
1) �̳и��ָ࣬�� ip��port ����xreactor���Ϳ�����������������״̬����ȡ���ݡ�
2�����Ժ������¼�����ϣ��ﵽ�����ֻ����������ȫ�������µ��¼�����ȥ��ɡ�
3��
*/
#include "xReactor.hpp"
#include "xbaseclass.hpp"
#include "basesock.hpp"
namespace SAEBASE{
//һ������ȡ8k�ֽڡ�
#define MAXREADSIZE		1024*8

class xTcpServerBase:public xEventHandler
{
public:
	int startTCPServer(xReactor* xreacotr,const char* ip,int port);
	int stop();
	//���Ҫ����client���¼���Ӧ����д��Ϊ������xEventHandler������ʵ��������Onaccept��clientHandle��ֵ��
	virtual int Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL);
	virtual int Ondata(int socketfd,char*date,int len);
	virtual int Onclose(int socketfd);
	int SendMsg(int peerfd,char*buf,int len);
	bool GetLocalInfo(IN int socket,OUT char*ip,OUT int &port)
	{
		return Network_function::getLocalInfo(socket,ip,port);
	}
	bool GetPeerInfo(IN int socket,OUT char*ip,OUT int &port)
	{
		return Network_function::getPeerInfo(socket,ip,port);
	}
	int getListenHandle()const
	{
		return m_listenfd;
	}
public:
	virtual handle_t GetHandler()const;
	virtual void HandleRead(int listentfd);
	virtual void HandlerWrite(){}
	virtual void HandlerError(){}
private:
	handle_t m_listenfd;
	handle_t m_acceptfd;
	xReactor* m_reactor;
};
//�ڵ���startTCPServer ֮�����֮ǰ��һ��Ҫ����xReactor��start()�ӿڣ������¼���Ӧ��
int xTcpServerBase::startTCPServer(xReactor* xreacotr,const char* ip,int port)
{
	InitSocket();
	m_Eventfd = CreateSocket(SOCK_STREAM);
	if(m_Eventfd==INVALID_SOCKET)
		return -1;
	m_listenfd = m_Eventfd;
	m_reactor = xreacotr;
	struct sockaddr_in addrsvr;
	addrsvr.sin_family = AF_INET;
	addrsvr.sin_port = htons(port);
	addrsvr.sin_addr.s_addr = inet_addr(ip);
	BindSocket(m_Eventfd,(SOCKADDR*)&addrsvr,sizeof(addrsvr));
	if(ListenSocket(m_Eventfd,10)==INVALID_SOCKET)
		return -1;
	if(m_reactor!=NULL)
		m_reactor->RegisterHandler(this,xReadEvent); // ע��д�¼���
	return 0;
}
int xTcpServerBase::Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle)
{
	return 0;
}
int xTcpServerBase::Ondata(int socketfd,char*date,int len)
{
	return 0;
}
int xTcpServerBase::Onclose(int socketfd)
{
	return 0;
}
int xTcpServerBase::SendMsg(int peerfd,char*buf,int len)
{
	return SendSocket(peerfd,buf,len);
}
handle_t xTcpServerBase::GetHandler()const
{
	if(m_Eventfd!=INVALID_SOCKET)
		return m_Eventfd;
	return -1;
}
void xTcpServerBase::HandleRead(int listentfd)
{
	if(listentfd==m_listenfd) // �����׽��ֵ�io�¼�
	{
		struct sockaddr_in clientaddr;
		socklen_t socklen=sizeof(struct sockaddr_in);
		int acceptfd=AccpetSocket(m_listenfd,(SOCKADDR*)&clientaddr,&socklen);
		if((SOCKET)acceptfd==INVALID_SOCKET)
			return ;
		xEventHandler * pclientEvent=NULL;
		this->Onaccept(acceptfd,NULL,0,pclientEvent);
		m_Eventfd=acceptfd;
		if(m_reactor)  //����ע��accept��fd,
		{
			if(pclientEvent==NULL)
				m_reactor->RegisterHandler(this,xReadEvent);
			else
			{
				pclientEvent->m_Eventfd=m_acceptfd;
				m_reactor->RegisterHandler(pclientEvent,xReadEvent);
			}
			//m_reactor->RegisterHandler(pclientEvent,xReadEvent);
		}
		return ;
	}
	else
	{
		char buf[MAXREADSIZE]={0};
		int len=MAXREADSIZE;
		int iret = ReadSocket(listentfd,buf,len);
		len=sizeof(buf);
		if(iret==0)
		{
			CloseSocket(listentfd);
			if(m_reactor)
				m_reactor->RemoveHandlerbyfd(listentfd);
			//ShutDownSocket(listentfd,0);
			this->Onclose(listentfd);
			//CloseSocket(listentfd);
		}
		else if(iret>=0)
			this->Ondata(listentfd,buf,iret);
		else
			return ;
	}

	return ;

}

}