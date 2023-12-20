ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

# Configure and make openssl
# In case of arm6vl, we need to add -latomic to the configure command and explicitly set linux-generic32 as target
RUN \
  cd ${src_dir} && \
  if [ "$(uname -m)" = "armv6l" ]; then ./Configure --prefix=${inst_dir} linux-generic32 -latomic; else ./Configure --prefix=${inst_dir}; fi && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cp README.md ${inst_dir} && \
  rm -rf build ${src_dir}

# rename lib64 to lib if necessary, rename lib to libs
RUN \
  /bin/bash -c "if [ -d ${inst_dir}/lib64 ]; then mv ${inst_dir}/lib64 ${inst_dir}/lib; fi" && \
  /bin/bash -c "if [ -d ${inst_dir}/lib ]; then mv ${inst_dir}/lib ${inst_dir}/libs; fi"

# delete unneccessary dirs and files
RUN \
  rm ${inst_dir}/libs/*a && \
  rm -rf ${inst_dir}/bin ${inst_dir}/include ${inst_dir}/include ${inst_dir}/ssl ${inst_dir}/share && \
  rm -rf ${inst_dir}/libs/engines-3 ${inst_dir}/libs/ossl-modules ${inst_dir}/libs/pkgconfig && \
  rm ${inst_dir}/README.md


FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
