cd %~dp0
@RD /S /Q "build"
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -DPYBIND11_NOPYTHON=TRUE -DPYBIND11_TEST=FALSE
@RD /S /Q "../install"
cmake --install . --prefix "../install"
