# Enforce GCC on Linux for now
if(UNIX AND NOT APPLE)
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "NAP only currently supports GCC on Linux")
    endif()
endif()

# Set our install prefix for project packaging
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin_package)

get_filename_component(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ REALPATH)
message(STATUS "Using NAP root: ${NAP_ROOT}")
get_filename_component(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty REALPATH)
message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

include(${NAP_ROOT}/cmake/dist_shared_native.cmake)
include(${NAP_ROOT}/cmake/cross_context_macros.cmake)

# Get our modules list from project.json
project_json_to_cmake()

# Fetch our module dependencies
fetch_module_dependencies("${NAP_MODULES}")

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
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/cmake)

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
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SHADERS data/shaders/*.frag data/shaders/*.vert)

# Create IDE groups
create_hierarchical_source_groups_for_files("${SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")
create_hierarchical_source_groups_for_files("${HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Headers")
create_hierarchical_source_groups_for_files("${SHADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Shaders")

# Add executable, include plist files if found
file(GLOB_RECURSE INFO_PLIST macos/*.plist)
if(APPLE AND INFO_PLIST)
    create_hierarchical_source_groups_for_files("${INFO_PLIST}" ${CMAKE_CURRENT_SOURCE_DIR}/macos "PropertyList")
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS} ${INFO_PLIST})
    copy_files_to_bin(${INFO_PLIST})
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
endif()

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
    if (${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        # Set project as startup project in Visual Studio
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Set our project output directory
set_output_directories()

# Pull in NAP core
find_package(napcore REQUIRED)
find_package(naprtti REQUIRED)
find_package(naputility REQUIRED)

# Pull in a project module if it exists
add_project_module()

# Find each NAP module
foreach(NAP_MODULE ${NAP_MODULES})
    find_nap_module(${NAP_MODULE})
endforeach()

target_link_libraries(${PROJECT_NAME} napcore naprtti naputility ${NAP_MODULES} ${PYTHON_LIBRARIES} ${SDL2_LIBRARY})

# Include any extra project CMake logic
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/project_extra.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/project_extra.cmake)
endif()

# Run FBX converter post-build
export_fbx(${CMAKE_SOURCE_DIR}/data/)

# Copy path mapping
deploy_single_path_mapping(${CMAKE_SOURCE_DIR})

# Package into packaged project on *nix
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/project.json DESTINATION .)
install(FILES ${NAP_ROOT}/cmake/project_creator/NAP.txt DESTINATION .)
if(NOT WIN32)
    # Set RPATH to search in ./lib
    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@executable_path/lib/")
        # Install Information Propertly List file if present
        if(INFO_PLIST)
            install(FILES ${INFO_PLIST} DESTINATION .)
        endif()
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib/")
    endif()
    install(TARGETS ${PROJECT_NAME} DESTINATION .)
endif()

# Package napkin if we're doing a build from against released NAP or we're packaging a project with napkin
if(NOT DEFINED PACKAGE_NAPKIN OR PACKAGE_NAPKIN)
    include(${CMAKE_CURRENT_LIST_DIR}/install_napkin_with_project.cmake)
endif()

# Package redistributable help on Windows
if(WIN32)
    INSTALL(FILES ${NAP_ROOT}/tools/platform/Microsoft\ Visual\ C++\ Redistributable\ Help.txt DESTINATION .)
endif()
