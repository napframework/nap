cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_computeflocking)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Only include project modules when desired, only build when not packaging
exclude_from_build_when_packaging(FALSE)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# APP SPECIFIC

# Check existence of bcm_host.h header file to see if we're building on Raspberry
if(${ARCH} MATCHES "armhf")
    MESSAGE(VERBOSE "Looking for bcm_host.h")
    INCLUDE(CheckIncludeFiles)

    # Raspbian bullseye bcm_host.h location
    CHECK_INCLUDE_FILES("/usr/include/bcm_host.h" COMPUTEFLOCKING_RASPBERRY)

    # otherwise, check previous location of bcm_host.h on older Raspbian OS's
    if(NOT COMPUTEFLOCKING_RASPBERRY)
        CHECK_INCLUDE_FILES("/opt/vc/include/bcm_host.h" COMPUTEFLOCKING_RASPBERRY)
    endif()
endif()


if(COMPUTEFLOCKING_RASPBERRY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE COMPUTEFLOCKING_RPI)
endif()
