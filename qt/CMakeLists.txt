########################################################################
# NAPQT Static Library
########################################################################

project(napqt)

include(${NAP_ROOT}/cmake/qtconfig.cmake)
include(${NAP_ROOT}/cmake/module_utilities.cmake)
find_qt()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_HEADERS src/*.ui)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

target_link_import_library(${PROJECT_NAME} Qt6::Core)
target_link_import_library(${PROJECT_NAME} Qt6::Gui)
target_link_import_library(${PROJECT_NAME} Qt6::Widgets)
target_link_import_library(${PROJECT_NAME} Qt6::OpenGL)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Deploy Qt plugins from thirdparty on Win64. Unlike macOS, Windows won't find
# the plugins under a plugins folder, the categories need to sit beside the binary.
add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QTDIR}/plugins/platforms ${LIB_DIR}/platforms
)

