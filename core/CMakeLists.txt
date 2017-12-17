project(napcore)

if (UNIX)
    file(GLOB SOURCES src/nap/*.cpp src/nap/*.h)
    filter_platform_specific_files(SOURCES)
    add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")
    file(GLOB FILEWATCHER_SOURCES
            src/nap/linux/FileWatcher/*.cpp
            src/nap/linux/FileWatcher/*.h)
    add_library(${PROJECT_NAME} SHARED
            ${SOURCES}
            ${FILEWATCHER_SOURCES}
            src/nap/linux/directorywatcher.cpp)
else()
    file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
    filter_platform_specific_files(SOURCES)
    add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
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

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} naputility naprtti)

if (UNIX)
    include_directories(${PROJECT_NAME} src/nap/linux/FileWatcher)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()
