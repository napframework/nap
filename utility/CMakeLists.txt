project(naputility)
find_package(moodycamel REQUIRED)
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC src ${MOODYCAMEL_INCLUDE_DIRS})

