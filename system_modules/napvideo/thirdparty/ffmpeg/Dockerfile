ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV package_version="3.4.2"

ENV src_dir="/input"
ENV package_dir="ffmpeg-${package_version}"
ENV package_archive="${package_dir}.tar.xz"
COPY ${package_archive} /

RUN \
  mkdir ${src_dir} && \
  cd ${src_dir} && \
  mv /${package_archive} . && \
  tar -xf ${package_archive} && \
  cd ${package_dir} && \
  ./configure --prefix=${inst_dir} \
      --enable-shared \
      --disable-static \
      --disable-programs \
      --disable-doc \
      --enable-avresample && \
  make -j `nproc` && \
  make -j `nproc` install && \
  patchelf --set-rpath \$ORIGIN/. ${inst_dir}/lib/*.so && \
  cd / && \
  rm -rf ${inst_dir}/share && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
