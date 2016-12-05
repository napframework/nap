cmake_minimum_required(VERSION 2.8)
project(rendertest)

#get nrender
add_subdirectory(../../../nrender build)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})


# get resources
file(GLOB DATA_FILES shaders/*.*)
file(COPY ${DATA_FILES} DESTINATION ${BIN_DIR}/shaders)
file(GLOB DATA_FILES data/*.*)
file(COPY ${DATA_FILES} DESTINATION ${BIN_DIR}/data)

# Copy over some crap window dlls
set(ASSIMP_DLL ${ASSIMP_DIR}/bin64/assimp.dll)
set(FREEIMGE_DLL ${FREEIMAGE_DIR}/Dist/x64/FreeImage.dll)
set(SDL2_DLL ../../../sdl2/lib/x64/SDL2.dll)
set(GLEW_DLL ../../../glew/bin/Release/x64/glew32.dll)


add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender)


set(FILES_TO_COPY
        ${ASSIMP_DLL}
        ${FREEIMGE_DLL}
        ${SDL2_DLL}
        ${GLEW_DLL}
        )

file(COPY ${FILES_TO_COPY} ../../../nrender/bin/MSVC/Debug/nrender.dll DESTINATION ${BIN_DIR}/Debug)
file(COPY ${FILES_TO_COPY} ../../../nrender/bin/MSVC/Release/nrender.dll DESTINATION ${BIN_DIR}/Release)


