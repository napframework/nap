cmake_minimum_required(VERSION 2.8)
project(audiotest)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} napcore mod_napaudio)

if (WIN32)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
            ../../../../thirdparty/sdl2/msvc/lib/x64/SDL2.dll
            ../../../../thirdparty/portaudio/msvc64/portaudio_x64.dll
            )
    # copy nrender (hack)
    copy_files_to_bin(${FILES_TO_COPY})
endif()

# create post build event
# copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
