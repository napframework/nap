@echo off
mkdir msvc64
REM cmake -H. -Bmsvc64  -G "Visual Studio 14 2015 Win64" -DPYBIND11_PYTHON_VERSION=3.5
REM If you have multiple versions of python and pybind cannot find the correct version,
REM try this:
cmake -H. -Bmsvc64  -G "Visual Studio 14 2015 Win64" -DPYTHON_EXECUTABLE=c:/python35/python.exe
