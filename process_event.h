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
#include "base/include/xOntimerBase.h"
using namespace std;
using namespace inifile;
using namespace SAEBASE;

//map<uint32_t, base_script_t> gKeymap;
typedef enum
{
	GF_normal_node=0x00,
	GF_knowledge_node=0x01,  //命中知识库
	GF_nothear=0x02,		//听不清

};
typedef enum{
	Session_resetsilence=0,
	Session_silenceinc,
	Session_nosilence,
	Session_playing,
	Session_noplayback,
	Session_silencefirst,
	Session_silenceSecond,
};
class FSprocess;
/* 与fs之间的通信*/
class FSsession:public xtaskbase
{
	public:
		FSsession():nodeState(1),m_channelpath(""),m_IsAsr(false),m_DB_talk_times(0),m_DB_outbound_label("G"),m_bhaveset(false),m_SessionWord(""),m_username(""),m_SessionState(GF_normal_node),
		m_silenceTime(0),m_silencestatus(Session_nosilence),m_playbackstatus(Session_noplayback){}
		virtual int run();
		void Action();
		void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid);
		void SetSessionID(string sessionid){m_parksessionID=sessionid;}
		string GetSessionID(){return m_parksessionID;};
		void InsertSessionResult();
		string Getcurrenttime();
		int GetUTCtimestamp();
		string Getrecordpath();
		void Onanswar();
		int Getnextstatus(string asrtext,string keyword);
		void ChangetheTypeCount(string strtype);
		void SetFinnallabel(int currentstatus,int nextstatus);
		void collection(string name,string Text,int nodeState=-1);
		void silenceAdd(int val);
		bool CheckoutIfsilence(); //	检测是否是静音，是则返回真
		void Onsilence();

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
		bool m_bhaveset;
		string m_SessionWord;	//记录客户
		string m_username;
		Mutex m_databaselock;
		int m_SessionState;		//会话状态，正常节点还是知识库和没听清
		vector<int> m_nodelist;			//记录关键字命中节点，不记录知识库
		int m_silenceTime;		//静音时间。
		int m_maxsilenceTime;
		int m_silencestatus;
		int m_playbackstatus;
		Mutex m_silenceLock;
		Mutex m_silencestatusLock;
};
/* 发送fs批量呼叫请求 one-task-one-thread*/
class FScall:public Threadbase
{
public:
	FScall():m_IsAllend(false)
	{

	}
	virtual ~FScall()
	{
		//if(m_inst)
		//	delete m_inst;
		cout<<"~FScall is called;"<<endl;
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
	std::vector<t_Userinfo>m_NumberSet;
	static FScall*Instance();
	static FScall*m_inst;
	bool m_IsAllend;		//电话号码全部拨打完毕
};
//管理通话任务类
class FScallManager
{
public:
	FScallManager(){};
	~FScallManager(){};
	map<string ,FScall*>m_TaskSet;
	void CheckEndCall();
};

/*
定时器线程，定时1秒做会话的静音检测。
*/
class slienceCheck:public OnTimerBase
{
public:
	slienceCheck(int timeout);
	~slienceCheck();
	virtual void timeout();

};

/* 处理fs回传消息中心， 使用线程池管理FSsession*/
class FSprocess :public Threadbase
{
public:
	FSprocess():m_slienceCheck(1)
	{
		//SessionPool.initsimplePool();
		//SessionPool.startPool();
	}
	~FSprocess()
	{
		cout<<"~FSprocess is called"<<endl;
	}
	void Initability();
	virtual void run();
	void startProcess();
	static FSsession* CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname,string username,int silenceTime);
	void *Inbound_Init(void *arg);
	static void *test_Process(void *arg);
	void  process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   map<string,base_script_t>& keymap,vector<base_knowledge_t>&knowledgelib);
	static esl_handle_t* getSessionhandle(){return m_sessionHandle;}
	FSsession* GetSessionbychannelid(string channel);
	FSsession*GetSessionbymainUUID(string strmainid);

	static map<string, base_script_t> m_gKeymap;
	static vector<base_knowledge_t>m_knowledgeSet;
//xthreadPool SessionPool;
	static map<string,FSsession*> m_SessionSet;
	static esl_handle_t* m_sessionHandle;
	static xMutex m_sessionlock;
	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
	static string m_recordPath;
	slienceCheck m_slienceCheck;
	static int m_userSetsilenseTime;
};

