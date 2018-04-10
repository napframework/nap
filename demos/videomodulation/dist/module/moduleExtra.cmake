# Temporary, until we have module configuration
set(DEPENDENT_MODULES mod_napvideo mod_naprender mod_napscene mod_napmath mod_napaudio)
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})
