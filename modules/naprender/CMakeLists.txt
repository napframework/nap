project(mod_naprender)

find_package(nrender REQUIRED)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDE_DIRS} src)

# link with external libs
#target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore nrender)