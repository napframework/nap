ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG image_name
ARG inst_dir

ENV package_version="2.0.12"
ENV package_dir="SDL2-${package_version}"
ENV package_archive="SDL2-${package_version}.tar.gz"

RUN \
  wget --progress=bar:force:noscroll https://www.libsdl.org/release/${package_archive} && \
  tar -xvf ${package_archive} && \
  cd ${package_dir} && \
  ./configure --prefix=${inst_dir} --enable-static=no --enable-sndio=no && \
  make -j `nproc` && \
  make -j `nproc` install && \
  rm -rf ../${package_archive} \
    ../${package_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
