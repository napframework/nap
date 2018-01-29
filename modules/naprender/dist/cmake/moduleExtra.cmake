target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/include/nrender)
find_package(glm REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})
find_package(glew REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
find_package(assimp REQUIRED)

if (WIN32 OR APPLE)
	# On Windows and macOS find our local FreeImage
    find_package(freeimage REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREEIMAGE_INCLUDE_DIRS})
else()
	# On Linux use system freeimage and pull in SDL2
    # TODO Review why we're adding SDL2 here for Linux and not other platforms
    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} freeimage ${SDL2_LIBRARIES})
endif()

include(${CMAKE_SOURCE_DIR}/../../cmake/nrender.cmake)
target_link_libraries(${PROJECT_NAME} nrender)