# Turn this option off in case you want to build napmidi without rtmidi.
# rtmidi is needed for accessing hardware midi devices.
set(NAP_ENABLE_RTMIDI ON)

if (NAP_ENABLE_RTMIDI)
    # Include sources and headers for midi port support that use rtmidi
    add_source_dir("midiport" "src/midiport")

    # Add compile definition to enable midi port support
    target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_ENABLE_RTMIDI)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rtmidi)
    target_link_import_library(${PROJECT_NAME} rtmidi)
endif()