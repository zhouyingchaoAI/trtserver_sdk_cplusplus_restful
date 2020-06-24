# trtserver_sdk_cplusplus_restful
use boost beast implement c++ request trtserver with http restful

## Prerequisites
```
get the opencvlib for test demo
https://drive.google.com/file/d/1LAwd24cV-EBLEOfqqbw_cD-KwvdUDRKj/view?usp=sharing
```

## Quick Start
```
cp opencvlib.tar to dnnsdk_sync/common/lib
cd dnnsdk_sync
mkdir build && cd build && cmake ..  make
cd dnnsdk_async/bin
./video_client 64 0
64:there are 64 chnnal live video to predict  0:camera0 can also appoint to video file path
