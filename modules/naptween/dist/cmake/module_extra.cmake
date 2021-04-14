include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET glm)
	set(GLM_FIND_QUIETLY TRUE)
    find_package(glm REQUIRED)
endif()

add_include_to_interface_target(mod_naptween ${GLM_INCLUDE_DIRS})