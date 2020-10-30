# Populate source archive info in JSON to source_archive.json
# OUT_PATH: Directory to create the archive to
# NAP_ROOT: NAP root path, as this macro is called uniquely
# BUILD_TIMESTAMP: The timestamp for the build
# BUILD_GIT_REVISION: The git revision used to populate the build
# BUILD_LABEL: Any provided build label

include(${NAP_ROOT}/cmake/version.cmake)
configure_file(${NAP_ROOT}/cmake/source_info.json.in ${OUT_PATH}/source_info.json @ONLY)