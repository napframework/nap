file(GLOB_RECURSE GUI src/imgui/*.cpp src/imgui/misc/cpp/*.cpp src/imgui/*.h src/imgui/misc/cpp/*.h)

source_group("imgui" FILES ${GUI})

target_include_directories(${PROJECT_NAME} PUBLIC src/imgui)
target_sources(${PROJECT_NAME} PRIVATE ${GUI})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY_IMGUI)

add_license(imgui ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/LICENSE.txt)

