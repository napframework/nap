if(WIN32)
    if(NOT DEFINED LIBSNDFILE_LIB_DIR)
        find_package(libsndfile REQUIRED)
    endif()

    if(NOT DEFINED PORTAUDIO_LIB_DIR)
        find_package(portaudio REQUIRED)
    endif()

    if(NOT DEFINED LIBMPG123_LIB_DIR)
        find_package(libmpg123 REQUIRED)
    endif()

    # Copy audio DLLs to project build directory
    set(FILES_TO_COPY
        ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
        ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll
        ${LIBMPG123_LIB_DIR}/libmpg123.dll
    )

    copy_files_to_bin(${FILES_TO_COPY})
endif()