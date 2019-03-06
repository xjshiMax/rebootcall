#include "process_event.h"
#include "common/DBOperator.h"
#include <iostream>
#include <string>
using namespace std;
int gIsAsr=0;
int FSsession::run()
{
	Action();
}
void FSsession::Action()
{
	string event_subclass, contact, from_user;
	map<uint32_t, base_script_t> nodeMap = FSprocess::m_gKeymap;
	string a_uuid=GetSessionID();
	printf("16 getparksessionid:%s\n",a_uuid.c_str());
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
			//???????????μ??????????????б?????
			esl_log(ESL_LOG_INFO, "sofia::register  %s, %d event_subclass=%s, contact=%s, from-user=%s\n", __FILE__, __LINE__, event_subclass.c_str(), contact.c_str(), from_user.c_str());
		}
		else if (event_subclass == "sofia::unregister")
		{
		}
		else if (event_subclass == "asr")
		{
			string asrResp = esl_event_get_header(event, "ASR-Response") ? esl_event_get_header(event, "ASR-Response") : "";
			esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
			printf("playback uuid:%s",strUUID.c_str());	
			//esl_execute(handle, "playback", "/root/txcall/play/2.wav", a_uuid.c_str());

			int pos = asrResp.find("text");
			if (pos>=0) {
				//printf("stop_asr strUUID:%s",strUUID.c_str());
				//esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", a_uuid.c_str());
				// esl_execute(handle, "playback", "/root/txcall/play/2a.wav", strUUID.c_str());
				printf("777777777777777777777777\n");
			}

			if (!gIsAsr && pos > 0)
			{
				gIsAsr = 1;

				esl_log(ESL_LOG_INFO, "asrResp=%s\n", asrResp.c_str());
				string tmp = codeHelper::GetInstance()->getAliAsrTxt(asrResp);
				esl_log(ESL_LOG_INFO, "asr_txt=%s\n", tmp.c_str());
				map<uint32_t, base_script_t>::iterator iter = nodeMap.find(atoi(nodeState.c_str()));
				string keyword = iter->second.userWord;
				esl_log(ESL_LOG_INFO, "Result is keyword :%s\n", keyword.c_str());

				if (!keyword.empty())
				{
					// parse
					multimap<int, string> mapWord;
					codeHelper::GetInstance()->getKeyWord(mapWord, keyword);
					multimap<int, string>::iterator it;
					it = mapWord.begin();

					if (mapWord.empty())
					{
						esl_log(ESL_LOG_INFO, "flow game over \n", NULL);
						esl_execute(handle, "hangup", NULL, a_uuid.c_str());
						return;
					}

					while (it != mapWord.end())
					{

						//������ƶ�?
						string tap = it->second;
						esl_log(ESL_LOG_INFO, "tap=%s \n", tap.c_str());
						char setVar[200];
						snprintf(setVar, sizeof setVar, "node_state=%d", it->first);

						//
						int pos = tap.find("肯定");
						int negPos = tap.find("否定");
						int s1 = tmp.find("什么");
						int s2 = tmp.find("没听清");
						int s3 = tmp.find("听不清");
						if (tmp.length() < 3 || s1 >= 0 || s2 >= 0 || s3 >= 0)
						{
							esl_log(ESL_LOG_INFO, "fpositive=44444444444\n", NULL);
							map<uint32_t, base_script_t>::iterator iter;
							iter = nodeMap.find(atoi(nodeState.c_str()));
							if (iter != nodeMap.end())
							{
								base_script_t node = iter->second;
								node.vox_base += ".wav";
								printf("100 stop_asr uuid:%s",strUUID.c_str());
							//	esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", a_uuid.c_str());
								esl_log(ESL_LOG_INFO, " uuid=%s\n",strUUID.c_str());

								esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), a_uuid.c_str());
								//esl_execute(handle, "playback", "/root/txcall/play/1.wav", a_uuid.c_str());
								//playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
								esl_log(ESL_LOG_INFO, "never change state state beause user not listen  \n", "setVar");
								return;
							}
						}
						else if (pos >= 0)
						{
							string positive_prob = codeHelper::GetInstance()->sentiment_classifyRequesst(tmp);
							float fpositive = atof(positive_prob.c_str());
							esl_log(ESL_LOG_INFO, "fpositive=%f\n", fpositive);

							if (fpositive > 0.5)
							{
							//	esl_execute(handle, "set", setVar, strUUID.c_str());

								map<uint32_t, base_script_t>::iterator iter;
								iter = nodeMap.find(it->first);
								if (iter != nodeMap.end())
								{
									base_script_t node = iter->second;
									node.vox_base += ".wav";
									printf("playback uuid:%s",strUUID.c_str());
								//	esl_execute(FSasrprocess::getSessionhandle(), "stop_asr", NULL, strUUID.c_str());
								//	esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", a_uuid.c_str());
									esl_log(ESL_LOG_INFO, "playback=%f\n", fpositive);
									//esl_log(ESL_LOG_INFO, "ng POS=====fpositive=%f handle=%d uuid=%s\n", fpositive,handle,strUUID.c_str());
								//	esl_execute(FSasrprocess::getSessionhandle(), "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", strUUID.c_str());
									esl_status_t t=esl_execute(handle, "playback", node.vox_base.c_str(), a_uuid.c_str());
									//esl_execute(handle, "playback", "/root/txcall/play/1.wav", a_uuid.c_str());
									// ::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
									// esl_log(ESL_LOG_INFO, "find var node :%s file Name=%s %d=\n", setVar, node.vox_base.c_str(),t);
									esl_log(ESL_LOG_INFO,"after playback");
									//esl_execute(handle, "set", setVar, strUUID.c_str());
									return;
								}
							}
						}
						else if (negPos >= 0)
						{
							string positive_prob = codeHelper::GetInstance()->sentiment_classifyRequesst(tmp);
							float fpositive = atof(positive_prob.c_str());

							if (fpositive < 0.5)
							{
							//	esl_execute(handle, "set", setVar, strUUID.c_str());
								//sleep(10);
								map<uint32_t, base_script_t>::iterator iter;
								iter = nodeMap.find(it->first);
								if (iter != nodeMap.end())
								{
									base_script_t node = iter->second;
									node.vox_base += ".wav";
									//esl_execute(FS, "stop_asr", NULL, strUUID.c_str());
									esl_log(ESL_LOG_INFO, "ng POS=====fpositive=%f  uuid=%s\n", fpositive,strUUID.c_str());

									//esl_status_t t=esl_execute(FSprocess::getSessionhandle(), "playback", node.vox_base.c_str(), strUUID.c_str());
									esl_execute(handle, "playback", "/root/txcall/play/1.wav", a_uuid.c_str());
									// ::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
									esl_log(ESL_LOG_INFO, "find var node :%s file Name=%s \n", setVar, node.vox_base.c_str());
									//sleep(10);
								//	esl_execute(handle, "set", setVar, strUUID.c_str());
									return;
								}
							}
						}
						it++;
						// vector<string> v(20);
						// codeHelper::GetInstance()->split(tap, v, ',');
						// for (int i = 0; i < v.size(); i++)
						// {
						// 	esl_log(ESL_LOG_INFO, "key =%s,tmp=:%s\n", v.at(i).c_str(), tmp.c_str());

						// 	if (tmp.find(v.at(i)) >= 0)
						// 	{
						// 		char setVar[200];
						// 		snprintf(setVar, sizeof setVar, "node_state=%d", it->first);

						map<uint32_t, base_script_t>::iterator iter;
						// 		iter = nodeMap.find(it->first);
						// 		if (iter != nodeMap.end())
						// 		{
						// 			base_script_t node = iter->second;
						// 			node.vox_base += ".wav";
						// 			::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
						// 			esl_log(ESL_LOG_INFO, "find var node :%s\n", setVar);

						// 			esl_execute(handle, "set", setVar, strUUID.c_str());
						// 			return;
						// 		}
						// 	}
						// }
					}
				}
			}

			// esl_execute(handle, "stop_asr", NULL, strUUID.c_str());
		}

		break;
	}
	case ESL_EVENT_DTMF:
	{
		string dtmf = esl_event_get_header(event, "DTMF-Digit") ? esl_event_get_header(event, "DTMF-Digit") : "";
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ?????1???????????????
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
		// <file> detect:<engine> {param1=val1,param2=val2}<grammar>
		sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(), "/home/records/aa.wav");
		SetSessionID(strUUID);
		printf("fffffffffffffff\n");
		esl_send_recv_timed(handle, tmp_cmd, 1000);

		map<uint32_t, base_script_t>::iterator iter; //=keymap.find(1);
		iter = nodeMap.find(1);
		if (iter != nodeMap.end())
		{
			base_script_t node = iter->second;
			node.vox_base += ".wav";
			esl_execute(handle, "set", "node_state=1", strUUID.c_str());
		//	esl_execute(handle, "start_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", strUUID.c_str());
			//sleep(10);
			//esl_execute(handle, "stop_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", strUUID.c_str());
			playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
			//esl_execute(FSprocess::getSessionhandle(), "playback", "/root/txcall/play/2a.wav", strUUID.c_str());
			//playDetectSpeech("/root/txcall/play/1.wav", handle, strUUID.c_str());
			//playDetectSpeech("/root/txcall/play/2.wav", handle, strUUID.c_str());
			//playDetectSpeech("/root/txcall/play/3.wav", handle, strUUID.c_str());

		}

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
		map<uint32_t, base_script_t>::iterator iter = nodeMap.find(atoi(nodeState.c_str()));
		string keyword = iter->second.userWord;
			gIsAsr=0;

		if (keyword.empty())
		{
			esl_log(ESL_LOG_INFO, "Result is keyword :%s\n", keyword.c_str());
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
		string body = esl_event_get_body(event) ? esl_event_get_body(event) : "";
		if (body.length() < 2)
		{

			return;
		}

		esl_log(ESL_LOG_INFO, "body :%s\n", body.c_str());
		int pos = body.find("<speed>");

		esl_log(ESL_LOG_INFO, "body :%s pos==%d\n", body.c_str(), pos);

		if (pos > 0)
		{
			esl_execute(handle, "detect_speech", "stop", strUUID.c_str());
			string tmp = codeHelper::GetInstance()->getXmlInput(body);
			esl_log(ESL_LOG_INFO, "Result is park tmp :%s\n", tmp.c_str());

			esl_log(ESL_LOG_INFO, "detect node_state= %s\n", nodeState.c_str());

			map<uint32_t, base_script_t>::iterator iter = nodeMap.find(atoi(nodeState.c_str()));
			string keyword = iter->second.userWord;
			esl_log(ESL_LOG_INFO, "Result is keyword :%s\n", keyword.c_str());

			if (!keyword.empty())
			{
				// parse
				multimap<int, string> mapWord;
				codeHelper::GetInstance()->getKeyWord(mapWord, keyword);
				multimap<int, string>::iterator it;
				it = mapWord.begin();

				if (mapWord.empty())
				{
					esl_log(ESL_LOG_INFO, "flow game over \n", NULL);
					esl_execute(handle, "hangup", NULL, strUUID.c_str());
					return;
				}

				while (it != mapWord.end())
				{

					//??????????
					string tap = it->second;
					esl_log(ESL_LOG_INFO, "tap=%s \n", tap.c_str());
					char setVar[200];
					snprintf(setVar, sizeof setVar, "node_state=%d", it->first);

					//
					int pos = tap.find("肯定");
					int negPos = tap.find("否定");
					int s1 = tmp.find("什么");
					int s2 = tmp.find("没听清");
					int s3 = tmp.find("听不清");
					if (tmp.length() < 3 || s1 >= 0 || s2 >= 0 || s3 >= 0)
					{
						esl_log(ESL_LOG_INFO, "fpositive=44444444444\n", NULL);
						map<uint32_t, base_script_t>::iterator iter;
						iter = nodeMap.find(atoi(nodeState.c_str()));
						if (iter != nodeMap.end())
						{
							base_script_t node = iter->second;
							node.vox_base += ".wav";
							//playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
							esl_log(ESL_LOG_INFO, "never change state state beause user not listen  \n", "setVar");
							return;
						}
					}
					else if (pos >= 0)
					{
						string positive_prob = codeHelper::GetInstance()->sentiment_classifyRequesst(tmp);
						float fpositive = atof(positive_prob.c_str());
						esl_log(ESL_LOG_INFO, "fpositive=%f\n", fpositive);

						if (fpositive > 0.5)
						{
							esl_execute(handle, "set", setVar, strUUID.c_str());
							map<uint32_t, base_script_t>::iterator iter;
							iter = nodeMap.find(it->first);
							if (iter != nodeMap.end())
							{
								base_script_t node = iter->second;
								node.vox_base += ".wav";
								//playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
								esl_log(ESL_LOG_INFO, "find var node :%s\n", setVar);

								esl_execute(handle, "set", setVar, strUUID.c_str());
								return;
							}
						}
					}
					else if (negPos >= 0)
					{
						string positive_prob = codeHelper::GetInstance()->sentiment_classifyRequesst(tmp);
						float fpositive = atof(positive_prob.c_str());
						esl_log(ESL_LOG_INFO, "ng POS=====fpositive=%f\n", fpositive);

						if (fpositive < 0.5)
						{
							esl_execute(handle, "set", setVar, strUUID.c_str());
							map<uint32_t, base_script_t>::iterator iter;
							iter = nodeMap.find(it->first);
							if (iter != nodeMap.end())
							{
								base_script_t node = iter->second;
								node.vox_base += ".wav";
								//playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
								esl_log(ESL_LOG_INFO, "find var node :%s\n", setVar);

								esl_execute(handle, "set", setVar, strUUID.c_str());
								return;
							}
						}
					}
					it++;
					// vector<string> v(20);
					// codeHelper::GetInstance()->split(tap, v, ',');
					// for (int i = 0; i < v.size(); i++)
					// {
					// 	esl_log(ESL_LOG_INFO, "key =%s,tmp=:%s\n", v.at(i).c_str(), tmp.c_str());

					// 	if (tmp.find(v.at(i)) >= 0)
					// 	{
					// 		char setVar[200];
					// 		snprintf(setVar, sizeof setVar, "node_state=%d", it->first);

					map<uint32_t, base_script_t>::iterator iter;
					// 		iter = nodeMap.find(it->first);
					// 		if (iter != nodeMap.end())
					// 		{
					// 			base_script_t node = iter->second;
					// 			node.vox_base += ".wav";
					// 			::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
					// 			esl_log(ESL_LOG_INFO, "find var node :%s\n", setVar);

					// 			esl_execute(handle, "set", setVar, strUUID.c_str());
					// 			return;
					// 		}
					// 	}
					// }
				}
			}
			else
			{
				esl_execute(handle, "hangup", NULL, strUUID.c_str());
				esl_log(ESL_LOG_INFO, "hangup uuid= :%s\n", strUUID.c_str());
			}
		}

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
	esl_execute(FSprocess::getSessionhandle(), "playback", "/root/txcall/play/1.wav", uuid.c_str());
	//esl_execute(FSprocess::getSessionhandle(), "playback", playFile.c_str(), uuid.c_str());
	// esl_execute(handle, "detect_speech", "unimrcp:baidu-mrcp2 hello hello", uuid.c_str());
	esl_execute(FSprocess::getSessionhandle(), "start_asr", "LTAIRLpr2pJFjQbY oxrJhiBZB5zLX7LKYqETC8PC8ulwh0", uuid.c_str());
	//sleep(10);
	//esl_execute(handle, "stop_asr", NULL, uuid.c_str());

}

