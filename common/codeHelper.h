
#include <iostream>
#include <sstream>
#include "../esl/esl_json.h"
#include "speech/common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <map>

using namespace std;

struct tts_config
{
  char api_key[40];       // �?写网页上申�?�的appkey �? $apiKey="g8eBUMSokVB1BHGmgxxxxxx"
  char secret_key[40];    // �?写网页上申�?�的APP SECRET �? $secretKey="94dc99566550d87f8fa8ece112xxxxx"
  char text[512 * 3 + 1]; // 需要合成的文本  最�?512�?汉字
  int text_len;           // 文本的长�?
  char cuid[20];
  int spd;
  int pit;
  int vol;
  int per;
  int aue;
  char format[4];
};

class codeHelper
{
private:
  //构造函数�?�有�?
  codeHelper()
  {
  }
  static codeHelper *m_pInstance;
  class CGarbo //它的�?一工作就是在析构函数中删除CSingleton的实�?
  {
  public:
    ~CGarbo()
    {
      if (codeHelper::m_pInstance)
        delete codeHelper::m_pInstance;
    }
  };

  unsigned char ToHex(unsigned char x);
  unsigned char FromHex(unsigned char x);
  string notYinghao(const char *str);
  string createRequestEntity(const string &phone, const string &status, const string &type, const string &content, const string &id, const string &recordId);
  string createMosRequestEntity(const string &phone,
                                const string &batchName,
                                const string &bizType,
                                const string &type,
                                const string &content);

  string createSimnetBody(const string &text1, const string &text2);

  RETURN_CODE fill_config(struct tts_config *config, const char *txt);

  RETURN_CODE run_tts(struct tts_config *config, const char *token, const char *fileName);
  int UTF2Uni(const char *src, std::wstring &t);
  std::string ws2s(const std::wstring &ws);


  int  UnicodeToUTF_8(unsigned long * InPutStr, int InPutStrLen,  char *OutPutStr);




  /**
 * 用以获取access_token的函数，使用时需要先在百度云控制台申请相应功能的应用，获得�?�应的API Key和Secret Key
 * @param access_token 获取得到的access token，调用函数时需传入该参�?
 * @param AK 应用的API key
 * @param SK 应用的Secret key
 * @return 返回0代表获取access token成功，其他返回值代表获取失�?
 */
  int get_access_token(std::string &access_token, const std::string &AK, const std::string &SK);

  static CGarbo Garbo; //定义一�?静态成员变量，程序结束时，系统会自动调用它的析构函�?
public:
  static codeHelper *GetInstance(); //获取实例
  std::string UrlDecode(const std::string &str);
  string emsCallbackRequest(const string &phone, const string &state,
                            const string &type, const string &content, const string &recordId, const string &order_id);
  string mosCallbackRequest(const string &phone,
                            const string &batchName,
                            const string &bizType,
                            const string &type,
                            const string &content);
string sentiment_classifyRequesst(const string &text);
  RETURN_CODE run(const char *fileName, const char *txt);
  char *simnet(const char *text1, const char *text2);

  string getXmlInput(const string &xmlStr);
  void getKeyWord(multimap<int, string> &keyWord, const string &word);
  void split(const string &s, vector<string> &sv, const char flag = ' ');
string getAliAsrTxt(const string& json);
};
