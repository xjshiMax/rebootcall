#include "process_event.h"
#include "common/DBOperator.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "base/glog/linux/glog/logging.h"
using namespace std;
#define _Use_ALI_SDK
#define MaxRobotNum 10
#define ERRORCMD	"do not support this cmd.only support start,pause,resume,stop"
#define ERRORTASKID	"not exit this taskid"
#define ERRORROBOT	"robot number is invalid"
#define ERRORJSON	"json parse failed"
#define CMDOK		"ok"
typedef enum{
	SC_Opening_Remarks=1000,
	SC_Add_In_Wechat,
	SC_Additive_Group,
	SC_Add_Back_Wechat,
	SC_Bye,
	SC_Contect_nextTime,
	SC_Seeking_Attention,
	SC_KnowledgeLib,
	SC_Hungup
}e_Speeckcase;
int FSsession::run()
{
	//Action();
	return 0;
}
void FSsession::ChangetheTypeCount(string strtype)
{
#ifdef _GUPIAO
	if(strtype=="A")
	{
		m_Atimes++;
	}
	else if(strtype=="B")
	{
		m_Btimes++;
	}
	else if(strtype=="C")
	{
		m_Ctimes++;
	}
#endif
}
void FSsession::SetFinnallabel(int currentstatus,int nextstatus)
{
	if(m_bhaveset)
		return;
	if(nextstatus==18)//您是我们特别邀请的尊贵用户，享有快速服务通道
		m_DB_outbound_label="A";
	else if((currentstatus==1&&nextstatus==5)||(currentstatus==2&&nextstatus==5))
		m_DB_outbound_label="B";
	else if((currentstatus==16&&nextstatus==15)||nextstatus==17)
		m_DB_outbound_label="C";
	else if((currentstatus==3&&nextstatus==10)||(currentstatus==14&&nextstatus==10)||(currentstatus==24&&nextstatus==10))
	{
		m_DB_outbound_label="D";
	}
	else if((currentstatus==8&&nextstatus==12)||(currentstatus==24&&nextstatus==12))
	{
		m_DB_outbound_label="E";
	}
	else if(nextstatus==6)
	{
		m_DB_outbound_label="F";
	}
	else if(nextstatus==13)  
	{
		m_DB_outbound_label="A1";
	}

}
void FSsession::collection(string name,string Text,int node)
{
	esl_log(ESL_LOG_INFO,"collection\n");
	LONGLONG time=GetUTCtimestamp();
	char strtime[32]={0};
	sprintf(strtime,"%d",time);
	m_SessionWord+=name+"(";
	m_SessionWord+=strtime;
	m_SessionWord+="): ";
	m_SessionWord+=Text;
	m_SessionWord+="\r\n";
	if(node!=-1)
		m_nodelist.push_back(node);
}
void FSsession::silenceAdd(int val)
{
	//esl_log(ESL_LOG_INFO,"silenceAdd,val=%d\n",val);
	xAutoLock l(m_silenceLock);
	if(m_silencestatus==Session_noanswar)
		return;
	if(m_playbackstatus==Session_playing)
	{
		esl_log(ESL_LOG_INFO,"silenceAdd,Session_playing\n");
		m_silenceTime=0;
		return;
	}
	if(val)
		m_silenceTime=m_silenceTime+val;
	else
	{
		m_silenceTime=0;	//置零
		m_silencestatus=Session_nosilence;
	}
}
bool FSsession::CheckoutIfsilence() //	检测是否是静音，是则返回真
{
	//esl_log(ESL_LOG_INFO,"CheckoutIfsilence\n");
	xAutoLock l(m_silenceLock);
	if(m_silenceTime>=m_maxsilenceTime)
	{
		m_silenceTime=0;
		return true;
	}
	else
		return false;
}
int FSsession::Getnextstatus(string asrtext,string keyword)
{
	if(m_silencestatus==Session_silencefirst||((m_SessionState&GF_knowledge_node)&&(!(m_SessionState&GF_nothear))))	//如果前面检测到静音，下一句无条件重复上一个节点
	{
		m_SessionState=GF_normal_node;
		int listsize=m_nodelist.size();
		if(listsize>=1)
			return m_nodelist[listsize-1];
	}
	else if((m_SessionState&GF_knowledge_node)&&(m_SessionState&GF_nothear))
	{
		m_SessionState=GF_normal_node;
		int listsize=m_nodelist.size();
		if(listsize>=2)
			return m_nodelist[listsize-2];
	}
	cJSON *root = cJSON_Parse(keyword.c_str());
	if(!root) //如果解析失败，直接返回到 SC_Contect_nextTime
	{
		esl_log(ESL_LOG_INFO,"json parse failed, turn to SC_Contect_nextTime\n");
		return SC_Contect_nextTime;
	}
	cJSON* item=root->child;
	bool checked=false;
	while(item!=NULL)
	{
		string strkey=item->string;
		int nodeNum=SC_Contect_nextTime;
		cJSON*cnode=cJSON_GetObjectItem(item,"node");
		if(!cnode)
		{
			esl_log(ESL_LOG_INFO,"node parse failed, turn to SC_Contect_nextTime\n");
		}
		nodeNum=cJSON_GetObjectItem(item,"node")->valueint;
		string strwork="";
		cJSON*cword=cJSON_GetObjectItem(item,"word");
		if(!cword)
		{
			esl_log(ESL_LOG_INFO,"node parse failed, turn to SC_Contect_nextTime\n");
		}
		strwork=cJSON_GetObjectItem(item,"word")->valuestring;
		//int nodeNum=cJSON_GetObjectItem(item,"node")->valueint;
		int iret1=strkey.find("out_tab");
		int iret2=strkey.find_first_of("out_tab");
		if(strkey.find("out_tab")!=string::npos)
		{
			while(strwork!="")//需要分割解析
			{
				size_t pos = strwork.find("#");
				if(pos != std::string::npos)
				{
					std::string x = strwork.substr(0,pos);
					if(asrtext.find(x)!=std::string::npos)
						break;
					strwork=strwork.substr(pos+1);
				}
				else
				{
					if(asrtext.find(strwork)!=std::string::npos)
						break;
					strwork="";
				}
				//strwork=strwork.substr(pos+1);
			}//while #
			if(strwork=="")
			{
				cJSON_Delete(root);
				return nodeNum;
			}
		}//out_tab
		else if(strkey.find("in_tab")!=string::npos)
		{
			cJSON*ptype=cJSON_GetObjectItem(item,"type");
			string strtype="#";
			if(ptype)
				strtype=ptype->valuestring;
			while(strwork!="")//需要分割解析
			{
				size_t pos = strwork.find("#");
				if(pos != std::string::npos)
				{
					std::string x = strwork.substr(0,pos);
					if(asrtext.find(x)!=std::string::npos)
					{
						ChangetheTypeCount(strtype);
						cJSON_Delete(root);
						return nodeNum;
					}
					strwork=strwork.substr(pos+1);
				}
				else
				{
					if(asrtext.find(strwork)!=std::string::npos)
					{
						ChangetheTypeCount(strtype);
						cJSON_Delete(root);
						return nodeNum;
					}
					strwork="";
				}
			}//while #
		}//in_tab
// 		else if(strkey.find("tab_yes")!=string::npos)
// 		{
// 			if(checked)
// 			{
// 				cJSON_Delete(root);
// 				return nodeNum;
// 			}
// 			string iretsent = codeHelper::GetInstance()->sentiment_classifyRequesst(asrtext);
// 			checked=true;
// 			float positive=atof(iretsent.c_str());
// 			if(positive>0.5)
// 			{
// 				cJSON_Delete(root);
// 				return nodeNum;
// 			}
// 		}
// 		else
// 		{
// 			if(checked)
// 			{
// 				cJSON_Delete(root);
// 				return nodeNum;
// 			}
// 			string iretsent = codeHelper::GetInstance()->sentiment_classifyRequesst(asrtext);
// 			checked=true;
// 			float positive=atof(iretsent.c_str());
// 			if(positive<=0.5)
// 			{
// 				cJSON_Delete(root);
// 				return nodeNum;
// 			}
// 		}
		else if(strkey.find("bot_tab")!=string::npos)// 兜底的分支
		{
			cJSON_Delete(root);
			esl_log(ESL_LOG_INFO,"bot_tab,node=%d\n",nodeNum);
			return nodeNum;
		}
		else
		{
			esl_log(ESL_LOG_INFO,"can not match any ,SC_Bye,node=5\n");
			return SC_Bye;
		}
		printf("%s\n",item->string);
		item=item->next;
	}

}
void FSsession::Onanswar()
{
// 	m_silencestatusLock.lock();
// 	m_silencestatus=Session_nosilence;
// 	m_silencestatusLock.unlock();
	m_DB_creatd_at = GetUTCtimestamp();
	char tmp_cmd[1024] = {0};
	string recordpath=Getrecordpath();
	char filename[64];
	struct tm *tblock;
	time_t timer = time(NULL);
	char strtime[64]={0};
	char onlyfilename[64]={0};
	tblock = localtime(&timer);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d",tblock->tm_year+1900,tblock->tm_mon+1,tblock->tm_mday,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
	sprintf(onlyfilename,"%s_%s.wav",destination_number.c_str(),strtime);
	sprintf(filename,"%s/%s",recordpath.c_str(),onlyfilename);
	//sprintf(filename,"%s/0000000000_1551959350.wav",recordpath.c_str(),GetUTCtimestamp());
	esl_log(ESL_LOG_INFO, "record file name:%s\n", filename);
	sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
	//sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
	esl_log(ESL_LOG_INFO, "esl_send_recv cmd: %s\n", tmp_cmd);
	esl_send_recv_timed(handle, tmp_cmd, 1000);
	this->m_DB_recording_file= onlyfilename;
	map<string, base_script_t>::iterator iter; //=keymap.find(1);
	map<string, base_script_t> nodeMap = FSprocess::m_gKeymap;
	char strSCID[32]={0};
	sprintf(strSCID,"%s_%d",m_speeckCraftID.c_str(),nodeState);
	iter = nodeMap.find(strSCID);
	if (iter != nodeMap.end())
	{
		base_script_t node = iter->second;
		//node.vox_base += ".wav";
		esl_execute(handle, "set", "node_state=1", strUUID.c_str());
		//playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
		esl_execute(handle, "playback", node.vox_base.c_str(), strUUID.c_str());
		collection("机器人",node.desc,nodeState);

	}
}
void FSsession::Onsilence()
{
	switch(m_silencestatus)
	{
	case Session_nosilence:
		{
			xAutoLock L(m_silencestatusLock);
			m_silencestatus=Session_silencefirst;
		//	m_silenceTime=0;
			m_IsAsr=false;
			map<string, base_script_t> nodeMap = FSprocess::m_gKeymap;
			char strcmdSCID[32]={0};
			sprintf(strcmdSCID,"%s_%d",m_speeckCraftID.c_str(),0);  //nodemap 里为0的节点，为静音播放节点
			map<string, base_script_t>::iterator iter= nodeMap.find(strcmdSCID);
			if (iter != nodeMap.end())
			{
				base_script_t node = iter->second;
				//node.vox_base += ".wav";
				esl_log(ESL_LOG_INFO, " uuid=%s\n",strUUID.c_str());
				esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), strUUID.c_str());
				collection("机器人",node.desc,-1);
				//	m_DB_talk_times+=1;
				esl_log(ESL_LOG_INFO, "playback the answar ,nodeState:0 \n");
			//	LOG(INFO)<<"playback the Onsilence ,nodeState:0";

			}
			else
			{
				esl_log(ESL_LOG_INFO, "not find the voice file ,nodeState:%d \n",nodeState);
			}
		}
		break;
	case Session_silencefirst:
		{
			xAutoLock L(m_silencestatusLock);
			m_silencestatus=Session_silenceSecond;
			m_IsAsr=false;
			map<string, base_script_t> nodeMap = FSprocess::m_gKeymap;
			char strcmdSCID[32]={0};
			sprintf(strcmdSCID,"%s_%d",m_speeckCraftID.c_str(),10);  //nodemap 里为0的节点，为静音播放节点
			map<string, base_script_t>::iterator iter= nodeMap.find(strcmdSCID);
			if (iter != nodeMap.end())
			{
				base_script_t node = iter->second;
				//node.vox_base += ".wav";
				printf("100 stop_asr uuid:%s",strUUID.c_str());
				esl_log(ESL_LOG_INFO, " uuid=%s\n",strUUID.c_str());
				esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), strUUID.c_str());
				collection("机器人",node.desc,-1);
				//	m_DB_talk_times+=1;
				esl_log(ESL_LOG_INFO, "playback the answar ,nodeState 0 \n");
			//	LOG(INFO)<<"playback the Onsilence ,nodeState:0"<<;



			}
			else
			{
				esl_log(ESL_LOG_INFO, "not find the voice file ,nodeState:%d \n",nodeState);
			}
		}
		break;
	case Session_silenceSecond:
		{

		}
		break;
	}
	
}
void FSsession::Action(esl_handle_t *phandle,esl_event_t *pevent)
{
	xAutoLock L(m_silencestatusLock);
	string event_subclass, contact, from_user;
	map<string, base_script_t> nodeMap = FSprocess::m_gKeymap;
	vector<base_knowledge_t> knowledgeset=FSprocess::m_knowledgeSet;
	string a_uuid=GetSessionID();
	char tmp_cmd[1024] = {0};
	//cout<<"event->event_id:"<<event->event_id<<endl;
	switch (event->event_id)
	{

	case ESL_EVENT_CUSTOM:
	{
			
			if(m_IsAsr)
			{
				m_IsAsr=false;
				string asrResp = esl_event_get_header(pevent, "ASR-Response") ? esl_event_get_header(pevent, "ASR-Response") : "";
				esl_log(ESL_LOG_INFO, "asrResp=%s,strUUID=%s,m_IsAsr=%d\n", asrResp.c_str(),strUUID.c_str(),m_IsAsr);
				LOG(INFO)<<"asrResp="<<asrResp;
				LOG(INFO)<<"strUUID="<<strUUID;
				LOG(INFO)<<"m_IsAsr="<<m_IsAsr;
				string asrParstText;
#ifdef _Use_ALI_SDK
				// 			int pos = asrResp.find("text");
				// 			if (pos<0) {
				// 
				// 				return ;
				// 			}
				// 			asrParstText = codeHelper::GetInstance()->getAliAsrTxt(asrResp);
				asrParstText=asrResp;
#else 
				cJSON *textRoot= cJSON_Parse(asrResp.c_str());
				cJSON*results_recognition=cJSON_GetObjectItem(textRoot,"results_recognition");
				if(!results_recognition)
					return ;
				int arr1=cJSON_GetArraySize(results_recognition);
				cJSON*word=cJSON_GetArrayItem(results_recognition,0);
				asrParstText=word->valuestring;
#endif		
				if(asrParstText=="")
					return;

				esl_log(ESL_LOG_INFO,"we find text in aspResp\n");
// 				m_silencestatusLock.lock();
// 				m_silencestatus=Session_nosilence;
// 				silenceAdd(Session_resetsilence);
// 				m_silencestatusLock.unlock();
				esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
				collection("客户",asrResp);
				string asrText=asrParstText;
				esl_log(ESL_LOG_INFO, "asr_txt=%s\n", asrText.c_str());
				char strSCID[32]={0};
				sprintf(strSCID,"%s_%d",m_speeckCraftID.c_str(),nodeState);
				map<string, base_script_t>::iterator tempiter = nodeMap.find(strSCID);
				if(tempiter==nodeMap.end()) return;
				string keywordText = tempiter->second.userWord;
				esl_log(ESL_LOG_INFO, "current nodestatus:%s,Result is keyword :%s\n", strSCID,keywordText.c_str());
				LOG(INFO)<<"current nodestatus:"<<strSCID<<" Result is keyword:"<<keywordText;
				char setVar[200];
			//	snprintf(setVar, sizeof setVar, "node_state=%d", it->first);
				vector<base_knowledge_t>::iterator knowledgeite=knowledgeset.begin();
				int nextstate=0;
				string know_path;
				bool b_getknow_path=false;
				while(knowledgeite!=knowledgeset.end())
				{
					if(knowledgeite->voice_version_id!=atoi(m_speeckCraftID.c_str()))
					{
						knowledgeite++;
						continue;
					}
					string strwork=knowledgeite->keyword;
					while(strwork!="")//需要分割解析
					{
						size_t pos = strwork.find("#");
						if(pos != std::string::npos)
						{
							if(pos==0&&pos+1<=strwork.length())
							{
								strwork=strwork.substr(pos+1);
								continue;
							}
							std::string x = strwork.substr(0,pos);
							if(asrText.find(x)!=std::string::npos)
							{
								string tempkeyword=knowledgeite->keyword;
								esl_log(ESL_LOG_INFO,"hit knowledge lib:%s",tempkeyword.c_str());
								nextstate=SC_KnowledgeLib;
								know_path=knowledgeite->record;
								collection("知识库",knowledgeite->desc);
								b_getknow_path=true;
								break;
							}
							strwork=strwork.substr(pos+1);
						}
						else
						{
							if(asrText.find(strwork)!=std::string::npos)
							{
								if(asrText.find(strwork)!=std::string::npos)
								{
									string tempkeyword=knowledgeite->keyword;
									esl_log(ESL_LOG_INFO,"hit knowledge lib:%s",tempkeyword.c_str());
									nextstate=SC_KnowledgeLib;
									know_path=knowledgeite->record;
									collection("知识库",knowledgeite->desc);
									b_getknow_path=true;
									break;
								}
							}
							strwork="";
						}
					}
					if(b_getknow_path)break;
					knowledgeite++;
				}
			
				if(nextstate!=SC_KnowledgeLib)
				{
					if(keywordText!="")
						nextstate=Getnextstatus(asrText,keywordText);
					else
						nextstate=SC_Hungup;
				}
				else
				{
					m_SessionState|=GF_knowledge_node;
				}
				SetFinnallabel(nodeState,nextstate);
				//m_silencestatus=Session_nosilence;
				silenceAdd(Session_resetsilence);
				switch(nextstate)
				{
				case SC_Opening_Remarks:
					{
						nodeState=SC_Opening_Remarks;
					}
					break;
				case SC_Add_In_Wechat:
					{
						nodeState=SC_Add_In_Wechat;
					}
					break;
				case	SC_Additive_Group:
					{
						nodeState=SC_Additive_Group;
					}
					break;
				case	SC_Add_Back_Wechat:
					{
						nodeState=SC_Add_Back_Wechat;

							//esl_execute(handle, "hangup", NULL, a_uuid.c_str());
					}
					break;
				case	SC_Bye:
					{
						nodeState=SC_Bye;
						//esl_execute(handle, "hangup", NULL, a_uuid.c_str());
					}
					break;
				case	SC_Contect_nextTime:
					{
						nodeState=SC_Contect_nextTime;
						//esl_execute(handle, "hangup", NULL, a_uuid.c_str());
					}
					break;
				case	SC_Seeking_Attention:
					{
						nodeState=SC_Seeking_Attention;
					}
					break;
				case SC_KnowledgeLib:
					{
						esl_status_t t=esl_execute(handle, "playback", know_path.c_str(), a_uuid.c_str());
						m_DB_talk_times+=1;
						esl_log(ESL_LOG_INFO, "playback know_voice ,nodeState:%d know_path=%s \n",nodeState,know_path.c_str());
						sleep(1);
						//m_IsAsr=true;
						return ;
					}
				case SC_Hungup:
					{
						m_DB_talk_times+=1;
						nodeState=SC_Hungup;
						esl_log(ESL_LOG_INFO, "call hangup ,nextstat:%d \n",nextstate);
						esl_execute(handle, "hangup", NULL, a_uuid.c_str());
						m_DB_hungup="robot_hangup";
						return;
					}
				default:
					nodeState = nextstate;
					break;

				}//switch
				char strcmdSCID[32]={0};
				sprintf(strcmdSCID,"%s_%d",m_speeckCraftID.c_str(),nodeState);
				map<string, base_script_t>::iterator iter= nodeMap.find(strcmdSCID);
				if (iter != nodeMap.end())
				{
					base_script_t node = iter->second;
					//node.vox_base += ".wav";
					printf("100 stop_asr uuid:%s",strUUID.c_str());
					esl_log(ESL_LOG_INFO, " uuid=%s\n",strUUID.c_str());
					esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), a_uuid.c_str());
					collection("机器人",node.desc,nodeState);
					if(nodeState==8||nodeState==21||nodeState==22||nodeState==23||nodeState==24||nodeState==25||nodeState==26||nodeState==27)
						m_SessionState|=GF_nothear;
					m_DB_talk_times+=1;
					esl_log(ESL_LOG_INFO, "playback the answar ,nodeState:%d \n",nodeState);
					LOG(INFO)<<"playback the answar ,nodeState:"<<nodeState;
				}
				else
				{
					esl_log(ESL_LOG_INFO, "not find the voice file ,nodeState:%d \n",nodeState);
				}
				return;
			}//m_IsAsr
		//	}//asr

			// esl_execute(handle, "stop_asr", NULL, strUUID.c_str());
		}//case custom

		break;
	case ESL_EVENT_DTMF:
	{
		string dtmf = esl_event_get_header(pevent, "DTMF-Digit") ? esl_event_get_header(pevent, "DTMF-Digit") : "";
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		strUUID = esl_event_get_header(pevent, "Caller-Unique-ID") ? esl_event_get_header(pevent, "Caller-Unique-ID") : "";
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
		//destination_number = esl_event_get_header(event, "Caller-Destination-Number")? esl_event_get_header(event, "Caller-Destination-Number") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : "";
		//const char *eventbody = esl_event_get_body(pevent);
		//printf("body:\n%s\n", eventbody);
		esl_log(ESL_LOG_INFO, "dtmf :%s\n", dtmf.c_str());
		printf("ESL_EVENT_DTMF:inbound dtmf :%s\n", dtmf.c_str());

		break;
	}
	case ESL_EVENT_CHANNEL_ORIGINATE:
	{

		//???????
// 		string is_callout, a_leg_uuid;
// 		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
// 		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
// 		string createTime = esl_event_get_header(event, "Caller-Channel-Created-Time") ? esl_event_get_header(event, "Caller-Channel-Created-Time") : "";
// 		a_leg_uuid = esl_event_get_header(event, "variable_origination_uuid") ? esl_event_get_header(event, "variable_origination_uuid") : "";

		break;
	}
	case ESL_EVENT_CHANNEL_BRIDGE:
	{
		// esl_execute(handle, "hangup", NULL, uuid);
	}
	break;
	case ESL_EVENT_CHANNEL_PARK:
	{
		esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_PARK:inbound park :%s\n", strUUID.c_str());
		LOG(INFO)<<"ESL_EVENT_CHANNEL_PARK:inbound park :"<<strUUID;
		esl_execute(handle, "answer", NULL, strUUID.c_str());
// 		string recordpath=Getrecordpath();
// 		char filename[48];
// 		sprintf(filename,"%s/%s_%d.wav",recordpath.c_str(),caller_id.c_str(),GetUTCtimestamp());
// 		//sprintf(filename,"%s/0000000000_1551959350.wav",recordpath.c_str(),GetUTCtimestamp());
// 		esl_log(ESL_LOG_INFO, "record file name:%s\n", filename);
// 		sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
// 		//sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
// 		esl_log(ESL_LOG_INFO, "esl_send_recv cmd: %s\n", tmp_cmd);
// 		esl_send_recv_timed(handle, tmp_cmd, 1000);
// 		this->m_DB_recording_file= filename;
// 		map<uint32_t, base_script_t>::iterator iter; //=keymap.find(1);
// 		iter = nodeMap.find(1);
// 		if (iter != nodeMap.end())
// 		{
// 			base_script_t node = iter->second;
// 			node.vox_base += ".wav";
// 			esl_execute(handle, "set", "node_state=1", strUUID.c_str());
// 			playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
// 
// 		}

		break;
	}
	case ESL_EVENT_CHANNEL_EXECUTE_COMPLETE:
	{
		esl_log(ESL_LOG_DEBUG, "ESL_EVENT_CHANNEL_EXECUTE_COMPLETE:inbound EXECUTE_COMPLETE :%s\n", strUUID.c_str());
		const char *application = esl_event_get_header(pevent, "Application");

		break;
	}
	case ESL_EVENT_CHANNEL_HANGUP:
	{
		string is_callout;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
		string bridged_uuid;
		string hangup_cause;
		hangup_cause = esl_event_get_header(pevent, "variable_sip_term_cause") ? esl_event_get_header(pevent, "variable_sip_term_cause") : "";
		{
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_HANGUP:CALL IN  :%s\n", strUUID.c_str());
			esl_log(ESL_LOG_INFO, "hangup cause:%s\n", hangup_cause.c_str());
			LOG(INFO)<<"ESL_EVENT_CHANNEL_HANGUP:CALL IN  :"<<strUUID<<" hangup cause:"<<hangup_cause;
		}
		this->m_DB_end_stamp=Getcurrenttime();
		if(this->m_silencestatus!=Session_noanswar)
			this->m_DB_duration=GetUTCtimestamp()-this->m_DB_creatd_at;
		break;
	}
	case ESL_EVENT_CHANNEL_HANGUP_COMPLETE:
	{
		strUUID = esl_event_get_header(pevent, "Caller-Unique-ID") ? esl_event_get_header(pevent, "Caller-Unique-ID") : "";
		//????????????????????????
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
		{
			esl_log(ESL_LOG_INFO, "CALL OUT HANGUP_COMPLETE :%s\n", strUUID.c_str());
			//record
		}
		break;
	}

	case ESL_EVENT_CHANNEL_DESTROY:
	{
		//to do??????????????????????????????????????????????
		string is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
		string hangupTime = esl_event_get_header(pevent, "Caller-Channel-Hangup-Time") ? esl_event_get_header(pevent, "Caller-Channel-Hangup-Time") : "";

		string recordFileName = esl_event_get_header(pevent, "variable_record_filename") ? esl_event_get_header(pevent, "variable_record_filename") : "";

		{
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_DESTROY call in uuid:%s \n", __FILE__, __LINE__, strUUID.c_str());
		}
		break;
	}
	case ESL_EVENT_CHANNEL_OUTGOING:
	{
		string is_callout;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
																																   /*const char *eventbody=esl_event_get_body(event);
			printf("body:\n%s\n",eventbody);*/
		break;
	}
	case ESL_EVENT_PLAYBACK_START:
	{
		m_playbackstatus=Session_playing;
		silenceAdd(Session_resetsilence);
		string is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????

		//??????
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_START %s\n", strUUID.c_str());
		}
		break;
	}
	case ESL_EVENT_PLAYBACK_STOP:
	{
		m_IsAsr=true;
		if(m_playbackstatus==Session_playing)
		{
			m_playbackstatus=Session_noplayback;
			esl_log(ESL_LOG_INFO, "m_playbackstatus Session_noplayback\n");
		}
		//????????????
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
	//	destination_number = esl_event_get_header(event, "Caller-Destination-Number")? esl_event_get_header(event, "Caller-Destination-Number") : "";
	//	strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_STOP %s\n", strUUID.c_str());
		}
		char strSCID[32]={0};
		sprintf(strSCID,"%s_%d",m_speeckCraftID.c_str(),nodeState);
		//string strscid=strSCID;
		map<string, base_script_t>::iterator iter = nodeMap.find(strSCID);
		if(iter!=nodeMap.end())
		{
			string keyword = iter->second.userWord;
			//m_IsAsr=true;

			if (keyword.empty())
			{
				esl_log(ESL_LOG_INFO, "handup Result is keyword :%s\n", keyword.c_str());
				esl_execute(handle, "hangup", NULL, strUUID.c_str());
				m_DB_hungup="robot_hangup";
				break;
			}
		}
		if(m_silencestatus==Session_silenceSecond)
		{
			m_IsAsr=false;
			esl_log(ESL_LOG_INFO, " m_silencestatus:Session_silenceSecond\n");
			esl_execute(handle, "hangup", NULL, strUUID.c_str());
			m_DB_hungup="robot_hangup";
			break;
		}
		break;
	}
	case ESL_EVENT_CHANNEL_PROGRESS:
	{
		a_uuid = esl_event_get_header(pevent, "variable_a_leg_uuid") ? esl_event_get_header(pevent, "variable_a_leg_uuid") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(pevent, "variable_is_callout") ? esl_event_get_header(pevent, "variable_is_callout") : ""; // ?????1???????????????
		/*const char *eventbody=esl_event_get_body(event);
			printf("body:\n%s\n",eventbody);*/
		string bridged_uuid;

		bridged_uuid = esl_event_get_header(pevent, "other-leg-unique-id") ? esl_event_get_header(pevent, "other-leg-unique-id") : "";
		//printf("%s, %d\tbridged_uuid:%s\n",__FILE__,__LINE__,bridged_uuid.c_str());
		{
			esl_log(ESL_LOG_INFO, "CHANNEL_PROGRESS %s\n", strUUID.c_str());

			string bridged_uuid;
			//bridged_uuid = esl_event_get_header(event, "variable_bridged_uuid")?esl_event_get_header(event, "variable_bridged_uuid"):"";
			printf("%s, %d\tbridged_uuid:%s\n", __FILE__, __LINE__, bridged_uuid.c_str());
		}
		break;
	}
	case ESL_EVENT_DETECTED_SPEECH:
	{
		esl_log(ESL_LOG_INFO, "ESL_EVENT_DETECTED_SPEECH %s\n", strUUID.c_str());


		//

		break;
	}
	case ESL_EVENT_TALK:
	{
		esl_log(ESL_LOG_INFO, "ESL_EVENT_TALK %s\n", strUUID.c_str());

		break;
	}
	}
	return ;
}
// void FSsession::playDetectSpeech(string playFile, esl_handle_t *handle, string uuid)
// {
// 	esl_execute(handle, "playback", playFile.c_str(), uuid.c_str());
// 
// }
void FSsession::InsertSessionResult()
{
	//SetFinnallabel();
	char querysql[4096]={0};
	string strsql="Insert into call_cdr_tbl (inbound_talk_times, caller_id_number, destination_number, start_stamp, end_stamp, duration, recording_file, task_name, outbound_label, task_id, created_at, updated_at,username,sessiontext,whohangup)values ";
	sprintf(querysql,"%s(%d,'%s','%s','%s','%s',%d,'%s','%s','%s',%d,%d,%d,'%s','%s','%s')",strsql.c_str(),m_DB_talk_times,caller_id.c_str(),destination_number.c_str(),m_DB_start_stamp.c_str(),m_DB_end_stamp.c_str(),m_DB_duration,
		m_DB_recording_file.c_str(),m_DB_task_name.c_str(),m_DB_outbound_label.c_str(),atoi(m_taskID.c_str()),m_DB_creatd_at,m_DB_updated_at,m_username.c_str(),m_SessionWord.c_str(),m_DB_hungup.c_str());
	esl_log(ESL_LOG_INFO,"insert the session redcord:%s\n",querysql);
	//db_operator_t::initDatabase();
	//m_databaselock.lock();
	db_operator_t::InsertSessionRe(querysql);
}
string FSsession::Getcurrenttime()
{
	struct tm *tblock;
	time_t timer = time(NULL);
	char strtime[64]={0};
	tblock = localtime(&timer);
	sprintf(strtime,"%04d-%02d-%02d %02d:%02d:%02d",tblock->tm_year+1900,tblock->tm_mon+1,tblock->tm_mday,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
	return strtime;
}
int FSsession::GetUTCtimestamp()
{
	return time(NULL);
}
string FSsession::Getrecordpath()
{
	string rootpath=FSprocess::m_recordPath;
// 	string currenttime=Getcurrenttime();
// 	currenttime=currenttime.erase(currenttime.find_first_of(" "));
// 	string recordpath=rootpath+"/"+currenttime;
// 	if(access(recordpath.c_str(),F_OK)==0) //存在直接返回
// 		return recordpath;
// 	else									//不存在则创建
// 	{
// 		mkdir(recordpath.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
// 	}
	return FSprocess::m_recordPath;
}
void FScall::run()
{
	LauchFScall();
}
int FScall::GetnumbrList()
{
	  //db_operator_t::initDatabase();
	 db_operator_t::GetnumberList(m_NumberSet,m_taskID);
	 db_operator_t::Getcallability(m_taskinfo,m_taskID);
// 	 if (m_robotNum<=0||m_robotNum>=MaxRobotNum)
// 	 {
// 		esl_log(ESL_LOG_INFO,"the invalid rebot num\n");
// 		 m_robotNum=MaxRobotNum;
// 	 }
	 esl_log(ESL_LOG_INFO,"FScall::GetnumbrList,m_robotNum=%d,m_recallTimes=%d\n",m_taskinfo.robotenum,m_taskinfo.recalltimes);
	  return 0;
}
void FScall::Initability()
{
	IniFile IniService;
	IniService.load("Service.ini");
	int iret=-1;
	m_fsip=IniService.getStringValue("FREESWITCH","IP",iret);
	if(iret!=0)
	{
		m_fsip="127.0.0.1";
	}
	iret=-1;
	m_fsPort=IniService.getIntValue("FREESWITCH","PORT",iret);
	if(iret!=0)
	{
		m_fsPort=8021;
	}
	iret=-1;
	m_fsPassword=IniService.getStringValue("FREESWITCH","PASSWORD",iret);
	if(iret!=0)
	{
		m_fsPassword="ClueCon";
	}
	iret=-1;
	m_originate_timeout=IniService.getIntValue("FREESWITCH","ORIGTIMEOUT",iret);
	if(iret!=0)
	{
		m_originate_timeout=45;
	}
	iret=-1;
}
FScall*FScall::Instance()
{
	return m_inst;
}
FScall*FScall::m_inst=new FScall;
bool FScall::Getablibity(string jsonstr)
{
	cJSON *root = cJSON_Parse(jsonstr.c_str());
	if(root==0)
	{
		esl_log(ESL_LOG_INFO,"please send correct json data \n");
		return false;
	}
	cJSON* item=root->child;
	char p[16]={0};
	cJSON *cspeechcid=cJSON_GetObjectItem(root,"speechCraftID");
	if(cspeechcid)
	{
		sprintf(p,"%d",cspeechcid->valueint);
		m_speechcraftID=p;
	}
	else
		m_speechcraftID="";
	cJSON *ctaskid=cJSON_GetObjectItem(root,"taskID");
	if(ctaskid)
	{
		memset(p,0,16);
		sprintf(p,"%d",ctaskid->valueint);
		m_taskID=p;
	}
	else
	{
		m_taskID="";
		esl_log(ESL_LOG_INFO,"can not find 'taskID' in json data\n");
		return false;
	}
	cJSON*ctaskname=cJSON_GetObjectItem(root,"taskName");
	if(ctaskname)
		m_taskName=ctaskname->valuestring;
	else
		m_taskName="suninfo_task";
	esl_log(ESL_LOG_INFO,"get the ability:m_speechcraft:%s,m_taskid:%s,m_taskname=%s\n",m_speechcraftID.c_str(),m_taskID.c_str(),m_taskName.c_str());
	cJSON_Delete(root);
	return true;
}
void FScall::StopTask()
{
	m_stop=true;
	m_IsAllend=true;
	m_CallStatus=CallStop;
	m_taskinfo.recalltimes=0;
	xAutoLock l(m_sessionlock);
	m_Sessioncond.signal();
}
void FScall::PauseTask()
{
	if(m_CallStatus==CallStop)
		return;
	m_stop=true;
	m_CallStatus=CallPause;
	xAutoLock l(m_sessionlock);
	m_Sessioncond.signal();

}
void FScall::ResumeTask()
{
	if(m_CallStatus!=CallPause)
		return;
	m_CallStatus=CallResume;
	m_stop=false;
	xAutoLock l(m_sessionlock);
	m_Sessioncond.signal();
	start();
	//LauchFScall();

}
int FScall::LauchFScall()
{
	if(m_taskID==""||m_speechcraftID=="suninfo_task")
	{
		esl_log(ESL_LOG_INFO,"get the worry taskid------do not lauchFScall---\n");
		return 0;
	}

 	esl_handle_t handle = {{0}};
    esl_status_t status;
    char uuid[128]; //
    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);
    status = esl_connect(&handle, m_fsip.c_str(), m_fsPort, NULL, m_fsPassword.c_str());

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }
	char callCmd[256];
	vector<t_Userinfo>::iterator ite=m_NumberSet.begin();
	if(m_CallStatus==CallInit||m_CallStatus==Recall)
		m_pPauseIte=m_NumberSet.begin();
	if(m_CallStatus==CallResume)
		ite=m_pPauseIte;
	m_CallStatus=CallStart;
	esl_log(ESL_LOG_INFO,"munberset.size=%d\n",m_NumberSet.size());
	int NumCancall=m_taskinfo.robotenum;
	while(ite!=m_NumberSet.end()&&!m_stop)
	{
		//xAutoLock L(m_sessionlock);
		if(NumCancall<=0)
		{
			xAutoLock L(m_sessionlock);
			esl_log(ESL_LOG_INFO,"FSprocess::m_Sessioncond.wait,m_robotNum=%d\n",m_taskinfo.robotenum);
			esl_log(ESL_LOG_INFO,"NumCancall:%d\n",NumCancall);
			m_Sessioncond.wait(m_sessionlock);
			NumCancall=m_taskinfo.robotenum-m_SessionSet.size();
		}
		//originate_time
		sprintf(callCmd,"bgapi originate {ignore_early_media=true,originate_timeout=%d,speechCraftID=%s,taskID=%s,taskname=%s,username=%s}sofia/gateway/ingw/88%s &park()",m_taskinfo.originate_timeout,m_speechcraftID.c_str(),m_taskID.c_str(),m_taskName.c_str(),(ite->username).c_str(),(ite->phonenum).c_str());
		//sprintf(callCmd,"bgapi originate {speechCraftID=%s,taskID=%s,taskname=%s}user/%s &park()",m_speechcraftID.c_str(),m_taskID.c_str(),m_taskName.c_str(),(*ite).c_str());
		esl_send_recv(&handle,callCmd);
		esl_log(ESL_LOG_INFO,"callCmd:%s,FSprocess::m_SessionSet.size()=%d\n",callCmd,m_SessionSet.size()); 
		--NumCancall;
		maxCallout++;
		esl_log(ESL_LOG_INFO,"NumCancall:%d\n",NumCancall);
		ite++;
		m_pPauseIte=ite;
	}
    if (handle.last_sr_event && handle.last_sr_event->body)
    {
        printf("[%s]\n", handle.last_sr_event->body);
    }
    else
    {
        printf("[%s] last_sr_reply\n", handle.last_sr_reply);
    }
	if(!m_stop)
	{
		m_IsAllend=true;
		m_CallStatus=CallStop;
	}
	return 0;
}
void FScall::Checksilence()
{
	map<string,FSsession*>::iterator ite=m_SessionSet.begin();
	while(ite!=m_SessionSet.end())
	{
		FSsession*p=ite->second;
		if(p)
		{
			p->silenceAdd(Session_silenceinc);
			if(p->CheckoutIfsilence())
			{
				p->silenceAdd(Session_resetsilence);
				p->Onsilence();
			}
		}

		ite++;
	}
}
int FScall::reLauchFSCall()
{
	if(m_CallStatus==Recall&&FinishCreateAllSession()&&m_taskinfo.recalltimes>0)
	{
		esl_log(ESL_LOG_INFO,"-----************-----reLauchFSCall m_CallStatus=%d,m_notAnswerSet=%d\n",m_CallStatus,m_notAnswerSet.size());
		m_taskinfo.recalltimes--;
		m_pPauseIte=m_NumberSet.end();
		m_IsAllend=false;
		maxSessionDestory=0;
		m_NumberSet.assign(m_notAnswerSet.begin(),m_notAnswerSet.end()); 
		m_notAnswerSet.clear();
		esl_log(ESL_LOG_INFO,"m_NumberSet.size()=%d,m_notAnswerSet=%d\n",m_NumberSet.size(),m_notAnswerSet.size());
		start();
	}
}
FSsession* FScall::GetSessionbychannelid(string channel)
{
	//sofia/internal/1006@192.168.2.143:35423
	//xAutoLock L(m_sessionlock);
	string tempchannel=channel;
	string callednumber=tempchannel.substr(0,tempchannel.find_first_of("@"));
	callednumber=callednumber.erase(0,callednumber.find_last_of("/")+1);
	map<string,FSsession*>::iterator ite=m_SessionSet.begin();
	while(ite!=m_SessionSet.end())
	{
		if((ite->second)->destination_number==callednumber)
		{
			return ite->second;
		}
		ite++;
	}
	return NULL;
}
FSsession*FScall::GetSessionbymainUUID(string& strmainid)
{
	map<string,FSsession*>::iterator ite=m_SessionSet.find(strmainid);
	if(ite!=m_SessionSet.end())
		return ite->second;
	else
		return NULL;
}
FSsession* FScall::CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname,string username,int silenceTime)
{
	FSsession* psession=new FSsession;
	psession->m_taskID=strtaskID;
	psession->m_speeckCraftID=strscraftID;
	psession->strUUID = strUUID;
	psession->caller_id=caller_id;
	psession->destination_number=destination_number;
	psession->SetSessionID(strUUID);
	psession->handle=handle;
	psession->event=event;
	psession->m_DB_task_name=taskname;
	//psession->m_DB_creatd_at = psession->GetUTCtimestamp();
	psession->m_DB_start_stamp=psession->Getcurrenttime();
	psession->m_DB_updated_at=0;
	psession->m_username=username;
	psession->m_maxsilenceTime=silenceTime;
	return psession;
}

