project(napkin)

include(../qtpre.cmake)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
file(GLOB_RECURSE UIS src/*.ui)
qt5_wrap_ui(UI_HEADERS ${UIS})


add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

include(../qtpost.cmake)

target_link_libraries(${PROJECT_NAME} napcore)





