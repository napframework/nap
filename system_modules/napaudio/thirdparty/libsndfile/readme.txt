!Use the docker file or instructions below to compile libsnd with the right (NAP) settings!

cd source
mkdir build
cd build
cmake -DENABLE_EXTERNAL_LIBS=1 -DCMAKE_INSTALL_PREFIX="/home/whatever" -DBUILD_PROGRAMS=0 DENABLE_MPEG=1 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 -DBUILD_EXAMPLES=0 -DINSTALL_MANPAGES=0 ..
make install


or build the libraries alone, without needing to package all of nap by calling 

docker buildx bake -f docker-bake_libonly.hcl arm64
docker buildx bake -f docker-bake_libonly.hcl amd64
docker buildx bake -f docker-bake_libonly.hcl armhf
