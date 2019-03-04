//2019-3-2
//实现tcp服务，接受java服务端传过来的消息。
#pragma once
#include "base/include/xTcpServerBase.h"
#include "process_event.h"
using namespace SAEBASE;
class TXTCPServer:public xTcpServerBase
{
    public:
    virtual int Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL);
	virtual int Ondata(int socketfd,char*date,int len);
	virtual int Onclose(int socketfd);
};
int TXTCPServer::Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle)
{
    cout<<"get one connect"<<endl;
    return 0;
}
int TXTCPServer::Ondata(int socketfd,char*date,int len)
{
    cout<<date<<endl;
    //收到批量请求
    FScall Onecall;
   // Onecall.setCallNumber(date);
    Onecall.start();

    return 0;
}
int TXTCPServer::Onclose(int socketfd)
{
    return 0;
}