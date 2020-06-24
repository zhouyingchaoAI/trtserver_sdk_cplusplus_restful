#include "utdnn.h"
#include "conlist.h"

void dnn_init(const char * srvip, unsigned int srvport, int srvtype)
{
    conlist::getInstance()->init(srvip, srvport, srvtype);
}

void  dnn_getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version)
{
    conlist::getInstance()->getstate(versions, ncnt, modelname, version);
}

void  dnn_addconn(const char * devip, int chn, const char * modelname)
{
    conlist::getInstance()->addconn(devip, chn, modelname);
}

void dnn_predict(const char *devip, int chn, const char *modelname, unsigned char *data, int w, int h, DNNTARGET *objs, int *size)
{
    conlist::getInstance()->predict(devip, chn, modelname, data, w, h, objs, size);
}


void dnn_rmconn(const char * devip, int chn, const char * modelname)
{
    conlist::getInstance()->rmconn(devip, chn, modelname);
}
