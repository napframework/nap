project(mod_napfsm)

file(GLOB sources src/*.cpp)


add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore)

