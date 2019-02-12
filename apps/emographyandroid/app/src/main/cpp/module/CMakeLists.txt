project(mod_${NAP_PROJECT_NAME})

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Dependent modules from module.json files, piped in from Gradle
set(DEPENDENT_NAP_MODULES "${NAP_PROJECTMODULE_MODULES}")

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore naprtti naputility RTTR::Core ${DEPENDENT_NAP_MODULES})