add_subdirectory(thirdparty/oscpack)
target_link_import_library(${PROJECT_NAME} oscpack)
if (WIN32)
    target_link_libraries(${PROJECT_NAME} Ws2_32 winmm)
endif()

add_license(oscpack ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/oscpack/source/LICENSE)