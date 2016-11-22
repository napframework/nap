cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /MP")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)

set(BUILD_CONF /${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE})

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Override binary directories
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
file(MAKE_DIRECTORY ${BIN_DIR})
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
file(MAKE_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)


# library targets
add_subdirectory(core)
add_subdirectory(napkin)

# executable targets
add_subdirectory(test/coretestapp)
add_subdirectory(test/unittest)
add_subdirectory(test/scriptserver)
add_subdirectory(test/jsonserializer)

# modules
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}/modules)


add_subdirectory(modules/napfsm)
add_subdirectory(modules/napgui)
add_subdirectory(modules/napmath)
#add_subdirectory(modules/napof)
add_subdirectory(modules/napinput)
add_subdirectory(modules/network)
#add_subdirectory(modules/napawesomium)
add_subdirectory(modules/napscriptserver)
add_subdirectory(modules/napjsonserializer)

#add_dependencies(napkin mod_napfsm)
add_dependencies(scriptserver mod_napscriptserver)
