#

cmake_minimum_required(VERSION 3.5)

include(cmake/targetarch.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if (WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /MP /bigobj")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
    endif()
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
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
        foreach (MOD ${MODULES})
            find_library(_LIB NAMES ${MOD} HINTS ${COMPILER_DIR})
            message(STATUS "Copy ${_LIB} to ${BIN_DIR}")
            file(COPY ${_LIB} DESTINATION ${BIN_DIR})
            unset(_LIB CACHE)
        endforeach ()

    endif()
endif()

# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)


# library targets
add_subdirectory(core)
add_subdirectory(utility)
add_subdirectory(rtti)
add_subdirectory(nrender)
add_subdirectory(coreapp)

# tools targets
add_subdirectory(tools/fbxconverter)

# executable targets
add_subdirectory(test/unittest)
add_subdirectory(test/scriptserver)
add_subdirectory(test/render)
add_subdirectory(test/tommy)
add_subdirectory(test/videoplayer)
add_subdirectory(test/serializationtest)

# Modules
add_subdirectory(modules/napfsm)
add_subdirectory(modules/napinput)
add_subdirectory(modules/napscriptserver)
add_subdirectory(modules/napjsonserializer)
add_subdirectory(modules/naprender)
add_subdirectory(modules/napgui)
add_subdirectory(modules/napmath)
add_subdirectory(modules/napsdlinput)
add_subdirectory(modules/napsdlwindow)
add_subdirectory(modules/napscene)
add_subdirectory(modules/napvideo)

#add_dependencies(napkin mod_napfsm)
add_dependencies(scriptserver mod_napscriptserver)
add_dependencies(CoreApp mod_napscriptserver)
add_dependencies(rendertest mod_naprender)
add_dependencies(unittest mod_naprender mod_napscriptserver)

