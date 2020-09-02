#include "utdnn.h"
#include "conlist.h"
#include "connection.h"

void dnn_init(void * pdnn, const char * srvip, unsigned int srvport)
{
    conlist::getInstance()->init(pdnn, srvip, srvport);
}

void  dnn_getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version)
{
    conlist::getInstance()->getstate(versions, ncnt, modelname, version);
}

void  * dnn_addconn(const char * modelname)
{
    return conlist::getInstance()->addconn(modelname);
}

void dnn_predict(void * pdnn, unsigned char *data, int w, int h, DNNTARGET *objs, int *size, int jpgsize)
{
	if(pdnn != NULL)
        ((connection*)pdnn)->predict(data, w, h, objs, size, jpgsize);
}


void dnn_rmconn(void * pdnn)
{
    conlist::getInstance()->rmconn(pdnn);
}
