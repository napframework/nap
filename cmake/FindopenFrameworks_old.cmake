find_path(
        OF_PATH
        NAMES libs/openFrameworks/ofMain.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../of_v0.9.6_linux64_release
        ${CMAKE_CURRENT_LIST_DIR}/../../openFrameworks
)

set(OF_LIBS "")
set(OF_INCLUDE_PATH "")
set(ADDONS_SRC "")
set(ADDONS_HEADERS "")
set(ADDONS_LIBS "")
set(ADDONS_INCLUDE_PATH "")
set(ADDONS_PATH ${OF_PATH}/addons)
set(USE_PRECOMPILED_LIB true)

# This can also be Win32
if(AR STREQUAL "x64" OR AR STREQUAL "Win32")
    message("-- openFrameworks: Compiling openFrameworks for ${AR}")
else()
    set(AR x64)
    message("-- openFrameworks: Architecture is not set. Defaulting to x64")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_MODE ON CACHE BOOL "turn on debug mode")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(DEBUG_MODE OFF CACHE BOOL "turn off debug mode")
else()
    set(DEBUG_MODE OFF CACHE BOOL "turn off debug mode")
endif()

if(MSVC)
    if (DEBUG_MODE)
        ADD_DEFINITIONS(
                -DWIN32
                -D_DEBUG
                -D_CONSOLE
                -DPOCO_STATIC
                -DCAIRO_WIN32_STATIC_BUILD
                -DDISABLE_SOME_FLOATING_POINT
                -D_UNICODE
                -DUNICODE
        )

        set(OF_LINKER_FLAGS
                /NODEFAULTLIB:PocoFoundationmdd.lib
                /NODEFAULTLIB:atlthunk.lib
                /NODEFAULTLIB:msvcrt
                /NODEFAULTLIB:libcmt
                /NODEFAULTLIB:LIBC
                /NODEFAULTLIB:LIBCMTD
                )
    else()
        ADD_DEFINITIONS(
                -DWIN32
                -DNDEBUG
                -D_CONSOLE
                -DPOCO_STATIC
                -DCAIRO_WIN32_STATIC_BUILD
                -DDISABLE_SOME_FLOATING_POINT
                -D_UNICODE
                -DUNICODE
        )

        set(OF_LINKER_FLAGS
                /NODEFAULTLIB:PocoFoundationmd.lib
                /NODEFAULTLIB:LIBCMT
                /NODEFAULTLIB:LIBC.LIBS
                /NODEFAULTLIB:atlthunk.lib
                )
    endif(DEBUG_MODE)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OF_LINKER_FLAGS}")
endif(MSVC)

# BOOST
if (MSVC)
    if(DEBUG_MODE)
        list(APPEND OF_LIBS
                ${OF_PATH}/libs/boost/lib/vs/${AR}/libboost_filesystem-vc140-mt-gd-1_58.lib
                ${OF_PATH}/libs/boost/lib/vs/${AR}/libboost_system-vc140-mt-gd-1_58.lib
                )
    else()
        list(APPEND OF_LIBS
                ${OF_PATH}/libs/boost/lib/vs/${AR}/libboost_filesystem-vc140-mt-1_58.lib
                ${OF_PATH}/libs/boost/lib/vs/${AR}/libboost_system-vc140-mt-1_58.lib
                )
    endif(DEBUG_MODE)
    list(APPEND OF_INCLUDE_PATH
            ${OF_PATH}/libs/boost/include
            ${OF_PATH}/libs/boost/include/boost
            )
endif()


# CAIRO
if (MSVC)
    list(APPEND OF_LIBS ${OF_PATH}/libs/cairo/lib/vs/${AR}/cairo-static.lib)
    list(APPEND OF_LIBS ${OF_PATH}/libs/cairo/lib/vs/${AR}/libpng.lib)
    list(APPEND OF_LIBS ${OF_PATH}/libs/cairo/lib/vs/${AR}/pixman-1.lib)
endif()

list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/cairo/include)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/cairo/include/libpng15)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/cairo/include/pixman-1)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/cairo/include/cairo)

if(UNIX) # Hack to get around OF's inconsistent include scheme
    find_path(CAIRO_INCLUDE_DIR NAMES cairo-features.h
            HINTS /usr/include/cairo)
    list(APPEND OF_INCLUDE_PATH ${CAIRO_INCLUDE_DIR})
