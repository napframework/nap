cmake_minimum_required(VERSION 3.8)
project(napqt)

include(../cmake/qtconfig.cmake)

nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_HEADERS src/*.ui)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src test)

qt5_use_modules(${PROJECT_NAME} Gui Widgets OpenGL)

target_link_libraries(${PROJECT_NAME} ${QT_LIBS} ${OPENGL_LIBRARIES})

add_executable(${PROJECT_NAME}_demo ${SOURCES})
target_include_directories(${PROJECT_NAME}_demo PUBLIC src test)
target_link_libraries(${PROJECT_NAME}_demo ${QT_LIBS} ${OPENGL_LIBRARIES})

nap_qt_post(${PROJECT_NAME})