ARG inst_dir="/tmp/out"
ARG image_name
FROM ${image_name} AS builder
ARG image_name
ARG arch
ARG inst_dir

COPY nap /input/nap
COPY thirdparty /input/thirdparty

ENV QT_DIR="/input/qt"
ARG armhf_qt_archive="qt-5.15.2-armhf-pi4-raspbian_buster.tar.xz"
ARG armhf_qt_url="http://apileofgrains.nl/naivi/qt/${armhf_qt_archive}"
ARG arm64_qt_archive="qt-5.15.2-arm64-ubuntu_20.04.tar.xz"
ARG arm64_qt_url="http://apileofgrains.nl/naivi/qt/${arm64_qt_archive}"

RUN \
  if arch | grep -q 'aarch64'; then \
    wget --progress=bar:force:noscroll ${arm64_qt_url} && \
    tar xf ${arm64_qt_archive} ; \
  else \
    wget --progress=bar:force:noscroll ${armhf_qt_url} && \
    tar xf ${armhf_qt_archive} ; \
  fi && \
  mv 5.15.2 /input/qt && \
  cd /input/nap && \
  ./package.sh --no-zip && \
  mkdir ${inst_dir} && \
  cp -a NAP-*/* ${inst_dir} && \
  rm -rf /input

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
