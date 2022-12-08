ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source/ ${src_dir}

RUN \
  cd ${src_dir} && \
  mkdir build && \
  cd build && \
  cmake ../ -DCMAKE_INSTALL_PREFIX=${inst_dir} -DENABLE_HLSL=OFF -DENABLE_RTTI=ON -DCMAKE_BUILD_TYPE=Release && \
  cmake --build . --target install --config Release -- -j `nproc` && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
