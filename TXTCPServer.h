//2019-3-2
//ʵ��tcp���񣬽���java����˴���������Ϣ��
#pragma once
#include "base/include/xTcpServerBase.h"
#include "process_event.h"
using namespace SAEBASE;
class TXTCPServer:public xTcpServerBase
{
    public:
    virtual int Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL)
	{
		cout<<"get one connect"<<endl;
		return 0;
	}
	virtual int Ondata(int socketfd,char*date,int len)
	{
		cout<<date<<endl;
		//�յ���������
		static FScall Onecall;
		Onecall.setCallNumber(date);
		//Onecall.S
		Onecall.start();

		return 0;
	}
	virtual int Onclose(int socketfd)
	{
		return 0;
	}

};