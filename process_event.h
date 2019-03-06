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
using namespace std;
using namespace SAEBASE;

//map<uint32_t, base_script_t> gKeymap;
class FSprocess;
/* ��fs֮���ͨ��*/
class FSsession:public xtaskbase
{
	public:
		FSsession():nodeState("1"),m_channelpath(""){}
		virtual int run();
		void Action();
		void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid);
		void SetSessionID(string sessionid){m_parksessionID=sessionid;}
		string GetSessionID(){return m_parksessionID;};
	public:
		esl_handle_t *handle;
		esl_event_t *event;
		string strUUID; 
		string caller_id ;
		string destination_number;
		string nodeState;		 //����״̬
		string m_parksessionID;  //�Ựid
		string m_speeckCraftID;  //����id
		string m_taskID;		 //����id
		string m_channelpath;	 //������channelֵ����ʼΪ�ա�

};
/* ����fs������������ one-task-one-thread*/
class FScall:public Threadbase
{
public:
	FScall()
	{

	}
	virtual void run();
	int GetnumbrList();
	int LauchFScall();
	void setCallNumber(string number){m_taskID=number;}
	string m_callnumber;
	string m_taskID;
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
	virtual void run();
	static FSsession* CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number);
	void *Inbound_Init(void *arg);
	static void *test_Process(void *arg);
	void  process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   const map<uint32_t,base_script_t>& keymap);
	static esl_handle_t* getSessionhandle(){return m_sessionHandle;}
	FSsession* GetSessionbychannelid(string channel);

	static map<uint32_t, base_script_t> m_gKeymap;
//xthreadPool SessionPool;
	static map<string,FSsession*> m_SessionSet;
	static esl_handle_t* m_sessionHandle;
	xMutex m_sessionlock;
};

