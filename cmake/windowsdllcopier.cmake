cmake_minimum_required(VERSION 3.5)

# Copy all of our Windows DLLs that have built in bin into our project dir.  The downside of this approach is that
# any modules that have been built will be copied over into the project dir, even if they're not required for the
# project.  However as projects should be packaged from the provided tool in released NAP maybe that isn't too
# much of an issue

# Verify we have a project name
if (NOT DEFINED COPIER_IN_PATH)
    message(FATAL_ERROR "No input path")
endif()

# Verify we have a project name
if (NOT DEFINED COPIER_OUTPUT_PATH)
    message(FATAL_ERROR "No output path")
endif()

# Iterate the files in the input path and copy
file(GLOB FILES_TO_COPY ${COPIER_IN_PATH})
foreach(copy_file ${FILES_TO_COPY})
    file(COPY ${copy_file} DESTINATION ${COPIER_OUTPUT_PATH})
endforeach()