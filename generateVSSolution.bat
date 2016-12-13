@echo off
mkdir vs64
mkdir vs32
cmake -H. -Bvs64  -G "Visual Studio 14 2015 Win64" -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
cmake -H. -Bvs32  -G "Visual Studio 14 2015" -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
pause