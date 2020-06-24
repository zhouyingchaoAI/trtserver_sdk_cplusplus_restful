#include "opencv2/opencv.hpp"
#include "pfclist.h"


using namespace std;
using namespace cv;



int main(int argc, char** argv)
{
    string cap = "vv.mp4";
    pfclist::getInstance()->start(cap, 16);
    while(1)
    {
        boost::thread::sleep(boost::get_system_time()
             + boost::posix_time::milliseconds(20));
        continue;
    }

    return EXIT_SUCCESS;
}