void FScall::CallEvent_handle(esl_handle_t *handle,
	esl_event_t *event,
	map<string, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib)
{
	char tmp_cmd[1024] = {0};
	string strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
	string caller_id = esl_event_get_header(event, "Caller-Caller-ID-Number") ? esl_event_get_header(event, "Caller-Caller-ID-Number") : "";
	string destination_number = esl_event_get_header(event, "Caller-Destination-Number") ? esl_event_get_header(event, "Caller-Destination-Number") : "";
	string strscraftID=esl_event_get_header(event, "Variable_speechCraftID") ? esl_event_get_header(event, "Variable_speechCraftID") : "";
	string strtaskID=esl_event_get_header(event, "Variable_taskID") ? esl_event_get_header(event, "Variable_taskID") : "";
	string strtaskname=esl_event_get_header(event, "Variable_taskname") ? esl_event_get_header(event, "Variable_taskname") : "";
	string tmpNodeState = esl_event_get_header(event, "Variable_node_state") ? esl_event_get_header(event, "Variable_node_state") : "";
	string strusername=esl_event_get_header(event, "Variable_username") ? esl_event_get_header(event, "Variable_username") : "";
	switch (event->event_id)
	{
	case ESL_EVENT_CUSTOM:
		{
			xAutoLock l(m_sessionlock);
			string event_subclass = esl_event_get_header(event, "Event-Subclass") ? esl_event_get_header(event, "Event-Subclass") : "";
			if (event_subclass == "asr"||event_subclass == "asrchanged")
			{
				string asrResp = esl_event_get_header(event, "ASR-Response") ? esl_event_get_header(event, "ASR-Response") : "";
				esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
// 				const char *eventbody = esl_event_get_body(event);
// 				esl_log(ESL_LOG_INFO, "eventbody=%s\n", eventbody);
				string coreuuid = esl_event_get_header(event, "Core-UUID") ? esl_event_get_header(event, "Core-UUID") : "";
				esl_log(ESL_LOG_INFO, "coreuuid=%s\n", coreuuid.c_str());
				string Channel = esl_event_get_header(event, "Channel") ? esl_event_get_header(event, "Channel") : "";
				string strmainUUID = esl_event_get_header(event, "mainUUID") ? esl_event_get_header(event, "mainUUID") : "";
				esl_log(ESL_LOG_INFO, "Channel=%s\n", Channel.c_str());
				FSsession*psession = GetSessionbymainUUID(strmainUUID);
				esl_log(ESL_LOG_INFO, "strmainUUID=%s\n", strmainUUID.c_str());
				esl_log(ESL_LOG_INFO,"find the session by channel\n");
				if(psession!=NULL)
				{
					esl_log(ESL_LOG_INFO,"get the right sesson by id \n");
					if (event_subclass == "asrchanged")
					{
						esl_log(ESL_LOG_INFO, "asrchanged \n");
						psession->silenceAdd(Session_resetsilence);
						break;
					}
					psession->handle=handle;
					psession->event=event;
					psession->Action(handle,event);
				}
			}

		}
		break;
	case ESL_EVENT_CHANNEL_ORIGINATE: //创建会话
		{
			xAutoLock l(m_sessionlock);
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_ORIGINATE call in uuid:%s \n", strUUID.c_str());
			FSsession*psession = CreateSession(handle,event,strtaskID,strscraftID,strUUID,caller_id,destination_number,strtaskname,strusername,FSprocess::m_userSetsilenseTime);
			if(psession==NULL)
			{
				esl_log(ESL_LOG_INFO,"CreateSession failed\n");
				return ;
			}
			m_SessionSet[strUUID]=psession;
		}
		break;
	case ESL_EVENT_CHANNEL_ANSWER: 
		{
			xAutoLock l(m_sessionlock);
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				FSsession*psession=ite->second;
				char asrparam[256]={0};
#ifdef _Use_ALI_SDK
				sprintf(asrparam,"LTAIq8nguveEsyhV BlRVE9ZgUFiajaeiZEr3eeUiMuyUNE %s E2lTCNTExMJKdvvu",(psession->strUUID).c_str());
#else
				sprintf(asrparam,"LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0 %s",(psession->strUUID).c_str());
#endif
				esl_log(ESL_LOG_INFO,"strUUID=%s,destination_number=%s,caller_id=%s,m_SessionSet.szie()=%d,event->event_id=%d\n",strUUID.c_str(),destination_number.c_str(),caller_id.c_str(),m_SessionSet.size(),event->event_id);
				LOG(INFO)<<"create a session:"<<strUUID;
				psession->handle=handle;
				psession->event=event;
				psession->Onanswar();
				esl_status_t t = esl_execute(handle, "start_asr", (const char*)asrparam, (psession->strUUID).c_str());
				esl_log(ESL_LOG_INFO, "start_asr:%d \n", t);
				psession->silenceAdd(Session_resetsilence);
			}

		}
		break;
	case ESL_EVENT_CHANNEL_DESTROY:  //销毁会话
		{
			xAutoLock l(m_sessionlock);
			string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
			string hangupTime = esl_event_get_header(event, "Caller-Channel-Hangup-Time") ? esl_event_get_header(event, "Caller-Channel-Hangup-Time") : "";
			string recordFileName = esl_event_get_header(event, "variable_record_filename") ? esl_event_get_header(event, "variable_record_filename") : "";
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_DESTROY call in uuid:%s \n", strUUID.c_str());
				FSsession*psession=ite->second;
				m_SessionSet.erase(strUUID);
				if(psession!=NULL)
				{
					if(psession->m_silencestatus==Session_noanswar)//这里表示未接通
					{
						t_Userinfo userinfo; 
						userinfo.phonenum=psession->caller_id;
						userinfo.username=psession->m_username;
						m_notAnswerSet.push_back(userinfo);
						psession->m_DB_outbound_label="";
						esl_log(ESL_LOG_INFO,"m_notAnswerSet.size()=%d\n",m_notAnswerSet.size());
					}
					//m_sessionlock.lock();
					esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", (psession->strUUID).c_str());
					//m_sessionlock.unlock();
					psession->m_DB_creatd_at=psession->GetUTCtimestamp();
					psession->InsertSessionResult();
					esl_log(ESL_LOG_INFO,"call stop_asr uuid:%s\n",(psession->strUUID).c_str());
					delete psession;
				}
				LOG(INFO)<<"after destory session, uuid:"<<strUUID<<" m_SessionSet.size:"<<m_SessionSet.size();
			}
			if(m_SessionSet.size()<m_taskinfo.robotenum)
			{
				m_Sessioncond.signal();
				esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_ANSWER signal \n");
			}
			maxSessionDestory++;
		}
		break;
	default:
		{
			xAutoLock l(m_sessionlock);
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				FSsession*psession=m_SessionSet[strUUID];
				if(psession!=NULL)
				{
					psession->handle=handle;
					psession->event=event;
					psession->Action(handle,event);
				}
			}
		}
		break;
	}
}
Mutex FScallManager::m_InstLock;
FScallManager* FScallManager::Instance()
{
	m_InstLock.lock();
	static FScallManager _manager(1);
	m_InstLock.unlock();
	return &_manager;
}
void FScallManager::timeout()
{
	//xAutoLock L(m_InstLock);
	map<string,FScall*>::iterator ite=m_TaskSet.begin();
	while(ite!=m_TaskSet.end())
	{
		FScall*pcall=ite->second;
		if(!pcall){ite++;continue;}
		pcall->Checksilence();
		ite++;
	}	
	if(m_count%100==0)
	{
		CheckEndCall();
	}
	m_count++;
}
void FScallManager::CheckEndCall()
{
	xAutoLock L(m_CallLock);
	map<string,FScall*>::iterator CheckIte=m_TaskSet.begin();
	while(CheckIte!=m_TaskSet.end())
	{
		FScall*pcall=CheckIte->second;
		if(pcall==NULL)
		{
			 m_TaskSet.erase(CheckIte++);
			 continue;
		}
		if(pcall->m_IsAllend&& pcall->m_SessionSet.empty()&&pcall->FinishCreateAllSession())
		{
			if(pcall->m_taskinfo.recalltimes<=0)
			{
				string taskid=CheckIte->first;
				esl_log(ESL_LOG_INFO,"********delete call:CheckIte->id:%s\n",taskid.c_str());
				m_TaskSet.erase(CheckIte++);
				delete pcall;
				pcall=NULL;
			}
			else
			{
				pcall->m_CallStatus=Recall;
				pcall->reLauchFSCall();
				CheckIte++;
			}
			 continue;
		}
		else
		{
			CheckIte++;
		}
	}
}
FScall*FScallManager::GetFSCallbyUUID(string& struuid)
{
	map<string,FScall*>::iterator ite=m_TaskSet.begin();
	while(ite!=m_TaskSet.end())
	{
		FScall*pcall=ite->second;
		if(!pcall){ite++;continue;}
		FSsession* psession=pcall->GetSessionbymainUUID(struuid);
		if(psession)
			return pcall;
		ite++;
	}
	return NULL;
}
string FScallManager::HandleMessage(string data)
{
	string cmd;
	string scid;
	string taskid;
	string taskname;
	if(!ParseData(data,cmd,scid,taskid,taskname))
		return ERRORJSON;
	esl_log(ESL_LOG_INFO,"::::cmd:%s\n",cmd.c_str());
	if(cmd=="start")  //开始任务或者重新开始
	{
		map<string,FScall*>::iterator ite=m_TaskSet.find(taskid);
		if(ite!=m_TaskSet.end())
		{
			FScall*pcall=ite->second;
			pcall->StopTask();
			m_TaskSet.erase(ite);
			m_TaskSet.insert(pair<string,FScall*>(pcall->m_taskID+"deleted",pcall));
		}
		FScall* Onecall=new FScall;
		Onecall->Initability();
		if (!Onecall->Getablibity(data))
			return ERRORJSON;
		//Onecall->maxSessionCreate=0;
		Onecall->GetnumbrList();
		if(Onecall->m_taskinfo.robotenum<=0||Onecall->m_taskinfo.robotenum>MaxRobotNum)
			return ERRORROBOT;
		Onecall->start();
		m_TaskSet.insert(pair<string,FScall*>(Onecall->m_taskID,Onecall));
		return CMDOK;
	}
	else if(cmd=="pause")	//暂停任务
	{
		map<string,FScall*>::iterator ite=m_TaskSet.find(taskid);
		if(ite!=m_TaskSet.end())
		{
			FScall*pcall=ite->second;
			pcall->PauseTask();
		}
		return CMDOK;
	}
	else if(cmd=="resume")	//恢复
	{
		map<string,FScall*>::iterator ite=m_TaskSet.find(taskid);
		if(ite!=m_TaskSet.end())
		{
			FScall*pcall=ite->second;
			pcall->ResumeTask();
		}
		return CMDOK;
	}
	else if(cmd=="stop")
	{
		map<string,FScall*>::iterator ite=m_TaskSet.find(taskid);
		if(ite!=m_TaskSet.end())
		{
			FScall*pcall=ite->second;
			pcall->StopTask();
			m_TaskSet.erase(ite);
			m_TaskSet.insert(pair<string,FScall*>(pcall->m_taskID+"deleted",pcall));
		}
		return CMDOK;
	}
	return ERRORCMD;
}
void FScallManager::CallEvent_handle(esl_handle_t *handle,
	esl_event_t *event,
	map<string, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib)
{
	//xAutoLock L(m_CallLock);
	string strtaskID=esl_event_get_header(event, "Variable_taskID") ? esl_event_get_header(event, "Variable_taskID") : "";
	string strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
	map<string ,FScall*>::iterator callite=m_TaskSet.find(strtaskID);
	if(callite!=m_TaskSet.end())
	{
		//xAutoLock L(m_CallLock);
		FScall*pcall=callite->second;
		if(pcall)
			pcall->CallEvent_handle( handle,event,keymap,knowledgelib);
	}
	else
	{
		//语音消息使用uuid寻找对应的fscall
		switch (event->event_id)
		{
		case ESL_EVENT_CUSTOM:
			{
				string event_subclass = esl_event_get_header(event, "Event-Subclass") ? esl_event_get_header(event, "Event-Subclass") : "";
				if (event_subclass == "asr")
				{
					//xAutoLock L(m_CallLock);
					string strmainUUID = esl_event_get_header(event, "mainUUID") ? esl_event_get_header(event, "mainUUID") : "";
					esl_log(ESL_LOG_INFO, "mainUUID=%s\n", strmainUUID.c_str());
					FScall*pcall = GetFSCallbyUUID(strmainUUID);
					if(pcall)
					{
						esl_log(ESL_LOG_INFO, "find fscall by mainuuid\n" );
						pcall->CallEvent_handle( handle,event,keymap,knowledgelib);
					}
				}



			}

		}
	}
}
bool FScallManager::ParseData(string jsonstr,string& cmd,string& scid,string& taskid,string& taskname)
{
	cJSON *root = cJSON_Parse(jsonstr.c_str());
	if(root==0)
	{
		esl_log(ESL_LOG_INFO,"please send correct json data \n");
		return false;
	}
	cJSON* item=root->child;
	char p[16]={0};
	cJSON *cscmd=cJSON_GetObjectItem(root,"txcallcmd");
	if(cscmd)
	{
		cmd=cscmd->valuestring;
	}
	else
	{
		esl_log(ESL_LOG_INFO,"json not include a cmd\n");
		return false;
	}
	cJSON *cspeechcid=cJSON_GetObjectItem(root,"speechCraftID");
	if(cspeechcid)
	{
		sprintf(p,"%d",cspeechcid->valueint);
		scid=p;
	}
	else
		scid="";
	cJSON *ctaskid=cJSON_GetObjectItem(root,"taskID");
	if(ctaskid)
	{
		memset(p,0,16);
		sprintf(p,"%d",ctaskid->valueint);
		taskid=p;
	}
	else
	{
		taskid="";
		esl_log(ESL_LOG_INFO,"can not find 'taskID' in json data\n");
		return false;
	}
	cJSON*ctaskname=cJSON_GetObjectItem(root,"taskName");
	if(ctaskname)
		taskname=ctaskname->valuestring;
	else
		taskname="suninfo_task";
	esl_log(ESL_LOG_INFO,"get the ability:cmd:%s,m_speechcraft:%s,m_taskid:%s,m_taskname=%s\n",cmd.c_str(),scid.c_str(),taskid.c_str(),taskname.c_str());
	cJSON_Delete(root);
	return true;
}

