project(nap)

file(GLOB sources src/*.cpp)
include_directories(src)

add_definitions(-DAPP_NAME="${PROJECT_NAME}")

add_executable(${PROJECT_NAME} ${sources})

if(MSVC)
    target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)
else()
    target_link_libraries(${PROJECT_NAME} napcore pthread mod_napjsonserializer)
endif()

# Copy the binary resources to the bin dir so the executable can load them.
file(GLOB RESOURCES src/*.json)
foreach (RES ${RESOURCES})
    file(COPY ${RES} DESTINATION ${BIN_DIR})
endforeach()