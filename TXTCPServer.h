//2019-3-2
//实现tcp服务，接受java服务端传过来的消息。
#pragma once
#include "base/include/xTcpServerBase.hpp"
#include "process_event.h"
class TCTCPServer:public xTcpServerBase
{
    public:
    virtual int Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL);
	virtual int Ondata(int socketfd,char*date,int len);
	virtual int Onclose(int socketfd);
}