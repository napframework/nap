project(mod_napmidi)

find_package(rtmidi REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

if (APPLE)
    set(LIBRARIES napcore ${RTMIDI_LIBRARIES} "-framework CoreMidi"  "-framework CoreAudio" "-framework CoreFoundation")
else()
    set(LIBRARIES napcore ${RTMIDI_LIBRARIES})
endif()


set(INCLUDES src ${RTMIDI_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
