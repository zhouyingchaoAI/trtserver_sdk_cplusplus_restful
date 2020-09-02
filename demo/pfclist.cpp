#include "pfclist.h"
#include <chrono>
#include <random>
using std::default_random_engine;
using std::uniform_int_distribution;

extern default_random_engine g_e;
extern uniform_int_distribution<unsigned> g_u; //随机数分布对象


pfclist::pfclist()
{
    m_chnsz = 0;
    m_updateflag = 0;
    m_matreq.clear();
    m_imid.clear();
    m_matshow.clear();
    m_isvrtype = 0;

}
int mmm=0;


void pfclist::start(string &cap, int chnsize, string svrip, int svrport, string modelname)
{
    dnn_init(NULL, svrip.c_str(), svrport);
    MODELINFO versions[10] = {0};
    int ncnt = 1;
    int version = -1;
    dnn_getstate(versions, &ncnt, modelname.c_str(), version);
    m_isvrtype = versions[0].type;

    m_chnsz = chnsize;
    m_cap = cap;

    for(int i = 0; i < chnsize; i++)
    {
        m_imid.insert(make_pair(i, 0));
        std::map<int, std::shared_ptr<SHOWMAT>> tmp;
        m_matreq.insert(make_pair(i, tmp));
        m_dnnhandel[i] = dnn_addconn(modelname.c_str());
    }
    boost::thread(boost::bind(&pfclist::run, this)).detach();

}

void pfclist::run()
{
    VideoCapture capture;
    if(m_cap == "0")
        capture.open(0);
    else
        capture.open(m_cap);
    for(int i=0; i<m_chnsz; i++)
        boost::thread(boost::bind(&pfclist::predict, this, i)).detach();

    boost::thread(boost::bind(&pfclist::showManyImages, this)).detach();

    int mmm=0;
    int chnsize = m_chnsz;

    while(true) {
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        cv::Mat frameImage;


        capture >> frameImage;

        if (frameImage.empty())
        {
            cout << "No Image !!!" << endl;
            continue;
        }
        auto show = std::make_shared<SHOWMATINFO>();
//        Mat Roi = frameImage(Rect(0, 0, 416, 416)).clone();

        resize(frameImage, frameImage, cv::Size(416, 416));
        show->image = frameImage.clone();
//        show->image = Roi;

        {
            WriteLock lock(m_mtxshow);
            if (m_matshow.find(0) == m_matshow.end())
                m_matshow.insert(make_pair(0, show));
            m_matshow[0] = show;
            m_updateflag = true;
        }

        {
            int index = g_u(g_e);
            if(chnsize > 1)
            {
                index = chnsize - 1;
                chnsize--;
            }

            std::cout << "random is " << index << "!!!!!!!!!!" << std::endl;

            PREMAT *premat = &(m_predMat[index]);
            Lock *lock1 = &(m_mtxpred[index]);
            WriteLock lock(*lock1);
            premat->image = frameImage.clone();
//            premat->image = Roi;
            premat->isupdate = true;
        }

//        m_mtxcv.lock();
//        waitKey(5);
//        m_mtxcv.unlock();

        //boost::this_thread::sleep(boost::posix_time::microseconds(40));
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        //std::cout << "push image took me " << time_span.count() << " seconds." << endl;
    }
}

void pfclist::run_one(int chn, Mat image)
{
    Mat img_32;
    img_32.empty();
    image.convertTo(img_32, CV_32FC1);

    // DNNTARGET *target = new DNNTARGET[100];
    DNNTARGET target[100] = {0};
    int size = 0;
    boost::posix_time::ptime pre = boost::posix_time::microsec_clock::local_time();

    if(m_isvrtype == DNN_SERVER_TRT)
        dnn_predict(m_dnnhandel[chn], img_32.data, img_32.rows, img_32.cols, target, &size);
    else {
        vector<uchar> jpgbuff(1024 * 1024);
        vector<int> param = vector<int>(2);
        param[0] = IMWRITE_JPEG_QUALITY;
        param[1] = 95;
        imencode(".jpg", image, jpgbuff, param);
        dnn_predict(m_dnnhandel[chn], &jpgbuff[0], 0, 0, target, &size, jpgbuff.size());
    }


    auto show = std::make_shared<SHOWMATINFO>();
    show->image = image.clone();
    for(int i = 0; i<size; i++)
    {
        int x = static_cast<int>((target+i)->tlx*image.cols);
        int y = static_cast<int>((target+i)->tly*image.rows);
        int w = static_cast<int>(((target+i)->brx - (target+i)->tlx)*image.cols);
        int h = static_cast<int>(((target+i)->bry - (target+i)->tly)*image.rows);
        Rect rect(x, y, w, h);
        show->rect.push_back(rect);
        show->label.push_back((target+i)->cid);
    }


    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_span = now - pre;
    string custtime= "cost time " + to_string(time_span.total_microseconds()/1000) + " ms";
    show->costtime=custtime;
    std::cout << "cost " << custtime << std::endl;

    {
        WriteLock lock(m_mtxshow);
        if(m_matshow.find(chn+1) == m_matshow.end())
            m_matshow.insert(make_pair(chn+1, show));
        m_matshow[chn+1] = show;
        m_updateflag = true;
    }

}

