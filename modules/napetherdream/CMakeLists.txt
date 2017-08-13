project(mod_napetherdream)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)

source_group("headers" FILES ${HEADERS})

filter_platform_specific_files(SOURCES)
source_group("sources" FILES ${SOURCES})

find_package(etherdream REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ETHERDREAM_INCLUDE_DIRS})

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} mod_naprender ${ETHERDREAM_LIBS})

if (WIN32)
    set(FILES_TO_COPY
            ../../../../thirdparty/etherdream/msvc/bin/Release/EtherDream.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()
