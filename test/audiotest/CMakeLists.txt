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

target_link_libraries(${PROJECT_NAME} napcore mod_napaudioadvanced mod_napapp mod_nappython mod_naposc mod_napmidi)

if (WIN32)
    # Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    # Copy over some crap window dlls
	copy_base_windows_graphics_dlls()
endif()
