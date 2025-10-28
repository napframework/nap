# Build or update libraries


## Libsndfile

Build libsndfile.

### Linux
On Linux run either of the following lines using the corresponding platform for the system in which you are building
```bash
libsndfile/build_libsndfile_linux.sh  arm64 
libsndfile/build_libsndfile_linux.sh  x86_64
```
This command will build libsndfile using the source code available and install it alongside its necesary dependencies. 


### Windows

Install libsndfile with multi format support using vcpkg

Install vcpkg if not already installed
```
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```
install libsndfile enabling multi file type support
```
C:\vcpkg\vcpkg install libsndfile[external-libs,mpeg]:x64-windows
```
run `libsndfile\copy_libsndfile.bat`. Modify its base paths if needed.

