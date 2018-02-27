if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

if(NOT TARGET GLEW)
    find_package(GLEW REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})

if(NOT TARGET assimp)
    find_package(assimp REQUIRED)
endif()

# FreeImage
if(NOT TARGET FreeImage)
    find_package(FreeImage REQUIRED)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${FREEIMAGE_INCLUDE_DIRS})

if(NOT TARGET nrender)
    include(${CMAKE_SOURCE_DIR}/../../cmake/nrender.cmake)
endif(NOT TARGET nrender)
target_link_libraries(${PROJECT_NAME} nrender)
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDES})

if(UNIX)
    # Package assimp into packaged project on *nix
    install(DIRECTORY "${THIRDPARTY_DIR}/assimp/lib/" 
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE)    

    # Package SDL2 into packaged project on *nix
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/lib/" 
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)    

    # Package glew into packaged project on *nix
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/lib/" 
            DESTINATION "lib")   
endif()    

if(UNIX AND NOT APPLE)
    # Package FreeImage into packaged project on *nix
    # TODO move into above block once we've gotten rid of the static lib
    install(DIRECTORY "${THIRDPARTY_DIR}/FreeImage/lib/" 
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)    
endif()