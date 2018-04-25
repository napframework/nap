find_package(oscpack REQUIRED)
target_link_libraries(${PROJECT_NAME} oscpack)
target_include_directories(${PROJECT_NAME} PUBLIC ${OSCPACK_INCLUDE_DIRS})

# Install oscpack licenses into packaged project
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION licenses/oscpack)

# Install oscpack shared lib into packaged project for Unix
if(UNIX)
    install(FILES ${OSCPACK_LIBS_RELEASE} DESTINATION lib)
endif()
