project(napkin)

# First, let cmake know where the Qt library path is, we go from there.
if (MSVC)
    # Add possible Qt installation paths to the HINTS section
    # The version probably doesn't have to match exactly (5.8.? is probably fine)
    find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../Qt/5.9.1/msvc2015_64
            )
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
endif ()

if (APPLE)
    find_path(QT_DIR lib/cmake/Qt5/Qt5Config.cmake
            HINTS
            ~/Qt/5.8/clang_64
            )
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
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

target_link_libraries(${PROJECT_NAME} napcore Qt5::Widgets Qt5::Core Qt5::Gui
        mod_naprender mod_napinput mod_napscene mod_nappython) # rendertest


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
