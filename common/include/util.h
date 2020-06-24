#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>
#include "iniconfig.h"
#include "Log.h"
#include "json/json.h"
#include "boost/asio.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/fstream.hpp"
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <WinCon.h>
#else
#include <sys/resource.h>
#endif

using namespace std;

typedef struct
{
    unsigned short u16Year;
    unsigned short u16Month;
    unsigned short u16Day;
    unsigned short u16Hour;
    unsigned short u16Minute;
    unsigned short u16Second;
}DATETIME;

void GetDateTime(std::string & strTime);

void GetDateTime(DATETIME * pDateTime);

void rundaemon(bool bConsole);

void writelog(const char *logStr, LOG_TYPE type = TYPE_RUN);

int readIniCfg(const char * section, const char * key, char * outVal,
                const char * defaultVal, const char * iniFile);

void writeIniCfg(const char * section, const char * key,
                 const char * val, const char * iniFile);

void getfilelist(std::string path, std::vector<std::string>& fnames);

bool base64encode(const std::string& input, std::string * output);

bool base64decode(const std::string& input, std::string * output);

bool readimage(const char * imFileName, std::string * imageData);

bool writeimage(const char * imFileName, std::string& imageData);

std::string json2string(Json::Value& json);

bool string2json(const char * str, Json::Value * json);

string Utf8ToGbk(const char *src_str);

string GBKToUTF8(const char* strGBK);

/** < B上告警(智能分析客户端不转换) */
int ut2Bsub(int super, vector<int>& utsub);

/** < B下任务参数(智能分析客户端不转换) */
void B2utsub(int super, int bsub, vector<int>& utsub);

#endif
