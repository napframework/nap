project(mod_napaudio)

find_package(portaudio REQUIRED)
find_package(libsndfile REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

source_group("component" src/component/*.*)
source_group("device" src/device/*.*)
source_group("node" src/node/*.*)
source_group("resource" src/resource/*.*)
source_group("utility" src/utility/*.*)

if (APPLE)
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES} "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
else()
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES} ${LIBSNDFILE_LIBRARIES})
endif()

set(INCLUDES src ${PORTAUDIO_INCLUDE_DIR} ${LIBSNDFILE_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

if (WIN32)
    set(FILES_TO_COPY
            ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()

