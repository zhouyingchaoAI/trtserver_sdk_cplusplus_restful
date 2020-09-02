cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(video_client)

#------- EXE: trtclient---------------------------------------------
FILE(GLOB_RECURSE CLIENT_SRC ./*.cpp)
add_executable(${PROJECT_NAME}  ${CLIENT_SRC} ${SRC_UTIL})

set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME   ${PROJECT_NAME}${CMAKE_DEBUG_POSTFIX}
    RELEASE_OUTPUT_NAME ${PROJECT_NAME}${CMAKE_RELEASE_POSTFIX})
#---------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} miniboost utdnn pthread
    opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui opencv_video opencv_calib3d opencv_features2d opencv_flann opencv_videoio)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/release/ DESTINATION bin)
