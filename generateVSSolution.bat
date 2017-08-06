@echo off
mkdir msvc64
cmake -H. -Bmsvc64  -G "Visual Studio 14 2015 Win64"
