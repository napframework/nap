ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV package_version_flat="830"
ENV package_dir="cryptopp${package_version_flat}"
ENV package_archive="${package_dir}.zip"

RUN \
  wget --progress=bar:force:noscroll https://cryptopp.com/${package_archive} && \
  mkdir ${package_dir} && \
  cd ${package_dir} && \
  unzip ../${package_archive} && \
  make static && \
  make install PREFIX=${inst_dir} && \
  xz ${inst_dir}/lib/libcryptopp.a && \
  rm -rf ${inst_dir}/bin ${inst_dir}/share && \
  cd .. && \
  rm -rf ../${package_archive} \
    ../${package_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
