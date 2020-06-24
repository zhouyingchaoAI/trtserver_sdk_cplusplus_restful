#include "Log.h"
#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <fcntl.h>
#include "boost/date_time.hpp"

boost::mutex CLog::m_mtx;

bool cmpval(const std::pair< string, boost::gregorian::date> &p1, 
			const std::pair< string, boost::gregorian::date> &p2)
{
	return p1.second < p2.second;
}

CLog::CLog()
{
	if (!boost::filesystem::exists("./Log/"))
		boost::filesystem::create_directory("./Log/");
}

string CLog::type2str(LOG_TYPE type)
{
	switch (type)
	{
	case TYPE_ALL:
		return "All";
	case TYPE_START:
		return "Start";
	case TYPE_CONN:
		return "Conn";
	case TYPE_REQ:
		return "Req";
	case TYPE_RUN:
		return "Run";
	};
}
void CLog::WriteLog(LOG_TYPE type, std::string logStr)
{
	string strdate = logStr.substr(0, logStr.find(" "));
	while (strdate.find("-") != string::npos)
		strdate.replace(strdate.find("-"), 1, "");
	
	char LogPath[512] = { 0 };
    sprintf(LogPath, "./Log/%sLog%s.txt", type2str(type).c_str(), strdate.c_str());

	boost::mutex::scoped_lock lock(m_mutex);
	if (boost::filesystem::exists(LogPath) && boost::filesystem::file_size(LogPath) >= 10 * 1024 * 1024)
	{
		string strtime = logStr.substr(logStr.find(" ") + 1, logStr.find(".") - logStr.find(" "));
		while(strtime.find(":") != string::npos)
			strtime.replace(strtime.find(":"), 1, "");
		string bkpath = LogPath;
		bkpath.replace(bkpath.find_last_of("."), 1, strtime.c_str());
		boost::filesystem::rename(LogPath, bkpath);
	}
    
    boost::filesystem::fstream out(LogPath, std::ios_base::app);
    out << logStr << "\n";
    out.close();

	vector<string> fnames;
	getfilelist("./Log/", fnames);
	map<string, boost::gregorian::date> mapfiles;
	for (vector<string>::iterator it = fnames.begin(); it != fnames.end(); it++)
		mapfiles.insert(std::make_pair(*it, boost::gregorian::from_undelimited_string((*it).substr((*it).find("Log") + 3, 8))));
	vector<std::pair< string, boost::gregorian::date>> vecfiles(mapfiles.begin(), mapfiles.end());
	std::sort(vecfiles.begin(), vecfiles.end(), cmpval);

	boost::filesystem::space_info info = boost::filesystem::space("./Log/");
	if (info.available < 5 * 1024 * 1024 * 1024)
	{
		for (int i = 0; i < vecfiles.size() / 3; i++)
		{
			char file[128] = { '\0' };
			sprintf(file, "./Log/%s", vecfiles[i].first.c_str());
			boost::filesystem::remove(file);
		}
		return;
	}
	if (dirsize("./Log/", fnames) > 1024 * 1024 * 1024)
	{
		uintmax_t nbytes = 0;
		for (int i = 0; i < vecfiles.size(); i++)
		{
			char file[128] = { '\0' };
			sprintf(file, "./Log/%s", vecfiles[i].first.c_str());
			nbytes += boost::filesystem::file_size(file);
			boost::filesystem::remove(file);
			if (nbytes >= 512 * 1024 * 1024)
				break;
		}
	}
}

uintmax_t CLog::dirsize(string dir, vector<string>& fnames)
{
	uintmax_t sz = 0;
	for (std::vector<std::string>::iterator it = fnames.begin();
		it != fnames.end(); it++)
	{
		char file[128] = { '\0' };
		sprintf(file, "./Log/%s", (*it).c_str());
		sz += boost::filesystem::file_size(file);
	}
	return sz;
}

void CLog::getLogMsg(LOG_TYPE type, std::string start, std::string end, std::vector<std::string> &vecLog)
{
	boost::posix_time::ptime tmstart(boost::posix_time::time_from_string(start));
	boost::posix_time::ptime tmend(boost::posix_time::time_from_string(end));

    boost::mutex::scoped_lock lock(m_mutex);
	vector<string> fnames;
	getfilelist("./Log/", fnames);

	string strtype = type2str(type);
	map< boost::posix_time::ptime, string> maplog;
    for(std::vector<std::string>::iterator it = fnames.begin();
        it != fnames.end(); it++)
    {
		if (type != TYPE_ALL && (*it).find(strtype) == string::npos)
			continue;
		boost::gregorian::date logday = boost::gregorian::from_undelimited_string((*it).substr((*it).find("Log")+3, 8));
        if(logday < tmstart.date() || logday > tmend.date())
            continue;

        char file[128] = {'\0'};
        sprintf(file, "./Log/%s", (*it).c_str());
        std::ifstream stream;
        stream.open(file);
        if(!stream.is_open())
            continue;

        while(!stream.eof())
        {
            char strline[2048] = {"\0"};
            stream.getline(strline, 2048);

			string logtime = strline;
			if (logtime.compare("") == 0)
				continue;
			boost::posix_time::ptime logtm(boost::posix_time::time_from_string(logtime.substr(0, 26)));
			if (logtm >= tmstart && logtm <= tmend)
			{
				while (maplog.find(logtm) != maplog.end())
					logtm += boost::posix_time::microsec(1);
				maplog.insert(make_pair(logtm, strline));
			}
        }
        stream.close();
    }

	vecLog.clear();
	for(map< boost::posix_time::ptime, string>::iterator it = maplog.begin(); it != maplog.end(); it++)
		vecLog.push_back(GBKToUTF8(it->second.c_str()));
}
