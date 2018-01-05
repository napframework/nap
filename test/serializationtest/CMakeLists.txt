project(serializationtest)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath)

# Build into a project directory under bin
set_output_directories()

if (WIN32)
    # Copy freeimage DLL
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over some crap window dlls
    copy_base_windows_graphics_dlls()

    # Copy all of our Windows DLLs that have build in bin into project dir
    # TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
    #      framework maybe this is OK?
    bulk_copy_windows_dlls_to_bin()
endif()

copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)