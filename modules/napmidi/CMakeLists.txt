project(mod_napmidi)

find_package(rtmidi REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

if (APPLE)
    set(LIBRARIES_RELEASE napcore ${RTMIDI_LIBRARIES_RELEASE} "-framework CoreMidi"  "-framework CoreAudio" "-framework CoreFoundation")
    set(LIBRARIES_DEBUG napcore ${RTMIDI_LIBRARIES_DEBUG} "-framework CoreMidi"  "-framework CoreAudio" "-framework CoreFoundation")
else()
    set(LIBRARIES_RELEASE napcore ${RTMIDI_LIBRARIES_RELEASE})
    set(LIBRARIES_DEBUG napcore ${RTMIDI_LIBRARIES_DEBUG})
endif()


set(INCLUDES src ${RTMIDI_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} optimized ${LIBRARIES_RELEASE} debug ${LIBRARIES_DEBUG})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