endif()


# KISS FFT
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/kiss/include)

# GSTREAMER
find_package(gstreamer)
if (UNIX)
    find_path(GST_INCLUDE_DIR NAMES gst/gst.h HINTS /usr/include/gstreamer-1.0)
    list(APPEND OF_INCLUDE_PATH ${GST_INCLUDE_DIR})
    find_path(GSTCONFIG_INCLUDE_DIR NAMES gst/gstconfig.h
            HINTS /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include)
    list(APPEND OF_INCLUDE_PATH ${GSTCONFIG_INCLUDE_DIR})
endif()

# GLIB
if (UNIX)
    find_package(PkgConfig)
    pkg_check_modules(GLIB2 QUIET glib-2.0)
    find_path(GLIB_INCLUDE_DIR NAMES glib.h HINTS /usr/include/glib-2.0)
    list(APPEND OF_INCLUDE_PATH ${GLIB_INCLUDE_DIR})

    find_path(GLIBCONFIG_INCLUDE_DIR NAMES glibconfig.h
            HINTS /usr/lib/x86_64-linux-gnu/glib-2.0/include)
    list(APPEND OF_INCLUDE_PATH ${GLIBCONFIG_INCLUDE_DIR})
endif()

# FMOEX
if (MSVC)
    if (AR STREQUAL "x64")
        list(APPEND OF_LIBS ${OF_PATH}/libs/fmodex/lib/vs/${AR}/fmodex64_vc.lib)
    else()
        list(APPEND OF_LIBS ${OF_PATH}/libs/fmodex/lib/vs/${AR}/fmodex_vc.lib)
    endif()
endif()
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/fmodex/include)


# FREEIMAGE
list(APPEND OF_LIBS ${OF_PATH}/libs/FreeImage/lib/vs/${AR}/FreeImage.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/FreeImage/include)


# FREETYPE
list(APPEND OF_LIBS ${OF_PATH}/libs/freetype/lib/vs/${AR}/libfreetype.lib)
list(APPEND OF_INCLUDE_PATH
        ${OF_PATH}/libs/freetype/include
        ${OF_PATH}/libs/freetype/include/freetype2
        )


# GLEW
list(APPEND OF_LIBS ${OF_PATH}/libs/glew/lib/vs/${AR}/glew32s.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/glew/include)


# GLFW
list(APPEND OF_LIBS ${OF_PATH}/libs/glfw/lib/vs/${AR}/glfw3.lib)
list(APPEND OF_INCLUDE_PATH
        ${OF_PATH}/libs/glfw/include
        ${OF_PATH}/libs/glfw/include/GLFW
        )


# GLU
list(APPEND OF_LIBS ${OF_PATH}/libs/glu/lib/vs/Win32/glu32.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/glu/include)


# GLUT
list(APPEND OF_LIBS ${OF_PATH}/libs/glut/lib/vs/${AR}/glut32.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/glut/include)


# OPENSSL
list(APPEND OF_LIBS ${OF_PATH}/libs/openssl/lib/vs/${AR}/libeay32md.lib)
list(APPEND OF_LIBS ${OF_PATH}/libs/openssl/lib/vs/${AR}/ssleay32md.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/openssl/include)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/openssl/include/openssl)


# POCO
if(DEBUG_MODE)
    list(APPEND OF_LIBS
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoCryptomdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoUtilmdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoXMLmdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoNetmdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoNetSSLmdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoFoundationmdd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoJSONmdd.lib
            )
else()
    list(APPEND OF_LIBS
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoCryptomd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoUtilmd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoXMLmd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoNetmd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoNetSSLmd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoFoundationmd.lib
            ${OF_PATH}/libs/poco/lib/vs/${AR}/PocoJSONmd.lib
            )
endif(DEBUG_MODE)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/poco/include)


# QUICKTIME
list(APPEND OF_LIBS ${OF_PATH}/libs/quicktime/lib/vs/Win32/qtmlClient.lib)
list(APPEND OF_LIBS ${OF_PATH}/libs/quicktime/lib/vs/Win32/QTSClient.lib)
list(APPEND OF_LIBS ${OF_PATH}/libs/quicktime/lib/vs/Win32/Rave.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/quicktime/include)


# RTAUDIO
if(DEBUG_MODE)
    list(APPEND OF_LIBS ${OF_PATH}/libs/rtAudio/lib/vs/${AR}/rtAudioD.lib)
