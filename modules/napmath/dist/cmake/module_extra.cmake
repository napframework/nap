include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()

add_include_to_interface_target(mod_napmath ${GLM_INCLUDE_DIRS})

if(WIN32)
	add_define_to_interface_target(mod_napmath NOMINMAX)
endif()

# Install thirdparty licenses into lib
install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)