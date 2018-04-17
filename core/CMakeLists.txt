include(${NAP_ROOT}/cmake/packaging_macros.cmake)
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
        src/nap/osx/module.cpp)
elseif(MSVC)
    add_library(${PROJECT_NAME} SHARED 
        ${SOURCES}
        src/nap/win32/directorywatcher.cpp
        src/nap/win32/module.cpp)
elseif(UNIX)
    file(GLOB FILEWATCHER_SOURCES
            src/nap/linux/FileWatcher/*.cpp
            src/nap/linux/FileWatcher/*.h)
    add_library(${PROJECT_NAME} SHARED
            ${SOURCES}
            ${FILEWATCHER_SOURCES}
            src/nap/linux/directorywatcher.cpp
	    src/nap/linux/module.cpp)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} naputility naprtti)

if (UNIX AND NOT APPLE)
    include_directories(${PROJECT_NAME} src/nap/linux/FileWatcher)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

# Package into release build
if(WIN32)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/$<CONFIG>
                                    ARCHIVE DESTINATION lib/$<CONFIG>)
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/$<CONFIG>)
endif()

if(WIN32)
    # Install for fbxconverter on Win64
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
elseif(APPLE)
    set(BUILT_RPATH "@loader_path/../../thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};@loader_path/../../thirdparty/python/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE})   
endif()

# Package headers
install(DIRECTORY "src/nap/" DESTINATION "include/nap/"
        FILES_MATCHING PATTERN "*.h"
        PATTERN "linux" EXCLUDE
        PATTERN "osx" EXCLUDE
        PATTERN "win32" EXCLUDE
)

# Package release version info
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION "include/nap/")
