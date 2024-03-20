macro(setup)
    bootstrap_environment()

    # Set global directories
    set(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty)

    # Set the build output directories
    if(WIN32)
        set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
        set(LIB_DIR ${BIN_DIR})
    else()
        set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
        set(LIB_DIR ${BIN_DIR}/lib)
    endif()

    # Create build output directories
    if (NOT EXISTS ${BIN_DIR}/)
        file(MAKE_DIRECTORY ${BIN_DIR}/)
    endif()
    if (NOT EXISTS ${LIB_DIR}/)
        file(MAKE_DIRECTORY ${LIB_DIR}/)
    endif()

    # Set the app installation directories
    if (DEFINED $ENV{APP_INSTALL_NAME})
        set(app_install_name $ENV{APP_INSTALL_NAME})
    else()
        if (APPLE)
            set(app_install_name "MyApp.app")
        else()
            set(app_install_name "MyApp")
        endif()
    endif()
    if (WIN32)
        # Install all artifacts in app root on windows
        set(CMAKE_INSTALL_BINDIR ${app_install_name})
        set(CMAKE_INSTALL_LIBDIR ${app_install_name})
        set(CMAKE_INSTALL_DATADIR ${app_install_name})
    else()
        if (APPLE)
            # Install in app bundle structure on MacOS
            set(CMAKE_INSTALL_BINDIR ${app_install_name}/Contents/MacOS)
            set(CMAKE_INSTALL_LIBDIR ${app_install_name}/Contents/MacOS/lib)
            set(CMAKE_INSTALL_DATADIR ${app_install_name}/Contents/Resources)
        else ()
            # Install libraries in lib, executable in app root on Linux
            set(CMAKE_INSTALL_BINDIR ${app_install_name})
            set(CMAKE_INSTALL_LIBDIR ${app_install_name}/"lib")
            set(CMAKE_INSTALL_DATADIR ${app_install_name})
        endif()
    endif()

    # Constants for searching library files
    if(APPLE)
        set(NAP_THIRDPARTY_PLATFORM_DIR "macos")
        set(implib_prefix "lib")
        set(implib_suffix ".dylib")
        set(dll_prefix "lib")
        set(dll_suffix ".dylib")
        set(static_lib_suffix ".a")
    elseif(WIN32)
        set(NAP_THIRDPARTY_PLATFORM_DIR "msvc")
        set(implib_prefix "")
        set(implib_suffix ".lib")
        set(dll_prefix "")
        set(dll_suffix ".dll")
        set(static_lib_suffix ".lib")
    else()
        set(NAP_THIRDPARTY_PLATFORM_DIR "linux")
        set(implib_prefix "lib")
        set(implib_suffix ".so")
        set(dll_prefix "lib")
        set(dll_suffix ".so")
        set(static_lib_suffix ".a")
    endif()
    set(static_lib_prefix "")

    foreach(configuration ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${configuration} configuration)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${configuration} ${BIN_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${configuration} ${LIB_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${configuration} ${LIB_DIR})
    endforeach()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
    set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})
endmacro()


macro(bootstrap_environment)
    # Enforce GCC on Linux for now (when doing packaging build at least)
    if(UNIX AND NOT APPLE)
        if(NOT NAP_BUILD_CONTEXT MATCHES "source" OR DEFINED NAP_PACKAGED_BUILD)
            if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
                message(FATAL_ERROR "NAP only currently supports GCC on Linux")
            endif()
        endif()
    endif()

    include(${NAP_ROOT}/cmake/targetarch.cmake)
    target_architecture(ARCH)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    if(WIN32)
        if(MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd4251 /MP /bigobj /Zc:preprocessor /wd5105")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

            if(DEFINED INCLUDE_DEBUG_SYMBOLS AND INCLUDE_DEBUG_SYMBOLS)
                set(PACKAGE_PDBS ON)
            endif()
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
        endif()
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-format-security -Wno-switch -fvisibility=hidden")
        if(DEFINED INCLUDE_DEBUG_SYMBOLS AND NOT INCLUDE_DEBUG_SYMBOLS)
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                # Verified for AppleClang, expected to also potentially work (at a later date) for Clang on Linux
                string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
            else()
                # Verified for GCC on Linux
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
            endif()
        endif()
    endif()

    # We don't actively support and work on macOS anymore.
    # This is not because we don't like their hardware, but because of the continuous tightening of restrictions of macOS,
    # Including aggressive gate-keeping, required app singing, forcing specific data structures, vague licence policies,
    # proprietary APIs, etc. It's simply not in line with our policies and what we stand for.
    # Feel free to continue support for macOS on your end.
    if(APPLE)
        message(STATUS "This is a NAP fork that supports macOS universal2 binary builds for Apple Silicon")
        #message(FATAL_ERROR "macOS is no longer supported as a target operating system. Our development focus has shifted to ensure compatibility with other open platforms.")
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    endif()

    # We don't actively support and work on Python bindings anymore.
    if(NAP_ENABLE_PYTHON)
        message(DEPRECATION "Python bindings are no longer in active development or supported")
        add_definitions(-DNAP_ENABLE_PYTHON)
    endif()

    # Automatically link Qt executables to qtmain target on Windows.
    cmake_policy(SET CMP0020 NEW)

    # Ignore COMPILE_DEFINITIONS
    cmake_policy(SET CMP0043 NEW)

    # Allow modifying link targets created in other directories (needed for the way extra libraries are brought in from
    # module_extra.cmake
    cmake_policy(SET CMP0079 NEW)

    # Restrict to debug and release build types
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

    # Add extra CMake find module path
    list(APPEND CMAKE_MODULE_PATH "${NAP_ROOT}/cmake/find_modules")

    if(UNIX AND NOT APPLE)
        # Ensure we have patchelf on Linux, preventing silent failures
        ensure_patchelf_installed()

        # Check if we're building on raspbian
        check_raspbian_os(RASPBIAN)
    endif()
endmacro()


# Ensure that patchelf is installed on a Linux system. Configuration will fail if it's missing.
macro(ensure_patchelf_installed)
    execute_process(COMMAND sh -c "which patchelf"
            OUTPUT_QUIET
            RESULT_VARIABLE EXIT_CODE)
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not locate patchelf. Please run check_build_environment.")
    endif()
endmacro()


# Check existence of bcm_host.h header file to see if we're building on Raspberry
macro(check_raspbian_os RASPBERRY)
    if(${ARCH} MATCHES "armhf")
        MESSAGE(VERBOSE "Looking for bcm_host.h")
        INCLUDE(CheckIncludeFiles)

        # Raspbian bullseye bcm_host.h location
        CHECK_INCLUDE_FILES("/usr/include/bcm_host.h" RASPBERRY)

        # otherwise, check previous location of bcm_host.h on older Raspbian OS's
        if(NOT RASPBERRY)
            CHECK_INCLUDE_FILES("/opt/vc/include/bcm_host.h" RASPBERRY)
        endif()
    endif()
endmacro()