cmake_minimum_required(VERSION 3.5)
get_filename_component(project_name_from_dir ${CMAKE_SOURCE_DIR} NAME)
project(${project_name_from_dir})

# Set our install prefix for project packaging
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin_package)

get_filename_component(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ REALPATH)
message(STATUS "Using NAP root: ${NAP_ROOT}")
get_filename_component(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty REALPATH)
message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

include(${NAP_ROOT}/cmake/distmacros.cmake)

# Use configure_file to result in changes in project.json triggering reconfigure.  Appears to be best current approach.
configure_file(${CMAKE_SOURCE_DIR}/project.json ProjectJsonTriggerDummy.json)

# Parse our project.json and import it
execute_process(COMMAND python ${NAP_ROOT}/tools/platform/projectInfoParseToCMake.py ${PROJECT_NAME})
include(cached_project_json.cmake)

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
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("Headers" FILES ${HEADERS})
source_group("Sources" FILES ${SOURCES})
source_group("Shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
    if (${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        # Set project as startup project in Visual Studio
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Cleanup dummy JSON file (created for project.json updates triggering reconfigure)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove ProjectJsonTriggerDummy.json
)       

# Set our project output directory
set_output_directories()

# Pull in NAP core
include(${CMAKE_CURRENT_LIST_DIR}/napcore.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/naprtti.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/naputility.cmake)

# Pull in a project module if it exists
add_project_module()

# Find each NAP module
foreach(NAP_MODULE ${NAP_MODULES})
    find_nap_module(${NAP_MODULE})
endforeach()

target_link_libraries(${PROJECT_NAME} napcore naprtti RTTR::Core naputility ${NAP_MODULES} ${PYTHON_LIBRARIES} ${SDL2_LIBRARY})

# Include anu extra project CMake logic
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/projectExtra.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/projectExtra.cmake)
endif()

# Add IDE targets for launching napkin
if(APPLE)
    add_custom_target("LAUNCH_NAPKIN" 
                      COMMAND open $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                      DEPENDS ${PROJECT_NAME})
elseif(WIN32)
    # TODO Haven't managed to get this to launch without blocking in Visual Studio yet (start /b doesn't work).  Maybe not the right way to go anyway.
    add_custom_target("LAUNCH_NAPKIN" 
                      COMMAND $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                      DEPENDS ${PROJECT_NAME})
endif()

# Copy data to bin post-build
copy_files_to_bin(${CMAKE_SOURCE_DIR}/project.json)
dist_export_fbx(${CMAKE_SOURCE_DIR}/data/)

# Copy our Python virtual environment configuration for non-packaged builds
if(UNIX)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
                -E copy 
                ${THIRDPARTY_DIR}/python/pyvenv.cfg
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
endif()

if(NOT WIN32)
    if (APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@executable_path/lib/")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib/")
        install(PROGRAMS ${NAP_ROOT}/tools/platform/install_ubuntu_1710_dependencies.sh
                DESTINATION .)
    endif()
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION .)    
    install(FILES ${CMAKE_SOURCE_DIR}/project.json DESTINATION .)
endif()

# Package napkin if we're doing a build from againat released NAP or we're packaging a project with napkin
if(NOT DEFINED PACKAGE_NAPKIN OR PACKAGE_NAPKIN)
    include(${CMAKE_CURRENT_LIST_DIR}/install_napkin_with_project.cmake)
endif()