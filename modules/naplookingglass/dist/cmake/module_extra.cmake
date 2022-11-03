include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET holoplaycore)
    find_package(holoplaycore REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS holoplaycore)

add_include_to_interface_target(mod_naplookingglass ${HOLOPLAYCORE_INCLUDE_DIR})

if(WIN32)
    # Add post-build step to set copy holoplaycore to bin
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
        -E copy
        $<TARGET_FILE:holoplaycore>
        $<TARGET_FILE_DIR:${PROJECT_NAME}> 
    )
elseif(APPLE)
    # Install portaudio lib into packaged app
    file(GLOB HOLOPLAYCORE_DYLIBS ${THIRDPARTY_DIR}/holoplaycore/bin/macos/libHoloPlayCore.dylib)
    install(FILES ${HOLOPLAYCORE_DYLIBS} DESTINATION lib)
endif()

# Install holoplaycore license into packaged project
install(FILES ${HOLOPLAYCORE_DIR}/LICENSE.txt DESTINATION licenses/holoplaycore)