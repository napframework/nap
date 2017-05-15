project(mod_napawesomium)

file(GLOB sources src/*.cpp)

find_package(awesomium REQUIRED)

add_library(${PROJECT_NAME} STATIC ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src ${AWESOMIUM_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

target_link_libraries(${PROJECT_NAME} napcore mod_napof mod_napinput ${AWESOMIUM_LIBRARIES})