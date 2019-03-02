#include "process_event.h"
#include <stdio.h>
#include <cstdlib>
#include <iconv.h>
#include <iostream>

void playDetectSpeech(string playFile, esl_handle_t *handle, string uuid)
{
	// string param = playFile + " detect:unimrcp {start-input-timers=true,no-input-timeout=90000,recognition-timeout=90000}hello";

	// esl_execute(handle, "play_and_detect_speech", param.c_str(), uuid.c_str());
	// esl_execute(handle, "set", "play_and_detect_speech_close_asr=true", uuid.c_str());
	//  esl_execute(handle, "detect_speech", "stop", uuid.c_str());

	esl_execute(handle, "playback", playFile.c_str(), uuid.c_str());
	esl_execute(handle, "detect_speech", "unimrcp:baidu-mrcp2 hello hello", uuid.c_str());
	esl_execute(handle, "park", NULL, uuid.c_str());
}

string nodeState;

void process_event(esl_handle_t *handle,
				   esl_event_t *event,
				   const map<uint32_t, base_script_t> &keymap)
{
	string caller_id, agentId;
	string destination_number;
	string a_uuid;
	char tmp_cmd[1024] = {0};
	string strUUID;
	strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
	caller_id = esl_event_get_header(event, "Caller-Caller-ID-Number") ? esl_event_get_header(event, "Caller-Caller-ID-Number") : "";
	destination_number = esl_event_get_header(event, "Caller-Destination-Number") ? esl_event_get_header(event, "Caller-Destination-Number") : "";

	string tmpNodeState = esl_event_get_header(event, "Variable_node_state") ? esl_event_get_header(event, "Variable_node_state") : "";

	if (tmpNodeState.length() > 0)
	{
		nodeState = tmpNodeState;
		esl_log(ESL_LOG_INFO, "node_state= %s\n", nodeState.c_str());
	}

	string event_subclass, contact, from_user;
	map<uint32_t, base_script_t> nodeMap = keymap;
	switch (event->event_id)
	{

	case ESL_EVENT_CUSTOM:
	{
		event_subclass = esl_event_get_header(event, "Event-Subclass") ? esl_event_get_header(event, "Event-Subclass") : "";
		contact = esl_event_get_header(event, "contact") ? esl_event_get_header(event, "contact") : "";
		from_user = esl_event_get_header(event, "from-user") ? esl_event_get_header(event, "from-user") : "";

		if (event_subclass == "sofia::register")
		{
			//ע����Ϣ�����µ����ݿ�͵�ǰ�ķֻ��б���?
			esl_log(ESL_LOG_INFO, "sofia::register  %s, %d event_subclass=%s, contact=%s, from-user=%s\n", __FILE__, __LINE__, event_subclass.c_str(), contact.c_str(), from_user.c_str());
		}
		else if (event_subclass == "sofia::unregister")
		{
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
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
		const char *eventbody = esl_event_get_body(event);
		printf("body:\n%s\n", eventbody);
		esl_log(ESL_LOG_INFO, "dtmf :%s\n", dtmf.c_str());
		printf("ESL_EVENT_DTMF:inbound dtmf :%s\n", dtmf.c_str());

		break;
	}
	case ESL_EVENT_CHANNEL_ORIGINATE:
	{
		//������ʼ
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
		// sprintf(tmp_cmd, "api uuid_record %s start %s 9999 \n\n", strUUID.c_str(), strFullname.c_str());

		esl_send_recv_timed(handle, tmp_cmd, 1000);

		map<uint32_t, base_script_t>::iterator iter; //=keymap.find(1);
		iter = nodeMap.find(1);
		if (iter != nodeMap.end())
		{
			base_script_t node = iter->second;
			node.vox_base += ".wav";
			esl_execute(handle, "set", "node_state=1", strUUID.c_str());
			::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
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
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
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
		//������ͳ�Ʋ�ɾ����Ӧ�ĺ�����Ϣ
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
		{
			esl_log(ESL_LOG_INFO, "CALL OUT HANGUP_COMPLETE :%s\n", strUUID.c_str());
		}
		break;
	}

	case ESL_EVENT_CHANNEL_DESTROY:
	{
		//to do�˴�ͳһ����������֪ͨ���������ݿ�ĳ���ֶ��Ƿ�Ӧ��������
		string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
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
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
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
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
																																   /*const char *eventbody=esl_event_get_body(event);
			printf("body:\n%s\n",eventbody);*/
		break;
	}
	case ESL_EVENT_PLAYBACK_START:
	{

		string is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?

		//���ſ�ʼ
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_START %s\n", strUUID.c_str());
		}
		break;
	}
	case ESL_EVENT_PLAYBACK_STOP:
	{
		//������������
		//uuid = esl_event_get_header(event, "Caller-Unique-ID");
		//a_uuid = esl_event_get_header(event, "variable_a_leg_uuid");
		destination_number = esl_event_get_header(event, "Caller-Destination-Number");
		strUUID = esl_event_get_header(event, "Caller-Unique-ID") ? esl_event_get_header(event, "Caller-Unique-ID") : "";
		string is_callout, a_leg_uuid;
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
		{
			esl_log(ESL_LOG_INFO, "CALL IN ESL_EVENT_PLAYBACK_STOP %s\n", strUUID.c_str());
		}
		map<uint32_t, base_script_t>::iterator iter = nodeMap.find(atoi(nodeState.c_str()));
		string keyword = iter->second.userWord;

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
		is_callout = esl_event_get_header(event, "variable_is_callout") ? esl_event_get_header(event, "variable_is_callout") : ""; // ����Ϊ1�������������?
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
							::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
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
								::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
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
								::playDetectSpeech(node.vox_base.c_str(), handle, strUUID.c_str());
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
}