void FScall::run()
{
	GetnumbrList();
	LauchFScall();
}
int FScall::GetnumbrList()
{
	  db_operator_t::initDatabase();
	  db_operator_t::GetnumberList(m_NumberSet,m_taskID);
	  return 0;
}
int FScall::LauchFScall()
{
 	esl_handle_t handle = {{0}};
    esl_status_t status;
    char uuid[128]; //从fs??获得的uuid
    //Then running the Call_Task string when added a new Task,then remove it
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "??先生，这里是??众银行???催??心打来的，我姓张。提醒您??众银行微粒贷已经逾期，麻烦您尽快抽空处理??");

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "ClueCon");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }
	//string strcallMsg="bgapi originate user/"+m_callnumber+" &park()";
    esl_send_recv(&handle, "bgapi originate user/1003 &park()");
	//esl_send_recv(&handle, (const char*)strcallMsg.c_str());
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "您好！???问您是陈大文陈先生??");

    if (handle.last_sr_event && handle.last_sr_event->body)
    {
        printf("[%s]\n", handle.last_sr_event->body);
    }
    else
    {
        printf("[%s] last_sr_reply\n", handle.last_sr_reply);
    }
	return 0;
}
map<uint32_t, base_script_t> FSprocess::m_gKeymap;
map<string,FSsession*> FSprocess::m_SessionSet;
esl_handle_t* FSprocess::m_sessionHandle=NULL;
void FSprocess::run()
{
	Inbound_Init((void*)"userinfo");
}
FSsession* FSprocess::CreateSession(esl_handle_t *handle,esl_event_t *event,string strtaskID,string strscraftID,string strUUID,string caller_id,string destination_number)
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
void *FSprocess::Inbound_Init(void *arg)
{

    esl_handle_t handle = {{0}};
    esl_status_t status;
    const char *uuid;

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "ClueCon");
	//status = esl_connect(&handle, "210.21.48.69", 8021, NULL, "tx@infosun");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }
	m_sessionHandle=&handle;
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
            process_event(&handle, handle.last_ievent, m_gKeymap);
        }
    }
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


	esl_send_recv(&handle, "bgapi originate {speechCraftID=12344321,taskID=1111}user/1006 &park()");
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
				   const map<uint32_t, base_script_t> &keymap)
{
	char tmp_cmd[1024] = {0};
	string strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
	string caller_id = esl_event_get_header(event, "Caller-Caller-ID-Number") ? esl_event_get_header(event, "Caller-Caller-ID-Number") : "";
	string destination_number = esl_event_get_header(event, "Caller-Destination-Number") ? esl_event_get_header(event, "Caller-Destination-Number") : "";
	string strscraftID=esl_event_get_header(event, "Variable_speechCraftID") ? esl_event_get_header(event, "Variable_speechCraftID") : "";
	string strtaskID=esl_event_get_header(event, "Variable_taskID") ? esl_event_get_header(event, "Variable_taskID") : "";
	string tmpNodeState = esl_event_get_header(event, "Variable_node_state") ? esl_event_get_header(event, "Variable_node_state") : "";
	printf("strscraftID:%s\n",strscraftID.c_str());
// 	if(strUUID=="")
// 		return;
	printf("strUUID=%s,destination_number=%s,caller_id=%s,m_SessionSet.szie()=%d,event->event_id=%d\n",strUUID.c_str(),destination_number.c_str(),caller_id.c_str(),m_SessionSet.size(),event->event_id);
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
				esl_log(ESL_LOG_INFO, "Channel=%s\n", Channel.c_str());
				FSsession*psession = GetSessionbychannelid(Channel);
				if(psession!=NULL)
					psession->Action();
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
			FSsession*psession = CreateSession(handle,event,strtaskID,strscraftID,strUUID,caller_id,destination_number);
			m_SessionSet[strUUID]=psession;
			//esl_execute(handle, "playback", "/root/txcall/play/2a.wav", strUUID.c_str());
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
			map<string,FSsession*>::iterator ite=m_SessionSet.find(strUUID);
			if(ite!=m_SessionSet.end())
			{
				FSsession*psession=ite->second;
				m_SessionSet.erase(strUUID);
				if(psession!=NULL)
					delete psession;
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
