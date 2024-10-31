#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir build
cd build
cmake -H../source -B. -DCMAKE_INSTALL_PREFIX=../linux/x86_64
cmake --build . --target install --config Release -- -j 8
cmake --build . --target install --config Debug -- -j 8
