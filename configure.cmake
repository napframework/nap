if(MSVC OR APPLE)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})

        set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})

        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})

    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

else()
    set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})

    # Override binary directories
    set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
    file(MAKE_DIRECTORY ${BIN_DIR})
    # Separate our lib outputs for packaging and non packaging (due to differing behaviour in core)
    if(DEFINED NAP_PACKAGED_BUILD)
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packagingLib/${BUILD_CONF})
    else()
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
    endif()
    file(MAKE_DIRECTORY ${LIB_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
endif()

macro(export_fbx SRCDIR)
    if (MSVC OR APPLE)
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>")
    else()
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH}")
    endif()

    set(FBXCONV_DIR "${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}")

    # Set project data out path
    set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data/${PROJECT_NAME}")

    # Ensure data output directory for project exists
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
                       COMMENT "Ensure project output directory exists for fbxconverter")

    # Do the export
    if (MSVC)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND set "PATH=${FBXCONV_DIR}/..;%PATH%"
                           COMMAND "${FBXCONV_DIR}/fbxconverter" -o ${OUTDIR} "${SRCDIR}/*.fbx"
                           COMMENT "Export FBX in '${SRCDIR}'")
    else()
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND "${FBXCONV_DIR}/fbxconverter" -o ${OUTDIR} "${SRCDIR}/*.fbx"
                           COMMENT "Export FBX in '${SRCDIR}'")
    endif()
endmacro()

macro(export_fbx_in_place SRCDIR)
    if (MSVC OR APPLE)
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>")
    else()
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH}")
    endif()

    set(FBXCONV_DIR "${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}")

    # Do the export
    if (MSVC)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND set "PATH=${FBXCONV_DIR}/..;%PATH%"
                           COMMAND "${FBXCONV_DIR}/fbxconverter" -o ${SRCDIR} "${SRCDIR}/*.fbx"
                           COMMENT "Export FBX in '${SRCDIR}'")
    else()
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND "${FBXCONV_DIR}/fbxconverter" -o ${SRCDIR} "${SRCDIR}/*.fbx"
                           COMMENT "Export FBX in '${SRCDIR}'")
    endif()
endmacro()

macro(copy_dir_to_bin SRCDIR DSTDIR)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRCDIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${DSTDIR}"
                       COMMENT "Copy dir '${SRCDIR}' -> '${DSTDIR}'")
endmacro()

macro(copy_files_to_bin)
    foreach(F ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy "${F}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()
endmacro()

macro(copy_lib_to_lib_dir LIB_NAME)
    if (MSVC OR APPLE)
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>")
    else()
        set(BUILD_CONF "${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH}")
    endif()
    set(lib_path ${CMAKE_SOURCE_DIR}/lib/${BUILD_CONF})

    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${LIB_NAME}> ${lib_path}
                       COMMENT "Copy ${IN_FILE} -> ${OUT_DIR}")
endmacro()

macro(copy_base_windows_graphics_dlls)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
        ${THIRDPARTY_DIR}/sdl2/msvc/lib/x64/SDL2.dll
        ${THIRDPARTY_DIR}/glew/msvc/bin/Release/x64/glew32.dll
        )
    copy_files_to_bin(${FILES_TO_COPY})
endmacro()