else()
    list(APPEND OF_LIBS ${OF_PATH}/libs/rtAudio/lib/vs/${AR}/rtAudio.lib)
endif(DEBUG_MODE)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/rtAudio/include)


# TESS2
list(APPEND OF_LIBS ${OF_PATH}/libs/tess2/lib/vs/${AR}/tess2.lib)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/tess2/include)


# UTF8CPP
list(APPEND OF_INCLUDE_PATH
        ${OF_PATH}/libs/utf8cpp/include
        ${OF_PATH}/libs/utf8cpp/include/utf8
        )


# VIDEO_INPUT
if(DEBUG_MODE)
    list(APPEND OF_LIBS ${OF_PATH}/libs/videoInput/lib/vs/${AR}/videoInputD.lib)
else()
    list(APPEND OF_LIBS ${OF_PATH}/libs/videoInput/lib/vs/${AR}/videoInput.lib)
endif(DEBUG_MODE)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/videoInput/include)


# SYSTEM LIBS
if(UNIX)
list(APPEND OF_LIBS
        cairo
        )
else()
find_library(MSIMG32 msimg32)
find_library(OPEN_GL_32 OpenGL32)
find_library(KERNEL_32 kernel32)
find_library(SETUP_API setupapi)
find_library(VFW_32 Vfw32)
find_library(COMCTL_32 comctl32)
find_library(DSOUND dsound)
find_library(USER_32 user32)
find_library(GDI_32 gdi32)
find_library(WINSPOOL winspool)
find_library(COMDLG_32 comdlg32)
find_library(ADVAPI_32 advapi32)
find_library(SHELL_32 shell32)
find_library(OLE_32 ole32)
find_library(UUID uuid)
find_library(CRYPT_32 crypt32)
find_library(WS2_32 Ws2_32)
find_library(ODBC_32 odbc32)
find_library(ODBCCP_32 odbccp32)
find_library(GLU_32 GLu32)
find_library(WINMM winmm)
find_library(WINMM Winmm)
list(APPEND OF_LIBS
        ${MSIMG32}
        ${OPEN_GL_32}
        ${KERNEL_32}
        ${SETUP_API}
        ${VFW_32}
        ${COMCTL_32}
        ${DSOUND}
        ${USER_32}
        ${GDI_32}
        ${WINSPOOL}
        ${COMDLG_32}
        ${ADVAPI_32}
        ${SHELL_32}
        ${OLE_32}
        ${UUID}
        ${CRYPT_32}
        ${WS2_32}
        ${ODBC_32}
        ${ODBCCP_32}
        ${GLU_32}
        ${WINMM}
        )
endif()

# OPENFRAMEORKS
list(APPEND OF_INCLUDE_PATH
        ${OF_PATH}/libs/openFrameworks
        ${OF_PATH}/libs/openFrameworks/types
        ${OF_PATH}/libs/openFrameworks/sound
        ${OF_PATH}/libs/openFrameworks/video
        ${OF_PATH}/libs/openFrameworks/3d
        ${OF_PATH}/libs/openFrameworks/math
        ${OF_PATH}/libs/openFrameworks/events
        ${OF_PATH}/libs/openFrameworks/utils
        ${OF_PATH}/libs/openFrameworks/gl
        ${OF_PATH}/libs/openFrameworks/app
        ${OF_PATH}/libs/openFrameworks/graphics
        ${OF_PATH}/libs/openFrameworks/communication
        )

if (USE_PRECOMPILED_LIB)
    if (DEBUG_MODE)
        set(OF_LIB_PATH ${OF_PATH}/libs/openFrameworksCompiled/lib/vs/${AR}/openframeworksLib_debug.lib)
    else()
        set(OF_LIB_PATH ${OF_PATH}/libs/openFrameworksCompiled/lib/vs/${AR}/openframeworksLib_release.lib)
    endif(DEBUG_MODE)

    if (EXISTS ${OF_LIB_PATH})
        list(APPEND OF_LIBS ${OF_LIB_PATH})
        message("-- openFrameworks: Using precompiled openFrameworks")
    else()
        set(USE_PRECOMPILED_LIB false)
        message("-- openFrameworks: Compiling openFrameworks from source")
    endif()
else()
    message("NOTICE [openFrameworks]: Compiling openFrameworks from source")
endif(USE_PRECOMPILED_LIB)