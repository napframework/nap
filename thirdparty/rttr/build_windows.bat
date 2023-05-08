@echo off
mkdir msvc64
cmake -Hsource -Bmsvc64  -G "Visual Studio 14 2015 Win64" -DBUILD_WITH_RTTI=OFF -DCMAKE_INSTALL_PREFIX="msvc/x86_64"
cmake --build msvc64 --target install --config Release
cmake --build msvc64 --target install --config Debug
