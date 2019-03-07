//2019-3-2
//实现tcp服务，接受java服务端传过来的消息。
#pragma once
#include "base/include/xTcpServerBase.h"
#include "process_event.h"
using namespace SAEBASE;
static bool IsInit=false;
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
		//收到批量请求
		static FScall Onecall;

		Onecall.Initability();
		IsInit=true;

		//Onecall.Initability();
		if (!Onecall.Getablibity(date))
			return 0;
		Onecall.start();

		return 0;
	}
	virtual int Onclose(int socketfd)
	{
		return 0;
	}

};