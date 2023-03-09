if(NOT DEFINED NAP_BUILD_CONTEXT)
    set(NAP_BUILD_CONTEXT "framework_release")
    get_filename_component(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ REALPATH)
    message(STATUS "Using NAP root: ${NAP_ROOT}")
    get_filename_component(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty REALPATH)
    message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

    # Set our install prefix for app packaging
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin_package)

    include(${NAP_ROOT}/cmake/macros_and_functions.cmake)

    bootstrap_environment()

    # Set our default build type if we haven't specified one (Linux)
    set_default_build_type()
endif()

unset(PACKAGING_INCLUDE_ONLY_WITH_NAIVI_APPS)
unset(PACKAGING_FORCE_PACKAGE_USER_APP)
unset(APP_CUSTOM_IDE_FOLDER)

# Get our modules list from app.json
app_json_to_cmake()

# Bring in any additional app logic (pre-target definition)
set(APP_EXTRA_PRE_TARGET_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/app_extra_pre_target.cmake)
if(EXISTS ${APP_EXTRA_PRE_TARGET_CMAKE_PATH})
    unset(SKIP_APP)
    include(${APP_EXTRA_PRE_TARGET_CMAKE_PATH})
    if(SKIP_APP)
        return()
    endif()
endif()

include_directories(${NAP_ROOT}/include/)

# Add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SHADERS data/shaders/*.frag data/shaders/*.vert data/shaders/*.comp)

# Create IDE groups
create_hierarchical_source_groups_for_files("${SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")
create_hierarchical_source_groups_for_files("${HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Headers")
create_hierarchical_source_groups_for_files("${SHADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Shaders")

# Declare target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

# Pull in NAP core
if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    find_package(napcore REQUIRED)
    find_package(naprtti REQUIRED)
    find_package(naputility REQUIRED)
endif()

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path STEM LAST_ONLY parent_dir)

if(NAP_BUILD_CONTEXT MATCHES "source")
    if(DEFINED APP_CUSTOM_IDE_FOLDER)
        set(app_folder_name ${APP_CUSTOM_IDE_FOLDER})
    elseif(parent_dir MATCHES "^apps$")
        set(app_folder_name Apps)
    else()
        set(app_folder_name Demos)
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${app_folder_name})
endif()

# Pull in a app module if it exists
add_app_module()

if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    # Fetch our (deep) module dependencies
    fetch_module_dependencies("${NAP_MODULES}")

    # Find each NAP module
    foreach(NAP_MODULE ${NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
endif()

target_link_libraries(${PROJECT_NAME} napcore naprtti naputility ${NAP_MODULES} ${SDL2_LIBRARY})
if(NAP_ENABLE_PYTHON)
    target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})
endif()

# Include any extra app CMake logic
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake)
endif()

# Run FBX converter post-build
export_fbx()

if(NAP_BUILD_CONTEXT MATCHES "source")
    add_dependencies(${PROJECT_NAME} fbxconverter)
endif()

# macOS specifics
if(APPLE)
    # Add the runtime paths for RTTR
    if(NAP_BUILD_CONTEXT MATCHES "source")
        macos_add_rttr_rpath()
    endif()

    # Add plist files if found
    file(GLOB_RECURSE INFO_PLIST macos/*.plist)
    if(INFO_PLIST)
        create_hierarchical_source_groups_for_files("${INFO_PLIST}" ${CMAKE_CURRENT_SOURCE_DIR}/macos "PropertyList")
        target_sources(${PROJECT_NAME} PUBLIC ${INFO_PLIST})
        copy_files_to_bin(${INFO_PLIST})
    endif()
endif()

# Copy path mapping
if(NOT NAP_PACKAGED_BUILD)
    deploy_single_path_mapping(${CMAKE_CURRENT_SOURCE_DIR} ${NAP_BUILD_CONTEXT})
endif()

if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    # Set our app output directory
    set_framework_release_output_directories()

    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
        # Set app as startup project in Visual Studio
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()

    # Deploy into packaged app
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION .)
    install(FILES ${CMAKE_SOURCE_DIR}/app.json DESTINATION .)
    install(FILES ${NAP_ROOT}/cmake/app_creator/NAP.txt DESTINATION .)

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

    # Package Napkin if we're doing a build from against released NAP or we're packaging a app with napkin
    if(NOT DEFINED PACKAGE_NAPKIN OR PACKAGE_NAPKIN)
        include(${CMAKE_CURRENT_LIST_DIR}/install_napkin_with_app.cmake)
    endif()

    # Package redistributable help on Windows
    if(WIN32)
        install(FILES ${NAP_ROOT}/tools/buildsystem/Microsoft\ Visual\ C++\ Redistributable\ Help.txt DESTINATION .)
    endif()

    # Provide Gatekeeper unquarantine script on macOS
    if(APPLE)
        set(app_template_dir "${NAP_ROOT}/cmake/app_creator/template")
        install(PROGRAMS "${app_template_dir}/Unquarantine App.command" DESTINATION .)
        install(FILES "${app_template_dir}/Help launching on macOS.txt" DESTINATION .)
    endif()
else()
    if(NAP_PACKAGED_BUILD)
        # Package into framework release
        package_app_into_framework_release(${parent_dir}/${PROJECT_NAME})
        # Don't build the apps while doing a framework packaging build unless explicitly requested
        set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endif()
