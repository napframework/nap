ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}
ENV ARM_BUILD_TYPE=hf

RUN \
  cd ${src_dir} && \
  cd Binaries && \
  rm -rf linux/* && \
  make && \
  rm -rf linux/*/objs && \
  rm -rf linux/*/*.a linux/*/yapi/*.a && \
  mkdir ${inst_dir} && \
  cp -a linux/*/* ${inst_dir} && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
