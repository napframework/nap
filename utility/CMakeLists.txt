project(naputility)
find_package(moodycamel REQUIRED)
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)

add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC src ${MOODYCAMEL_INCLUDE_DIRS})

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/$<CONFIG>-${ARCH})
    if(PACKAGE_PDBS)
        install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.dir/$<CONFIG>/${PROJECT_NAME}.pdb DESTINATION lib/$<CONFIG>-${ARCH})
    endif()
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/$<CONFIG>-${ARCH})
else()
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/${CMAKE_BUILD_TYPE}-${ARCH})
endif()

# Package headers
install(DIRECTORY "src/utility/" DESTINATION "include/utility/"
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "direntw32.h" EXCLUDE
        )
# Windows-only header
if (MSVC)
    install(FILES src/utility/direntw32.h DESTINATION include/utility)
endif()

# Package moodycamel into platform release
install(DIRECTORY ${MOODYCAMEL_INCLUDE_DIRS}/ DESTINATION thirdparty/moodycamel)
