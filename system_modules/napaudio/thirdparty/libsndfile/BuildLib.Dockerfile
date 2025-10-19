ARG from_builder=ubuntu:20.04
FROM ${from_builder} AS builder
ARG from_builder

ARG DEBIAN_FRONTEND=noninteractive
ARG inst_dir="/tmp/out"
ARG TARGETPLATFORM
ARG TARGETARCH
ARG extra_installs

# Build and codec deps
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git pkg-config ca-certificates \
    autoconf automake libtool \
    libasound2-dev \
    libflac-dev libogg-dev libvorbis-dev libopus-dev \
    libmpg123-dev libmp3lame-dev \
    ${extra_installs} \
 && rm -rf /var/lib/apt/lists/*

ARG src_dir="/src"
COPY source ${src_dir}
WORKDIR ${src_dir}


RUN autoreconf -vif && ./configure --enable-werror --disable-full-suite --enable-external-libs --enable-mpeg --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install

RUN rm -rf ${inst_dir}/bin ${inst_dir}/share ${inst_dir}/lib/*a && \
  cp ${src_dir}/COPYING ${inst_dir}/ 


FROM scratch
COPY --from=builder /tmp/out/ /
