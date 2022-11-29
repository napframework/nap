if(NAP_BUILD_CONTEXT MATCHES "source")
    if(UNIX AND NOT APPLE AND ${ARCH} STREQUAL "armhf")
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z muldefs")
    endif()
else()
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/mod_napsequencegui)
endif()
