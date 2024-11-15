#!/usr/bin/env bash
mkdir build
cd build
cmake ../source -DCMAKE_INSTALL_PREFIX="../macos/universal" -DSPIRV_CROSS_FORCE_PIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build . --config Release --target install
