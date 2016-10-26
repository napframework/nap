project(coretestapp)

include_directories(.)
file(GLOB SOURCES *.cpp *.h)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} napcore)