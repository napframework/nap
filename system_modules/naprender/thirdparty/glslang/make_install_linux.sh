#!/usr/bin/env bash
mkdir build
cd build
cmake ../source -DCMAKE_INSTALL_PREFIX="../linux/x86_64" -DENABLE_HLSL=OFF -DENABLE_RTTI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target install
