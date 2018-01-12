project(mod_napartnet)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_package(artnet REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ARTNET_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

target_link_libraries(${PROJECT_NAME} napcore mod_napmath mod_napscene ${ARTNET_LIBS})

if (WIN32)
    set(FILES_TO_COPY
            ${ARTNET_LIBS_DIR}/libartnet.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
elseif(UNIX AND NOT APPLE)
    set(ARTNET_LIB_FILE ${ARTNET_LIBS_DIR}/libartnet.so)

    # HACK, somehow on some systems, libartnet.so gets linked with libartnet.so.1 because it already exists
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${ARTNET_LIB_FILE}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1"
            COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1")
endif()
