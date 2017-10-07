project(mod_napaudio)

find_package(portaudio REQUIRED)
find_package(libsndfile REQUIRED)
find_package(libmpg123 REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

source_group("core" src/core/*.*)
source_group("node" src/node/*.*)
source_group("object" src/object/*.*)
source_group("resource" src/resource/*.*)
source_group("service" src/service/*.*)
source_group("utility" src/utility/*.*)

if (APPLE)
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES} ${LIBMPG123_LIBRARIES} "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
else()
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES} ${LIBMPG123_LIBRARIES})
endif()

set(INCLUDES src ${PORTAUDIO_INCLUDE_DIR} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} mod_napmath)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

if (WIN32)
    set(FILES_TO_COPY
            ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()

