project(mod_napscene)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

find_package(glm REQUIRED)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS} src)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} napcore)

# Package into release build
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "modules/${PROJECT_NAME}/lib/$<CONFIG>")
install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
        FILES_MATCHING PATTERN "*.h")
