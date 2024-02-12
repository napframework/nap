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

set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_link_libraries(${PROJECT_NAME} ${QT_LIBS} ${OPENGL_LIBRARIES})

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
