#include "util.h"
#include "time.h"
#include <stdio.h>
#include "boost/date_time.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/binary_from_base64.hpp"
#include "boost/archive/iterators/transform_width.hpp"
#include <fstream>
#include "typedef.h"

boost::mutex g_iniMtx;
boost::mutex g_imgMtx;

void GetDateTime(std::string & strTime)
{
    time_t tmNow;
    struct tm * ptmNow;

    time(&tmNow);
    ptmNow = localtime(&tmNow);

    char s8Time[64] = {0};
    sprintf(s8Time, "%04d-%02d-%02d %02d:%02d:%02d", ptmNow->tm_year+1900, ptmNow->tm_mon+1,
                    ptmNow->tm_mday, ptmNow->tm_hour, ptmNow->tm_min, ptmNow->tm_sec);

    strTime = s8Time;
}

void GetDateTime(DATETIME * pDateTime)
{
    time_t tmNow;
    struct tm * ptmNow;

    time(&tmNow);
    ptmNow = localtime(&tmNow);

    pDateTime->u16Year   = ptmNow->tm_year + 1900;
    pDateTime->u16Month  = ptmNow->tm_mon + 1;
    pDateTime->u16Day    = ptmNow->tm_mday;
    pDateTime->u16Hour   = ptmNow->tm_hour;
    pDateTime->u16Minute = ptmNow->tm_min;
    pDateTime->u16Second = ptmNow->tm_sec;
}

void writelog(const char *logStr, LOG_TYPE type)
{
	string str = to_iso_extended_string(boost::posix_time::microsec_clock::local_time());
	str.replace(str.find("T"), 1, " ");
	char msg[1024] = { 0 };
	sprintf(msg, "%s %s", str.c_str(), logStr);
	printf("%s\n", msg);
    CLog::GetInstance()->WriteLog(type, msg);
}

void writeIniCfg(const char * section, const char * key,
                 const char * val, const char * iniFile)
{
    boost::mutex::scoped_lock lock(g_iniMtx);
    if(!boost::filesystem::exists(iniFile))
    {
        FILE * f = fopen(iniFile, "a");
        if(NULL == f)
            return;
        fclose(f);
    }

    char attr[128] = {0};
    sprintf(attr, "%s.%s", section, key);

    using namespace boost::property_tree;
    ptree tree;
    ini_parser::read_ini(iniFile, tree);
    tree.put<std::string>(attr, val);
    ini_parser::write_ini(iniFile, tree);
}

int readIniCfg(const char * section, const char * key, char * outVal,
                const char * defaultVal, const char * iniFile)
{
    boost::mutex::scoped_lock lock(g_iniMtx);
    if(!boost::filesystem::exists(iniFile))
    {
        FILE * f = fopen(iniFile, "a");
        if(NULL == f)
            return 0;
        fclose(f);
    }

    char attr[128] = {0};
    sprintf(attr, "%s.%s", section, key);

    using namespace boost::property_tree;
    ptree tree;
    ini_parser::read_ini(iniFile, tree);

    if(tree.find(section) != tree.not_found()
         && tree.get_child(section).find(key) != tree.get_child(section).not_found())
    {
        strcpy(outVal, tree.get<std::string>(attr).c_str());
        return strlen(outVal);
    }
    else if(defaultVal != NULL)
    {
        strcpy(outVal, defaultVal);
        tree.put<std::string>(attr, outVal);
        ini_parser::write_ini(iniFile, tree);
        return strlen(outVal);
    }
    return 0;
}

bool base64encode(const std::string& input, std::string * output)
{
    typedef boost::archive::iterators::base64_from_binary
            <boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8> >
            Base64EncodeIterator;

    std::stringstream result;
    std::copy(Base64EncodeIterator(input.begin()) , Base64EncodeIterator(input.end()),
         std::ostream_iterator<char>(result));

    size_t equal_count = (3 - input.length() % 3) % 3;
    for (size_t i = 0; i < equal_count; i++) {
        result.put('=');
    }

    *output = result.str();
    return output->empty() == false;
}

bool base64decode(const std::string& input, std::string * output)
{
    typedef boost::archive::iterators::transform_width
              <boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6>
              Base64DecodeIterator;
    std::stringstream result;
    try {
        std::copy(Base64DecodeIterator(input.begin()) , Base64DecodeIterator(input.end()),
                std::ostream_iterator<char>(result));
    } catch(...) {
      return false;
    }
    *output = result.str();
    return output->empty() == false;
}

