project(mod_napmath)

file(GLOB sources src/*.cpp src/*.h)

find_package(glm REQUIRED)

add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC src ${GLM_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore)

