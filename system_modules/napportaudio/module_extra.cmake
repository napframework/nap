add_source_dir("service" "src/audio/service")

add_subdirectory(thirdparty/portaudio)
target_link_import_library(${PROJECT_NAME} portaudio)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} atomic)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

