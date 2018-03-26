project(mod_napartnet)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_package(artnet REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ARTNET_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

set(DEPENDENT_NAP_MODULES napcore mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} artnet)

if (WIN32)
    set(FILES_TO_COPY
            ${ARTNET_LIBS_DIR}/libartnet.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
elseif(UNIX AND NOT APPLE)
    set(ARTNET_LIB_FILE ${ARTNET_LIBS_DIR}/libartnet.so)

    # HACK, somehow on some systems, libartnet.so gets linked with libartnet.so.1 because it already exists
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${ARTNET_LIB_FILE}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1"
            COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1")
endif()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/libartnet/bin")
endif()
package_module()

# Package libartnet into platform release
install(FILES ${THIRDPARTY_DIR}/libartnet/AUTHORS DESTINATION thirdparty/libartnet)
install(FILES ${THIRDPARTY_DIR}/libartnet/COPYING DESTINATION thirdparty/libartnet)
install(FILES ${THIRDPARTY_DIR}/libartnet/README DESTINATION thirdparty/libartnet)
install(FILES $<TARGET_FILE:artnet> DESTINATION thirdparty/libartnet/bin)
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
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napartnet/lib/${build_type}/libmod_napartnet.dylib
                                      ERROR_QUIET
                                      )")
    endforeach()
endif()
