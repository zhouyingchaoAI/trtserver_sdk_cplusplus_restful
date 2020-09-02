#include "opencv2/opencv.hpp"
#include "pfclist.h"
#include <random>


using namespace std;
using namespace cv;
using std::default_random_engine;
using std::uniform_int_distribution;

int g_chnsize = 2;
default_random_engine g_e;
uniform_int_distribution<unsigned> g_u(0, g_chnsize-1); //随机数分布对象


int main(int argc, char** argv)
{
    string cap = "0";
    int chnsize = g_chnsize;

    if (argc>=2)
        chnsize = atoi(argv[1]);
    if (argc==3)
        cap = argv[2];


    pfclist::getInstance()->start(cap, chnsize, "192.168.60.88", 8000, "helmet");
//    pfclist::getInstance()->start(cap, chnsize, "192.168.60.98", 8000, 0, "sitecar");
    while(1)
    {
        boost::thread::sleep(boost::get_system_time()
             + boost::posix_time::milliseconds(20));
        continue;
    }

    return EXIT_SUCCESS;
}


