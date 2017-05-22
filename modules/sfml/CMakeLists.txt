project(mod_sfml)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sfml/cmake/Modules" ${CMAKE_MODULE_PATH})
#find_package(SFML 2 REQUIRED system window graphics network audio)

file(GLOB sources src/*.cpp)

if (UNIX)
else()
include_directories(src SFML-2.3.2/include)
add_subdirectory(SFML-2.3.2)
endif()

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore sfml-graphics sfml-window sfml-system)