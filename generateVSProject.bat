echo off
md build
cd build
cmake .. -G "Visual Studio 14 2015 Win64"
cd ..
pause