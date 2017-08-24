project(napkin)

include(../qt.cmake)

if (MSVC)
    find_path(QT_DIR
            NAMES Qt5/Qt5Config.cmake
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../Qt/5.9.1/msvc2015_64/lib/cmake
            )
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
endif ()

set(CMAKE_AUTOMOC ON)

#set(QT_MODULES  Xml Multimedia 3DCore 3DInput 3DLogic 3DRender)
set(QT_MODULES Widgets)

add_qt(${QT_MODULES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UIS src/*.ui)
qt5_wrap_ui(UI_HEADERS ${UIS})


add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

qtpost(${QT_MODULES})

target_link_libraries(${PROJECT_NAME} napcore Qt5::Widgets)





