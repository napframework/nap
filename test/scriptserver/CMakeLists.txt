project(scriptserver)

file(GLOB sources src/*.cpp src-mongoose/**/*.cpp src-mongoose/*.c)
include_directories(src)
include_directories(src-mongoose)
include_directories(json-rpc-lean-include)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} napcore pthread)