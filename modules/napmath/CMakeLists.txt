project(mod_napmath)

file(GLOB sources src/*.cpp src/*.h)

find_package(glm REQUIRED)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src ${GLM_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} napcore)

# Package into release build
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "modules/${PROJECT_NAME}/lib")
install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
        FILES_MATCHING PATTERN "*.h")
