#include "process_event.h"
#include "common/DBOperator.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
using namespace std;
#define _Use_ALI_SDK
typedef enum{
	SC_Opening_Remarks=1,
	SC_Add_In_Wechat,
	SC_Additive_Group,
	SC_Add_Back_Wechat,
	SC_Bye,
	SC_Contect_nextTime,
	SC_Seeking_Attention,
	SC_KnowledgeLib
}e_Speeckcase;
int FSsession::run()
{
	Action();
}
void FSsession::ChangetheTypeCount(string strtype)
{
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
}
void FSsession::SetFinnallabel()
{
	if(nodeState==SC_Add_Back_Wechat||(m_Atimes>=2&&m_Ctimes==0)) //命中两个A类关键字
	{
		m_DB_outbound_label="A";
	}
	else if(m_Ctimes>=1)
	{
		m_DB_outbound_label="C";
	}
	else if(m_Btimes>=1)
	{
		m_DB_outbound_label="B";
	}
	else
		m_DB_outbound_label="D";
}
int FSsession::Getnextstatus(string asrtext,string keyword)
{
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
		}
		else if(strkey.find("tab_yes")!=string::npos)
		{
			if(checked)
			{
				cJSON_Delete(root);
				return nodeNum;
			}
			string iretsent = codeHelper::GetInstance()->sentiment_classifyRequesst(asrtext);
			checked=true;
			float positive=atof(iretsent.c_str());
			if(positive>0.5)
			{
				cJSON_Delete(root);
				return nodeNum;
			}
		}
		else
		{
			if(checked)
			{
				cJSON_Delete(root);
				return nodeNum;
			}
			string iretsent = codeHelper::GetInstance()->sentiment_classifyRequesst(asrtext);
			checked=true;
			float positive=atof(iretsent.c_str());
			if(positive<=0.5)
			{
				cJSON_Delete(root);
				return nodeNum;
			}
		}
		printf("%s\n",item->string);
		item=item->next;
	}

}
void FSsession::Onanswar()
{
	char tmp_cmd[1024] = {0};
	string recordpath=Getrecordpath();
	char filename[48];
	struct tm *tblock;
	time_t timer = time(NULL);
	char strtime[64]={0};
	tblock = localtime(&timer);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d",tblock->tm_year+1900,tblock->tm_mon+1,tblock->tm_mday,tblock->tm_hour,tblock->tm_min,tblock->tm_sec);
	sprintf(filename,"%s/%s_%s.wav",recordpath.c_str(),caller_id.c_str(),strtime);
	//sprintf(filename,"%s/0000000000_1551959350.wav",recordpath.c_str(),GetUTCtimestamp());
	esl_log(ESL_LOG_INFO, "record file name:%s\n", filename);
	sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
	//sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(),filename);
	esl_log(ESL_LOG_INFO, "esl_send_recv cmd: %s\n", tmp_cmd);
	esl_send_recv_timed(handle, tmp_cmd, 1000);
	this->m_DB_recording_file= filename;
	map<uint32_t, base_script_t>::iterator iter; //=keymap.find(1);
	map<uint32_t, base_script_t> nodeMap = FSprocess::m_gKeymap;
	iter = nodeMap.find(1);
	if (iter != nodeMap.end())
	{
		base_script_t node = iter->second;
		node.vox_base += ".wav";
		esl_execute(handle, "set", "node_state=1", strUUID.c_str());
		playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());

	}
}

