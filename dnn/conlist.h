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

    void init(void * pdnn, const char * srvip, unsigned int srvport);

    void  getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version = -1);

    void * addconn(const char * modelname);

    void rmconn(void * pdnn);

private:
    conlist();

    conlist(conlist const&);

    conlist& operator=(conlist const&);

    ~conlist();

private:
    static boost::mutex  m_mtx;
    string               m_strip;
    unsigned int         m_iport;
};

#endif // __CONLIST_H__
