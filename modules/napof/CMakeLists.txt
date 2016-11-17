project(mod_napof)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-conversion-null")

find_package(openFrameworks REQUIRED)
find_package(cairo REQUIRED)

add_library(${PROJECT_NAME} STATIC ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC
        ${OF_INCLUDE_PATH}
        src
        fontconfig
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore mod_napgui ${OF_LIBS} )