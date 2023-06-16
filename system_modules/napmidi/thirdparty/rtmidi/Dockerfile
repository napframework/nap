ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  cd ${src_dir} && \
  ./configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cp README.md ${inst_dir} && \
  rm -rf build ${src_dir} && \
  rm ${inst_dir}/lib/*a

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
