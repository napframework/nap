project(scriptserver)

file(GLOB sources src/*.cpp)
include_directories(src)

add_executable(${PROJECT_NAME} ${sources})

if(MSVC)
    target_link_libraries(${PROJECT_NAME} napcore mod_napscriptserver)
else()
    target_link_libraries(${PROJECT_NAME} napcore pthread mod_napscriptserver)
endif()
