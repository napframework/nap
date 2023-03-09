if(NOT DEFINED NAP_BUILD_CONTEXT)
    set(NAP_BUILD_CONTEXT "framework_release")
endif()

# Support building user modules from their own (build system) app (in Framework Release context)
if(NOT MODULE_INTO_PARENT)
    get_filename_component(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ REALPATH)
    message(STATUS "Using NAP root: ${NAP_ROOT}")
    get_filename_component(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty REALPATH)
    message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

    include(${NAP_ROOT}/cmake/macros_and_functions.cmake)

    bootstrap_environment()

    # Set our default build type if we haven't specified one (Linux)
    set_default_build_type()
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
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)

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

# Don't build app modules when packaging release
if(IMPORTING_APP_MODULE AND NAP_PACKAGED_BUILD AND NOT BUILD_APPS)
    set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
unset(IMPORTING_APP_MODULE)

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
target_link_libraries(${PROJECT_NAME} napcore naprtti naputility RTTR::Core ${SDL2_LIBRARY})
if (NAP_ENABLE_PYTHON)
    target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})
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

if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    include_directories(${NAP_ROOT}/include)

    # Create IDE groups
    create_hierarchical_source_groups_for_files("${SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")
    create_hierarchical_source_groups_for_files("${HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Headers")

    if (WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
        if (NOT MODULE_INTO_PARENT AND ${CMAKE_VERSION} VERSION_GREATER "3.6.0")
            # Set module as startup project in Visual Studio
            set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
        endif()
    endif()

    # Set our module output directory
    set_module_output_directories()

    # Install module data into packaged app
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION modules/${MODULE_NAME} CONFIGURATIONS Release)
    endif()

    # On macOS & Linux install module into packaged app
    if (NOT WIN32)
        install(FILES $<TARGET_FILE:${PROJECT_NAME}> DESTINATION lib CONFIGURATIONS Release)
        install(FILES $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.json DESTINATION lib CONFIGURATIONS Release)

        # On Linux set our user modules to use their directory for RPATH when installing
        if(NOT APPLE)
            install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/${MODULE_NAME}.so\")
                          execute_process(COMMAND patchelf
                                                  --set-rpath
                                                  $ORIGIN/.
                                                  ${CMAKE_INSTALL_PREFIX}/lib/${MODULE_NAME}.so
                                          RESULT_VARIABLE EXIT_CODE)
                          if(NOT \${EXIT_CODE} EQUAL 0)
                              message(FATAL_ERROR \"Failed to set RPATH on ${MODULE_NAME} using patchelf. Is patchelf installed?\")
                          endif()")
        endif()
    endif()
else()
    if(parent_dir MATCHES "^system_modules$")
        # Package system modules into platform release
        package_system_module_into_framework_release()
    elseif(parent_dir MATCHES "^modules")
        # Package additional modules into platform release
        if (NAP_PACKAGED_BUILD)
            package_module_into_framework_release()
            # Don't build the apps while doing a framework packaging build unless explicitly requested
            set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif ()
    endif()
endif()
