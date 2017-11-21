project(mod_napsdlwindow)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

if (WIN32)
    if (MSVC)
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/msvc/lib/x64")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/mingw/x86_64-w64-mingw32")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/mingw/x86_64-w64-mingw32)
    endif()
endif()
find_package(SDL2 REQUIRED)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${SDL2_INCLUDE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} napcore)

# Package into release build
if (WIN32)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "modules/${PROJECT_NAME}/lib/$<CONFIG>")
else()
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "modules/${PROJECT_NAME}/lib/$<CONFIG>")
endif()

install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
        FILES_MATCHING PATTERN "*.h")
