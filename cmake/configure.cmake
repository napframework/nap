

# Based on the Qt 5 processor detection code, so should be very accurate
# https://qt.gitorious.org/qt/qtbase/blobs/master/src/corelib/global/qprocessordetection.h
# Currently handles arm (v5, v6, v7), x86 (32/64), ia64, and ppc (32/64)

# Regarding POWER/PowerPC, just as is noted in the Qt source,
# "There are many more known variants/revisions that we do not handle/detect."

set(archdetect_c_code "
#if defined(__arm__) || defined(__TARGET_ARCH_ARM)
    #if defined(__ARM_ARCH_7__) \\
        || defined(__ARM_ARCH_7A__) \\
        || defined(__ARM_ARCH_7R__) \\
        || defined(__ARM_ARCH_7M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 7)
        #error cmake_ARCH armv7
    #elif defined(__ARM_ARCH_6__) \\
        || defined(__ARM_ARCH_6J__) \\
        || defined(__ARM_ARCH_6T2__) \\
        || defined(__ARM_ARCH_6Z__) \\
        || defined(__ARM_ARCH_6K__) \\
        || defined(__ARM_ARCH_6ZK__) \\
        || defined(__ARM_ARCH_6M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 6)
        #error cmake_ARCH armv6
    #elif defined(__ARM_ARCH_5TEJ__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 5)
        #error cmake_ARCH armv5
    #else
        #error cmake_ARCH arm
    #endif
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #error cmake_ARCH i386
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #error cmake_ARCH x86_64
#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
    #error cmake_ARCH ia64
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__) \\
      || defined(_ARCH_COM) || defined(_ARCH_PWR) || defined(_ARCH_PPC)  \\
      || defined(_M_MPPC) || defined(_M_PPC)
    #if defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
        #error cmake_ARCH ppc64
    #else
        #error cmake_ARCH ppc
    #endif
#endif

#error cmake_ARCH unknown
")

# Set ppc_support to TRUE before including this file or ppc and ppc64
# will be treated as invalid architectures since they are no longer supported by Apple

