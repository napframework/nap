project(napkin)


find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

qt5_use_modules(${PROJECT_NAME} Core Widgets)

target_link_libraries(${PROJECT_NAME} napcore Qt5::Widgets
        mod_naprender mod_napinput mod_napscene mod_nappython ) # rendertest





