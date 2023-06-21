@echo off
mkdir msvc64
cmake -Hsource -Bmsvc64  -G "Visual Studio 14 2015 Win64" -DBUILD_WITH_RTTI=OFF 
