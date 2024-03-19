include(${NAP_ROOT}/cmake/module_utilities.cmake)

add_subdirectory(thirdparty/freetype)
target_link_import_library(${PROJECT_NAME} freetype)

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
        CONFIGURATIONS Release)
endif()