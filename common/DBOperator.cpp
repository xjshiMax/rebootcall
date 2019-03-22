#include "DBOperator.h"
#include "../database/config/inirw.h"
#include "../database/dbPool.h"
#include "../base/inifile/inifile.h"
#include "../base/include/xAutoLock.h"
using namespace inifile;
using namespace SAEBASE;
Mutex _databaselock;
string db_operator_t::m_ivr_node_flow_tbl="ivr_node_flow_tbl";
string db_operator_t::m_knowledge_base_tbl="knowledge_base_tbl";
string db_operator_t::m_ai_callout_tbl="ai_callout_tbl";
string db_operator_t::m_call_cdr_tbl="call_cdr_tbl";
bool db_operator_t::initDatabase()
{
	//bool bRes = false;
	//inirw *configRead = inirw::GetInstance("./database.conf");
	//char servername[100] = {0};
	//configRead->iniGetString("database", "servername", servername, sizeof servername, "0");
	//char username[100] = {0};
	//configRead->iniGetString("database", "username", username, sizeof username, "0");
	//char password[100] = {0};
	//configRead->iniGetString("database", "password", password, sizeof password, "0");
	IniFile IniService;
	IniService.load("database.conf");
	int iret=-1;
	string servername=IniService.getStringValue("database","servername",iret);
	string username=IniService.getStringValue("database","username",iret);
	string password=IniService.getStringValue("database","password",iret);
	m_ivr_node_flow_tbl=IniService.getStringValue("datatable","ivr_node_flow_tbl",iret);
	m_knowledge_base_tbl=IniService.getStringValue("datatable","knowledge_base_tbl",iret);
	m_ai_callout_tbl=IniService.getStringValue("datatable","ai_callout_tbl",iret);
	m_call_cdr_tbl=IniService.getStringValue("datatable","call_cdr_tbl",iret);
    DBPool::GetInstance()->initPool(servername.c_str(), username.c_str(), password.c_str(), 20);
    return true;

}

