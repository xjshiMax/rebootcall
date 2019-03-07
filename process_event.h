//#ifndef __NWAY_PROCESS_EVENT_H__
//#define __NWAY_PROCESS_EVENT_H__
#pragma once
#include<stdio.h>
#ifdef WIN32
#include <io.h>
#else
//#include <syswait.h>
#include <unistd.h>
#include <sys/io.h>
#endif

#include <stdlib.h>
#include "esl/esl.h"
#include <string>
#include "common/codeHelper.h"

#include <map>
#include "common/structdef.h" 
#include <stdio.h>
#include <cstdlib>
#include <iconv.h>
#include <iostream>
#include "base/include/xthreadPool.h"
#include "base/inifile/inifile.h"
using namespace std;
using namespace inifile;
using namespace SAEBASE;

//map<uint32_t, base_script_t> gKeymap;
class FSprocess;
/* 与fs之间的通信*/
class FSsession:public xtaskbase
{
	public:
		FSsession():nodeState(1),m_channelpath(""),m_IsAsr(false),m_DB_talk_times(0),m_DB_outbound_label("B"){}
		virtual int run();
		void Action();
		void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid);
		void SetSessionID(string sessionid){m_parksessionID=sessionid;}
		string GetSessionID(){return m_parksessionID;};
		void InsertSessionResult();
		string Getcurrenttime();
		int GetUTCtimestamp();
		string Getrecordpath();
		int Getnextstatus(string asrtext,string keyword);
		void ChangetheTypeCount(string strtype);
		void SetFinnallabel();
	public:
		esl_handle_t *handle;
		esl_event_t *event;
		string strUUID; 
		string caller_id ;
		string destination_number;
		int nodeState;		 //流程状态
		string m_parksessionID;  //会话id
		string m_speeckCraftID;  //话术id
		string m_taskID;		 //任务id
		string m_channelpath;	 //语音的channel值，初始为空。
		bool m_IsAsr;

		//插入数据库结果字段
		int m_DB_talk_times;		//会话进行论数
		string m_DB_start_stamp;	//会话开始时间戳
		string m_DB_end_stamp;		//会话结束时间戳，以hung up 时间为准
		int m_DB_duration;		//通话时长
		string m_DB_recording_file;	//录音文件位置
		string m_DB_outbound_label;	//最后获取的客户类型
		int m_DB_creatd_at;			//插入数据库时间 utc
		int m_DB_updated_at;		//更新数据库时间
		string m_DB_task_name;

		int m_Atimes;
		int m_Btimes;
		int m_Ctimes;
};
/* 发送fs批量呼叫请求 one-task-one-thread*/
class FScall:public Threadbase
{
public:
	FScall()
	{

	}
	void Initability();
	virtual void run();
	int GetnumbrList();
	int LauchFScall();
	void setCallNumber(string number){m_taskID=number;}
	bool Getablibity(string jsonstr); //json信息
	string m_callnumber;
	string m_taskID;		//任务id
	string m_taskName;		//任务名称
	string m_speechcraftID;  //话术id
	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
	std::vector<string>m_NumberSet;
};

/* 处理fs回传消息中心， 使用线程池管理FSsession*/
class FSprocess :public Threadbase
{
public:
	FSprocess()
	{
		//SessionPool.initsimplePool();
		//SessionPool.startPool();
	}
	void Initability();
	virtual void run();
	static FSsession* CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname);
	void *Inbound_Init(void *arg);
	static void *test_Process(void *arg);
	void  process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   const map<uint32_t,base_script_t>& keymap,vector<base_knowledge_t>&knowledgelib);
	static esl_handle_t* getSessionhandle(){return m_sessionHandle;}
	FSsession* GetSessionbychannelid(string channel);

	static map<uint32_t, base_script_t> m_gKeymap;
	static vector<base_knowledge_t>m_knowledgeSet;
//xthreadPool SessionPool;
	static map<string,FSsession*> m_SessionSet;
	static esl_handle_t* m_sessionHandle;
	xMutex m_sessionlock;
	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
};

