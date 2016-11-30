project(unittest)

include_directories(src)
file(GLOB_RECURSE SOURCES src/*.cpp src/**/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)

