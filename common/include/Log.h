#ifndef  __LOG_H__
#define  __LOG_H__

#include "boost/thread/mutex.hpp"
#include <vector>
#include <map>

enum LOG_TYPE
{
	TYPE_ALL   = 0,
	TYPE_START = 1,
	TYPE_CONN  = 2,
	TYPE_REQ   = 3,
	TYPE_RUN   = 4
};

class CLog
{
public:
    void WriteLog(LOG_TYPE type, std::string logStr);

    void getLogMsg(LOG_TYPE type, std::string start, std::string end, std::vector<std::string>& vecLog);

    static CLog * GetInstance()
    {
		boost::mutex::scoped_lock lock(m_mtx);
		static CLog * m_instance = new CLog();
        return m_instance;
    }

private:
    CLog();

    ~CLog()
	{
	}

    CLog(CLog const&);

    CLog& operator=(CLog const&);

	uintmax_t dirsize(std::string dir, std::vector<std::string>& fnames);

	std::string type2str(LOG_TYPE type);

private:
	static boost::mutex       m_mtx;
    boost::mutex              m_mutex;
};

#endif
