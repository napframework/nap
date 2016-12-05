project(unittest)

include_directories(src)
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h src/**/*.cpp src/**/*.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)

