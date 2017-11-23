project(mod_napvideo)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

find_package(FFmpeg REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src ${FFMPEG_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

target_link_libraries(${PROJECT_NAME} napcore mod_naprender ${FFMPEG_LIBRARIES})

# Package into platform release
package_module()

# Package dependent includes 
# TODO discuss moving elsewhere, the idea here was that the object using the dependencies installs them

# FFMPEG
if (WIN32)
    install(DIRECTORY "../../../thirdparty/ffmpeg/" DESTINATION "thirdparty/ffmpeg")
endif()
