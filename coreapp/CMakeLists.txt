project(CoreApp)

file(GLOB sources src/*.cpp)
include_directories(src ${RAPIDJSON_INCLUDE_DIRS})
add_definitions(-DAPP_NAME="${PROJECT_NAME}")
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

set(_LIBS napcore)

if(MSVC)
else()
    list(APPEND _LIBS pthread)
endif()

target_link_libraries(${PROJECT_NAME} ${_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

# Copy the binary resources to the bin dir so the executable can load them.
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/resources resources)
