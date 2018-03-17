cmake_minimum_required(VERSION 3.5)

#if(WIN32)
#    find_file(PYEXE
#              NAMES python.exe
#              HINTS
#              c:/python35
#              c:/python36
#              )
#    if(PYEXE)
#        set(PYTHON_EXECUTABLE ${PYEXE})
#    endif()
#endif()

# Keep the current directory as project root
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using NAP Root: ${CMAKE_CURRENT_LIST_DIR}")

# Thirdparty libraries reside in this location, keep an absolute path
get_filename_component(THIRDPARTY_DIR ../thirdparty REALPATH BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")


include(cmake/targetarch.cmake)

include(cmake/qtconfig.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(WIN32)
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

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


include(configure.cmake)

if(WIN32)
    if(MINGW)
        # Copy required MinGW dll's to bin dir
        get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} PATH)
        set(MODULES gcc_s_dw2-1 stdc++-6 winpthread-1)
        foreach(MOD ${MODULES})
            find_library(_LIB NAMES ${MOD} HINTS ${COMPILER_DIR})
            message(STATUS "Copy ${_LIB} to ${BIN_DIR}")
            file(COPY ${_LIB} DESTINATION ${BIN_DIR})
            unset(_LIB CACHE)
        endforeach()

    endif()
endif()

# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)


# library targets
add_subdirectory(core)
add_subdirectory(utility)
add_subdirectory(rtti)
add_subdirectory(nrender)

# tools targets
add_subdirectory(tools/fbxconverter)
add_subdirectory(napkin)

# executable targets
add_subdirectory(test/render)
add_subdirectory(test/tommy)
add_subdirectory(test/serializationtest)
add_subdirectory(test/audiotest)

# app targets
add_subdirectory(apps/kalvertoren)
add_subdirectory(apps/etherdream)
add_subdirectory(apps/artnetcolor)
add_subdirectory(apps/steef)
add_subdirectory(apps/skeleton)

# demo targets
add_subdirectory(demos/helloworld)
add_subdirectory(demos/vinyl)
add_subdirectory(demos/multiwindow)
add_subdirectory(demos/heightmap)
add_subdirectory(demos/heightmap/module)
add_subdirectory(demos/dynamicgeo)
add_subdirectory(demos/dynamicgeo/module)
add_subdirectory(demos/videomodulation)
add_subdirectory(demos/videomodulation/module)
add_subdirectory(demos/blobtrace)
add_subdirectory(demos/audioplayback)
add_subdirectory(demos/lineblending)
add_subdirectory(demos/lineblending/module)

add_subdirectory(unittests)

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
add_subdirectory(modules/napsdlwindow)
add_subdirectory(modules/napscene)
add_subdirectory(modules/napcameracontrol)
add_subdirectory(modules/nappython)
add_subdirectory(modules/napvideo)
add_subdirectory(modules/napetherdream)
add_subdirectory(modules/napaudio)
add_subdirectory(modules/napaudioadvanced)
add_subdirectory(modules/napartnet)
add_subdirectory(modules/naposc)
add_subdirectory(modules/napsvg)
add_subdirectory(modules/napapp)
add_subdirectory(modules/napimgui)
add_subdirectory(modules/napmidi)

# Leftover components can go here for now
add_subdirectory(modules/napsandbox)

# app modules
add_subdirectory(apps/kalvertoren/module)