map<string, base_script_t> FSprocess::m_gKeymap;
vector<base_knowledge_t>FSprocess::m_knowledgeSet;
string FSprocess::m_recordPath="/home/path";
int FSprocess::m_userSetsilenseTime;
int FSprocess::m_robotNum=10;
void FSprocess::Initability()
{
	IniFile IniService;
	IniService.load("Service.ini");
	int iret=-1;
	m_fsip=IniService.getStringValue("FREESWITCH","IP",iret);
	if(iret!=0)
	{
		m_fsip="127.0.0.1";
	}
	iret=-1;
	m_fsPort=IniService.getIntValue("FREESWITCH","PORT",iret);
	if(iret!=0)
	{
		m_fsPort=8021;
	}
	iret=-1;
	m_fsPassword=IniService.getStringValue("FREESWITCH","PASSWORD",iret);
	if(iret!=0)
	{
		m_fsPassword="ClueCon";
	}
	iret=-1;
	string recordpath=IniService.getStringValue("RECORDPATH","path",iret);
	if(iret==0)
	{
		m_recordPath=recordpath;
	}
	iret=-1;
	m_userSetsilenseTime=IniService.getIntValue("FREESWITCH","SILENCE",iret);
	if(iret!=0)
	{
		m_userSetsilenseTime=7;
	}
	iret=-1;
	m_robotNum=IniService.getIntValue("PROCESS","ROBOTNUM",iret);
	if(iret!=0)
	{
		m_robotNum=10;
	}
}
void FSprocess::startProcess()
{
	Initability();
	start();

}
void FSprocess::run()
{
	Inbound_Init((void*)"userinfo");
}

