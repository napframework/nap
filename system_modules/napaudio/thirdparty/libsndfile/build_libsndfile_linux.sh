#!/usr/bin/env bash
# Usage: ./linux_build.sh <platform> 
# Example: ./linux_build.sh x86_64. platform should be either arm64 or x86_64
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "Usage: $0 <platform>"
  exit 1
fi
TARGET_NAME=$1

if [["${TARGET_NAME}" != "arm64"]] && [["${TARGET_NAME}" != "x86_64"]]; then
  echo "You must pass the target platform name, either arm64 or x86_64"
  exit 1
fi

sudo apt update && apt install -y --no-install-recommends \
    build-essential pkg-config ca-certificates \
    autoconf automake libtool autogen \
    libasound2-dev \
    libflac-dev libogg-dev libvorbis-dev libopus-dev \
    libmpg123-dev libmp3lame-dev \
    cmake patchelf \
    python-is-python3

rm -rf /var/lib/apt/lists/*


SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

LIBSNDFILE_INST_DIR="${SCRIPTDIR}/linux/${TARGET_NAME}"


[ -d "build" ] && rm -rf "build"
mkdir build
cd build
cmake -DENABLE_EXTERNAL_LIBS=1 -DCMAKE_INSTALL_PREFIX=${LIBSNDFILE_INST_DIR} -DBUILD_PROGRAMS=0 DENABLE_MPEG=1 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 -DBUILD_EXAMPLES=0 -DINSTALL_MANPAGES=0 ..
make
make install

rm -rf ${LIBSNDFILE_INST_DIR}/bin ${LIBSNDFILE_INST_DIR}/share ${LIBSNDFILE_INST_DIR}/lib/*a && \
  cp ${src_dir}/COPYING ${LIBSNDFILE_INST_DIR}/ 

./extract_apt_libs.sh "${TARGET_NAME}"