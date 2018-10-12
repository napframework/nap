cmake_minimum_required(VERSION 3.8)
project(napqt)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# First, let cmake know where the Qt library path is, we go from there.
if (WIN32)
    if (MSVC)
        # Add possible Qt installation paths to the HINTS section
        # The version probably doesn't have to match exactly (5.8.? is probably fine)
        find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
                HINTS
                ${CMAKE_CURRENT_LIST_DIR}/../Qt/5.9.1/msvc2015_64
                )
    else()
        find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
                HINTS
                ${CMAKE_CURRENT_LIST_DIR}/../Qt/5.9.1/mingw53_32
                )
    endif()
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_library(Qt5OpenGL REQUIRED)
find_package(OpenGL)

set(CMAKE_AUTOMOC ON)


file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE UI_HEADERS src/*.ui)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

qt5_use_modules(${PROJECT_NAME} Gui Widgets OpenGL)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

if (WIN32)
    qt5_copy_dll(${PROJECT_NAME} Gui)
    qt5_copy_dll(${PROJECT_NAME} Widgets)
    qt5_copy_dll(${PROJECT_NAME} OpenGL)
endif()
