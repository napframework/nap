# create project
project(nrender)

# set some compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)

#set(SOURCES src/nimage.cpp src/nbitmap.cpp)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# add to include path and expose to depending projects
target_include_directories(${PROJECT_NAME} PUBLIC src ${SDL2_INCLUDE_DIRS})

# Link with SDL and GLEW
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} GL freeimage assimp)

