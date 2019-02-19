#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <pthread.h>
#include "process_event.h"
#include "common/structdef.h"
#include "common/DBOperator.h"
// #include "common/codeHelper.h"

using namespace std;
pthread_mutex_t agentMutex;     //即时呼叫的互斥变量
pthread_mutex_t infoMutex;      //用于处理callinfo的互斥变量
pthread_mutex_t calloutMutex;   // 外呼任务的互斥变量
pthread_mutex_t configMutex;    //基本配置模块的互斥变量
pthread_mutex_t clickDialMutex; //点击呼叫模块的互斥变量

map<uint32_t, base_script_t> gKeymap;

void *Inbound_Init(void *arg)
{

    esl_handle_t handle = {{0}};
    esl_status_t status;
    const char *uuid;

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "tx@infosun");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }
    esl_log(ESL_LOG_INFO, "Connected to FreeSWITCH\n");
    esl_events(&handle, ESL_EVENT_TYPE_PLAIN,
               "DETECTED_SPEECH RECORD_START RECORD_STOP PLAYBACK_START PLAYBACK_STOP CHANNEL_OUTGOING CHANNEL_PARK CHANNEL_EXECUTE_COMPLETE CHANNEL_ORIGINATE TALK NOTALK PHONE_FEATURE CHANNEL_HANGUP_COMPLETE CHANNEL_CREATE CHANNEL_BRIDGE DTMF CHANNEL_DESTROY CHANNEL_HANGUP CHANNEL_BRIDGE CHANNEL_ANSWER CUSTOM sofia::register sofia::unregister");
    esl_log(ESL_LOG_INFO, "%s\n", handle.last_sr_reply);

    handle.event_lock = 1;
    while ((status = esl_recv_event(&handle, 1, NULL)) == ESL_SUCCESS)
    {
        if (handle.last_ievent)
        {
            process_event(&handle, handle.last_ievent, gKeymap);
        }
    }

end:

    esl_disconnect(&handle);

    return (void *)0;
}

//外呼处理
void *CallOut_Task_Process(void *arg)
{

    esl_handle_t handle = {{0}};
    esl_status_t status;
    char uuid[128]; //从fs中获得的uuid
    //Then running the Call_Task string when added a new Task,then remove it

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "tx@infosun");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }


    //为了cps保证30以内，需要每路延时30ms
#ifdef WIN32
    Sleep(30);
#else
    //usleep(3000);
    struct timeval tempval;
    tempval.tv_sec = 0;
    tempval.tv_usec = 30;
    select(0, NULL, NULL, NULL, &tempval);
#endif

    esl_disconnect(&handle);

    return (void *)0;
}

void *test_Process(void *arg)
{
    esl_handle_t handle = {{0}};
    esl_status_t status;
    char uuid[128]; //从fs中获得的uuid
    //Then running the Call_Task string when added a new Task,then remove it
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "马先生，这里是微众银行委催中心打来的，我姓张。提醒您微众银行微粒贷已经逾期，麻烦您尽快抽空处理。");

    esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

    status = esl_connect(&handle, "127.0.0.1", 8021, NULL, "tx@infosun");

    if (status != ESL_SUCCESS)
    {
        esl_log(ESL_LOG_INFO, "Connect Error: %d\n", status);
        exit(1);
    }

    esl_send_recv(&handle, "bgapi originate user/1003 &park()");
    // codeHelper::GetInstance()->run("/root/txcall/tts/testcc", "您好！请问您是陈大文陈先生吗");

    if (handle.last_sr_event && handle.last_sr_event->body)
    {
        printf("[%s]\n", handle.last_sr_event->body);
    }
    else
    {
        printf("[%s] last_sr_reply\n", handle.last_sr_reply);
    }
}


int main(int argc, char const *argv[])
{
    bool bSuccess = false;
    pthread_mutex_init(&agentMutex, NULL);
    pthread_mutex_init(&infoMutex, NULL);
    pthread_mutex_init(&calloutMutex, NULL);
    pthread_mutex_init(&configMutex, NULL);
    pthread_mutex_init(&clickDialMutex, NULL);

    db_operator_t::initDatabase();
    db_operator_t::SelectSql(gKeymap, 1);
    
    map<uint32_t, base_script_t>::iterator strmap_iter = gKeymap.begin();
    for (; strmap_iter != gKeymap.end(); strmap_iter++)
    {
        // cout << strmap_iter->first << ' ' << strmap_iter->second << endl;
        base_script_t node=strmap_iter->second;
        // codeHelper::GetInstance()->run(node.vox_base.c_str(), node.desc.c_str());
        printf("node==%d,descript=%s\n", strmap_iter->first,node.desc.c_str());

    }

string str="肯定";
int n=str.find("肯定");
printf("%d\n",n);


    int ret = 0;
    pthread_t pthid1, pthid2, pthid3, pthid4, pthid5;

    ret = pthread_create(&pthid1, NULL, Inbound_Init, NULL);
    if (ret) // 非0则创建失败
    {
        perror("createthread 1 failed.\n");
        return 1;
    }
    ret = pthread_create(&pthid2, NULL, CallOut_Task_Process, NULL);
    if (ret) // 非0则创建失败
    {
        perror("createthread 3 failed.\n");
        return 1;
    }

    ret = pthread_create(&pthid3, NULL, test_Process, NULL);
    if (ret) // 非0则创建失败
    {
        perror("createthread 3 failed.\n");
        return 1;
    }

    pthread_join(pthid1, NULL);
    pthread_join(pthid2, NULL);
    pthread_join(pthid3, NULL);
    pthread_join(pthid4, NULL);

    pthread_mutex_destroy(&infoMutex);
    pthread_mutex_destroy(&agentMutex);
    pthread_mutex_destroy(&calloutMutex);
    pthread_mutex_destroy(&configMutex);
    pthread_mutex_destroy(&clickDialMutex);

    return 0;
}
