#pragma once
#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <pthread.h>
#include "common/structdef.h"
#include "common/DBOperator.h"
// #include "common/codeHelper.h"

//#include "base/include/xReactorwithThread.h"
#include "TXTCPServer.h"
#include "base/include/xReactorwithThread.h"
//#include "process_event.h"
#include "base/inifile/inifile.h"
#include "base/glog/linux/glog/logging.h"
using namespace std;
using namespace inifile;
using namespace SAEBASE;

///map<uint32_t, base_script_t> gKeymap;

int main(int argc, char const *argv[])
{
    bool bSuccess = false;

    db_operator_t::initDatabase();
    db_operator_t::SelectSqlAllSC(FSprocess::m_gKeymap);
    db_operator_t::GetKnowledge(FSprocess::m_knowledgeSet);
    map<string, base_script_t>::iterator strmap_iter = FSprocess::m_gKeymap.begin();
    for (; strmap_iter != FSprocess::m_gKeymap.end(); strmap_iter++)
    {
        // cout << strmap_iter->first << ' ' << strmap_iter->second << endl;
        base_script_t node=strmap_iter->second;
        // codeHelper::GetInstance()->run(node.vox_base.c_str(), node.desc.c_str());
        printf("node==%s,descript=%s\n", strmap_iter->first.c_str(),node.desc.c_str());

    }
    //启动tcp服务
    xReactorwithThread ReactorInst;
    TXTCPServer BussinessTCP;
    IniFile IniService;
    IniService.load("Service.ini");
    int iret=-1;
    string strIP=IniService.getStringValue("JAVABUSINESS","IP",iret);
    if(iret!=0)
    {
        strIP="0.0.0.0";
    }
    iret=-1;
    int Port = IniService.getIntValue("JAVABUSINESS","PORT",iret);
    if(iret!=0)
    {
        Port=8070;
    }
	//读取日志配置
	iret=-1;
	string logpath=IniService.getStringValue("LOGCONF","logPath",iret);
	if(iret!=0)
	{
		logpath="/home";
	}
	iret =-1;
	int loglevel=IniService.getIntValue("LOGCONF","loglevel",iret);
	if(iret!=0)
	{
		loglevel=0;
	}
	FLAGS_log_dir =logpath;
	google::InitGoogleLogging("infosun");
	FLAGS_minloglevel = loglevel;
	FLAGS_colorlogtostderr = true;  // Set log color
	FLAGS_logbufsecs = 0;  // Set log output speed(s)
	FLAGS_max_log_size = 1024;  // Set max log file size
	FLAGS_stop_logging_if_full_disk = true;  // If disk is ful

    ReactorInst.startReactorWithThread();
    BussinessTCP.startTCPServer(&ReactorInst,strIP.c_str(),Port);
	LOG(INFO)<<"start tcp server";
    //注册fs事件响应。
    FSprocess FSprocessInst;
	FSprocessInst.Initability();
    FSprocessInst.start();

	//注册语音转文本事件
	//FSasrprocess FSasrprocessInst;
	//FSasrprocessInst.start();
//        pthread_t pthid3;
//        int ret = pthread_create(&pthid3, NULL, FSprocess::test_Process, NULL);
//        if (ret) // 闈?0鍒欏垱寤哄け璐?
//        {
//            perror("createthread 3 failed.\n");
//            return 1;
//        }
    FSprocessInst.join();
	esl_log(ESL_LOG_INFO,"FSprocessInst thread is end\n");
	ReactorInst.join();
	esl_log(ESL_LOG_INFO,"out of main\n");
	google::ShutdownGoogleLogging();

    return 0;
}
