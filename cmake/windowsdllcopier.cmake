cmake_minimum_required(VERSION 3.5)

# Copy all of our Windows DLLs that have build in bin into project dir
# TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
#      framework maybe this is OK?

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
