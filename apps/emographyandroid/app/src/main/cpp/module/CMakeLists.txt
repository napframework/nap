project(mod_emography)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# TODO ANDROID pull in from interdependent module.json files
set(DEPENDENT_NAP_MODULES mod_napscene mod_napdatabase mod_napmath mod_napapi)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore naprtti RTTR::Core ${DEPENDENT_NAP_MODULES})