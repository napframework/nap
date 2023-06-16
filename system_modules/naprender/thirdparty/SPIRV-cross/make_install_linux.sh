#!/usr/bin/env bash
mkdir build
cd build
cmake ../source -DCMAKE_INSTALL_PREFIX="../linux/install" -DSPIRV_CROSS_FORCE_PIC=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target install
