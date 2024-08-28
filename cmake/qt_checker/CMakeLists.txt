cmake_minimum_required(VERSION 3.18.4)
project(qt_checker)

# Add possible Qt installation paths to the HINTS section
find_path(QT_DIR lib/cmake/Qt6/Qt6Config.cmake
          HINTS
          $ENV{QT_DIR}
          )
# Find_package for Qt5 will pick up the Qt installation from CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6OpenGL REQUIRED)
message(STATUS "Found Qt ${Qt5Core_VERSION}")
