#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

#include <string>
#include <stdint.h>
using namespace std;

/**
 * @brief 流程描述
 *   detail description
 *
 */
struct base_script_t {
    // virtual destruct
    virtual ~base_script_t() {}
    uint32_t  voice_version_id;          ///
    uint32_t type;        ///<流程类型
    uint32_t nodeId;           ///当前结点ID
    string desc;          ///<流程描述
    uint32_t taskId; // 出口ID
    string userWord;      ///<比较关键�? 比�?�：关键�?1:出口1|关键�?2:出口2
    string vox_base;      ///<�?音文件根�?�?


    std::string bill_info();
};

typedef struct base_knowledge{
	uint32_t  voice_version_id;
	string desc; 
	string record;
	string keyword; //#�ָ����
	uint32_t taskId;
}base_knowledge_t;

#endif
