@echo off
mkdir vs
cd vs
cmake .. -G "Visual Studio 14 2015 Win64" -DCMAKE_PREFIX_PATH=../../Qt/5.7/msvc2015_64 -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
cd..
pause