#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir build
cd build
../../cmake/macos/x86_64/bin/cmake ../source -DCMAKE_INSTALL_PREFIX="../macos/x86_64" -DENABLE_HLSL=OFF -DENABLE_RTTI=ON -DCMAKE_BUILD_TYPE=Release
../../cmake/macos/x86_64/bin/cmake --build . --config Release --target install
