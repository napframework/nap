ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG image_name
ARG inst_dir

ENV cmake_version="3.21.3"

RUN \
  export CMAKE_DIR=cmake-${cmake_version} && \
  export CMAKE_ARCH=$CMAKE_DIR.tar.gz && \
  wget --progress=bar:force:noscroll https://github.com/Kitware/CMake/releases/download/v${cmake_version}/$CMAKE_ARCH && \
  tar -xzf $CMAKE_ARCH && \
  cd $CMAKE_DIR && \
  ./bootstrap --parallel=`nproc` --prefix=${inst_dir} && \
  make -j `nproc` && \
  make install && \
  rm -rf ../$CMAKE_ARCH \
    ../$CMAKE_DIR

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
