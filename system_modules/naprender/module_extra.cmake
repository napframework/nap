
#function(copy_assimp_dll)
#    if(NOT WIN32)
#        return()
#    endif()
#
#    file(GLOB ASSIMP_DLL ${ASSIMP_ROOT_DIR}/bin/*.dll)
#    add_custom_command(
#        TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSIMP_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/
#        )
#endfunction()

# Find Assimp
set(assimp_parent ${NAP_ROOT}/system_modules/naprender/thirdparty/assimp)
set(assimp_DIR ${assimp_parent}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/cmake/assimp-3.3)
set(ASSIMP_LICENSE_FILES ${assimp_parent}/source/LICENSE)
find_package(assimp REQUIRED)

# Find other package dependencies
find_package(SDL2)
find_package(freeimage REQUIRED)
find_package(vulkansdk REQUIRED)
find_package(SPIRVCross REQUIRED)
find_package(glslang REQUIRED)

# Add includes
set(INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${FREEIMAGE_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
    ${VULKANSDK_INCLUDE_DIRS}
    ${SPIRVCROSS_INCLUDE_DIR}
    ${GLSLANG_INCLUDE_DIR}
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_METAL_EXT=1)
endif()

# Add libraries
set(LIBRARIES
    ${VULKANSDK_LIBS}
    ${SDL2_LIBRARY}
    ${FREEIMAGE_LIBRARIES}
    )

# TODO Investigate why we're using a static lib for Win64 only
if(MSVC)
    list(APPEND LIBRARIES
         ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}${CMAKE_STATIC_LIBRARY_SUFFIX}
         )
elseif(UNIX)
    list(APPEND LIBRARIES
         ${ASSIMP_LIBRARY_DIRS}/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIBRARIES}${CMAKE_SHARED_LIBRARY_SUFFIX}
         )
endif()

if(UNIX AND NOT APPLE AND ${ARCH} STREQUAL "armhf")
    list(APPEND LIBRARIES atomic)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_link_libraries(${PROJECT_NAME} debug ${SPIRVCROSS_LIBS_DEBUG} optimized ${SPIRVCROSS_LIBS_RELEASE})
target_link_libraries(${PROJECT_NAME} debug "${GLSLANG_LIBS_DEBUG}" optimized "${GLSLANG_LIBS_RELEASE}")

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
            CONFIGURATIONS Release)
endif()
