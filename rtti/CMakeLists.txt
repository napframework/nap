include(${NAP_ROOT}/cmake/packaging_macros.cmake)
project(naprtti)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)

if (WIN32)
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc64/install/cmake")
    else()
        set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc32/install/cmake")
    endif()
elseif(APPLE)
    find_path(
            RTTR_DIR
            NAMES rttr-config.cmake
            HINTS
            ${THIRDPARTY_DIR}/rttr/xcode/install/cmake
    )
else()
    find_path(
            RTTR_DIR
            NAMES rttr-config.cmake
            HINTS
            ${THIRDPARTY_DIR}/rttr/install/cmake
            ${THIRDPARTY_DIR}/rttr/linux/install/cmake
    )
endif()

find_package(RTTR CONFIG REQUIRED Core)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

# Let find_python find our prepackaged Python in thirdparty
find_python_in_thirdparty()
set(pybind11_DIR "${THIRDPARTY_DIR}/pybind11/install/share/cmake/pybind11")
find_package(pybind11 REQUIRED) 
# Ensure we've found Python3
if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
    message(FATAL_ERROR "Python found in ${PYTHON_PREFIX} is not v3")
endif()
# Set up our Python virtual environment configuration so that we can find our Python libs later
if(APPLE)
    set(THIRDPARTY_PYTHON_PREFIX ${THIRDPARTY_DIR}/python/osx/install)
    configure_file(${THIRDPARTY_DIR}/python/pyvenv.cfg.in
                   ${THIRDPARTY_DIR}/python/osx/pyvenv.cfg
                   @ONLY)
endif()

add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME} NAP_SHARED_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} pybind11::embed)
target_link_libraries(${PROJECT_NAME} RTTR::Core)
target_link_libraries(${PROJECT_NAME} naputility)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

if (WIN32)
    # Copy over dlls
    copy_files_to_bin($<TARGET_FILE:RTTR::Core>)
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/*.dll)
    copy_files_to_bin(${PYTHON_DLLS})    
elseif(APPLE)
    # Copy over RTTR for macOS
    # TODO Update this to be an RPATH change?
    copy_lib_to_lib_dir("RTTR::Core")
endif()

if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id 
                               @rpath/librttr_core.0.9.6.dylib 
                               ${THIRDPARTY_DIR}/rttr/xcode/install/bin/librttr_core.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core")

    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id 
                               @rpath/librttr_core_d.0.9.6.dylib 
                               ${THIRDPARTY_DIR}/rttr/xcode/install/bin/librttr_core_d.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core_d")    
endif()

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/$<CONFIG>
                                    LIBRARY DESTINATION lib/$<CONFIG>
                                    ARCHIVE DESTINATION lib/$<CONFIG>)

    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/$<CONFIG>)  
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE})   
endif()

install(DIRECTORY "src/rtti/" DESTINATION "include/rtti/"
        FILES_MATCHING PATTERN "*.h")

# Package dependent includes 
# TODO discuss moving elsewhere, the idea here was that the object using the dependencies installs them

# Package pybind into platform release
install(DIRECTORY ${THIRDPARTY_DIR}/pybind11/install/ DESTINATION thirdparty/pybind11)
install(FILES ${THIRDPARTY_DIR}/pybind11/LICENSE DESTINATION thirdparty/pybind11)

# Package RTTR into platform release
if (MSVC)
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/msvc64/install/" DESTINATION "thirdparty/rttr")
elseif(APPLE)
    set(BUILT_RPATH "@loader_path/../../thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};@loader_path/../../thirdparty/python/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/xcode/install/" DESTINATION "thirdparty/rttr")

    foreach(build_conf Release Debug)
        macos_replace_single_install_name_link_install_time("Python"
                                                            ${CMAKE_INSTALL_PREFIX}/lib/${build_conf}/lib${PROJECT_NAME}.dylib 
                                                            "@rpath"
                                                            )
    endforeach()
elseif(UNIX)
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/linux/install/" DESTINATION "thirdparty/rttr")
endif()
