ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

RUN \
  sudo apt install libasound2-dev libflac-dev libogg-dev libtool libvorbis-dev libopus-dev libmp3lame-dev libmpg123-dev  && \
  cd ${src_dir} && \
  autoreconf -f -i && \
  ./configure --enable-external-libs --enable-mpeg --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  rm -rf ${inst_dir}/bin ${inst_dir}/share ${inst_dir}/lib/*a && \
  cp ${src_dir}/COPYING ${inst_dir}/ && \
  rm -rf ${src_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
