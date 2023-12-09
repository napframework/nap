ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

# Configure and make openssl
RUN \
  cd ${src_dir} && \
  ./Configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cp README.md ${inst_dir} && \
  rm -rf build ${src_dir}

# rename lib64 to lib and remove .a files from lib dir
RUN \
  /bin/bash -c "if [ -d ${inst_dir}/lib64 ]; then mv ${inst_dir}/lib64 ${inst_dir}/lib; fi" && \
  rm ${inst_dir}/lib/*a

# delete unneccessary dirs and files
RUN \
  rm -rf ${inst_dir}/bin ${inst_dir}/include ${inst_dir}/include ${inst_dir}/ssl ${inst_dir}/share && \
  rm -rf ${inst_dir}/lib/engines-3 ${inst_dir}/lib/ossl-modules ${inst_dir}/lib/pkgconfig && \
  rm ${inst_dir}/README.md


FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