bool db_operator_t::SelectSql(map<uint32_t,base_script_t>& vSpeech,int32_t voiceVer)
{
    int nSuccess = 0;

    Statement *state;
    Connection *cmd;
    ResultSet *result;
    try
    {
        cmd = DBPool::GetInstance()->GetConnection();
        if (cmd == NULL)
        {
            printf("Connection *cmd = dbIn==NULL....\n");
        }

        state = cmd->createStatement();
        state->execute("use txacall");


        char query[256] = {0};
        sprintf(query, "select * from %s where voice_version_id='%d'",m_ivr_node_flow_tbl.c_str(), voiceVer);

        result = state->executeQuery(query);
        base_script_t node;
        while (result->next())
        {
            node.voice_version_id = result->getInt("voice_version_id");
            node.type = result->getInt("type");
            node.nodeId = result->getInt("node_id");
            node.desc = result->getString("descript");
            node.userWord = result->getString("user_word");
            node.vox_base = result->getString("recordfile");
            node.taskId = result->getInt("task_id");
             vSpeech.insert(pair<uint32_t, base_script_t>(node.nodeId, node));
        }
    }
    catch (sql::SQLException &ex)
    {
        printf("SelectSql error:%s\n", ex.what());
        nSuccess = -1;
    }
    delete result;
    delete state;
    DBPool::GetInstance()->ReleaseConnection(cmd);

    return nSuccess;
}
//读取全部的话术
bool db_operator_t::SelectSqlAllSC(map<string,base_script_t>& vSpeech)
{
	int nSuccess = 0;

	Statement *state;
	Connection *cmd;
	ResultSet *result;
	try
	{
		cmd = DBPool::GetInstance()->GetConnection();
		if (cmd == NULL)
		{
			printf("Connection *cmd = dbIn==NULL....\n");
		}

		state = cmd->createStatement();
		state->execute("use txacall");


		char query[256] = {0};
		sprintf(query, "select * from %s ",m_ivr_node_flow_tbl.c_str());

		result = state->executeQuery(query);
		base_script_t node;
		while (result->next())
		{
			node.voice_version_id = result->getInt("voice_version_id");
			node.type = result->getInt("type");
			node.nodeId = result->getInt("node_id");
			node.desc = result->getString("descript");
			node.userWord = result->getString("user_word");
			node.vox_base = result->getString("recordfile");
			node.taskId = result->getInt("task_id");
			char strnodeID[32];
			sprintf(strnodeID,"%d_%d",node.voice_version_id,node.nodeId);
			vSpeech.insert(pair<string, base_script_t>(strnodeID, node));
		}
	}
	catch (sql::SQLException &ex)
	{
		printf("SelectSql error:%s\n", ex.what());
		nSuccess = -1;
	}
	delete result;
	delete state;
	DBPool::GetInstance()->ReleaseConnection(cmd);

	return nSuccess;
}
bool db_operator_t::GetKnowledge(vector<base_knowledge_t>&knowledgelib)
{
	int nSuccess = 0;

	Statement *state=NULL;
	Connection *cmd;
	ResultSet *result=NULL;
	try
	{
		cmd = DBPool::GetInstance()->GetConnection();
		if (cmd == NULL)
		{
			printf("Connection *cmd = dbIn==NULL....\n");
		}

		state = cmd->createStatement();
		state->execute("use txacall");


		char query[256] = {0};
		sprintf(query, "select * from %s ",m_knowledge_base_tbl.c_str());

		result = state->executeQuery(query);
		base_knowledge_t node;
		while (result->next())
		{
			node.voice_version_id = result->getInt("voice_version_id");
			//node.type = result->getInt("type");
			//node.nodeId = result->getInt("node_id");
			node.desc = result->getString("desp");
			node.keyword = result->getString("keyword");
			node.record = result->getString("record");
			//node.taskId = result->getInt("task_id");
			knowledgelib.push_back(node);
		}
	}
	catch (sql::SQLException &ex)
	{
		printf("SelectSql error:%s\n", ex.what());
		nSuccess = -1;
	}
	if(result)
		delete result;
	if(state)
		delete state;
	DBPool::GetInstance()->ReleaseConnection(cmd);

	return nSuccess;
}
bool db_operator_t::GetnumberList(vector<t_Userinfo>&numberlist,string taskid)
{
	int nSuccess = 0;
	Statement *state=NULL;
	Connection *cmd;
	ResultSet *result=NULL;
	try
	{
		cmd = DBPool::GetInstance()->GetConnection();
		if (cmd == NULL)
		{
			printf("Connection *cmd = dbIn==NULL....\n");
		}

		state = cmd->createStatement();
		state->execute("use txacall");


		char query[256] = {0};
		sprintf(query, "select * from %s where task_id='%s'",m_ai_callout_tbl.c_str(), taskid.c_str());
		//string query="select * from call_car_tbl where task_id='"
		result = state->executeQuery(query);
		string node;
		//t_Userinfo userinfo;
		while (result->next())
		{
			t_Userinfo userinfo;
			userinfo.phonenum=result->getString("phone");
			userinfo.username=result->getString("name");
			numberlist.push_back(userinfo);
		}
	}
	catch (sql::SQLException &ex)
	{
		printf("SelectSql error:%s\n", ex.what());
		nSuccess = -1;
	}
	if(result)
		delete result;
	if(state)
		delete state;
	DBPool::GetInstance()->ReleaseConnection(cmd);

	return nSuccess;
}
bool db_operator_t::InsertSessionRe(string insertsql)
{
	xAutoLock L(_databaselock);
	int nSuccess = 0;
	Statement *state=NULL;
	Connection *cmd=NULL;
	char*p_query=NULL;
	ResultSet *result=NULL;
	try
	{
		cmd = DBPool::GetInstance()->GetConnection();
		if (cmd == NULL)
		{
			printf("Connection *cmd = dbIn==NULL....\n");
		}

		state = cmd->createStatement();
		state->execute("use txacall");
		p_query=new char[insertsql.length()+1];
		memset(p_query,0,insertsql.length()+1);

		//char query[512] = {0};
		sprintf(p_query,"%s",insertsql.c_str());
		result = state->executeQuery(p_query);
		if(p_query)
		{
			delete [] p_query;
			p_query=NULL;
		}
		//result = state->executeQuery("Insert into call_cdr_tbl (inbound_talk_times, caller_id_number, destination_number, start_stamp, end_stamp, duration, recording_file, task_name, outbound_label, task_id, created_at, updated_at)values (0,'0000000000','1006','2019-03-07 11:56:32','2019-03-07 11:56:34',2,'/home/records/2019-03-07/00000000001551930992.wav','banksale','A',1111,1551930994,0)");
// 		string node;
// 		while (result->next())
// 		{
// 			//node=result->getString("destination_number");
// 			//numberlist.push_back(node);
// 		}
	}
	catch (sql::SQLException &ex)
	{
		if(p_query)
		{
			delete [] p_query;
			p_query=NULL;
		}
			printf("SelectSql error:%s\n", ex.what());
		nSuccess = -1;
	}
	if(result)
		delete result;
	if(state)
		delete state;
	DBPool::GetInstance()->ReleaseConnection(cmd);

	return nSuccess;
}