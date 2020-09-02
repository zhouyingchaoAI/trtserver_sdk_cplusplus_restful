#ifndef PFCLIST_H
#define PFCLIST_H
#include "opencv2/opencv.hpp"
#include "utdnn.h"
#include "json/json.h"
#include "boost/thread.hpp"
#include "boost/date_time.hpp"

using namespace std;
using namespace cv;

typedef boost::shared_mutex Lock;
typedef boost::unique_lock< Lock > WriteLock;
typedef boost::shared_lock< Lock > ReadLock;

struct SHOWMAT
{
    Mat image;
    string time;
};

struct SHOWMATINFO
{
    Mat image;
    string costtime;
    std::vector<Rect> rect;
    std::vector<int> label;
};

struct PREMAT
{
    Mat image;
    bool isupdate;
};


class pfclist
{
public:
    static pfclist * getInstance()
    {
        static pfclist instance;
        return &instance;
    }
    pfclist();

    void showManyImages();
    void start(string &cap, int chnsize, string svrip, int svrport, string modelname);
    void run();
    void run_one(int chn, Mat frameImage);
    void predict(int chn);



private:
    Scalar m_colorid[3]={Scalar(255, 0, 0), Scalar(255, 255, 0), Scalar(255, 0, 255)};
    string m_cap;
    int m_chnsz;

    std::map<int, std::map<int, std::shared_ptr<SHOWMAT>>> m_matreq;

    Lock m_mtxshow;

    //Lock m_mtxcv;
    std::map<int, std::shared_ptr<SHOWMATINFO>>    m_matshow;
    std::map<int, int>                m_imid;
    bool                              m_updateflag;

    std::array<void*, 10>                 m_dnnhandel;
    std::array<Lock, 10>                 m_mtxpred;
    std::array<PREMAT, 10>                  m_predMat;
    int                                     m_isvrtype;
    string                                  m_strsvrip;
    int                                     m_isvrport;
    string                                  m_strmodelname;

//    Lock                              m_mtxpred;
//    int                               m_predflag;
//    Mat                               m_predMat;



};

#endif // PFCLIST_H
