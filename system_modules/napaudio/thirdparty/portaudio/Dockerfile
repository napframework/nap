ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV package_file_version="v190700_20210406"
ENV package_dir="portaudio"
ENV package_archive="pa_stable_${package_file_version}.tgz"

RUN \
  wget --progress=bar:force:noscroll http://files.portaudio.com/archives/${package_archive} && \
  tar -xf ${package_archive} && \
  cd ${package_dir} && \
  ./configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cp LICENSE.txt ${inst_dir}/ && \
  rm ${inst_dir}/lib/*a && \
  rm -rf ../${package_archive} \
    ../${package_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
