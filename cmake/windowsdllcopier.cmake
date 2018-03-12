cmake_minimum_required(VERSION 3.5)

# Copy all of our Windows DLLs that have built in bin into our project dir.  The downside of this approach is that
# any modules that have been built will be copied over into the project dir, even if they're not required for the
# project.  However as projects should be packaged from the provided tool in released NAP maybe that isn't too
# much of an issue

# Verify we have a project name
if(NOT DEFINED COPIER_IN_PATH)
    message(FATAL_ERROR "No input path")
endif()

# Verify we have a project name
if(NOT DEFINED COPIER_OUTPUT_PATH)
    message(FATAL_ERROR "No output path")
endif()

# Make output path if it doesn't exist yet
if(NOT EXISTS COPIER_OUTPUT_PATH)
    file(MAKE_DIRECTORY ${COPIER_OUTPUT_PATH})
endif()

# Iterate the files in the input path and copy
file(GLOB FILES_TO_COPY ${COPIER_IN_PATH})
foreach(copy_file ${FILES_TO_COPY})
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -E copy_if_different
                            ${copy_file}
                            ${COPIER_OUTPUT_PATH}
                            )
endforeach()