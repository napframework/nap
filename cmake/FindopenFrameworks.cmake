find_path(
        OF_PATH
        NAMES libs/openFrameworks/ofMain.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../of_v0.9.6_linux64_release
        ${CMAKE_CURRENT_LIST_DIR}/../../of_v0.9.7_linux64_release
        ${CMAKE_CURRENT_LIST_DIR}/../../openFrameworks
)

find_package(Freetype REQUIRED)
find_package(X11 REQUIRED)
find_package(gstreamer REQUIRED)
# ======= INCLUDES
set(OF_LIBS
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/openFrameworksCompiled/lib/linux64/libopenFrameworks.a
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/glfw/lib/linux64/libglfw3.a
        ${FREETYPE_LIBRARIES}
        boost_system
        fontconfig
        ${X11_LIBRARIES} Xxf86vm
        GL
        )

MACRO(subdirlist result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${curdir}/${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

list(APPEND OF_INCLUDE_PATH ${FREETYPE_INCLUDE_DIRS})

subdirlist(OF_INCLUDE_PATH ${OF_PATH}/libs/openFrameworks)
list(APPEND OF_INCLUDE_PATH ${OF_PATH}/libs/openFrameworks)
list(APPEND OF_INCLUDE_PATH
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/tess2/include
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/utf8cpp/include
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/poco/include
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/glfw/include
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/kiss/include
        /home/bmod/Documents/of_v0.9.7_linux64_release/libs/fmodex/include
        )

# CAIRO
find_package(cairo REQUIRED)
#find_path(CAIRO_INCLUDE_DIR NAMES cairo-features.h
#        HINTS /usr/include/cairo)
list(APPEND OF_INCLUDE_PATH ${CAIRO_INCLUDE_DIRS})
list(APPEND OF_LIBS ${CAIRO_LIBRARIES})

# GST
find_path(GST_INCLUDE_DIR NAMES gst/gst.h HINTS /usr/include/gstreamer-1.0)
list(APPEND OF_INCLUDE_PATH ${GST_INCLUDE_DIR})
find_path(GSTCONFIG_INCLUDE_DIR NAMES gst/gstconfig.h
        HINTS /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include)
list(APPEND OF_INCLUDE_PATH ${GSTCONFIG_INCLUDE_DIR})
#find_package(gstreamer REQUIRED)
#list(APPEND OF_LIBS ${GSTREAMER_LIBRARIES})


# GLIB
find_package(PkgConfig)
pkg_check_modules(GLIB2 QUIET glib-2.0)
find_path(GLIB_INCLUDE_DIR NAMES glib.h HINTS /usr/include/glib-2.0)
list(APPEND OF_INCLUDE_PATH ${GLIB_INCLUDE_DIR})

find_path(GLIBCONFIG_INCLUDE_DIR NAMES glibconfig.h
        HINTS /usr/lib/x86_64-linux-gnu/glib-2.0/include)
list(APPEND OF_INCLUDE_PATH ${GLIBCONFIG_INCLUDE_DIR})


# ======= LIBRARIES




#set(OF_INCLUDE_PATH "")
