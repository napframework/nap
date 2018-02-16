if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

if(NOT TARGET glew)
    find_package(GLEW REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})

if(NOT TARGET assimp)
    find_package(assimp REQUIRED)
endif()

if (WIN32 OR APPLE)
    # On Windows and macOS find our local FreeImage
    if(NOT TARGET freeimage)
        find_package(freeimage REQUIRED)
    endif()
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREEIMAGE_INCLUDE_DIRS})
else()
    # On Linux use system freeimage and pull in SDL2
    # TODO Review why we're adding SDL2 here for Linux and not other platforms
    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} freeimage ${SDL2_LIBRARIES})
endif()

if(NOT TARGET nrender)
    include(${CMAKE_SOURCE_DIR}/../../cmake/nrender.cmake)
endif(NOT TARGET nrender)
target_link_libraries(${PROJECT_NAME} nrender ${NRENDER_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDES})

if(APPLE)
    # Package glew into packaged project on macOS
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/lib/" 
            DESTINATION "lib")

    # Package assimp into packaged project on macOS
    install(DIRECTORY "${THIRDPARTY_DIR}/assimp/lib/" 
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE)       

    # Package SDL2 into packaged project on macOS
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/lib/" 
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)
endif()    