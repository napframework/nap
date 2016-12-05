project(jsonserializer)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} napcore pthread mod_napjsonserializer)