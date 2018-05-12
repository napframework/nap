cmake_minimum_required(VERSION 3.5)
# Get our module name
if (IMPORTING_PROJECT_MODULE)
    set(MODULE_NAME "mod_${PROJECT_NAME}")    
else()
    get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
endif(IMPORTING_PROJECT_MODULE) 

project(${MODULE_NAME})

# Enforce GCC on Linux for now
if(UNIX AND NOT APPLE)
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "NAP only currently supports GCC on Linux")
    endif()
endif()

# Support building user modules from their own (build system) project
if(NOT MODULE_INTO_PROJ)
    get_filename_component(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ REALPATH)
    message(STATUS "Using NAP root: ${NAP_ROOT}")
    get_filename_component(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty REALPATH)
    message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

    include(${NAP_ROOT}/cmake/targetarch.cmake)
    target_architecture(ARCH)

    include(${NAP_ROOT}/cmake/dist_shared.cmake)

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
endif(NOT MODULE_INTO_PROJ)

# Fetch our module dependencies
module_json_in_directory_to_cmake(${CMAKE_CURRENT_SOURCE_DIR})
fetch_module_dependencies("${DEPENDENT_NAP_MODULES}")

include_directories(${NAP_ROOT}/include/)

# Add source
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)

# Create IDE groups
create_hierarchical_source_groups_for_files("${SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")
create_hierarchical_source_groups_for_files("${HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Headers")

# Compile target as shared lib
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
if(IMPORTING_PROJECT_MODULE)
    set(MODULE_FOLDER_NAME ProjectModule)
else()
    set(MODULE_FOLDER_NAME UserModules)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${MODULE_FOLDER_NAME})    

# Add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
    if (NOT MODULE_INTO_PROJ AND ${CMAKE_VERSION} VERSION_GREATER "3.6.0")
        # Set project as startup project in Visual Studio
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
endif()

# Pull in NAP core
if (NOT MODULE_INTO_PROJ)
    include(${CMAKE_CURRENT_LIST_DIR}/napcore.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/naprtti.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/naputility.cmake)
endif()

target_link_libraries(${PROJECT_NAME} napcore naprtti naputility RTTR::Core ${PYTHON_LIBRARIES} ${SDL2_LIBRARY})
if (MODULE_INTO_PROJ)
    target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
endif()

# Bring in any additional module requirements
set(MODULE_EXTRA_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
    include (${MODULE_EXTRA_CMAKE_PATH})
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Find each NAP module
foreach(NAP_MODULE ${NAP_MODULES})
    find_nap_module(${NAP_MODULE})
endforeach()
target_link_libraries(${PROJECT_NAME} ${NAP_MODULES})
unset(NAP_MODULES)

# Set our module output directory
set_module_output_directories()

# On Windows copy over module.json post-build
if(WIN32)
    add_custom_command(
        TARGET ${MODULE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/module.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/${MODULE_NAME}.json
        )
endif()

# On macOS & Linux install module into packaged project
if (NOT WIN32)
    install(FILES $<TARGET_FILE:${PROJECT_NAME}> DESTINATION lib CONFIGURATIONS Release)
    if(UNIX)
        install(FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib${PROJECT_NAME}.json DESTINATION lib CONFIGURATIONS Release)
    else()
        install(FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.json DESTINATION lib CONFIGURATIONS Release)
    endif()

    # On Linux set our user modules tp use their directory for RPATH when installing
    if(NOT APPLE)
        install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so\")
                      execute_process(COMMAND patchelf 
                                              --set-rpath 
                                              $ORIGIN/.
                                              ${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so)")
    endif()
endif()
