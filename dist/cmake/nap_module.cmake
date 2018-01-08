cmake_minimum_required(VERSION 3.5)
get_filename_component(module_name_from_dir ${CMAKE_SOURCE_DIR} NAME)
project(${module_name_from_dir})

set(NAP_ROOT "${CMAKE_SOURCE_DIR}/../../")
include(${NAP_ROOT}/cmake/targetarch.cmake)
include(${NAP_ROOT}/cmake/distmacros.cmake)

# Set our default build type if we haven't specified one (Linux)
set_default_build_type()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if (WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd4251 /MP /bigobj")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif()
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-format-security -Wno-switch")
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)

# Restrict to debug and release build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH "${NAP_ROOT}/cmake")

include(${NAP_ROOT}/cmake/configure.cmake)

if(WIN32)
    if(MINGW)
        # Copy required MinGW dll's to bin dir
        get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} PATH)
        set(MODULES gcc_s_dw2-1 stdc++-6 winpthread-1)
        foreach (MOD ${MODULES})
            find_library(_LIB NAMES ${MOD} HINTS ${COMPILER_DIR})
            message(STATUS "Copy ${_LIB} to ${BIN_DIR}")
            file(COPY ${_LIB} DESTINATION ${BIN_DIR})
            unset(_LIB CACHE)
        endforeach ()
    endif()
endif()

include_directories(${NAP_ROOT}/include/)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# target_link_libraries(${PROJECT_NAME} napcore)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
    if (${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        # Set project as startup project in Visual Studio
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif()

# Pull in NAP core
find_package(napcore REQUIRED)
find_package(naprtti REQUIRED)

message("PYTHON_LIBRARIES ${PYTHON_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} napcore naprtti RTTR::Core ${PYTHON_LIBRARIES} ${SDL2_LIBRARY})

# Set our module output directory
set_module_output_directories()
