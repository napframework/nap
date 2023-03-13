ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV python_version="3.6.4"

RUN \
  export PYTHON_DIR=Python-${python_version} && \
  export PYTHON_ARCH=$PYTHON_DIR.tar.xz && \
  wget --progress=bar:force:noscroll https://www.python.org/ftp/python/${python_version}/$PYTHON_ARCH && \
  tar -xf $PYTHON_ARCH && \
  cd $PYTHON_DIR && \
  ./configure --enable-shared --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  patchelf --set-rpath \$ORIGIN/../lib ${inst_dir}/bin/python3.6 && \
  patchelf --set-rpath \$ORIGIN/../lib ${inst_dir}/bin/python3.6m && \
  rm -rf ${inst_dir}/lib/python3.6/test && \
  rm -rf ${inst_dir}/lib/python3.6/tkinter && \
  rm -rf ${inst_dir}/share && \
  rm -f ${inst_dir}/lib/python3.6/config-*/libpython3.6m.a && \
  cp LICENSE ${inst_dir}/ && \
  rm -rf ../$PYTHON_ARCH \
    ../$PYTHON_DIR

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
