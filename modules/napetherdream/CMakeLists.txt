project(mod_napetherdream)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/ *.cpp *.h)
filter_platform_specific_files(SOURCES)
add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")

find_package(etherdream REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ETHERDREAM_INCLUDE_DIRS})

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} mod_naprender ${ETHERDREAM_LIBS})

if (WIN32)
    set(FILES_TO_COPY
            ../../../../thirdparty/etherdream/msvc/bin/Debug/EtherDream.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()
