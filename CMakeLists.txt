cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(DLCLIENT)

add_compile_options(-std=c++11)
add_definitions(-DDLL_EXPORT)

if(CMAKE_BUILD_TYPE STREQUAL DEBUG OR CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-D_DEBUG)
endif()


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
IF(EXISTS "/etc/os-release")
    FILE(READ "/etc/os-release" MYOS)
    IF(MYOS MATCHES "Ubuntu" AND MYOS MATCHES "18.04")
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
        #for newest Ubuntu version which use RUNPATH instead of RPATH
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,--disable-new-dtags")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
        string(APPEND CMAKE_MODULE_LINKER_FLAGS " -Wl,--disable-new-dtags")
    ENDIF()
ENDIF()
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/../bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/../lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/../bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/../install)

SET(CMAKE_RELEASE_POSTFIX "")
SET(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "./")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/common/include)
aux_source_directory(${CMAKE_SOURCE_DIR}/common/src SRC_COM)
LINK_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/common/lib)
LINK_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/common/lib/opencv)


execute_process(COMMAND rm -r ${CMAKE_SOURCE_DIR}/common/lib/opencv)
execute_process(COMMAND tar -xvf ${CMAKE_SOURCE_DIR}/common/lib/opencvlib.tar -C ${CMAKE_SOURCE_DIR}/common/lib/)


aux_source_directory(${CMAKE_SOURCE_DIR}/util SRC_UTIL)
add_subdirectory(./dnn)
add_subdirectory(./demo)


