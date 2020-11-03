if(DEFINED GLM_INCLUDE_DIRS)
    set(GLM_FIND_QUIETLY 1)
endif()
find_path(GLM_INCLUDE_DIRS
          NAMES glm/glm.hpp
          NO_CMAKE_FIND_ROOT_PATH
          HINTS
          ${CMAKE_CURRENT_LIST_DIR}/../../glm
          ${THIRDPARTY_DIR}/glm
          )

mark_as_advanced(GLM_INCLUDE_DIRS)

if(GLM_INCLUDE_DIRS)
    set(GLM_FOUND TRUE)
endif()

mark_as_advanced(GLM_FOUND)
mark_as_advanced(GLM_CXX_FLAGS)

if(GLM_FOUND)
    if(NOT GLM_FIND_QUIETLY)
        message(STATUS "Found glm header files in ${GLM_INCLUDE_DIRS}")
        if(DEFINED GLM_CXX_FLAGS)
            message(STATUS "Found glm C++ extra compilation flags: ${GLM_CXX_FLAGS}")
        endif()
    endif()
elseif(GLM_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find glm, consider adding glm path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package glm was not found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glm REQUIRED_VARS GLM_INCLUDE_DIRS)
