if(NOT DEFINED NAP_BUILD_CONTEXT)
    set(NAP_BUILD_CONTEXT "framework_release")
endif()

# Fetch our module dependencies
module_json_in_directory_to_cmake(${CMAKE_CURRENT_SOURCE_DIR})

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cmake_find_modules)
endif()

# Bring in any additional module logic (pre-target definition)
set(MODULE_EXTRA_PRE_TARGET_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module_extra_pre_target.cmake)
if (EXISTS ${MODULE_EXTRA_PRE_TARGET_CMAKE_PATH})
    unset(SKIP_MODULE)
    include(${MODULE_EXTRA_PRE_TARGET_CMAKE_PATH})
    if(SKIP_MODULE)
        message(STATUS "Skipping target: ${PROJECT_NAME}")
        return()
    endif()
endif()

# Add source
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h src/*.hpp)

# Compile target as shared lib
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Set IDE folder
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path STEM LAST_ONLY parent_dir)
if(IMPORTING_APP_MODULE)
    set(module_folder_name AppModules)
    # Ensure not still declared for dependent module search
elseif(parent_dir MATCHES "^system_modules$")
    set(module_folder_name SystemModules)
else()
    set(module_folder_name Modules)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${module_folder_name})

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Pull in NAP core
if (NOT MODULE_INTO_PARENT)
    find_package(napcore REQUIRED)
    find_package(naprtti REQUIRED)
    find_package(naputility REQUIRED)
endif()

find_rttr()
target_link_libraries(${PROJECT_NAME} napcore naprtti naputility RTTR::Core)
if (NAP_ENABLE_PYTHON)
    if (MODULE_INTO_PARENT)
        target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
    endif()
endif()

# Bring in any additional module logic
set(MODULE_EXTRA_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
    include(${MODULE_EXTRA_CMAKE_PATH})
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Find each NAP module for Framework Release
if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    foreach(NAP_MODULE ${DEEP_DEPENDENT_NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
endif()
target_link_libraries(${PROJECT_NAME} ${DEEP_DEPENDENT_NAP_MODULES})

# Add extra RPATHs from module library search paths
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE "")
    set(MACOS_EXTRA_RPATH_DEBUG "")
    foreach(rpath ${DEEP_DEPENDENT_RPATHS})
        string(REPLACE "{BUILD_TYPE}" "Release" release_rpath "${rpath}")
        list(APPEND MACOS_EXTRA_RPATH_RELEASE ${release_rpath})
        string(REPLACE "{BUILD_TYPE}" "Debug" debug_rpath "${rpath}")
        list(APPEND MACOS_EXTRA_RPATH_DEBUG ${debug_rpath})
    endforeach()
elseif(UNIX)
    set(LINUX_EXTRA_RPATH "")
    foreach(rpath ${DEEP_DEPENDENT_RPATHS})
        string(REPLACE "{BUILD_TYPE}" ${CMAKE_BUILD_TYPE} rpath "${rpath}")
        list(APPEND LINUX_EXTRA_RPATH ${rpath})
    endforeach()
endif()
