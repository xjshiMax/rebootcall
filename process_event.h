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
/* 与fs之间的通信*/
class FSsession:public xtaskbase
{
	public:
		FSsession():nodeState("1"){}
		virtual int run();
		void Action();
		void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid);
	string strUUID; 
	string caller_id ;
	string destination_number;
	esl_event_t *event;
	esl_handle_t *handle;
	string nodeState;
};
/* 发送fs批量呼叫请求 one-task-one-thread*/
class FScall:public Threadbase
{
public:
	virtual void run();
	int LauchFScall();
	//void setCallNumber(string number){m_callnumber=number;}
	//string m_callnumber;
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
virtual void run();
static void *Inbound_Init(void *arg);
static void *test_Process(void *arg);
static void  process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   const map<uint32_t,base_script_t>& keymap);
static map<uint32_t, base_script_t> m_gKeymap;
//xthreadPool SessionPool;
static map<string,FSsession> m_SessionSet;
};


