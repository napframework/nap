if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

# Install thirdparty licenses into lib
install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)