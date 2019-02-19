#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

#include <string>
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
    string userWord;      ///<比较关键词 比如：关键词1:出口1|关键词2:出口2
    string vox_base;      ///<语音文件根路径


    std::string bill_info();
};


#endif
