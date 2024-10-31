# Skip certain features when building against Raspberry Pi
if (RPI_MODEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE COMPUTEFLOCKING_RPI)
endif()