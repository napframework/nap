project(scriptserver)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

if(MSVC)
    target_link_libraries(${PROJECT_NAME} napcore mod_napscriptserver)
else()
    target_link_libraries(${PROJECT_NAME} napcore pthread mod_napscriptserver)
endif()
