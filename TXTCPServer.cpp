#include "TXTCPServer.h"
int TXTCPServer::Onaccept(int socketfd,char*date,int len,IN xEventHandler *clientHandle=NULL)
{
    cout<<"get one connect"<<endl;
    return 0;
}
int TXTCPServer::Ondata(int socketfd,char*date,int len)
{
    cout<<date<<endl;
    //收到批量请求
    FScall Onecall;
    Onecall.start();

    retrurn 0;
}
int TXTCPServer::Onclose(int socketfd)
{
    return 0;
}