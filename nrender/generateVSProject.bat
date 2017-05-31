echo off
cmake -H. -Bmsvc64 -G "Visual Studio 14 2015 Win64"
cmake -H. -Bmsvc32 -G "Visual Studio 14 2015"
cd ..
pause