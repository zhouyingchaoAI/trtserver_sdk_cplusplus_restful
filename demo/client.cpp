#include "opencv2/opencv.hpp"
#include "pfclist.h"


using namespace std;
using namespace cv;



int main(int argc, char** argv)
{
    string cap = "vv.mp4";
    int chnsize = 4;

    if (argc>=2)
        chnsize = atoi(argv[1]);
    if (argc==3)
        cap = argv[2];

    pfclist::getInstance()->start(cap, chnsize);
    while(1)
    {
        boost::thread::sleep(boost::get_system_time()
             + boost::posix_time::milliseconds(20));
        continue;
    }

    return EXIT_SUCCESS;
}