bool readimage(const char * imFileName, std::string * imageData)
{
    boost::mutex::scoped_lock lock(g_imgMtx);
    std::ifstream file(imFileName, std::ios::binary);
    if(!file.is_open())
        return false;

    std::vector<char> data;
    file >> std::noskipws;
    std::copy(std::istream_iterator<char>(file),
              std::istream_iterator<char>(), std::back_inserter(data));
    file.close();

    *imageData = std::string(data.begin(), data.end());
    return true;
}

bool writeimage(const char * imFileName, std::string& imageData)
{
    boost::mutex::scoped_lock lock(g_imgMtx);
    FILE * out = std::fopen(imFileName, "wb");
    if(out == NULL)
        return false;

    std::fwrite(imageData.c_str(), 1, imageData.size(), out);
    std::fclose(out);
    return true;
}

std::string json2string(Json::Value &json)
{
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ostringstream os;
    writer->write(json, &os);
    return os.str();
}

bool string2json(const char * str, Json::Value * json)
{
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());

    std::string error;
    if(!reader->parse(str, str+strlen(str), json, &error))
        return false;
    return true;
}

void getfilelist(const std::string dir, std::vector<std::string>& fnames)
{
    boost::filesystem::path path(dir);
    if (!boost::filesystem::exists(path))
        return;
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(path); iter!=end_iter; ++iter)
    {
        if (boost::filesystem::is_regular_file(iter->status()))
        {
            fnames.push_back(iter->path().filename().string());
        }

        if (boost::filesystem::is_directory(iter->status()))
        {
            getfilelist(iter->path().filename().string(),fnames);
        }
    }
}

string Utf8ToGbk(const char *src_str)
{
#ifdef _WIN32
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
#else
    return src_str;
#endif
}

string GBKToUTF8(const char* strGBK)
{
#ifdef _WIN32
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
#else
	return strGBK;
#endif
}

int ut2Bsub(int super, vector<int>& utsub)
{
    if(super == SUPER_UTIVA)
        return 0;
    int bsub = 0;
    for(int i = 0; i < utsub.size(); i++)
        bsub += (1<<utsub[i]);
    return bsub;
}

void B2utsub(int super, int bsub, vector<int>& utsub)
{
    utsub.clear();
    int bits = 0;
    switch(super)
    {
    case SUPER_WORKING:
        bits = 3;
        break;
    case SUPER_ACTIVITY:
        bits = 6;
        break;
    case SUPER_CLASSIFY:
        bits = 4;
        break;
    case SUPER_ITEMLEFT:
    case SUPER_INVADE:
    case SUPER_VECHILE:
    case SUPER_METER:
        bits = 2;
        break;
    case SUPER_FIRE:
    case SUPER_TEMP_VAL:
    case SUPER_TEMP_DIFF:
    case SUPER_TEMP_CHANGE:
    case SUPER_CONNECTOR:
    case SUPER_SWITCH:
        bits = 1;
        break;
    case SUPER_APPEARANCE:
        bits = 8;
        break;
    case SUPER_VQLTY:
        bits = 13;
        break;
    default:
        return;
    }
    for(int i = 0; i < bits; i++)
    {
        if((1<<i)&bsub)
        {
            utsub.push_back(i);
            bsub -= (1<<i);
        }
    }
    if(bsub > 0)
        utsub.clear();
}

void rundaemon(bool bConsole)
{
#ifdef _WIN32
    if(!bConsole)
        FreeConsole();
#else
    pid_t pid;
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);

    pid = fork();
    if(pid == -1)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    if(pid > 0)
    {
        printf("parent process exit\n");
        _exit(EXIT_SUCCESS);
    }
    printf("child process start\n");
    if(setsid() == -1)
    {
        perror("setsid error");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD,SIG_IGN);
    for(int i = 3; i < (rl.rlim_max == RLIM_INFINITY ? 1024 : rl.rlim_max); i++)
        close(i);

    /*
    int fd0, fd1, fd2;
    dup2(0, fd0);
    dup2(1, fd1);
    dup2(2, fd2);
    for(int i = 0; i < 3; i++)
        close(i);
    if(!bConsole)
    {
        printf("close output...\n");
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }
    else
    {
        dup2(fd0, 0);
        dup2(fd1, 1);
        dup2(fd2, 2);
        printf("recover output...\n");
    }
    */
    umask(0);
#endif
}
