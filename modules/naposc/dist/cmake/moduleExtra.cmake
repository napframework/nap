find_package(oscpack REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${OSCPACK_INCLUDE_DIRS})

# Install oscpack licenses into packaged project
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION licenses/oscpack)