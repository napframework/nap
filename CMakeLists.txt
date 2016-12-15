#

cmake_minimum_required(VERSION 2.8)

include(cmake/targetarch.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0043 NEW)

# Allow extra Find{project}.cmake files to be found by projects
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/targetarch.cmake)
target_architecture(ARCH)



if (MSVC)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})

        set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})

        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})

    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

message(WARNING ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

else()
    set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})

    # Override binary directories
    set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
    file(MAKE_DIRECTORY ${BIN_DIR})
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
    file(MAKE_DIRECTORY ${LIB_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
endif()



# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)


# library targets
add_subdirectory(core)
add_subdirectory(coreapp)

# executable targets
add_subdirectory(test/unittest)
add_subdirectory(test/scriptserver)
add_subdirectory(test/render)

# Modules
add_subdirectory(modules/napfsm)
add_subdirectory(modules/naprender)
add_subdirectory(modules/napinput)
add_subdirectory(modules/napscriptserver)
add_subdirectory(modules/napjsonserializer)
add_subdirectory(modules/napgui)

#add_dependencies(napkin mod_napfsm)
add_dependencies(scriptserver mod_napscriptserver)
add_dependencies(rendertest mod_naprender)
