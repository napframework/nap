ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"

ENV package_version="1.2.189.0"
ENV package_archive="vulkansdk-linux-x86_64-${package_version}.tar.gz"

# TODO reduce targets and produce a build that only includes what's wanted?

RUN \
  mkdir ${src_dir}/ && \
  cd ${src_dir} && \
  wget --progress=bar:force:noscroll https://sdk.lunarg.com/sdk/download/${package_version}/linux/${package_archive} && \
  tar -xf ${package_archive} && \
  cd ${package_version} && \
  rm -rf x86_64 && \
  ./vulkansdk headers loader tools layers vulkan-extensionlayer && \
  mkdir -p ${inst_dir} && \
  cp -a x86_64/* ${inst_dir}/ && \
  cd / && \
  rm -rf ${src_dir} && \
  echo "Now manually reduce to what you want to keep in thirdparty"

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
