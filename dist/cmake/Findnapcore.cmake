if (WIN32)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/napcore.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_DEBUG ${NAPCORE_LIBS_DIR}/Debug/napcore.lib)
    set(NAPCORE_LIBS_RELEASE ${NAPCORE_LIBS_DIR}/Release/napcore.lib)
elseif (APPLE)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/napcore.dylib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE ${NAPCORE_LIBS_DIR}/Release/napcore.dylib)
    set(NAPCORE_LIBS_DEBUG ${NAPCORE_LIBS_DIR}/Debug/napcore.dylib)
elseif (ANDROID)
    find_path(
        NAPCORE_LIBS_DIR
        NO_CMAKE_FIND_ROOT_PATH
        NAMES Release/${ANDROID_ABI}/napcore.so
        HINTS ${NAP_ROOT}/lib/
    )
    set(NAPCORE_LIBS_RELEASE ${NAPCORE_LIBS_DIR}/Release/${ANDROID_ABI}/napcore.so)
    set(NAPCORE_LIBS_DEBUG ${NAPCORE_LIBS_DIR}/Debug/${ANDROID_ABI}/napcore.so)
elseif (UNIX)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Debug/napcore.so
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE ${NAPCORE_LIBS_DIR}/Release/napcore.so)
    set(NAPCORE_LIBS_DEBUG ${NAPCORE_LIBS_DIR}/Debug/napcore.so)
endif()

# Setup as interface library
add_library(napcore INTERFACE)
target_link_libraries(napcore INTERFACE debug ${NAPCORE_LIBS_DEBUG})
target_link_libraries(napcore INTERFACE optimized ${NAPCORE_LIBS_RELEASE})
set_target_properties(napcore PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/include)

# Show headers in IDE
file(GLOB core_headers ${CMAKE_CURRENT_LIST_DIR}/../include/nap/*.h)
target_sources(napcore INTERFACE ${core_headers})
source_group(NAP\\Core FILES ${core_headers})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(napcore REQUIRED_VARS NAPCORE_LIBS_DIR)

if (WIN32)
    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${NAPCORE_LIBS_DIR}/$<CONFIG>/napcore.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )

    # Copy PDB post-build, if we have them
    if(EXISTS ${NAPCORE_LIBS_DIR}/Debug/napcore.pdb)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NAPCORE_LIBS_DIR}/$<CONFIG>/napcore.pdb $<TARGET_FILE_DIR:${PROJECT_NAME}>/            
            )
    endif()
endif()

# Install into packaged project for macOS/Linux
if(NOT WIN32 AND NOT ANDROID)
    install(FILES ${NAPCORE_LIBS_RELEASE} DESTINATION lib CONFIGURATIONS Release)    

    # On Linux use lib directory for RPATH
    if(NOT APPLE)
        install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/napcore.so\")
                      execute_process(COMMAND patchelf
                                              --set-rpath 
                                              $ORIGIN/.
                                              ${CMAKE_INSTALL_PREFIX}/lib/napcore.so)
                      ")
    endif()      
endif()
