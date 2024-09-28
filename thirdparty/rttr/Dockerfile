ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG image_name
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  mkdir ${inst_dir} && \
  cd ${src_dir} && \
  cmake -H. -Bbuild && \
  cd build && \
  cmake --build . --target install --config Release -- -j `nproc` && \
  cmake --build . --target install --config Debug -- -j `nproc` && \
  cp -a install/* ${inst_dir} && \
  cd .. && \
  rm -rf build ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
