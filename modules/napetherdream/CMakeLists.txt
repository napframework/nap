project(mod_napetherdream)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)
add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")

find_package(etherdream REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ETHERDREAM_INCLUDE_DIRS})

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} etherdreamlib)

if (WIN32)
    set(FILES_TO_COPY
            ${ETHERDREAM_LIBS_DIR}/EtherDream.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()

# Package into platform release
if(UNIX)
    set(EXTRA_RPATH ../../../../thirdparty/etherdream/bin)
endif()
package_module()

# Package etherdreamlib into platform release
install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION thirdparty/etherdream/bin)
if(APPLE)
    # Ensure our etherdream install name id is set properly, this is really for intall into packaging
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libEtherDream.dylib $<TARGET_FILE:etherdreamlib>
                       COMMENT "Setting install name for etherdream")


    foreach(build_type Release Debug)
        install(CODE "execute_process(COMMAND ${CMAKE_NAME_INSTALL_TOOL} 
                                              -change
                                              /usr/local/lib/libEtherDream.dylib
                                              @rpath/libEtherDream.dylib
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napetherdream/lib/${build_type}/libmod_napetherdream.dylib 
                                              )")
        ensure_macos_file_has_rpath_at_install("${CMAKE_INSTALL_PREFIX}/modules/mod_napetherdream/lib/${build_type}/libmod_napetherdream.dylib"
                                               "@loader_path/../../../../thirdparty/etherdream/bin")
    endforeach()
elseif(UNIX)
    # Add Etherdream to module RPATH
    get_target_property(BUILT_RPATH ${PROJECT_NAME} INSTALL_RPATH)
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/../../../../thirdparty/etherdream/bin")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
endif()
