cmake_minimum_required(VERSION 3.4.1)

set(LIB_NAME nap-service)
add_library(${LIB_NAME} SHARED
            nap-service.cpp
            emographyandroidapp.cpp
            emographyandroidapp.h
            androidwrapper.cpp
            androidwrapper.h
            androidinstance.h
            )

include(${NAP_ROOT}/cmake/nap_android_project.cmake)