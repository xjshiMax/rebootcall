#include "DBOperator.h"
#include "../database/config/inirw.h"
#include "../database/dbPool.h"

bool db_operator_t::initDatabase()
{
    bool bRes = false;
    inirw *configRead = inirw::GetInstance("./database.conf");
    char servername[100] = {0};
    configRead->iniGetString("database", "servername", servername, sizeof servername, "0");
    char username[100] = {0};
    configRead->iniGetString("database", "username", username, sizeof username, "0");
    char password[100] = {0};
    configRead->iniGetString("database", "password", password, sizeof password, "0");


printf("%s\n",username);
    DBPool::GetInstance()->initPool(servername, username, password, 20);
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
        sprintf(query, "select * from ivr_node_flow_tbl where voice_version_id='%d'", voiceVer);

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
bool db_operator_t::GetnumberList(vector<string>&numberlist,string taskid)
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
		sprintf(query, "select * from call_cdr_tbl where task_id='%s'", taskid.c_str());
		//string query="select * from call_car_tbl where task_id='"
		result = state->executeQuery(query);
		string node;
		while (result->next())
		{
// 			node.voice_version_id = result->getInt("voice_version_id");
// 			node.type = result->getInt("type");
// 			node.nodeId = result->getInt("node_id");
// 			node.desc = result->getString("descript");
// 			node.userWord = result->getString("user_word");
// 			node.vox_base = result->getString("recordfile");
// 			node.taskId = result->getInt("task_id");
			node=result->getString("destination_number");
			numberlist.push_back(node);
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