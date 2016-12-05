project(rendertest)

# get nrender
add_subdirectory(../../../nrender bin)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Override cmake's standard output directories to be closer to the source dir
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# get resources
file(GLOB DATA_FILES shaders/*.*)
file(COPY ${DATA_FILES} DESTINATION ${BIN_DIR}/shaders)
file(GLOB DATA_FILES data/*.*)
file(COPY ${DATA_FILES} DESTINATION ${BIN_DIR}/data)

# Copy over some crap window dlls
set(ASSIMP_DLL ${ASSIMP_DIR}/bin64/assimp.dll)
set(FREEIMGE_DLL ${FREEIMAGE_DIR}/Dist/x64/FreeImage.dll)
set(SDL2_DLL C:/naivi/code/sdl2/lib/x64/SDL2.dll)
set(GLEW_DLL C:/naivi/code/glew/bin/Release/x64/glew32.dll)

file(COPY ${ASSIMP_DLL} DESTINATION ${BIN_DIR})
file(COPY ${FREEIMGE_DLL} DESTINATION ${BIN_DIR})
file(COPY ${SDL2_DLL} DESTINATION ${BIN_DIR})
file(COPY ${GLEW_DLL} DESTINATION ${BIN_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender)