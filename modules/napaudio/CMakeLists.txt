project(mod_napaudio)

find_package(portaudio REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

if (APPLE)
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES} "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
else()
    set(LIBRARIES napcore ${PORTAUDIO_LIBRARIES})
endif()

set(INCLUDES src ${PORTAUDIO_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

