find_package(mpg123 REQUIRED)
target_link_libraries(${PROJECT_NAME} mpg123)

# Not installing mpg123 here as it's already installed in nap_audio which audioadvanced depends on