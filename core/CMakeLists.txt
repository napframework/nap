project(napcore)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc64/install/cmake")
else()
    set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc32/install/cmake")
endif()

find_package(RTTR CONFIG REQUIRED Core)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} RTTR::Core)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()


if (WIN32)
    # Copy over dlls
    set(FILES_TO_COPY $<TARGET_FILE:RTTR::Core>)
    copy_files_to_bin(${FILES_TO_COPY})
endif()