macro(copy_windows_ffmpeg_dlls)
    file(GLOB FFMPEGDLLS ${THIRDPARTY_DIR}/ffmpeg/bin/*.dll)
    copy_files_to_bin(${FFMPEGDLLS})
endmacro()

# Copy all of our Windows DLLs that have build in bin into project dir.  See notes in windowsdllcopier.cmake.
macro(bulk_copy_windows_dlls_to_bin)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -DCOPIER_IN_PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>/../*.dll 
                               -DCOPIER_OUTPUT_PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>/ 
                               -P 
                               ${CMAKE_SOURCE_DIR}/cmake/windowsdllcopier.cmake
                       COMMENT "Bulk copying Windows library DLLs to ${PROJECT_NAME}'s bin dir")
endmacro()

# Helper function to filter out platform-specific files
# The function outputs the following new variables with the platform-specific sources:
# - WIN32_SOURCES
# - OSX_SOURCES
# - LINUX_SOURCES
function(filter_platform_specific_files UNFILTERED_SOURCES)
    set(LOCAL_WIN32_SOURCES)
    set(LOCAL_OSX_SOURCES)
    set(LOCAL_LINUX_SOURCES)
    foreach(TMP_PATH ${${UNFILTERED_SOURCES}})
        string(FIND ${TMP_PATH} "/win32/" WIN32_EXCLUDE_DIR_FOUND)
        if(NOT ${WIN32_EXCLUDE_DIR_FOUND} EQUAL -1)
            message(STATUS "Win32 File: " ${TMP_PATH})
            list(APPEND LOCAL_WIN32_SOURCES ${TMP_PATH})
        else()
            string(FIND ${TMP_PATH} "/osx/" OSX_EXCLUDE_DIR_FOUND)
            if(NOT ${OSX_EXCLUDE_DIR_FOUND} EQUAL -1)
                list(APPEND LOCAL_OSX_SOURCES ${TMP_PATH})
            else()
                string(FIND ${TMP_PATH} "/linux/" LINUX_EXCLUDE_DIR_FOUND)
                if(NOT ${LINUX_EXCLUDE_DIR_FOUND} EQUAL -1)
                    list(APPEND LOCAL_LINUX_SOURCES ${TMP_PATH})
                endif()
            endif()
        endif()
    endforeach(TMP_PATH)

    set(WIN32_SOURCES ${LOCAL_WIN32_SOURCES} PARENT_SCOPE)
    set(OSX_SOURCES ${LOCAL_OSX_SOURCES} PARENT_SCOPE)
    set(LINUX_SOURCES ${LOCAL_LINUX_SOURCES} PARENT_SCOPE)
endfunction()

# Helper macro to add platform-specific files to the correct directory and
# to only compile the platform-specific files that match the current platform
macro(add_platform_specific_files WIN32_SOURCES OSX_SOURCES LINUX_SOURCES)

    # Add to solution folders
    if(MSVC)
        # Sort header and cpps into solution folders for Win32
        foreach(TMP_PATH ${WIN32_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Win32" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Win32" FILES ${TMP_PATH})
            endif()
        endforeach()

        # Sort header and cpps into solution folders for OSX
        foreach(TMP_PATH ${OSX_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\OSX" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\OSX" FILES ${TMP_PATH})
            endif()
        endforeach()

        # Sort header and cpps into solution folders for Linux
        foreach(TMP_PATH ${LINUX_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Linux" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Linux" FILES ${TMP_PATH})
            endif()
        endforeach()
    endif()

    # Unfortunately, there's no clean way to add a file to the solution (for browsing purposes, etc) but
    # exclude it from the build. The hacky way to do it is to treat the file as a 'header' (even though it's not)
    if(NOT WIN32)
        set_source_files_properties(${WIN32_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

    if(NOT APPLE)
        set_source_files_properties(${OSX_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

    if(APPLE OR NOT UNIX)
        set_source_files_properties(${LINUX_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()
endmacro()

# Change our project output directories (when building against NAP source)
macro(set_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/${PROJECT_NAME})
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})
        set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/${PROJECT_NAME})
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    endif()
endmacro()

macro(prepareqt)
    ## First, let cmake know where the Qt library path is, we go from there.
    if(MSVC OR APPLE)
        # Pick up QT_DIR environment variable
        if(DEFINED ENV{QT_DIR})
            set(QTDIR $ENV{QT_DIR})
            message(STATUS "Using QT_DIR environment variable: ${QTDIR}")
        endif()

        # Get QT prefix from homebrew on macOS
        if (APPLE)
            EXEC_PROGRAM(/usr/bin/env
                         ARGS brew --prefix qt5
                         OUTPUT_VARIABLE MACOS_QT_PATH)
        endif()

        # Add possible Qt installation paths to the HINTS section
        # The version probably doesn't have to match exactly (5.8.? is probably fine)
        find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
                  HINTS
                  ${QTDIR}
                  ${NAP_ROOT}/../../Qt/5.9.1/msvc2015_64
                  ${NAP_ROOT}/../../Qt/5.9.2/msvc2015_64
                  ~/Qt/5.8/clang_64
                  ${MACOS_QT_PATH}
                  )
        # Find_package for Qt5 will pick up the Qt installation from CMAKE_PREFIX_PATH
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

        if(NOT DEFINED QT_DIR)
            message(WARNING
                    "The QT5 Directory could not be found, "
                    "consider setting the QT_DIR environment variable "
                    "to something like: \"C:/dev/Qt/5.9.1/msvc2015_64\"")
        endif()
    endif()


    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Gui REQUIRED)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    add_definitions(-DQT_NO_KEYWORDS)

    set(NAPKIN_QT_LIBRARIES
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
        )
endmacro()

macro(prepareqtpost)
    qt5_use_modules(${PROJECT_NAME} Core Widgets Gui)
    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           $<TARGET_FILE:Qt5::Widgets>
                           $<TARGET_FILE:Qt5::Core>
                           $<TARGET_FILE:Qt5::Gui>
                           $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           COMMENT "Copy Qt DLLs")
    endif()
endmacro()

# Let find_python find our prepackaged Python in thirdparty
macro(find_python_in_thirdparty)
    # Set our pre built Python location for *nix
    if(UNIX)
        set(PYTHONLIBS_FOUND 1)
        if(APPLE)
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/osx/install)
        elseif(UNIX)
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/linux/install)
        endif()
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/lib/libpython3.6m${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include/python3.6m)
    endif()
endmacro()

# Install virtual env config file so our Python exec lib can find the modules in thirdparty
macro(install_python_virtualenv_config)
    if(APPLE)
        add_custom_command(TARGET ${PROJECT_NAME} 
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   ${THIRDPARTY_DIR}/python/osx/pyvenv.cfg
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endif()    
endmacro()
