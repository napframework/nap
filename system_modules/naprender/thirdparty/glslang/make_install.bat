@echo off
mkdir build
cd build
cmake ../source -DCMAKE_INSTALL_PREFIX="../msvc/x86_64" -G "Visual Studio 16 2019" -DENABLE_HLSL=OFF -DENABLE_RTTI=ON
cmake --build . --config Release --target install
cmake --build . --config Debug --target install
