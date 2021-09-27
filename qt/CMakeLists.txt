# Exclude for Android
if(ANDROID)
    return()
endif()

########################################################################
# NAPQT Static Library
########################################################################

project(napqt)

nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_HEADERS src/*.ui)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_link_libraries(${PROJECT_NAME} ${QT_LIBS} ${OPENGL_LIBRARIES})

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

########################################################################
# DEMO
########################################################################

file(GLOB_RECURSE DEMO_SOURCES demo/*.cpp)

set(QT_DEMO ${PROJECT_NAME}_demo)
add_executable(${QT_DEMO} ${DEMO_SOURCES})
set_target_properties(${QT_DEMO} PROPERTIES FOLDER Test)
target_include_directories(${QT_DEMO} PRIVATE demo src)
target_link_libraries(${QT_DEMO} napqt)

nap_qt_post(${QT_DEMO})

if(MSVC)
    # Deploy Qt plugins from thirdparty on Win64. Unlike macOS, Windows won't find 
    # the plugins under a plugins folder, the categories need to sit beside the binary.
    add_custom_command(
        TARGET ${QT_DEMO}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QTDIR}/plugins/platforms $<TARGET_FILE_DIR:${QT_DEMO}>/platforms
        )
endif()
