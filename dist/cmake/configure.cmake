

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
        if (NOT ARCH)
            set(ARCH unknown)
        endif()
    endif()

    set(${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()


target_architecture(ARCH)


if (MSVC OR APPLE)
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

macro(export_fbx_in_place SRCDIR)
    # Set the binary name
    if (MSVC)
        set(FBXCONVERTER_BIN "fbxconverter.exe")
    else()
        set(FBXCONVERTER_BIN "fbxconverter")
    endif()

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${FBXCONVERTER_BIN}" -o ${SRCDIR} "${SRCDIR}/*.fbx"
        COMMENT "Export FBX in '${SRCDIR}'")
endmacro()

macro(export_fbx SRCDIR)
    # Set project data out path
    set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data/${PROJECT_NAME}")

    # Ensure data output directory for project exists
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
        COMMENT "Ensure project output directory exists for fbxconverter")

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fbxconverter" -o ${OUTDIR} "${SRCDIR}/*.fbx"
        COMMENT "Export FBX in '${SRCDIR}'")
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

macro(copy_base_windows_graphics_dlls)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
            ../../../../thirdparty/sdl2/msvc/lib/x64/SDL2.dll
            ../../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll
            )
    # copy nrender (hack)
    copy_files_to_bin(${FILES_TO_COPY})
endmacro()

macro(copy_windows_ffmpeg_dlls)
    file(GLOB FFMPEGDLLS ${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/ffmpeg/bin/*.dll)
    copy_files_to_bin(${FFMPEGDLLS})
endmacro()

# Helper function to filter out platform-specific files
# The function outputs the following new variables with the platform-specific sources:
# - WIN32_SOURCES
# - OSX_SOURCES
# - LINUX_SOURCES
function(filter_platform_specific_files UNFILTERED_SOURCES)
	set (LOCAL_WIN32_SOURCES)
	set (LOCAL_OSX_SOURCES)
	set (LOCAL_LINUX_SOURCES)
	foreach (TMP_PATH ${${UNFILTERED_SOURCES}})
		string (FIND ${TMP_PATH} "/win32/" WIN32_EXCLUDE_DIR_FOUND)
		if (NOT ${WIN32_EXCLUDE_DIR_FOUND} EQUAL -1)
			MESSAGE(STATUS "Win32 File: " ${TMP_PATH} )
			list (APPEND LOCAL_WIN32_SOURCES ${TMP_PATH})
		else()
			string (FIND ${TMP_PATH} "/osx/" OSX_EXCLUDE_DIR_FOUND)
			if (NOT ${OSX_EXCLUDE_DIR_FOUND} EQUAL -1)
				list (APPEND LOCAL_OSX_SOURCES ${TMP_PATH})
			else()
				string (FIND ${TMP_PATH} "/linux/" LINUX_EXCLUDE_DIR_FOUND)
				if (NOT ${LINUX_EXCLUDE_DIR_FOUND} EQUAL -1)
					list (APPEND LOCAL_LINUX_SOURCES ${TMP_PATH})
				endif()
			endif ()
		endif ()
	endforeach(TMP_PATH)

	set (WIN32_SOURCES ${LOCAL_WIN32_SOURCES} PARENT_SCOPE)
	set (OSX_SOURCES ${LOCAL_OSX_SOURCES} PARENT_SCOPE)
	set (LINUX_SOURCES ${LOCAL_LINUX_SOURCES} PARENT_SCOPE)
endfunction()

# Helper macro to add platform-specific files to the correct directory and
# to only compile the platform-specific files that match the current platform
macro(add_platform_specific_files WIN32_SOURCES OSX_SOURCES LINUX_SOURCES)

	# Add to solution folders
	if (MSVC)
		# Sort header and cpps into solution folders for Win32
		foreach (TMP_PATH ${WIN32_SOURCES})
			string (FIND ${TMP_PATH} ".cpp" IS_CPP)
			if (NOT ${IS_CPP} EQUAL -1)
				source_group("Source Files\\Win32" FILES ${TMP_PATH})
			else()
				source_group("Header Files\\Win32" FILES ${TMP_PATH})
			endif()
		endforeach()

		# Sort header and cpps into solution folders for OSX
		foreach (TMP_PATH ${OSX_SOURCES})
			string (FIND ${TMP_PATH} ".cpp" IS_CPP)
			if (NOT ${IS_CPP} EQUAL -1)
				source_group("Source Files\\OSX" FILES ${TMP_PATH})
			else()
				source_group("Header Files\\OSX" FILES ${TMP_PATH})
			endif()
		endforeach()

		# Sort header and cpps into solution folders for Linux
		foreach (TMP_PATH ${LINUX_SOURCES})
			string (FIND ${TMP_PATH} ".cpp" IS_CPP)
			if (NOT ${IS_CPP} EQUAL -1)
				source_group("Source Files\\Linux" FILES ${TMP_PATH})
			else()
				source_group("Header Files\\Linux" FILES ${TMP_PATH})
			endif()
		endforeach()
	endif()

	# Unfortunately, there's no clean way to add a file to the solution (for browsing purposes, etc) but
	# exclude it from the build. The hacky way to do it is to treat the file as a 'header' (even though it's not)
	if (NOT WIN32)
		set_source_files_properties(${WIN32_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
	endif()

	if (NOT APPLE)
		set_source_files_properties(${OSX_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
	endif()

	if (APPLE OR NOT UNIX)
		set_source_files_properties(${LINUX_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
	endif()
endmacro()
