//2019-3-2
//ʵ��tcp���񣬽���java����˴���������Ϣ��
#pragma once
#include "base/include/xTcpServerBase.h"
#include "process_event.h"
using namespace SAEBASE;
static bool IsInit=false;
class TXTCPServer:public xTcpServerBase
{
    public:
	~TXTCPServer()
	{cout<<"~TXTCPServer"<<endl;}
    virtual int Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL)
	{
		cout<<"get one connect"<<endl;
		return 0;
	}
	virtual int Ondata(int socketfd,char*date,int len)
	{
		cout<<date<<endl;
		//�յ���������
		FScallManager* CallManager=FScallManager::Instance();
		//CallManager->CheckEndCall();
		CallManager->HandleMessage(date);
		return 0;
	}
	virtual int Onclose(int socketfd)
	{
		return 0;
	}

};