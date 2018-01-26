project(napkin)


## First, let cmake know where the Qt library path is, we go from there.
if (MSVC OR APPLE)
    # Pick up QT_DIR environment variable
    if (DEFINED ENV{QT_DIR})
        set(QTDIR $ENV{QT_DIR})
        message(STATUS "Using QT_DIR environment variable: ${QTDIR}")
    endif()

    # Get QT prefix from homebrew on macOS
    if (APPLE)
        EXEC_PROGRAM(/usr/bin/env
                     ARGS brew --prefix qt5
                     OUTPUT_VARIABLE MACOS_QT_PATH)
    endif()

    # Add possible Qt installation paths to the HINTS section
    # The version probably doesn't have to match exactly (5.8.? is probably fine)
    find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
            HINTS
            ${QTDIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../Qt/5.9.1/msvc2015_64
            ${CMAKE_CURRENT_LIST_DIR}/../../Qt/5.9.2/msvc2015_64
            ~/Qt/5.8/clang_64
            ${MACOS_QT_PATH}
            )
    # Find_package for Qt5 will pick up the Qt installation from CMAKE_PREFIX_PATH
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

    if (NOT DEFINED QT_DIR)
        message(WARNING
                "The QT5 Directory could not be found, "
                "consider setting the QT_DIR environment variable "
                "to something like: \"C:/dev/Qt/5.9.1/msvc2015_64\"")
    endif()
endif ()


find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_include_directories(${PROJECT_NAME} PUBLIC src)

qt5_use_modules(${PROJECT_NAME} Core Widgets Gui)

set(NAPKIN_LIB ${PROJECT_NAME}_lib)

# create library from app for unittests
add_library(${NAPKIN_LIB} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})

set(LIBS
        napcore
        mod_napscene
        mod_nappython
        mod_napmath
        Qt5::Widgets Qt5::Core Qt5::Gui
        )

target_link_libraries(${PROJECT_NAME} ${LIBS})

target_link_libraries(${NAPKIN_LIB} ${LIBS})
target_include_directories(${NAPKIN_LIB} PUBLIC src)


if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt5::Widgets>
            $<TARGET_FILE:Qt5::Core>
            $<TARGET_FILE:Qt5::Gui>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copy Qt DLLs")
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
        COMMENT "Copy Resources")
