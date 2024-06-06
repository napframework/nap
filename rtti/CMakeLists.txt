project(naprtti)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

include(${NAP_ROOT}/cmake/find_utilities.cmake)
find_rttr()

set(rapidjson_FIND_QUIETLY TRUE)
find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME} NAP_SHARED_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS})

if (NAP_ENABLE_PYTHON)
    # Find the Python interpreter in thirdparty
    configure_python()

    # Pybind uses the interpreter to locate the python libraries and includes, next to pybind includes
    set(pybind11_parent_dir ${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11)
    set(pybind11_DIR ${pybind11_parent_dir}/install/share/cmake/pybind11)
    find_package(pybind11 REQUIRED)

    # Add pybind includes and libraries
    target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} pybind11::embed)
    target_link_directories(${PROJECT_NAME} PUBLIC ${PYTHON_LIB_DIR})
endif()

target_link_libraries(${PROJECT_NAME} naputility)
target_link_libraries(${PROJECT_NAME} RTTR::Core)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:RTTR::Core>
        ${LIB_DIR})

install(TARGETS ${PROJECT_NAME} LIBRARY OPTIONAL)
install(FILES $<TARGET_FILE:RTTR::Core> TYPE LIB OPTIONAL)
