if(NOT TARGET oscpack)
    find_package(oscpack REQUIRED)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})
else()
    target_link_libraries(${PROJECT_NAME} oscpack)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${OSCPACK_INCLUDE_DIRS})

# Install oscpack licenses into packaged project
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION licenses/oscpack)

# Install oscpack shared lib into packaged project for Unix
if(APPLE)
    install(FILES $<TARGET_FILE:oscpack> DESTINATION lib)    
elseif(UNIX)
    file(GLOB OSCPACK_DYLIBS ${THIRDPARTY_DIR}/oscpack/lib/liboscpack*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${OSCPACK_DYLIBS} DESTINATION lib)
endif()
