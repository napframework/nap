ARG inst_dir="/tmp/out"
ARG image_name
FROM ${image_name} AS builder
ARG image_name
ARG arch
ARG inst_dir

COPY nap /input/nap
COPY thirdparty /input/thirdparty
COPY qt-${arch}/* /input/qt
ENV QT_DIR="/input/qt"

RUN \
    cd /input && \
    ls -la /input/qt && \
    cd /input/nap && \
    ./package.sh --no-zip && \
    mkdir ${inst_dir} && \
    cp -a NAP-*/* ${inst_dir} && \
    rm -rf /input

FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
