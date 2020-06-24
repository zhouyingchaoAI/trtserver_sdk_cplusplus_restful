#ifndef __CONLIST_H__
#define __CONLIST_H__

#include "connection.h"

class conlist
{
public:
    static conlist * getInstance()
    {
        boost::mutex::scoped_lock lock(m_mtx);
        static conlist instance;
        return &instance;
    }

    void init(const char * srvip, unsigned int srvport, int srvtype);

    void  getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version = -1);

    void addconn(const char * devip, int chn, const char * modelname);

    void predict(const char * devip, int chn, const char * modelname,
                 unsigned char * data, int w, int h, unsigned int imgid);
    void predict(const char * devip, int chn, const char * modelname,
                 unsigned char * data, int w, int h, DNNTARGET * objs, int *size);

    void rmconn(const char * devip, int chn, const char * modelname);

private:
    conlist();

    conlist(conlist const&);

    conlist& operator=(conlist const&);

    ~conlist();

private:
    static boost::mutex  m_mtx;
    string               m_strip;
    unsigned int         m_iport;
    int                  m_itype;

    boost::mutex         m_conmtx;
    map<string, boost::shared_ptr<connection>> m_mapcon;
};

#endif // __CONLIST_H__
