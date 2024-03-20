include (${NAP_ROOT}/cmake/module_utilities.cmake)

add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/SDL2)
add_subdirectory(thirdparty/FreeImage)

target_link_import_library(${PROJECT_NAME} assimp)
target_link_import_library(${PROJECT_NAME} SDL2)
target_link_import_library(${PROJECT_NAME} FreeImage)

# Find other package dependencies
find_package(vulkansdk REQUIRED)
find_package(SPIRVCross REQUIRED)
find_package(glslang REQUIRED)

# Add includes
set(INCLUDES ${VULKANSDK_INCLUDE_DIRS} ${SPIRVCROSS_INCLUDE_DIR} ${GLSLANG_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_METAL_EXT=1)
endif()

# Add libraries
set(LIBRARIES ${VULKANSDK_LIBS})

# TODO Investigate why we're using a static lib for Win64 only
#if(MSVC)
#    list(APPEND LIBRARIES
#         ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}${CMAKE_STATIC_LIBRARY_SUFFIX}
#         )
#elseif(UNIX)
#    list(APPEND LIBRARIES
#         ${ASSIMP_LIBRARY_DIRS}/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIBRARIES}${CMAKE_SHARED_LIBRARY_SUFFIX}
#         )
#endif()

if(UNIX AND NOT APPLE AND ${ARCH} STREQUAL "armhf")
    list(APPEND LIBRARIES atomic)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_link_libraries(${PROJECT_NAME} debug ${SPIRVCROSS_LIBS_DEBUG} optimized ${SPIRVCROSS_LIBS_RELEASE})
target_link_libraries(${PROJECT_NAME} debug "${GLSLANG_LIBS_DEBUG}" optimized "${GLSLANG_LIBS_RELEASE}")

# Copy vulkan shared library to bin and install
# We are not copying the whole vulkansdk target because it links OS installed libraries as well
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${VULKAN_LIB}
        ${LIB_DIR})
install(FILES ${VULKAN_LIB} TYPE LIB OPTIONAL)

# Copy MoltenVK_icd.json to bin and install
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkansdk/macos/universal/share/vulkan/icd.d/MoltenVK_icd.json
        ${LIB_DIR}/MoltenVK_icd.json)
install(FILES ${LIB_DIR}/MoltenVK_icd.json TYPE LIB OPTIONAL)

