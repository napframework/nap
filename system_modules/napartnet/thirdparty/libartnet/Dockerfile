ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG image_name
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

ENV CFLAGS="-Wno-error"

RUN \
  cd ${src_dir} && \
  autoreconf -vfi && \
  ./configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  rm -rf ${inst_dir}/lib/*a && \
  rm -rf build ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
