 find_path(
        NRENDER_INCLUDE_DIRS
        NAMES nopengl.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../nrender/src 
)

list(APPEND NRENDER_INCLUDE_DIRS 
    ${CMAKE_CURRENT_LIST_DIR}/../../glm 
    ${CMAKE_CURRENT_LIST_DIR}/../../sdl2/include 
    ${CMAKE_CURRENT_LIST_DIR}/../../assimp/include 
    ${CMAKE_CURRENT_LIST_DIR}/../../glew/include 
    ${CMAKE_CURRENT_LIST_DIR}/../../freeimage/Source 
    )

message(WARNING ${NRENDER_INCLUDE_DIRS})

mark_as_advanced(NRENDER_INCLUDE_DIRS)

if(NRENDER_INCLUDE_DIRS)
    set(NRENDER_FOUND TRUE)
endif()

mark_as_advanced(NRENDER_FOUND)
mark_as_advanced(NRENDER_CXX_FLAGS)

if(NRENDER_FOUND)
    if(NOT NRENDER_FIND_QUIETLY)
        message(STATUS "Found nrender header files in ${NRENDER_INCLUDE_DIRS}")
        if(DEFINED NRENDER_CXX_FLAGS)
            message(STATUS "Found nrender C++ extra compilation flags: ${NRENDER_CXX_FLAGS}")
        endif()
    endif()
elseif(NRENDER_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find nrender, consider adding nrender path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package nrender was not found")
endif()