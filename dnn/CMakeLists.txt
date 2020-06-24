cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(utdnn)

if(MSVC)
    add_definitions(-DUTDNN_EXPORTS)
endif()

FILE(GLOB_RECURSE DNN_SRC ./*.cpp)

add_library(${PROJECT_NAME} SHARED ${DNN_SRC} ${SRC_COM})

if(MSVC)
    target_link_libraries(${PROJECT_NAME} optimized miniboost)
    target_link_libraries(${PROJECT_NAME} debug     miniboostd)
else()
    target_link_libraries(${PROJECT_NAME} miniboost)
endif()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib
    )
