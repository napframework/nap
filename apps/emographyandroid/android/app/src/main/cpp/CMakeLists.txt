cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../../../../..)
add_library(nap-service SHARED
            nap-service.cpp
            ${PROJECT_ROOT}/src/emographyserviceapp.cpp
            ${PROJECT_ROOT}/src/emographyserviceapp.h)

set(NAP_ROOT ${CMAKE_SOURCE_DIR}/../../../../../../../NAP-0.2.1-Android)
set(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty)

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/cmake)

find_package(rttr REQUIRED)
find_package(napcore REQUIRED)
find_package(naprtti REQUIRED)
find_package(mod_napscene REQUIRED)
find_package(mod_napdatabase REQUIRED)
find_package(mod_napapp REQUIRED)
find_package(mod_napinput REQUIRED)
find_package(mod_napmath REQUIRED)
find_package(mod_napapi REQUIRED)

# Add project module
add_subdirectory(module)

# Add third party include path, other paths will be brought in through the properties on the linked
# targets below
target_include_directories(nap-service PRIVATE ${NAP_ROOT}/thirdparty
                                               ${PROJECT_ROOT}/src
                                               ${PROJECT_ROOT}/module/src)

# This needs to be a list of all dependencies; populated manually. project.json and
# module.json files aren't being processed for dependencies (for now?)
target_link_libraries(nap-service
                      android
                      log
                      RTTR::Core
                      naprtti
                      napcore
                      mod_napscene
                      mod_napinput
                      mod_napmath
                      mod_napapp
                      mod_napapi
                      mod_emography
                      )
