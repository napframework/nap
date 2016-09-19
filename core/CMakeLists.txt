project(napcore)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} dl)
endif ()
