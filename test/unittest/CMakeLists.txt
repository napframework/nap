project(unittest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)
file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)

copy_dir_to_bin(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/resources)

