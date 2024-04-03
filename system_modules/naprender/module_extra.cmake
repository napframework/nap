include (${NAP_ROOT}/cmake/nap_utilities.cmake)

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

if (APPLE)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${MOLTENVK_LIB}
            ${LIB_DIR})
    install(FILES ${MOLTENVK_LIB} TYPE LIB OPTIONAL)
endif()

# Copy MoltenVK_icd.json to bin and install
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkansdk/macos/universal/share/vulkan/icd.d/MoltenVK_icd.json
        ${LIB_DIR}/MoltenVK_icd.json)
install(FILES ${LIB_DIR}/MoltenVK_icd.json TYPE LIB OPTIONAL)

# Copy thirdparty licenses
add_license(assimp ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/assimp/source/LICENSE)
add_license(FreeImage ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/FreeImage/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/license)
add_license(glslang ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang/source/LICENSE.txt)
add_license(SDL2 ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/COPYING.txt)
add_license(SPIRV-cross ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SPIRV-cross/source/LICENSE)
add_license(vulkansdk ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vulkansdk/LICENSE.txt)