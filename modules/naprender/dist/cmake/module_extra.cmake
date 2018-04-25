if(NOT TARGET nrender)
    include(${CMAKE_SOURCE_DIR}/../../cmake/nrender.cmake)
endif(NOT TARGET nrender)
target_link_libraries(${PROJECT_NAME} nrender assimp)
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDES})

if(UNIX)
    # Package assimp into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/lib/
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE)    

    # Package SDL2 into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/lib/
            DESTINATION "lib"
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)    

    # Package glew into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/glew/lib/
            DESTINATION lib)   

    # Package FreeImage into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/FreeImage/lib/
            DESTINATION lib
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)    
endif()    

# Install thirdparty licenses into lib
install(DIRECTORY ${THIRDPARTY_DIR}/FreeImage/license/ DESTINATION licenses/FreeImage)
install(FILES ${THIRDPARTY_DIR}/glew/LICENSE.txt DESTINATION licenses/glew)
install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)
install(FILES ${THIRDPARTY_DIR}/assimp/LICENSE DESTINATION licenses/assimp)
install(FILES ${THIRDPARTY_DIR}/SDL2/COPYING.txt DESTINATION licenses/SDL2)
