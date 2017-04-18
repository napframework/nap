project(napcore)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

