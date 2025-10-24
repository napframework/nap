ARG from_builder=ubuntu:24.04
FROM ${from_builder} AS builder
ARG from_builder

ARG DEBIAN_FRONTEND=noninteractive
ARG inst_dir="/tmp/out"
ARG TARGETPLATFORM
ARG TARGETARCH
ARG TARGET_NAME

COPY extract_apt_libs.sh ${inst_dir}/extract_apt_libs.sh

ARG extra_installs

# Build and codec deps
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential pkg-config ca-certificates \
    autoconf automake libtool autogen \
    libasound2-dev \
    libflac-dev libogg-dev libvorbis-dev libopus-dev \
    libmpg123-dev libmp3lame-dev \
    cmake patchelf\
    ${extra_installs} \
 && rm -rf /var/lib/apt/lists/*


ENV LIBSNDFILE_INST_DIR="/tmp/out/libsndfile/linux/${TARGET_NAME}"

ARG src_dir="/src"
COPY libsndfile/source ${src_dir}
WORKDIR ${src_dir}

RUN autoreconf -vif && ./configure --enable-werror --disable-full-suite --disable-test-coverage --enable-external-libs --enable-mpeg --prefix=${LIBSNDFILE_INST_DIR} && \
  make -j `nproc` && \
  make -j `nproc` install


# RUN [ -d "build" ] && rm -rf "build"
# RUN mkdir build && \
# cd build && \
# cmake -DENABLE_EXTERNAL_LIBS=1 -DCMAKE_INSTALL_PREFIX=${LIBSNDFILE_INST_DIR} -DBUILD_PROGRAMS=0 DENABLE_MPEG=1 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 -DBUILD_EXAMPLES=0 -DINSTALL_MANPAGES=0 ..  && \
# make install




RUN rm -rf ${LIBSNDFILE_INST_DIR}/bin ${LIBSNDFILE_INST_DIR}/share ${LIBSNDFILE_INST_DIR}/lib/*a && \
  cp ${src_dir}/COPYING ${LIBSNDFILE_INST_DIR}/ 


WORKDIR ${inst_dir}
RUN ./extract_apt_libs.sh ${TARGET_NAME} .


FROM scratch
COPY --from=builder /tmp/out/ /
