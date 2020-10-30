cmake_minimum_required(VERSION 3.15.4)
project(qt_checker)

# Add possible Qt installation paths to the HINTS section
find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
          HINTS
          $ENV{QT_DIR}
          )
# Find_package for Qt5 will pick up the Qt installation from CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
message(STATUS "Found Qt ${Qt5Core_VERSION}")
