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
/* ��fs֮���ͨ��*/
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
		int nodeState;		 //����״̬
		string m_parksessionID;  //�Ựid
		string m_speeckCraftID;  //����id
		string m_taskID;		 //����id
		string m_channelpath;	 //������channelֵ����ʼΪ�ա�
		bool m_IsAsr;

		//�������ݿ����ֶ�
		int m_DB_talk_times;		//�Ự��������
		string m_DB_start_stamp;	//�Ự��ʼʱ���
		string m_DB_end_stamp;		//�Ự����ʱ�������hung up ʱ��Ϊ׼
		int m_DB_duration;		//ͨ��ʱ��
		string m_DB_recording_file;	//¼���ļ�λ��
		string m_DB_outbound_label;	//����ȡ�Ŀͻ�����
		int m_DB_creatd_at;			//�������ݿ�ʱ�� utc
		int m_DB_updated_at;		//�������ݿ�ʱ��
		string m_DB_task_name;

		int m_Atimes;
		int m_Btimes;
		int m_Ctimes;
};
/* ����fs������������ one-task-one-thread*/
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
	bool Getablibity(string jsonstr); //json��Ϣ
	string m_callnumber;
	string m_taskID;		//����id
	string m_taskName;		//��������
	string m_speechcraftID;  //����id
	string m_fsip;
	int m_fsPort;
	string m_fsPassword;
	std::vector<string>m_NumberSet;
};

/* ����fs�ش���Ϣ���ģ� ʹ���̳߳ع���FSsession*/
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

