ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  cd ${src_dir} && \
  mkdir build && \
  cd build && \
  cmake -DCMAKE_INSTALL_PREFIX=${inst_dir} -DCMAKE_C_FLAGS=-fPIC -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Debug .. && \
  make && \
  mkdir -p ${inst_dir}/Debug && \
  cp libserial.a ${inst_dir}/Debug && \
  cd .. && \
  rm -rf build && \
  mkdir build && \
  cd build && \
  cmake -DCMAKE_INSTALL_PREFIX=${inst_dir} -DCMAKE_C_FLAGS=-fPIC -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release .. && \
  make && \
  mkdir -p ${inst_dir}/Release && \
  cp libserial.a ${inst_dir}/Release && \
  rm -rf build ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
