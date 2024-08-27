!Use the docker file or instructions below to compile libsnd with the right (NAP) settings!

cd source
mkdir build
cd build
cmake -DENABLE_EXTERNAL_LIBS=0 -DCMAKE_INSTALL_PREFIX="/home/whatever" -DBUILD_PROGRAMS=0 DENABLE_MPEG=0 -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_SHARED_LIBS=1 -DBUILD_EXAMPLES=0 -DINSTALL_MANPAGES=0 ..
make install
