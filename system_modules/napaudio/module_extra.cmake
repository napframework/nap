# Set this flag in order to link libsndfile and mpg123 and build support for reading audio files
set(NAP_AUDIOFILE_SUPPORT ON)

# Add sources to target
if (NAP_AUDIOFILE_SUPPORT)
    # Add compile definition to enable audio file support
    target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_AUDIOFILE_SUPPORT)
else()
    # Filter out sources for audio file functionality
    set(AUDIO_FILE_SUPPORT_FILTER ".*audiofileutils.*" ".*audiofileresource.*")
endif()

add_source_dir("core" "src/audio/core")
add_source_dir("node" "src/audio/node")
add_source_dir("component" "src/audio/component")
add_source_dir("service" "src/audio/service")
add_source_dir("resource" "src/audio/resource" ${AUDIO_FILE_SUPPORT_FILTER})
add_source_dir("utility" "src/audio/utility" ${AUDIO_FILE_SUPPORT_FILTER})

# Add thirdparty libraries for audio file support
if (NAP_AUDIOFILE_SUPPORT)
    add_subdirectory(thirdparty/libsndfile)
    add_subdirectory(thirdparty/mpg123)

    target_link_import_library(${PROJECT_NAME} libsndfile)
    target_link_import_library(${PROJECT_NAME} mpg123)

    if (APPLE)
        list(APPEND LIBRARIES "-framework CoreFoundation")
    elseif(UNIX)
        list(APPEND LIBRARIES atomic)
    endif()
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

    add_license(libsndfile ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsndfile/source/COPYING)
    add_license(mpg123 ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mpg123/source/COPYING)
endif()