function(target_architecture output_var)
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        # On OS X we use CMAKE_OSX_ARCHITECTURES *if* it was set
        # First let's normalize the order of the values

        # Note that it's not possible to compile PowerPC applications if you are using
        # the OS X SDK version 10.6 or later - you'll need 10.4/10.5 for that, so we
        # disable it by default
        # See this page for more information:
        # http://stackoverflow.com/questions/5333490/how-can-we-restore-ppc-ppc64-as-well-as-full-10-4-10-5-sdk-support-to-xcode-4

        # Architecture defaults to i386 or ppc on OS X 10.5 and earlier, depending on the CPU type detected at runtime.
        # On OS X 10.6+ the default is x86_64 if the CPU supports it, i386 otherwise.

        foreach(osx_arch ${CMAKE_OSX_ARCHITECTURES})
            if("${osx_arch}" STREQUAL "ppc" AND ppc_support)
                set(osx_arch_ppc TRUE)
            elseif("${osx_arch}" STREQUAL "i386")
                set(osx_arch_i386 TRUE)
            elseif("${osx_arch}" STREQUAL "x86_64")
                set(osx_arch_x86_64 TRUE)
            elseif("${osx_arch}" STREQUAL "ppc64" AND ppc_support)
                set(osx_arch_ppc64 TRUE)
            else()
                message(FATAL_ERROR "Invalid OS X arch name: ${osx_arch}")
            endif()
        endforeach()

        # Now add all the architectures in our normalized order
        if(osx_arch_ppc)
            list(APPEND ARCH ppc)
        endif()

        if(osx_arch_i386)
            list(APPEND ARCH i386)
        endif()

        if(osx_arch_x86_64)
            list(APPEND ARCH x86_64)
        endif()

        if(osx_arch_ppc64)
            list(APPEND ARCH ppc64)
        endif()
    else()
        file(WRITE "${CMAKE_BINARY_DIR}/arch.c" "${archdetect_c_code}")

        enable_language(C)

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of ifdefs that selects a
        # particular #error preprocessor directive whose message string contains the
        # target architecture. The program will always fail to compile (both because
        # file is not a valid C program, and obviously because of the presence of the
        # #error preprocessor directives... but by exploiting the preprocessor in this
        # way, we can detect the correct target architecture even when cross-compiling,
        # since the program itself never needs to be run (only the compiler/preprocessor)
        try_run(
                run_result_unused
                compile_result_unused
                "${CMAKE_BINARY_DIR}"
                "${CMAKE_BINARY_DIR}/arch.c"
                COMPILE_OUTPUT_VARIABLE ARCH
                CMAKE_FLAGS CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
        )

        # Parse the architecture name from the compiler output
        string(REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

        # Get rid of the value marker leaving just the architecture name
        string(REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

        # If we are compiling with an unknown architecture this variable should
        # already be set to "unknown" but in the case that it's empty (i.e. due
        # to a typo in the code), then set it to unknown
        if(NOT ARCH)
            set(ARCH unknown)
        endif()
    endif()

    set(${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()


target_architecture(ARCH)


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
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
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
    # TODO Output fbxconverter to tools dir at some point? Maybe.. This doesn't impact packaged location.
    # set(FBXCONV_DIR "${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/tools")

    # Set project data out path
    set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data/${PROJECT_NAME}")
    # TODO For now output to a project subfolder under the project data.  Left this way for Now
    # to make transition easier.
    # set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data")

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

# Copy all of our Windows DLLs that have build in bin into project dir
# TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
#      framework maybe this is OK?
macro(bulk_copy_windows_dlls_to_bin)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -DCOPIER_IN_PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>/../*.dll -DCOPIER_OUTPUT_PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>/ -P ${CMAKE_SOURCE_DIR}/cmake/windowsdllcopier.cmake
                       COMMENT "Bulk copying Windows library DLLs -> $<TARGET_FILE_DIR:${PROJECT_NAME}>/")
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

# Package module into platform release
macro(package_module)
    install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
            FILES_MATCHING PATTERN "*.h")

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake/ DESTINATION modules/${PROJECT_NAME}/)
    endif()

    if (WIN32)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>
                                        LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>
                                        ARCHIVE DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>)
    elseif(APPLE)
        set(BUILT_RPATH "@loader_path/../../../../thirdparty/rttr/bin/")
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>)
    else()
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/${CMAKE_BUILD_TYPE})
    endif()
endmacro()

# Set the packaged RPATH of a module for its dependent modules.
# Currently Linux-only
macro(set_installed_linux_module_rpath_for_dependent_modules DEPENDENT_NAP_MODULES TARGET_NAME)
    set(NAP_ROOT_LOCATION_TO_ORIGIN "../../../..")
    set_installed_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${TARGET_NAME} ${NAP_ROOT_LOCATION_TO_ORIGIN})
endmacro()

# Set the packaged RPATH of a Linux binary object for its dependent modules
macro(set_installed_linux_object_for_dependent_modules DEPENDENT_NAP_MODULES TARGET_NAME NAP_ROOT_LOCATION_TO_ORIGIN)
    # Add our core lib path first
    set(BUILT_RPATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/lib/${CMAKE_BUILD_TYPE}")

    # Iterate over each module and append to path
    foreach(module ${DEPENDENT_NAP_MODULES})
        # if (NOT BUILT_RPATH STREQUAL "")
        #     set(BUILT_RPATH "${BUILT_RPATH}:")
        # endif()

        set(THIS_MODULE_PATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/modules/${module}/lib/${CMAKE_BUILD_TYPE}")
        # message("Adding ${module} as ${THIS_MODULE_PATH}")
        set(BUILT_RPATH "${BUILT_RPATH}:${THIS_MODULE_PATH}")
    endforeach(module)
    # message("Built rpath: ${BUILT_RPATH}")
    set_target_properties(${TARGET_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE
                                                    INSTALL_RPATH ${BUILT_RPATH})
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
