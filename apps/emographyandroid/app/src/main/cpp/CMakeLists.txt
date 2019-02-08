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


#------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty)

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/cmake)

find_package(rttr REQUIRED)
find_package(napcore REQUIRED)
find_package(naprtti REQUIRED)
foreach(NAP_MODULE ${NAP_MODULES})
    find_package(${NAP_MODULE} REQUIRED)
endforeach()

# Add project module
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module)
    add_subdirectory(module)
endif()

# Add third party include path, other paths will be brought in through the properties on the linked
# targets below
target_include_directories(${LIB_NAME} PRIVATE ${THIRDPARTY_DIR})

# Add module to include paths
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module)
    target_include_directories(${LIB_NAME} PRIVATE module/src)
endif()

target_link_libraries(${LIB_NAME}
                      android
                      log
                      RTTR::Core
                      naprtti
                      napcore
                      ${NAP_MODULES}
                      mod_emography
                      )

#------------------------------------------------