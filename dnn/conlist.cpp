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

void conlist::init(const char * srvip, unsigned int srvport, int srvtype)
{
    if(m_strip.compare("") == 0) //更改服务器时，释放原有资源，需要更新map中每个连接对象的信息
    {
        boost::mutex::scoped_lock lock(m_conmtx);
        for(map<string, boost::shared_ptr<connection>>::iterator it = m_mapcon.begin(); it != m_mapcon.end(); it++)
            it->second->update(srvip, srvport, srvtype);
    }
    m_strip = srvip;
    m_iport = srvport;
    m_itype = srvtype;
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
    if(*ncnt >= static_cast<int>(modellist.size()))
    {
        std::copy(&modellist[0], &modellist[0]+modellist.size(), versions);
        *ncnt = static_cast<int>(modellist.size());
        return;
    }
    *ncnt = 0;
    return;
}

void conlist::addconn(const char * devip, int chn, const char * modelname, pfnDnnCb pcb, void * puser)
{
    char id[256] = {0};
    sprintf(id, "%s_%d_%s", devip, chn, modelname);
    boost::mutex::scoped_lock lock(m_conmtx);
    if(m_mapcon.find(id) == m_mapcon.end())
    {
        boost::shared_ptr<connection> pconn(new connection(chn, m_strip, m_iport, m_itype, modelname, pcb, puser));
        m_mapcon.insert(make_pair(id, pconn));
    }
}

void conlist::predict(const char * devip, int chn, const char * modelname,
                      unsigned char * data, int w, int h, unsigned int imgid)
{
    char id[256] = {0};
    sprintf(id, "%s_%d_%s", devip, chn, modelname);
    boost::mutex::scoped_lock lock(m_conmtx);
    map<string, boost::shared_ptr<connection>>::iterator it = m_mapcon.find(id);
    if(it != m_mapcon.end())
        it->second->predict(data, w, h, modelname, imgid);
}

void conlist::rmconn(const char * devip, int chn, const char * modelname)
{
    char id[256] = {0};
    sprintf(id, "%s_%d_%s", devip, chn, modelname);
    boost::mutex::scoped_lock lock(m_conmtx);
    map<string, boost::shared_ptr<connection>>::iterator it = m_mapcon.find(id);
    if(it != m_mapcon.end())
        m_mapcon.erase(it);
}
