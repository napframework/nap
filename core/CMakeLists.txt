project(napcore)

file(GLOB SOURCES src/nap/*.cpp src/nap/*.h)
if(APPLE)
    add_library(${PROJECT_NAME} SHARED 
        ${SOURCES}
        src/nap/osx/directorywatcher.cpp)
elseif(MSVC)
    add_library(${PROJECT_NAME} SHARED 
        ${SOURCES}
        src/nap/win32/directorywatcher.cpp)
elseif(UNIX)
    file(GLOB FILEWATCHER_SOURCES
            src/nap/linux/FileWatcher/*.cpp
            src/nap/linux/FileWatcher/*.h)
    add_library(${PROJECT_NAME} SHARED
            ${SOURCES}
            ${FILEWATCHER_SOURCES}
            src/nap/linux/directorywatcher.cpp)  
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

if (UNIX AND NOT APPLE)
    include_directories(${PROJECT_NAME} src/nap/linux/FileWatcher)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()
