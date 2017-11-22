find_path(
        GLEW_INCLUDE_DIRS
        NAMES GL/glew.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/glew/include
)

mark_as_advanced(GLEW_INCLUDE_DIRS)

if(GLEW_INCLUDE_DIRS)
    set(GLEW_FOUND TRUE)
endif()

mark_as_advanced(GLM_FOUND)