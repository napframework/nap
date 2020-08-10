include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET glm)
    set(GLM_FIND_QUIETLY TRUE)
    find_package(glm REQUIRED)
endif()

add_include_to_interface_target(mod_napscene ${GLM_INCLUDE_DIRS})

# Install thirdparty licenses into lib
install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)
