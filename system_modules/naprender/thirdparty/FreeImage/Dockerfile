ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV package_version="3.18.0"
ENV package_version_flat="3180"
ENV package_dir="FreeImage"
ENV package_archive="FreeImage${package_version_flat}.zip"

ENV sourceforge_mirror="https://ixpeering.dl.sourceforge.net"

# TODO eventually the ARM Neon optimisation disabling should be removed when possible for performance
#      https://sourceforge.net/p/freeimage/bugs/325/

RUN \
  wget --progress=bar:force:noscroll ${sourceforge_mirror}/project/freeimage/Source%20Distribution/${package_version}/${package_archive} && \
  unzip ${package_archive} && \
  cd ${package_dir} && \
  if arch | grep -q 'aarch64'; then \
    echo "matched"; \
    sed -i 's/CFLAGS += \$(INCLUDE)/CFLAGS += \$(INCLUDE) -DPNG_ARM_NEON_OPT=0/g' Makefile.gnu; \
    sed -i 's/CXXFLAGS += \$(INCLUDE)/CXXFLAGS += \$(INCLUDE) -DPNG_ARM_NEON_OPT=0/g' Makefile.gnu; \
  fi && \
  make -j `nproc` && \
  make DESTDIR=${inst_dir} -j `nproc` install && \
  mv ${inst_dir}/usr/* ${inst_dir}/ && \
  rm -rf ${inst_dir}/usr && \
  cp license* ${inst_dir} && \
  rm -rf ../${package_archive} \
    ../${package_dir}

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
