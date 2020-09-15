#include "conlist.h"

boost::mutex conlist::m_mtx;

conlist::conlist()
{
    m_strip = "";
}

conlist::~conlist()
{
    m_strip = "";
}

void conlist::init(void * pdnn, const char * srvip, unsigned int srvport)
{
    if(pdnn != NULL && m_strip.compare(srvip) != 0) //更改服务器时，释放原有资源，需要更新map中每个连接对象的信息
        ((connection*)pdnn)->update(srvip, srvport);
    m_strip = srvip;
    m_iport = srvport;
}

void conlist::getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version)
{
    char url[256] = {0};
    sprintf(url, "http://%s:%d/api/status/%s?format=json",m_strip.c_str(),m_iport,modelname);
    vector<MODELINFO> modellist;
    modellist.clear();
    connection::getstate(m_strip, m_iport, url, modelname, modellist);
    if(version != -1)
    {
        for (auto model:modellist)
        {
            if(model.version == version)
            {
                std::copy(&model, &model+1, versions);
                *ncnt = 1;
                return;
            }
        }
        return;
    }
    int nnn = *ncnt;
    if(*ncnt >= static_cast<int>(modellist.size()))
    {
        std::copy(&modellist[0], &modellist[0]+modellist.size(), versions);
        *ncnt = static_cast<int>(modellist.size());
        return;
    }
    *ncnt = 0;
    return;
}

void * conlist::addconn(const char * modelname)
{
    return new connection(m_strip, m_iport, modelname);
}

void conlist::rmconn(void * pdnn)
{
	if (pdnn != NULL)
		delete (connection*)pdnn;
}
