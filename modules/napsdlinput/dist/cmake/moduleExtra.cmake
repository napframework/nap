# Including in both napsdlinput and napsdlwindow in case one is used without the other (eg. audio-only apps)
if(NOT TARGET SDL2)
    set(ENV{SDL2DIR} ${NAP_ROOT}/thirdparty/SDL2/)
    find_package(SDL2 REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})

if (WIN32)
    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $ENV{SDL2DIR}/lib/SDL2.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
   )
endif()