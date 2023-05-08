ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV package_version="1.25.6"
ENV package_dir="mpg123-${package_version}"
ENV package_archive="${package_dir}.tar.bz2"

RUN \
  wget --progress=bar:force:noscroll http://mpg123.de/download/${package_archive} && \
  tar -xf ${package_archive} && \
  cd ${package_dir} && \
  ./configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  rm -rf ../${package_archive} \
    ../${package_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
