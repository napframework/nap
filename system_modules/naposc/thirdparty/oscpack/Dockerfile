ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  cd ${src_dir} && \
  sed -i 's/COPTS  := -Wall -Wextra -O3/COPTS  := -Wall -Wextra -O3 -fPIC/g' Makefile && \
  sed -i 's/PREFIX := \/usr\/local/PREFIX := \/tmp\/out/g' Makefile && \
  mkdir -p ${inst_dir}/lib && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cd ${inst_dir}/lib/ && \
  rm liboscpack.so && \
  ln -s libos* liboscpack.so && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
