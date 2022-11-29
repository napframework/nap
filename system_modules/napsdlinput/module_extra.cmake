if(NAP_BUILD_CONTEXT MATCHES "source")
    if(WIN32)
        if(MSVC)
            set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/msvc/x86_64/lib/x64)
            set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/msvc/x86_64)
        else()
            set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
            set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
        endif()
    elseif(APPLE)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/macos/x86_64/lib)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/macos/x86_64)
    elseif(UNIX)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/linux/${ARCH}/lib)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/linux/${ARCH})
    endif()
    find_package(SDL2 REQUIRED)

    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
else()
    if(NOT TARGET SDL2)
        set(ENV{SDL2DIR} ${NAP_ROOT}/thirdparty/SDL2/)
        find_package(SDL2 REQUIRED)
    endif()
    add_include_to_interface_target(mod_napsdlinput ${SDL2_INCLUDE_DIR})

    if (WIN32)
        # Copy over DLL post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $ENV{SDL2DIR}/lib/SDL2.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
       )
    endif()
endif()
