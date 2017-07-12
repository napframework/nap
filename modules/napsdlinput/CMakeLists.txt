project(mod_napsdlinput)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

if (WIN32)
    if (MSVC)
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/msvc/lib/x64")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/mingw/x86_64-w64-mingw32")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/mingw/x86_64-w64-mingw32)
    endif()
endif()
find_package(SDL2 REQUIRED)

# compile shared lib as target
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${SDL2_INCLUDE_DIR})

# link with external libs
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} napcore mod_napinput)