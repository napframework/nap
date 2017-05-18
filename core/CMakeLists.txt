project(napcore)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)
filter_platform_specific_files(SOURCES)

if (WIN32)
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc64/install/cmake")
    else()
        set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc32/install/cmake")
    endif()
elseif(APPLE)
    set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/xcode/install/cmake")
else()
    set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/install/cmake")
endif()

find_package(RTTR CONFIG REQUIRED Core)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)

if (MSVC)
    target_link_libraries(${PROJECT_NAME} RTTR::Core)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} RTTR::Core "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} RTTR::Core dl pthread)
endif ()



if (WIN32)
    # Copy over dlls
    set(FILES_TO_COPY $<TARGET_FILE:RTTR::Core>)
    copy_files_to_bin(${FILES_TO_COPY})
endif()
