#ifndef PFCLIST_H
#define PFCLIST_H
#include "opencv2/opencv.hpp"
#include "utdnn.h"
#include "json/json.h"
#include "boost/thread.hpp"
#include "boost/date_time.hpp"

using namespace std;
using namespace cv;

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
    void start(string &cap, int chnsize=4);
    void run();
    void run_one(int chn, Mat frameImage);
    void predict(int chn);


private:
    static void dnncb(unsigned int chn, unsigned int fid, const char * model, DNNTARGET * objs, int size, void * puser);

    void dnncbhander(unsigned int chn, unsigned int fid, const char * model, DNNTARGET * objs, int size);




private:
    Scalar m_colorid[3]={Scalar(255, 0, 0), Scalar(255, 255, 0), Scalar(255, 0, 255)};
    string m_cap;
    int m_chnsz;
    std::mutex m_mtxreq;
    std::map<int, std::map<int, std::shared_ptr<SHOWMAT>>> m_matreq;

    std::mutex m_mtxshow;
    std::mutex m_mtxcv;
    std::map<int, std::shared_ptr<SHOWMATINFO>>    m_matshow;
    std::map<int, int>                m_imid;
    bool                              m_updateflag;

    std::mutex                        m_mtxpred;
    bool                              m_predflag;
    Mat                               m_predMat;



};

#endif // PFCLIST_H
