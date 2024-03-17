project(napcore)

file(GLOB SOURCES src/nap/*.cpp src/nap/*.h)

# Add whether we're a NAP release or not
configure_file(src/nap/packaginginfo.h.in packaginginfo.h @ONLY)
list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/packaginginfo.h)

# Populate version info
include(${NAP_ROOT}/cmake/version.cmake)
configure_file(src/nap/version.h.in version.h @ONLY)

list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/version.h)

if(APPLE)
    add_library(${PROJECT_NAME} SHARED 
                ${SOURCES}
                src/nap/osx/directorywatcher.cpp
                src/nap/osx/module.cpp
                src/nap/osx/core_env.cpp
                src/nap/native/modulemanager_ext.cpp
                src/nap/native/logger_ext.cpp
                src/nap/native/core_ext.cpp
                src/nap/native/resourcemanager_ext.cpp
                )
elseif(MSVC)
    add_library(${PROJECT_NAME} SHARED 
                ${SOURCES}
                src/nap/win32/directorywatcher.cpp
                src/nap/win32/module.cpp
                src/nap/win32/core_env.cpp
                src/nap/native/modulemanager_ext.cpp
                src/nap/native/logger_ext.cpp
                src/nap/native/core_ext.cpp
                src/nap/native/resourcemanager_ext.cpp
                )
elseif(UNIX)
    file(GLOB FILEWATCHER_SOURCES
                src/nap/linux/FileWatcher/*.cpp
                src/nap/linux/FileWatcher/*.h)
    add_library(${PROJECT_NAME} SHARED
                ${SOURCES}
                ${FILEWATCHER_SOURCES}
                src/nap/linux/directorywatcher.cpp
                src/nap/linux/module.cpp
                src/nap/linux/core_env.cpp
                src/nap/native/modulemanager_ext.cpp
                src/nap/native/logger_ext.cpp
                src/nap/native/core_ext.cpp
                src/nap/native/resourcemanager_ext.cpp
                )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} naprtti)

if (UNIX AND NOT APPLE)
    include_directories(${PROJECT_NAME} src/nap/linux/FileWatcher)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()