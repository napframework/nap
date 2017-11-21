project(naprtti)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)
filter_platform_specific_files(SOURCES)

if (WIN32)
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc64/install/cmake")
    else()
        set(RTTR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/msvc32/install/cmake")
    endif()
elseif(APPLE)
    find_path(
            RTTR_DIR
            NAMES rttr-config.cmake
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/xcode/install/cmake
    )
else()
    find_path(
            RTTR_DIR
            NAMES rttr-config.cmake
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/install/cmake
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rttr/linux/install/cmake
    )
endif()

find_package(RTTR CONFIG REQUIRED Core)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

set(pybind11_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/pybind11/install/share/cmake/pybind11")
find_package(pybind11) 

add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME} NAP_SHARED_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} pybind11::embed)
target_link_libraries(${PROJECT_NAME} RTTR::Core)
target_link_libraries(${PROJECT_NAME} naputility)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

if (WIN32)
    # Copy over dlls
    set(FILES_TO_COPY $<TARGET_FILE:RTTR::Core>)
    copy_files_to_bin(${FILES_TO_COPY})
endif()

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/nap/$<CONFIG>)
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/nap/$<CONFIG>)
endif()

install(DIRECTORY "src/rtti/" DESTINATION "include/nap/rtti/"
        FILES_MATCHING PATTERN "*.h")

# Package dependent includes TODO discuss moving elsewhere??
install(DIRECTORY "../../thirdparty/pybind11/install/include/pybind11/" DESTINATION "include/thirdparty/pybind11")

if (MSVC)
    install(DIRECTORY "../../thirdparty/rttr/msvc64/install/include/rttr/" DESTINATION "include/thirdparty/rttr")
elseif(APPLE)
    install(DIRECTORY "../../thirdparty/rttr/xcode/install/include/rttr/" DESTINATION "include/thirdparty/rttr")
    install(DIRECTORY "../../thirdparty/rttr/xcode/install/bin/" DESTINATION "lib/thirdparty/Release"
            FILES_MATCHING PATTERN "*core.*dylib")
    install(DIRECTORY "../../thirdparty/rttr/xcode/install/bin/" DESTINATION "lib/thirdparty/Debug"
            FILES_MATCHING PATTERN "*_d.*dylib")
elseif(UNIX)
    install(DIRECTORY "../../thirdparty/rttr/linux/install/include/rttr/" DESTINATION "include/thirdparty/rttr")
endif()
