ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG image_name
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  cd ${src_dir} && \
  mkdir build && \
  cd build && \
  cmake ../ -DCMAKE_INSTALL_PREFIX=${inst_dir} -DSPIRV_CROSS_FORCE_PIC=ON -DCMAKE_BUILD_TYPE=Release && \
  cmake --build . --config Release --target install -- -j `nproc` && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
