cmake_minimum_required(VERSION 2.8)
project(rendertest)

#get nrender
add_subdirectory(../../../nrender build)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.frag shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender)


if (MSVC)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
            ${ASSIMP_DIR}/bin64/assimp.dll
            ${FREEIMAGE_DIR}/msvc/Dist/x64/FreeImage.dll
            ../../../thirdparty/sdl2/msvc/lib/x64/SDL2.dll
            ../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll
            )

    # copy nrender (hack)
    file(COPY ${FILES_TO_COPY} DESTINATION ${BIN_DIR}/Debug)
    file(COPY ${FILES_TO_COPY} DESTINATION ${BIN_DIR}/Release)

    # create post build event
    set(DATA_DIR ${CMAKE_CURRENT_LIST_DIR}/data)
    set(SHADER_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)

    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DATA_DIR}" "${BIN_DIR}/Debug/data"
                       COMMENT "Copy Data")

    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${DATA_DIR}" "${BIN_DIR}/Release/data"
                       COMMENT "Copy Data")

    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_DIR}" "${BIN_DIR}/Debug/shaders"
                       COMMENT "Copy Shaders")

    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${SHADER_DIR}" "${BIN_DIR}/Release/shaders"
                       COMMENT "Copy Shaders")
endif()