ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  cd ${src_dir} && \
  autoreconf -f -i && \
  ./configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  rm -rf ${inst_dir}/bin ${inst_dir}/share ${inst_dir}/lib/*a && \
  cp ${src_dir}/COPYING ${inst_dir}/ && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
