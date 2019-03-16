#ifndef __DBOPERATOR_H
#define __DBOPERATOR_H

#include <map>
#include <vector>
#include "structdef.h"
#include <iostream>
#include  <stdio.h>
using namespace std;

class db_operator_t {
public:
    static bool initDatabase();
    static bool SelectSql(map<uint32_t,base_script_t>& vSpeech,int32_t voiceVer);
	static bool SelectSqlAllSC(map<string,base_script_t>& vSpeech);
	static bool GetKnowledge(vector<base_knowledge_t>&knowledgelib);
	static bool GetnumberList(vector<string>&numberlist,string taskid);
	static bool InsertSessionRe(string insertsql);

};

#endif

