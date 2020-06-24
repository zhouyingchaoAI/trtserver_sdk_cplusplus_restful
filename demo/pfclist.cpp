#include "pfclist.h"
#include <chrono>


pfclist::pfclist()
{
    m_chnsz = 0;
    m_updateflag = false;
    m_predflag = false;
    m_matreq.clear();
    m_imid.clear();
    m_matshow.clear();

}
int mmm=0;
void pfclist::dnncbhander(unsigned int chn, unsigned int fid, const char *model, DNNTARGET *objs, int size)
{
    m_mtxreq.lock();
    if(m_matreq.find(int(chn)) == m_matreq.end())
    {
        m_mtxreq.unlock();
        return;
    }

    if(m_matreq.find(int(chn))->second.find(int(fid)) == m_matreq.find(int(chn))->second.end())
    {
        m_mtxreq.unlock();
        return;
    }

    Mat image = m_matreq.find(int(chn))->second.find(int(fid))->second->image;
    boost::posix_time::ptime time = boost::posix_time::from_iso_string(m_matreq.find(int(chn))->second.find(int(fid))->second->time);
    m_matreq.find(int(chn))->second.erase(m_matreq.find(int(chn))->second.begin(), m_matreq.find(int(chn))->second.find(int(fid)));
    m_mtxreq.unlock();

    auto show = std::make_shared<SHOWMATINFO>();
    show->image = image;
    for(int i = 0; i<size; i++)
    {
        int x = static_cast<int>((objs+i)->tlx*image.cols);
        int y = static_cast<int>((objs+i)->tly*image.rows);
        int w = static_cast<int>(((objs+i)->brx - (objs+i)->tlx)*image.cols);
        int h = static_cast<int>(((objs+i)->bry - (objs+i)->tly)*image.rows);
        Rect rect(x, y, w, h);
        show->rect.push_back(rect);
        show->label.push_back((objs+i)->cid);
    }
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_span = now - time;
    string custtime= "cost time " + to_string(time_span.total_microseconds()/1000) + " ms";
    show->costtime=custtime;

    m_mtxshow.lock();
    if(m_matshow.find(chn+1) == m_matshow.end())
        m_matshow.insert(make_pair(chn+1, show));
    m_matshow[chn+1] = show;
    m_updateflag = true;
    m_mtxshow.unlock();

}


void pfclist::dnncb(unsigned int chn, unsigned int imgid, const char * modelname, DNNTARGET * objs, int size, void * puser)
{
    if(puser != NULL)
        ((pfclist*)puser)->dnncbhander(chn, imgid, modelname, objs, size);
}


void pfclist::start(string &cap, int chnsize)
{
    dnn_init("192.168.60.73", 8000);
    m_chnsz = chnsize;
    m_cap = cap;

    for(int i = 0; i < chnsize; i++)
    {
        m_imid.insert(make_pair(i, 0));
        std::map<int, std::shared_ptr<SHOWMAT>> tmp;
        m_matreq.insert(make_pair(i, tmp));
        dnn_addconn("192.168.60.73", i, "helmet", dnncb, this);
    }
    boost::thread(boost::bind(&pfclist::run, this)).detach();

}

void pfclist::run()
{
    VideoCapture capture0(0);
//    capture0.set(CAP_PROP_FRAME_WIDTH, 416);
//    capture0.set(CAP_PROP_FRAME_HEIGHT, 416);
    boost::thread(boost::bind(&pfclist::predict, this)).detach();
    boost::thread(boost::bind(&pfclist::showManyImages, this)).detach();

    int mmm=0;
    while(true) {
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        cv::Mat frameImage;

        capture0 >> frameImage;

        if (frameImage.empty())
        {
            cout << "No Image !!!" << endl;
            continue;
        }
        auto show = std::make_shared<SHOWMATINFO>();
        show->image = frameImage.clone();

        m_mtxshow.lock();
        if (m_matshow.find(0) == m_matshow.end())
            m_matshow.insert(make_pair(0, show));
        m_matshow[0] = show;
        m_updateflag = true;
        m_mtxshow.unlock();

        m_mtxpred.lock();
        m_predMat = frameImage.clone();
        m_predflag = true;
        m_mtxpred.unlock();

        m_mtxcv.lock();
        waitKey(40);
        m_mtxcv.unlock();

        //boost::this_thread::sleep(boost::posix_time::microseconds(40));
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        //std::cout << "push image took me " << time_span.count() << " seconds." << endl;
    }
}

void pfclist::run_one(int chn, Mat frameImage)
{
    m_mtxreq.lock();
    m_imid[chn]++;
    auto reqmat = std::make_shared<SHOWMAT>();
    reqmat->image = frameImage;
    reqmat->time=boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time());
    if (m_matreq.find(chn) == m_matreq.end())
        m_matreq.find(chn)->second.insert(make_pair(m_imid[chn], reqmat));
    resize(frameImage, frameImage, cv::Size(416, 416));
    m_matreq[chn][m_imid[chn]] = reqmat;
    m_mtxreq.unlock();

    Mat img_32;
    img_32.empty();
    frameImage.convertTo(img_32, CV_32FC1);
    dnn_predict("192.168.60.73", chn, "helmet", img_32.data, img_32.rows, img_32.cols, m_imid[chn]);

}

void pfclist::predict()
{
    while(true)
    {
        Mat image;
        bool flag = false;
//        m_mtxpred.lock();
        flag = m_predflag;
        if(flag)
            image =m_predMat;
//        m_mtxpred.unlock();

        if(flag)
        {
            for(int i = 0; i < m_chnsz; i++)
                run_one(i, image);
        }

    }
}


void pfclist::showManyImages()
{
    string winname = "所有分析通道图";
    cout << "enter display" << endl;
//    m_mtxcv.lock();
//    namedWindow(winname);
//    m_mtxcv.unlock();
    while(true)
    {
        cout << "enter display" << endl;
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        if (!m_updateflag)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            continue;
        }

//        m_mtxshow.lock();
//        m_updateflag = false;

        if(m_matshow.size() != m_chnsz+1)
        {
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
        case 11:nSizeWindows = Size(4, 3); break;
        case 17:nSizeWindows = Size(5, 4); break;
        default:nSizeWindows = Size(10, 7);
        }

        int nShowImageSize = 300;
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
        for (int i=0; i<m_matshow.size(); i++){

            if ((i%nSizeWindows.width == 0) && (tempPosX != posX)){
                tempPosX = posX;;
                tempPosY += (nSplitLineSize + nShowImageSize);
            }

            Mat tempImage = showWindowsImages
                (Rect(tempPosX, tempPosY, nShowImageSize, nShowImageSize));
            for(int j=0; j<m_matshow.find(i)->second->rect.size(); j++)
            {
                cv::rectangle(m_matshow.find(i)->second->image,
                              m_matshow.find(i)->second->rect[j],
                              m_colorid[m_matshow.find(i)->second->label[j]], 4, LINE_8,0);
            }
            m_mtxcv.lock();
            putText(m_matshow.find(i)->second->image, m_matshow.find(i)->second->costtime, Point(5, 40),
                    FONT_HERSHEY_SIMPLEX | FONT_ITALIC, 1.0, Scalar(0, 0, 255), 2, 8);
            m_mtxcv.unlock();


            resize(m_matshow.find(i)->second->image, tempImage,
                Size(nShowImageSize, nShowImageSize));
            tempPosX += (nSplitLineSize + nShowImageSize);

        }


        m_mtxcv.lock();
        imshow(winname, showWindowsImages);
        waitKey(10);
        m_mtxcv.unlock();

    }

}
