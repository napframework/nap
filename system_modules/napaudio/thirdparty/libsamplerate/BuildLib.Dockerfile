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
    build-essential pkg-config ca-certificates \
    autoconf automake libtool \
    ${extra_installs} \
 && rm -rf /var/lib/apt/lists/*

ARG src_dir="/src"
COPY source ${src_dir}
WORKDIR ${src_dir}


RUN autoreconf -vif &&   ./configure --disable-static --enable-shared --disable-sndfile --disable-alsa --disable-fftw --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install

RUN rm -rf ${inst_dir}/bin ${inst_dir}/share ${inst_dir}/lib/*a && \
  cp ${src_dir}/COPYING ${inst_dir}/ 


FROM scratch
COPY --from=builder /tmp/out/ /
