project(mod_naprender)

find_package(nrender REQUIRED)

file(GLOB sources src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDE_DIRS})

#target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)