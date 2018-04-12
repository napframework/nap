# Find GLEW
if(WIN32)
    if(MSVC)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/glew/lib/Release/x64)
    else()
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/glew/lib/Release/x64)
    endif()
elseif(UNIX)
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew)
    set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/glew/lib)    
endif()

find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
if(NOT TARGET GLEW)
    find_package(GLEW REQUIRED)
endif()

find_package(OpenGL)
find_package(glm REQUIRED)

if(NOT TARGET FreeImage)
    find_package(FreeImage REQUIRED)
endif()

set(ENV{SDL2DIR} ${THIRDPARTY_DIR}/SDL2/)
find_package(SDL2 REQUIRED)

set(NRENDER_LIBRARIES
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    GLEW
    )

if(APPLE)
    list(APPEND NRENDER_LIBRARIES
         ${FREEIMAGE_LIBRARIES}
         )
else()
    list(APPEND NRENDER_LIBRARIES
         FreeImage
         )
endif()

if (WIN32)
    find_path(
        NRENDER_LIBS_DIR
        NAMES Release/nrender.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/nrender.lib)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/nrender.lib)
elseif (APPLE)
    find_path(
        NRENDER_LIBS_DIR
        NAMES Release/libnrender.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/libnrender.a)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/libnrender.a)
elseif (UNIX)
    find_path(
        NRENDER_LIBS_DIR
	NAMES Debug/libnrender.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/libnrender.a)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/libnrender.a)
endif()

set(NRENDER_INCLUDES ${NAP_ROOT}/include/nrender/)

if (NOT NRENDER_LIBS_DIR)
    message(FATAL_ERROR "Couldn't find NRender")
endif()

# Setup as interface library
add_library(nrender INTERFACE)
target_link_libraries(nrender INTERFACE optimized ${NRENDER_LIBS_RELEASE})
target_link_libraries(nrender INTERFACE debug ${NRENDER_LIBS_DEBUG})
target_link_libraries(nrender INTERFACE ${NRENDER_LIBRARIES})

# Show headers in IDE
file(GLOB nrender_headers ${CMAKE_CURRENT_LIST_DIR}/../include/nrender/*.h)
target_sources(nrender INTERFACE ${nrender_headers})
source_group(NAP\\NRender FILES ${nrender_headers})