cmake_minimum_required(VERSION 3.18.4)
project(NAP)

# Keep the current directory as project root
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using NAP Root: ${CMAKE_CURRENT_LIST_DIR}")

# Thirdparty libraries reside in this location, keep an absolute path
get_filename_component(THIRDPARTY_DIR ../thirdparty REALPATH BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

# Enforce GCC on Linux for now (when doing packaging build at least)
if(UNIX AND NOT APPLE)
    if(DEFINED NAP_PACKAGED_BUILD AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "NAP only currently supports GCC on Linux (and this is enforced when doing release builds)")
    endif()
endif()

include(cmake/targetarch.cmake)
target_architecture(ARCH)

include(cmake/qtconfig.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd4251 /MP /bigobj")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        if(DEFINED INCLUDE_DEBUG_SYMBOLS AND INCLUDE_DEBUG_SYMBOLS)
            set(PACKAGE_PDBS ON)
        endif()
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
        #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif()
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-format-security -Wno-switch -fvisibility=hidden")
    if(DEFINED INCLUDE_DEBUG_SYMBOLS AND NOT INCLUDE_DEBUG_SYMBOLS)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Verified for AppleClang, expected to also potentially work (at a later date) for Clang on Linux
            string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        else()
            # Verified for GCC on Linux
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        endif()
    endif()
endif()

# We don't actively support and work on macOS anymore.
# This is not because we don't like their hardware, but because of the continuous tightening of restrictions of macOS,
# Including aggressive gate-keeping, required app singing, forcing specific data structures, vague licence policies,
# proprietary APIs, etc. It's simply not in line with our policies and what we stand for.
# Feel free to continue support for macOS on your end.
if(APPLE)
    message(DEPRECATION "macOS (as a target) is no longer in active development or supported")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
endif()

# We don't actively support and work on python bindings anymore.
if(NAP_ENABLE_PYTHON)
    message(DEPRECATION "Python bindings are no longer in active development or supported")
    add_definitions(-DNAP_ENABLE_PYTHON)
endif()

# Automatically link Qt executables to qtmain target on Windows.
cmake_policy(SET CMP0020 NEW)

# Ignore COMPILE_DEFINITIONS
cmake_policy(SET CMP0043 NEW)

# Restrict to debug and release build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(cmake/configure.cmake)
include(cmake/cross_context_macros.cmake)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/packaging_staging)
if(UNIX AND NOT APPLE)
    # Ensure we have patchelf on Linux, preventing silent failures
    ensure_patchelf_installed()

    # Check if we're building on raspbian
    check_raspbian_os(RASPBIAN)
endif()

# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

# library targets
add_subdirectory(core)
add_subdirectory(utility)
add_subdirectory(rtti)
add_subdirectory(qt)

# tools targets
add_subdirectory(tools/fbxconverter)
add_subdirectory(tools/napkin)
add_subdirectory(tools/keygen)
add_subdirectory(tools/licensegenerator)

# START_SOURCE_ARCHIVE_REMOVED_SECTION (don't remove)
# executable targets
add_subdirectory(test/render)

# app targets, add your own app targets here!
add_subdirectory(apps/example)
add_subdirectory(apps/example/module)
# END_SOURCE_ARCHIVE_REMOVED_SECTION (don't remove)

# demo targets
add_subdirectory(demos/helloworld)
add_subdirectory(demos/vinyl)
add_subdirectory(demos/multiwindow)
add_subdirectory(demos/heightmap)
add_subdirectory(demos/heightmap/module)
add_subdirectory(demos/dynamicgeo)
add_subdirectory(demos/dynamicgeo/module)
add_subdirectory(demos/computeparticles)
add_subdirectory(demos/computeparticles/module)
add_subdirectory(demos/computeflocking)
add_subdirectory(demos/computeflocking/module)
add_subdirectory(demos/videomodulation)
add_subdirectory(demos/videomodulation/module)
add_subdirectory(demos/blobtrace)
add_subdirectory(demos/audioanalysis)
add_subdirectory(demos/audioplayback)
add_subdirectory(demos/oscmidi)
add_subdirectory(demos/oscmidi/module)
add_subdirectory(demos/lineblending)
add_subdirectory(demos/lineblending/module)
add_subdirectory(demos/curveball)
add_subdirectory(demos/curveball/module)
add_subdirectory(demos/copystamp)
add_subdirectory(demos/copystamp/module)
add_subdirectory(demos/websocketserver)
add_subdirectory(demos/websocketserver/module)
add_subdirectory(demos/facedetection)
add_subdirectory(demos/facedetection/module)
add_subdirectory(demos/sequencer)
add_subdirectory(demos/paintobject)
add_subdirectory(demos/licensecheck)
add_subdirectory(demos/tween)
add_subdirectory(demos/artnetsend)
add_subdirectory(demos/artnetreceive)
add_subdirectory(demos/artnetreceive/module)
add_subdirectory(demos/pipins)
add_subdirectory(demos/webportal)
add_subdirectory(demos/python)
add_subdirectory(demos/lights)
add_subdirectory(demos/lights/module)
add_subdirectory(demos/sky)
add_subdirectory(demos/sky/module)

if(APPLE)
    set(GENERATE_XCODE_PROJECT_TARGET generateXcodeProject)
    add_custom_target(generateXcodeProject cmake -H. -Bxcode -G Xcode -DCMAKE_BUILD_TYPE=Debug ../)
endif()
#add_dependencies(${PROJECT_NAME} ${GENERATE_XCODE_PROJECT_TARGET})

# Modules
add_subdirectory(modules/napinput)
add_subdirectory(modules/naprender)
add_subdirectory(modules/napmath)
add_subdirectory(modules/napsdlinput)
add_subdirectory(modules/napscene)
add_subdirectory(modules/napcameracontrol)
add_subdirectory(modules/nappython)
add_subdirectory(modules/napvideo)
add_subdirectory(modules/napetherdream)
add_subdirectory(modules/napaudio)
add_subdirectory(modules/napartnet)
add_subdirectory(modules/naposc)
add_subdirectory(modules/napsvg)
add_subdirectory(modules/napapp)
add_subdirectory(modules/napimgui)
add_subdirectory(modules/napmidi)
add_subdirectory(modules/napyoctopuce)
add_subdirectory(modules/napfont)
add_subdirectory(modules/napdatabase)
add_subdirectory(modules/napapi)
add_subdirectory(modules/napparameter)
add_subdirectory(modules/napparametergui)
add_subdirectory(modules/napserial)
add_subdirectory(modules/napwebsocket)
add_subdirectory(modules/napapiwebsocket)
add_subdirectory(modules/napopencv)
add_subdirectory(modules/napsoem)
add_subdirectory(modules/napcolor)
add_subdirectory(modules/napsequence)
add_subdirectory(modules/napsequencegui)
add_subdirectory(modules/napsequenceaudio)
add_subdirectory(modules/napsequenceaudiogui)
add_subdirectory(modules/naplicense)
add_subdirectory(modules/napudp)
add_subdirectory(modules/naptween)
add_subdirectory(modules/napcalendar)
add_subdirectory(modules/napasio)
add_subdirectory(modules/nappipins)
add_subdirectory(modules/napportal)
add_subdirectory(modules/naprenderadvanced)

# Add additional directories if specified from command line.
# For example: -DADDITIONAL_SUB_DIRECTORIES="apps/myapp;apps/myapp/module"
if(ADDITIONAL_SUB_DIRECTORIES)
    foreach(SUB_DIR ${ADDITIONAL_SUB_DIRECTORIES})
        message(STATUS "Adding sub directory: ${SUB_DIR}")
        add_subdirectory(${SUB_DIR})
    endforeach()
endif()


# Packaging
include(cmake/packaging_macros.cmake)
package_nap()
