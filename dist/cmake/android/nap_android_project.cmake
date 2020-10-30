project(${NAP_PROJECT_NAME})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty)

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/cmake)

include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

find_package(rttr REQUIRED)
find_package(napcore REQUIRED)
find_package(naprtti REQUIRED)
find_package(naputility REQUIRED)
foreach(NAP_MODULE ${NAP_MODULES})
    if(NAP_MODULE STREQUAL "mod_${NAP_PROJECT_NAME}")
        continue()
    endif()
    find_nap_module(${NAP_MODULE})
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
                      naputility
                      ${NAP_MODULES}
                      )

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module)
    target_link_libraries(${LIB_NAME} mod_${NAP_PROJECT_NAME})
endif()