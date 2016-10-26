project(scriptserver)

file(GLOB sources src/*.cpp)
include_directories(src)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} napcore jsoncpp pthread)