void FSsession::Action()
{
	string event_subclass, contact, from_user;
	map<uint32_t, base_script_t> nodeMap = FSprocess::m_gKeymap;
	vector<base_knowledge_t> knowledgeset=FSprocess::m_knowledgeSet;
	string a_uuid=GetSessionID();
	char tmp_cmd[1024] = {0};
	//cout<<"event->event_id:"<<event->event_id<<endl;
	switch (event->event_id)
	{

	case ESL_EVENT_CUSTOM:
	{
		event_subclass = esl_event_get_header(event, "Event-Subclass") ? esl_event_get_header(event, "Event-Subclass") : "";
		contact = esl_event_get_header(event, "contact") ? esl_event_get_header(event, "contact") : "";
		from_user = esl_event_get_header(event, "from-user") ? esl_event_get_header(event, "from-user") : "";

		if (event_subclass == "sofia::register")
		{

			esl_log(ESL_LOG_INFO, "sofia::register  %s, %d event_subclass=%s, contact=%s, from-user=%s\n", __FILE__, __LINE__, event_subclass.c_str(), contact.c_str(), from_user.c_str());
		}
		else if (event_subclass == "sofia::unregister")
		{
		}
		else if (event_subclass == "asr")
		{
			string asrResp = esl_event_get_header(event, "ASR-Response") ? esl_event_get_header(event, "ASR-Response") : "";
			esl_log(ESL_LOG_INFO, "asrResp=%s,strUUID=%s,m_IsAsr=%d\n", asrResp.c_str(),strUUID.c_str(),m_IsAsr);	
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
			if(m_IsAsr)
			{
				m_IsAsr=false;
				esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
				string asrText=asrParstText;
				esl_log(ESL_LOG_INFO, "asr_txt=%s\n", asrText.c_str());
				map<uint32_t, base_script_t>::iterator tempiter = nodeMap.find(nodeState);
				string keywordText = tempiter->second.userWord;
				esl_log(ESL_LOG_INFO, "current nodestatus:%d,Result is keyword :%s\n", nodeState,keywordText.c_str());

					//esl_log(ESL_LOG_INFO, "keywordText=%s \n", keywordText.c_str());
					char setVar[200];
				//	snprintf(setVar, sizeof setVar, "node_state=%d", it->first);
					vector<base_knowledge_t>::iterator knowledgeite=knowledgeset.begin();
					int nextstate=0;
					string know_path;
					bool b_getknow_path=false;
					while(knowledgeite!=knowledgeset.end())
					{
						string strwork=knowledgeite->keyword;
						while(strwork!="")//需要分割解析
						{
							size_t pos = strwork.find("#");
							if(pos != std::string::npos)
							{
								std::string x = strwork.substr(0,pos);
								if(asrText.find(x)!=std::string::npos)
								{
									string tempkeyword=knowledgeite->keyword;
									esl_log(ESL_LOG_INFO,"hit knowledge lib:%s",tempkeyword.c_str());
									nextstate=SC_KnowledgeLib;
									know_path=knowledgeite->record;
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
					if(!nextstate&&keywordText!="")
					{
						nextstate=Getnextstatus(asrText,keywordText);
					}
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
							m_IsAsr=true;
							return ;
						}

					}//switch
					map<uint32_t, base_script_t>::iterator iter;
					iter = nodeMap.find(nodeState);
					if (iter != nodeMap.end())
					{
						base_script_t node = iter->second;
						node.vox_base += ".wav";
						printf("100 stop_asr uuid:%s",strUUID.c_str());
						esl_log(ESL_LOG_INFO, " uuid=%s\n",strUUID.c_str());
						esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), a_uuid.c_str());
						m_DB_talk_times+=1;
						esl_log(ESL_LOG_INFO, "playback the answar ,nodeState:%d \n",nodeState);
					//	return;
					}
					else
					{
						esl_log(ESL_LOG_INFO, "not find the voice file ,nodeState:%d \n",nodeState);
					}
					if(nextstate==SC_Add_Back_Wechat||nextstate==SC_Bye||nextstate==SC_Contect_nextTime)
					{
						esl_log(ESL_LOG_INFO, "call hangup ,nextstat:%d \n",nextstate);
						esl_execute(handle, "hangup", NULL, a_uuid.c_str());
					}
					m_IsAsr=true;
					return;
				}
			}//asr

			// esl_execute(handle, "stop_asr", NULL, strUUID.c_str());
		}//case custom

		break;
	case ESL_EVENT_DTMF:
	{
		string dtmf = esl_event_get_header(event, "DTMF-Digit") ? esl_event_get_header(event, "DTMF-Digit") : "";
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : "";
		const char *eventbody = esl_event_get_body(event);
		printf("body:\n%s\n", eventbody);
		esl_log(ESL_LOG_INFO, "dtmf :%s\n", dtmf.c_str());
		printf("ESL_EVENT_DTMF:inbound dtmf :%s\n", dtmf.c_str());

		break;
	}
	case ESL_EVENT_CHANNEL_ORIGINATE:
	{

		//???????
		string is_callout, a_leg_uuid;
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
		string createTime = esl_event_get_header(event, "Caller-Channel-Created-Time") ? esl_event_get_header(event, "Caller-Channel-Created-Time") : "";

		a_leg_uuid = esl_event_get_header(event, "variable_origination_uuid") ? esl_event_get_header(event, "variable_origination_uuid") : "";

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
		const char *application = esl_event_get_header(event, "Application");

		break;
	}
	case ESL_EVENT_CHANNEL_HANGUP:
	{
		string is_callout;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		string bridged_uuid;
		string hangup_cause;
		hangup_cause = esl_event_get_header(event, "variable_sip_term_cause") ? esl_event_get_header(event, "variable_sip_term_cause") : "";
		{
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_HANGUP:CALL IN  :%s\n", strUUID.c_str());
			esl_log(ESL_LOG_INFO, "hangup cause:%s\n", hangup_cause.c_str());
		}
		this->m_DB_end_stamp=Getcurrenttime();
		this->m_DB_duration=GetUTCtimestamp()-this->m_DB_creatd_at;
		break;
	}
	case ESL_EVENT_CHANNEL_HANGUP_COMPLETE:
	{
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		//????????????????????????
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		{
			esl_log(ESL_LOG_INFO, "CALL OUT HANGUP_COMPLETE :%s\n", strUUID.c_str());
			//record
		}
		break;
	}

	case ESL_EVENT_CHANNEL_DESTROY:
	{
		//to do??????????????????????????????????????????????
		string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		string hangupTime = esl_event_get_header(event, "Caller-Channel-Hangup-Time") ? esl_event_get_header(event, "Caller-Channel-Hangup-Time") : "";

		string recordFileName = esl_event_get_header(event, "variable_record_filename") ? esl_event_get_header(event, "variable_record_filename") : "";

		{
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_DESTROY call in uuid:%s \n", __FILE__, __LINE__, strUUID.c_str());
		}
		break;
	}
	case ESL_EVENT_CHANNEL_ANSWER:
	{
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		//const char *eventbody=esl_event_get_body(event);
		//printf("body:\n%s\n",eventbody);
		string bridged_uuid;

		bridged_uuid = esl_event_get_header(event, "other-leg-unique-id") ? esl_event_get_header(event, "other-leg-unique-id") : "";
		{
			esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_ANSWER call in uuid:%s \n", strUUID.c_str());
		}
		break;
	}

	case ESL_EVENT_CHANNEL_OUTGOING:
	{
		string is_callout;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
																																   /*const char *eventbody=esl_event_get_body(event);
			printf("body:\n%s\n",eventbody);*/
		break;
	}
	case ESL_EVENT_PLAYBACK_START:
	{

		string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????

		//??????
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_START %s\n", strUUID.c_str());
		}
		break;
	}
	case ESL_EVENT_PLAYBACK_STOP:
	{
		//????????????
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_STOP %s\n", strUUID.c_str());
		}
		map<uint32_t, base_script_t>::iterator iter = nodeMap.find(nodeState);
		string keyword = iter->second.userWord;
		//m_IsAsr=true;

		if (keyword.empty())
		{
			esl_log(ESL_LOG_INFO, "handup Result is keyword :%s\n", keyword.c_str());
			esl_execute(handle, "hangup", NULL, strUUID.c_str());
		}

		break;
	}
	case ESL_EVENT_CHANNEL_PROGRESS:
	{
		a_uuid = esl_event_get_header(event, "variable_a_leg_uuid") ? esl_event_get_header(event, "variable_a_leg_uuid") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
		/*const char *eventbody=esl_event_get_body(event);
			printf("body:\n%s\n",eventbody);*/
		string bridged_uuid;

		bridged_uuid = esl_event_get_header(event, "other-leg-unique-id") ? esl_event_get_header(event, "other-leg-unique-id") : "";
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
void FSsession::playDetectSpeech(string playFile, esl_handle_t *handle, string uuid)
{
	// string param = playFile + " detect:unimrcp {start-input-timers=true,no-input-timeout=90000,recognition-timeout=90000}hello";

	// esl_execute(handle, "play_and_detect_speech", param.c_str(), uuid.c_str());
	// esl_execute(handle, "set", "play_and_detect_speech_close_asr=true", uuid.c_str());
	//  esl_execute(handle, "detect_speech", "stop", uuid.c_str());
	//esl_execute(FSprocess::getSessionhandle(), "playback", "/root/txcall/play/1a.wav", uuid.c_str());
	esl_execute(handle, "playback", playFile.c_str(), uuid.c_str());
	// esl_execute(handle, "detect_speech", "unimrcp:baidu-mrcp2 hello hello", uuid.c_str());
	//esl_execute(FSprocess::getSessionhandle(), "start_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", uuid.c_str());
	m_IsAsr=true;
	//sleep(10);
	//esl_execute(handle, "stop_asr", NULL, uuid.c_str());

}
void FSsession::InsertSessionResult()
{
	SetFinnallabel();
	char querysql[512]={0};
	string strsql="Insert into call_cdr_tbl (inbound_talk_times, caller_id_number, destination_number, start_stamp, end_stamp, duration, recording_file, task_name, outbound_label, task_id, created_at, updated_at)values ";
	sprintf(querysql,"%s(%d,'%s','%s','%s','%s',%d,'%s','%s','%s',%d,%d,%d)",strsql.c_str(),m_DB_talk_times,caller_id.c_str(),destination_number.c_str(),m_DB_start_stamp.c_str(),m_DB_end_stamp.c_str(),m_DB_duration,
		m_DB_recording_file.c_str(),m_DB_task_name.c_str(),m_DB_outbound_label.c_str(),atoi(m_taskID.c_str()),m_DB_creatd_at,m_DB_updated_at);
	esl_log(ESL_LOG_INFO,"insert the session redcord:%s\n",querysql);
	//db_operator_t::initDatabase();
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
	string rootpath="/home/records";
	string currenttime=Getcurrenttime();
	currenttime=currenttime.erase(currenttime.find_first_of(" "));
	string recordpath=rootpath+"/"+currenttime;
	if(access(recordpath.c_str(),F_OK)==0) //存在直接返回
		return recordpath;
	else									//不存在则创建
	{
		mkdir(recordpath.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
	}
	return recordpath;
}
void FScall::run()
{
	GetnumbrList();
	LauchFScall();
}
int FScall::GetnumbrList()
{
	  //db_operator_t::initDatabase();
	  db_operator_t::GetnumberList(m_NumberSet,m_taskID);
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
	//esl_send_recv(&handle, "bgapi originate {speechCraftID=12344321,taskID=1111,taskname=banksale}user/1006 &park()");
    //esl_send_recv(&handle, "bgapi originate user/1003 &park()");
	char callCmd[256];
	vector<string>::iterator ite=m_NumberSet.begin();
	esl_log(ESL_LOG_INFO,"munberset.size=%d\n",m_NumberSet.size());
	while(ite!=m_NumberSet.end())
	{
		sprintf(callCmd,"bgapi originate {speechCraftID=%s,taskID=%s,taskname=%s}sofia/gateway/ingw/%s &park()",m_speechcraftID.c_str(),m_taskID.c_str(),m_taskName.c_str(),(*ite).c_str());
		//sprintf(callCmd,"bgapi originate {speechCraftID=%s,taskID=%s,taskname=%s}user/%s &park()",m_speechcraftID.c_str(),m_taskID.c_str(),m_taskName.c_str(),(*ite).c_str());
		esl_send_recv(&handle,callCmd);
		esl_log(ESL_LOG_INFO,"callCmd:%s\n",callCmd);
		sleep(1);
		ite++;
	}
    if (handle.last_sr_event && handle.last_sr_event->body)
    {
        printf("[%s]\n", handle.last_sr_event->body);
    }
    else
    {
        printf("[%s] last_sr_reply\n", handle.last_sr_reply);
    }
	m_IsAllend=true;
	return 0;
}

void FScallManager::CheckEndCall()
{
	map<string,FScall*>::iterator CheckIte=m_TaskSet.begin();
	while(CheckIte!=m_TaskSet.end())
	{
		FScall*pcall=CheckIte->second;
		if(pcall==NULL)
		{
			 m_TaskSet.erase(CheckIte++);
			 continue;
		}
		if(pcall->m_IsAllend)
		{
			 m_TaskSet.erase(CheckIte++);
			 delete pcall;
			 pcall=NULL;
			 continue;
		}
		else
		{
			CheckIte++;
		}
	}
}
map<uint32_t, base_script_t> FSprocess::m_gKeymap;
vector<base_knowledge_t>FSprocess::m_knowledgeSet;
map<string,FSsession*> FSprocess::m_SessionSet;
esl_handle_t* FSprocess::m_sessionHandle=NULL;
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
}
void FSprocess::run()
{
	Inbound_Init((void*)"userinfo");
}
FSsession* FSprocess::CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number,string taskname)
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
	psession->m_DB_creatd_at = psession->GetUTCtimestamp();
	psession->m_DB_start_stamp=psession->Getcurrenttime();
	psession->m_DB_updated_at=0;
	return psession;
}
FSsession* FSprocess::GetSessionbychannelid(string channel)
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
// 			if((ite->second)->m_channelpath!="") //已经有一个语音通道了
// 			{
// 				ite++;
// 				continue;
// 			}
// 			else
//			{
//				(ite->second)->m_channelpath=channel;
//			}
			return ite->second;
		}
		ite++;
	}
	return NULL;
}
FSsession*FSprocess::GetSessionbymainUUID(string strmainid)
{
	map<string,FSsession*>::iterator ite=m_SessionSet.find(strmainid);
	if(ite!=m_SessionSet.end())
		return ite->second;
	else
		return NULL;
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
	m_sessionHandle=&handle;
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
        }
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
				   const map<uint32_t, base_script_t> &keymap,vector<base_knowledge_t>&knowledgelib)
{
	char tmp_cmd[1024] = {0};
	string strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
	string caller_id = esl_event_get_header(event, "Caller-Caller-ID-Number") ? esl_event_get_header(event, "Caller-Caller-ID-Number") : "";
	string destination_number = esl_event_get_header(event, "Caller-Destination-Number") ? esl_event_get_header(event, "Caller-Destination-Number") : "";
	string strscraftID=esl_event_get_header(event, "Variable_speechCraftID") ? esl_event_get_header(event, "Variable_speechCraftID") : "";
	string strtaskID=esl_event_get_header(event, "Variable_taskID") ? esl_event_get_header(event, "Variable_taskID") : "";
	string strtaskname=esl_event_get_header(event, "Variable_taskname") ? esl_event_get_header(event, "Variable_taskname") : "";
	string tmpNodeState = esl_event_get_header(event, "Variable_node_state") ? esl_event_get_header(event, "Variable_node_state") : "";
	//string strmainUUID=esl_event_get_header(event, "Variable_mainUUID") ? esl_event_get_header(event, "Variable_mainUUID") : "";
	//printf("strscraftID:%s\n",strscraftID.c_str());
// 	if(strUUID=="")
// 		return;
	//printf("strUUID=%s,destination_number=%s,caller_id=%s,m_SessionSet.szie()=%d,event->event_id=%d\n",strUUID.c_str(),destination_number.c_str(),caller_id.c_str(),m_SessionSet.size(),event->event_id);
	switch (event->event_id)
	{
	case ESL_EVENT_CUSTOM:
		{

			string event_subclass = esl_event_get_header(event, "Event-Subclass") ? esl_event_get_header(event, "Event-Subclass") : "";
			if (event_subclass == "asr")
			{
				string asrResp = esl_event_get_header(event, "ASR-Response") ? esl_event_get_header(event, "ASR-Response") : "";
				esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
				const char *eventbody = esl_event_get_body(event);
				esl_log(ESL_LOG_INFO, "eventbody=%s\n", eventbody);
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
					psession->handle=handle;
					psession->event=event;
					psession->Action();
				}
			}

		}
		break;
	case ESL_EVENT_CHANNEL_ANSWER:  //创建会话
		{
			string is_callout, a_leg_uuid;
			is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
			//const char *eventbody=esl_event_get_body(event);
			//printf("body:\n%s\n",eventbody);
			string bridged_uuid = esl_event_get_header(event, "other-leg-unique-id") ? esl_event_get_header(event, "other-leg-unique-id") : "";
			{
				esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_ANSWER call in uuid:%s \n", strUUID.c_str());
			}
			FSsession*psession = CreateSession(handle,event,strtaskID,strscraftID,strUUID,caller_id,destination_number,strtaskname);
			if(psession==NULL)
			{
				esl_log(ESL_LOG_INFO,"CreateSession failed\n");
				return ;
			}
			m_SessionSet[strUUID]=psession;
			m_sessionlock.lock();
			char asrparam[256]={0};
#ifdef _Use_ALI_SDK
			sprintf(asrparam,"LTAIq8nguveEsyhV BlRVE9ZgUFiajaeiZEr3eeUiMuyUNE %s E2lTCNTExMJKdvvu",(psession->strUUID).c_str());
#else
			sprintf(asrparam,"LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0 %s",(psession->strUUID).c_str());
#endif
			//esl_execute(handle, "start_asr", (const char*)asrparam, (psession->strUUID).c_str());
			//esl_execute(handle, "start_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", (psession->strUUID).c_str());
			//esl_execute(handle, "playback", "/root/txcall/play/2a.wav", strUUID.c_str());
			m_sessionlock.unlock();
			esl_log(ESL_LOG_INFO,"strUUID=%s,destination_number=%s,caller_id=%s,m_SessionSet.szie()=%d,event->event_id=%d\n",strUUID.c_str(),destination_number.c_str(),caller_id.c_str(),m_SessionSet.size(),event->event_id);
			psession->handle=handle;
			psession->event=event;
			psession->Onanswar();
			esl_execute(handle, "start_asr", (const char*)asrparam, (psession->strUUID).c_str());
		}
		break;
	case ESL_EVENT_CHANNEL_DESTROY:  //销毁会话
		{
			string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
			string hangupTime = esl_event_get_header(event, "Caller-Channel-Hangup-Time") ? esl_event_get_header(event, "Caller-Channel-Hangup-Time") : "";
			string recordFileName = esl_event_get_header(event, "variable_record_filename") ? esl_event_get_header(event, "variable_record_filename") : "";
			{
				esl_log(ESL_LOG_INFO, "ESL_EVENT_CHANNEL_DESTROY call in uuid:%s \n", strUUID.c_str());
			}
			//esl_execute(FSprocess::getSessionhandle(), "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", strUUID.c_str());
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				FSsession*psession=ite->second;
				m_SessionSet.erase(strUUID);
				if(psession!=NULL)
				{
					m_sessionlock.lock();
					esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", (psession->strUUID).c_str());
					m_sessionlock.unlock();
					psession->m_DB_creatd_at=psession->GetUTCtimestamp();
					psession->InsertSessionResult();
					esl_log(ESL_LOG_INFO,"call stop_asr uuid:%s\n",(psession->strUUID).c_str());
					delete psession;
				}
			}
			esl_log(ESL_LOG_INFO, "after destory session, uuid:%s,m_SessionSet.size()=%d \n",  strUUID.c_str(),m_SessionSet.size());
		}
		break;
	default:
		{
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				FSsession*psession=m_SessionSet[strUUID];
				if(psession!=NULL)
				{
					psession->handle=handle;
					psession->event=event;
					psession->Action();
				}
			}
		}
		break;
	}
}
