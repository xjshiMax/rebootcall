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
/*#include "base/include/xAtomicInt32.h"*/
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
	Session_noanswar,
	Session_playing,
	Session_noplayback,
	Session_silencefirst,
	Session_silenceSecond,
};
typedef enum
{
	CallInit,
	CallStart,
	CallPause,
	CallResume,
	CallStop,
	Recall
};
class FSprocess;
/* 与fs之间的通信*/
class FSsession:public xtaskbase
{
	public:
		FSsession():nodeState(1),m_channelpath(""),m_IsAsr(false),m_DB_talk_times(0),m_DB_duration(0),m_DB_outbound_label("D"),m_bhaveset(false),m_SessionWord(""),m_username(""),m_SessionState(GF_normal_node),
			m_silenceTime(0),m_silencestatus(Session_noanswar),m_playbackstatus(Session_noplayback),m_DB_hungup("customer hung up"){}
		virtual int run();
		void Action(esl_handle_t *phandle,esl_event_t *pevent);
		//void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid);
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
		string m_DB_hungup;			//挂断方，主挂，被挂

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
	FScall():m_IsAllend(false),m_stop(false),m_CallStatus(CallInit),maxSessionDestory(0),maxCallout(0)
	{
	//	m_Sessioncond
	}
	virtual ~FScall()
	{
		//if(m_inst)
		//	delete m_inst;
		m_Sessioncond.broadCast();
		cout<<"~FScall is called;"<<endl;
	}
	void Initability();
	virtual void run();
	int GetnumbrList();
	int LauchFScall();
	int reLauchFSCall();
	void Checksilence();
	void setCallNumber(string number){m_taskID=number;}
	bool Getablibity(string jsonstr); //json信息
	void StopTask();
	void PauseTask();
	void ResumeTask();
	void CallEvent_handle(esl_handle_t *handle,
		esl_event_t *event,
		map<string, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib);
	FSsession* CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname,string username,int silenceTime);
	FSsession* GetSessionbychannelid(string channel);
	FSsession*GetSessionbymainUUID(string& strmainid);
	bool FinishCreateAllSession(){return (m_NumberSet.size()==maxSessionDestory)||(m_CallStatus==CallStop&&maxCallout==maxSessionDestory);}
	//bool FinishbeforeStop();
	int maxSessionDestory;
	int maxCallout;
	string m_callnumber;
	string m_taskID;		//任务id
	string m_taskName;		//任务名称
	string m_speechcraftID;  //话术id
	int m_originate_timeout;	//拨打电话时设置最长不接听自动挂断时间
	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
	std::vector<t_Userinfo>m_NumberSet;
	static FScall*Instance();
	static FScall*m_inst;
	bool m_IsAllend;		//电话号码全部拨打完毕
	bool m_stop;
	int m_CallStatus;
	vector<t_Userinfo>::iterator m_pPauseIte;
// 	int m_robotNum;
// 	int m_recallTimes;
	 t_Task_Info m_taskinfo;
	map<string,FSsession*> m_SessionSet;
	std::vector<t_Userinfo> m_notAnswerSet;
	xMutex m_sessionlock;
	xCondition m_Sessioncond;
};

struct t_aliConfigxml
{
	string appkey;
	string User;
	string AccessKeyId;
	string AccessKeySecret;
	int index;
	int MaxIndex;
	//vector<string> UUIDSet;
	t_aliConfigxml():MaxIndex(10),index(0)
	{
		appkey="";
		User="";
		AccessKeyId="";
		AccessKeySecret="";
	}
};

//管理通话任务类
class FScallManager:public OnTimerBase
{
public:
	FScallManager(int timeout):OnTimerBase(timeout),m_count(0){};
	~FScallManager(){};
	map<string ,FScall*>m_TaskSet;
	static FScallManager* Instance();
	void CheckEndCall();
	string HandleMessage(string data);
	void CallEvent_handle(esl_handle_t *handle,
		esl_event_t *event,
		map<string, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib);
	FScall*GetFSCallbyUUID(string& struuid);
	bool ParseData(string jsonstr,string& cmd,string& scid,string& taskid,string& taskname);
	void StartTask();
	void StopTask();
	void PauseTask();
	void ResumeTask();
	virtual void timeout();
	static Mutex m_InstLock;
	Mutex m_CallLock;
	int m_count;
	//static map<string,t_aliConfigxml> m_aliconfigmap;
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
	~FSprocess()
	{
		cout<<"~FSprocess is called"<<endl;
	}
	void Initability();
	void LoadaliSDKonfig(string strpath);
	static t_aliConfigxml& Getavaliable(string uuid);
	static void SetfreeRES(string uuid);
	virtual void run();
	void startProcess();
//	static FSsession* CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname,string username,int silenceTime);
	void *Inbound_Init(void *arg);
	static void *test_Process(void *arg);
	void  process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   map<string,base_script_t>& keymap,vector<base_knowledge_t>&knowledgelib);

	int getRoboteNum();
	static map<string, base_script_t> m_gKeymap;
	static vector<base_knowledge_t>m_knowledgeSet;

	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
	static string m_recordPath;
	//slienceCheck m_slienceCheck;
	static int m_userSetsilenseTime;
	static int m_robotNum;
	static esl_handle_t *m_timeouthandle;
	static map<string,t_aliConfigxml> m_aliconfigmap;
	static map<string,string> m_uuid_aliconfig; // uuid---appkey
	static string m_prefixnum;
};

