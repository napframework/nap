cmake_minimum_required(VERSION 3.18.4)
project(mod_napartnet)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(artnet REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ARTNET_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} artnet Ws2_32.lib)
else()
  target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} artnet)
endif()

if(WIN32)
    # Copy artnet DLL to build directory
    if(NOT TARGET artnet)
        find_package(artnet REQUIRED)
    endif()
    copy_artnet_dll()
elseif(UNIX AND NOT APPLE)
    set(ARTNET_LIB_FILE ${ARTNET_LIBS_DIR}/libartnet.so)

    # HACK, somehow on some systems, libartnet.so gets linked with libartnet.so.1 because it already exists
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${ARTNET_LIB_FILE}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1"
            COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1")
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/libartnet/bin)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/libartnet/bin)
endif()
package_module()

# Package libartnet into platform release
install(FILES ${THIRDPARTY_DIR}/libartnet/AUTHORS DESTINATION thirdparty/libartnet)
install(FILES ${THIRDPARTY_DIR}/libartnet/COPYING DESTINATION thirdparty/libartnet)
install(FILES ${THIRDPARTY_DIR}/libartnet/README DESTINATION thirdparty/libartnet)
if(ANDROID)
    install(FILES $<TARGET_FILE:artnet> DESTINATION thirdparty/libartnet/bin/${ANDROID_ABI})
else()
    install(FILES $<TARGET_FILE:artnet> DESTINATION thirdparty/libartnet/bin)
endif()
install(DIRECTORY ${ARTNET_DIR}/artnet DESTINATION thirdparty/libartnet/include)
if(WIN32)
    install(FILES $<TARGET_FILE_DIR:artnet>/libartnet.lib DESTINATION thirdparty/libartnet/bin)
elseif(APPLE)
    # Ensure our libArtnet install name id is set properly, this is really for intall into packaging
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libArtnet.dylib $<TARGET_FILE:artnet>
                       COMMENT "Setting install name for libartnet")

    foreach(build_type Release Debug)
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                              -add_rpath
                                              @loader_path/../../../../thirdparty/libartnet/bin
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napartnet/lib/${build_type}/mod_napartnet.dylib
                                      ERROR_QUIET
                                      )")
    endforeach()
endif()
