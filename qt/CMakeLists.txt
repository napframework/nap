cmake_minimum_required(VERSION 3.18.4)

# Exclude for Android
if(ANDROID)
    return()
endif()

project(napqt)

include(../cmake/qtconfig.cmake)

nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_HEADERS src/*.ui)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src test)

set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_link_libraries(${PROJECT_NAME} ${QT_LIBS} ${OPENGL_LIBRARIES})

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
        COMMENT "Copy Resources")

add_executable(${PROJECT_NAME}_demo ${SOURCES})
target_include_directories(${PROJECT_NAME}_demo PUBLIC src test)
target_link_libraries(${PROJECT_NAME}_demo ${QT_LIBS} ${OPENGL_LIBRARIES})

nap_qt_post(${PROJECT_NAME})
