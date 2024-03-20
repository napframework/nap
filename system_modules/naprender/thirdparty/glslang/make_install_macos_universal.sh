#!/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir build
cd build
cmake ../source -DCMAKE_INSTALL_PREFIX="../macos/universal" -DENABLE_HLSL=OFF -DENABLE_RTTI=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build . --clean-first --config Release --target install
