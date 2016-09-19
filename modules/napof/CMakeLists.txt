project(mod_napof)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-conversion-null")

set(OF_DIR "../../../openFrameworks")

add_library(${PROJECT_NAME} SHARED ${sources})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${OF_DIR}/libs/openFrameworks
        ${OF_DIR}/libs/openFrameworks/math
        ${OF_DIR}/libs/openFrameworks/sound
        ${OF_DIR}/libs/openFrameworks/types
        ${OF_DIR}/libs/openFrameworks/utils
        ${OF_DIR}/libs/openFrameworks/video
        ${OF_DIR}/libs/openFrameworks/3d
        ${OF_DIR}/libs/openFrameworks/app
        ${OF_DIR}/libs/openFrameworks/communication
        ${OF_DIR}/libs/openFrameworks/events
        ${OF_DIR}/libs/openFrameworks/gl
        ${OF_DIR}/libs/openFrameworks/graphics
        ${OF_DIR}/libs/glut/include
        ${OF_DIR}/libs/rtAudio/include
        ${OF_DIR}/libs/quicktime/include
        ${OF_DIR}/libs/freetype/include
        ${OF_DIR}/libs/freetype/include/freetype2
        ${OF_DIR}/libs/freeImage/include
        ${OF_DIR}/libs/fmodex/include
        ${OF_DIR}/libs/videoInput/include
        ${OF_DIR}/libs/glew/include/
        ${OF_DIR}/libs/glu/include
        ${OF_DIR}/libs/tess2/include
        ${OF_DIR}/libs/cairo/include/cairo
        ${OF_DIR}/libs/poco/include
        ${OF_DIR}/libs/glfw/include
        ${OF_DIR}/libs/boost/include
        ${OF_DIR}/libs/utf8cpp/include
        ${OF_DIR}/libs/gl/include
        ${OF_DIR}/libs/openssl/include
        ${OF_DIR}/libs/addons
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/libs
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/libs/External
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/libs/External/DirectShow
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/libs/External/DirectShow/BaseClasses
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/libs/External/snappy
        ${OF_DIR}/addons/ofxDSHapVideoPlayer/src
        ../../../spout2/SpoutSDK/Source
        src
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore mod_napgui)