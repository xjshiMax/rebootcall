#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <pthread.h>
#include "process_event.h"
#include "common/structdef.h"
#include "common/DBOperator.h"
// #include "common/codeHelper.h"

#include "base/include/xReactorwithThread.h"
#include "TXTCPServer.h"
#include "base/inifile/inifile.h"

using namespace std;
using namespace inifile;




int main(int argc, char const *argv[])
{
    bool bSuccess = false;


    db_operator_t::initDatabase();
    db_operator_t::SelectSql(gKeymap, 1);
    
    map<uint32_t, base_script_t>::iterator strmap_iter = gKeymap.begin();
    for (; strmap_iter != gKeymap.end(); strmap_iter++)
    {
        // cout << strmap_iter->first << ' ' << strmap_iter->second << endl;
        base_script_t node=strmap_iter->second;
        // codeHelper::GetInstance()->run(node.vox_base.c_str(), node.desc.c_str());
        printf("node==%d,descript=%s\n", strmap_iter->first,node.desc.c_str());

    }
    //启动tcp服务
    xReactorwithThread ReactorInst;
    TXTServer BussinessTCP;
    IniFile IniService;
    IniService.load(Service.ini);
    int iret=-1;
    string strIP=IniService.getStringValue("JAVABUSINESS","IP",&iret);
    if(iret!=0)
    {
        strIP="0.0.0.0";
    }
    iret=-1;
    int Port = IniService.getIntValue("JAVABUSINESS","PORT",iret);
    if(iret!=0)
    {
        Port="8070";
    }

    ReactorInst.run();
    BussinessTCP.startTCPServer(ReactorInst,strIP.c_str(),Port);

    //注册fs事件响应。
    FSprocess FSprocessInst;
    FSprocess.start();

    ret = pthread_create(&pthid3, NULL, FSprocessInst::test_Process, NULL);
    if (ret) // 闈?0鍒欏垱寤哄け璐?
    {
        perror("createthread 3 failed.\n");
        return 1;
    }
    FSprocess.join();




    return 0;
}