int FSprocess::getRoboteNum()
{

}
void *FSprocess::Inbound_Init(void *arg)
{

    esl_handle_t handle = {{0}};
    esl_status_t status;
    const char *uuid;

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, m_fsip.c_str(), m_fsPort, NULL, m_fsPassword.c_str());
	//status = esl_connect(&handle, "210.21.48.69", 8021, NULL, "tx@infosun");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }
//	m_sessionHandle=&handle;
	//esl_execute(FSprocess::getSessionhandle(), "start_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0","");
    esl_log(ESL_LOG_INFO, "Connected to FreeSWITCH\n");
  //  esl_events(&handle, ESL_EVENT_TYPE_PLAIN,
    //           "DETECTED_SPEECH RECORD_START RECORD_STOP PLAYBACK_START PLAYBACK_STOP CHANNEL_OUTGOING CHANNEL_PARK CHANNEL_EXECUTE_COMPLETE CHANNEL_ORIGINATE TALK NOTALK PHONE_FEATURE CHANNEL_HANGUP_COMPLETE CHANNEL_CREATE CHANNEL_BRIDGE DTMF CHANNEL_DESTROY CHANNEL_HANGUP CHANNEL_BRIDGE CHANNEL_ANSWER CUSTOM sofia::register sofia::unregister asr");
	esl_events(&handle, ESL_EVENT_TYPE_PLAIN,
		"DETECTED_SPEECH RECORD_START RECORD_STOP PLAYBACK_START PLAYBACK_STOP CHANNEL_OUTGOING  CHANNEL_PARK CHANNEL_EXECUTE_COMPLETE CHANNEL_ORIGINATE TALK NOTALK PHONE_FEATURE CHANNEL_HANGUP_COMPLETE CHANNEL_CREATE CHANNEL_BRIDGE DTMF CHANNEL_DESTROY CHANNEL_HANGUP CHANNEL_BRIDGE CHANNEL_ANSWER  CUSTOM sofia::register sofia::unregister asr ");

	esl_log(ESL_LOG_INFO, "%s\n", handle.last_sr_reply);

    handle.event_lock = 1;
    while ((status = esl_recv_event(&handle, 1, NULL)) == ESL_SUCCESS)
    {
        if (handle.last_ievent)
        {
            process_event(&handle, handle.last_ievent, m_gKeymap,m_knowledgeSet);
		//	handle.last_ievent=NULL;
        }
		//esl_log(ESL_LOG_INFO,"status=%d,--------\n",status);
    }
	esl_log(ESL_LOG_INFO,"status=%d,--------\n",status);
    esl_disconnect(&handle);

    return (void *)0;
}



 void *FSprocess::test_Process(void *arg)
{
    esl_handle_t handle = {{0}};
    esl_status_t status;
    char uuid[128]; //从fs??获得的uuid
    //Then running the Call_Task string when added a new Task,then remove it
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "??先生，这里是??众银行???催??心打来的，我姓张。提醒您??众银行微粒贷已经逾期，麻烦您尽快抽空处理??");

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

   // status = esl_connect(&handle, "210.21.48.69", 8021, NULL, "tx@infosun");
	status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "ClueCon");
    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }


	esl_send_recv(&handle, "bgapi originate {speechCraftID=12344321,taskID=1111,taskname=banksale}user/1006 &park()");
	//esl_send_recv(&handle, "bgapi originate {speechCraftID=12344321,taskID=1111,taskname=banksale}sofia/gateway/ingw/%s &park()");
	//esl_send_recv(&handle, "bgapi originate user/1006 &park()");
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "您好！???问您是陈大文陈先生??");

    if (handle.last_sr_event && handle.last_sr_event->body)
    {
        printf("[%s]\n", handle.last_sr_event->body);
    }
    else
    {
        printf("[%s] last_sr_reply\n", handle.last_sr_reply);
    }
}




void FSprocess::process_event(esl_handle_t *handle,
				   esl_event_t *event,
				    map<string, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib)
{	
	FScallManager* callmanager= FScallManager::Instance();
	callmanager->CallEvent_handle(handle,event,keymap,knowledgelib);
	


}
