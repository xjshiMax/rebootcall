#ifndef __DBOPERATOR_H
#define __DBOPERATOR_H

#include <map>
#include "structdef.h"

using namespace std;

class db_operator_t {
public:
    static bool initDatabase();
    static bool SelectSql(map<uint32_t,base_script_t>& vSpeech,int32_t voiceVer);
};

#endif