void pfclist::predict(int chn)
{
    Mat image;
    bool flag = false;
    while(true)
    {
        PREMAT *premat = &(m_predMat[chn]);
        Lock *lock1 = &(m_mtxpred[chn]);
        {
            ReadLock lock(*lock1);
            flag = premat->isupdate;
            if(flag>0)
            {
                image = premat->image;
            }
        }

        if(flag > 0)
            run_one(chn, image);
        boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    }
}


void pfclist::showManyImages()
{
    string winname = "所有分析通道图";
//    m_mtxcv.lock();
    namedWindow(winname);
//    m_mtxcv.unlock();
    while(true)
    {
//        cout << "enter display" << endl;

//        {
//            ReadLock lock(m_mtxshow);
//            if (!m_updateflag)
//            {
//                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
//                continue;
//            }

//        }


//        {
//            WriteLock lock(m_mtxshow);
//            m_updateflag = false;
//        }


        std::map<int, std::shared_ptr<SHOWMATINFO>> matshow;
        matshow.clear();

        ReadLock lock(m_mtxshow);
        for(auto a:m_matshow)
        {
            matshow[a.first] = a.second;
        }

        lock.unlock();



        if(matshow.size() != m_chnsz+1)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            continue;
        }

        Size nSizeWindows;

        switch (m_chnsz+1){
        case 1:nSizeWindows = Size(1, 1); break;
        case 2:nSizeWindows = Size(2, 1); break;
        case 3:
        case 4:nSizeWindows = Size(2, 2); break;
        case 5:
        case 6:nSizeWindows = Size(3, 2); break;
        case 7:
        case 8:nSizeWindows = Size(4, 2); break;
        case 9:nSizeWindows = Size(3, 3); break;
        case 11:nSizeWindows = Size(4, 4); break;
        case 21:nSizeWindows = Size(6, 4); break;
        case 31:nSizeWindows = Size(7, 5); break;
        case 65:nSizeWindows = Size(9, 8); break;
        case 101:nSizeWindows = Size(11, 10); break;
        case 301:nSizeWindows = Size(61, 5); break;
        default:nSizeWindows = Size(101, 4);
        }

        int nShowImageSize = 200;
        int nSplitLineSize = 15;
        int nAroundLineSize = 50;

        const int imagesHeight = nShowImageSize*
            nSizeWindows.width + nAroundLineSize +
            (nSizeWindows.width - 1)*nSplitLineSize;
        const int imagesWidth = nShowImageSize*
            nSizeWindows.height + nAroundLineSize +
            (nSizeWindows.height - 1)*nSplitLineSize;
        //cout << imagesWidth << "  " << imagesHeight << endl;
        Mat showWindowsImages(imagesWidth, imagesHeight, CV_8UC3, Scalar(0, 0, 0));

        int posX = (showWindowsImages.cols - (nShowImageSize*nSizeWindows.width +
            (nSizeWindows.width - 1)*nSplitLineSize)) / 2;
        int posY = (showWindowsImages.rows - (nShowImageSize*nSizeWindows.height +
            (nSizeWindows.height - 1)*nSplitLineSize)) / 2;
        //cout << posX << "  " << posY << endl;
        int tempPosX = posX;
        int tempPosY = posY;

        int i = 0;

        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        for (int i=0; i<matshow.size(); i++)
        {

            if ((i%nSizeWindows.width == 0) && (tempPosX != posX)){
                tempPosX = posX;;
                tempPosY += (nSplitLineSize + nShowImageSize);
            }

            Mat tempImage = showWindowsImages
                (Rect(tempPosX, tempPosY, nShowImageSize, nShowImageSize));
            if(1)
            {
                for(int j=0; j<matshow.find(i)->second->rect.size(); j++)
                {
                    cv::rectangle(matshow.find(i)->second->image,
                                  matshow.find(i)->second->rect[j],
                                  m_colorid[matshow.find(i)->second->label[j]], 4, LINE_8,0);
                }
            }

            putText(matshow.find(i)->second->image, matshow.find(i)->second->costtime, Point(5, 40),
                    FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar(0, 0, 255), 2, 8);

//            tempImage = (matshow.find(i)->second->image)(Rect(0, 0, nShowImageSize, nShowImageSize));

            resize(matshow.find(i)->second->image, tempImage,
                Size(nShowImageSize, nShowImageSize));
            tempPosX += (nSplitLineSize + nShowImageSize);

        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
//        std::cout << "show image took me " << time_span.count() << " seconds." << endl;

//        m_mtxcv.lock();
        imshow(winname, showWindowsImages);
        waitKey(10);
//        m_mtxcv.unlock();


    }

}
