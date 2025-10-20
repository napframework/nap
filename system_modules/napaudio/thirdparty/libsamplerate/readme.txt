generated with

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DLIBSAMPLERATE_EXAMPLES:BOOL="0" -DCMAKE_INSTALL_PREFIX="/home/whatever"  ..




or build the libraries alone, without needing to package all of nap by calling 

docker buildx bake -f docker-bake_libonly.hcl arm64
docker buildx bake -f docker-bake_libonly.hcl amd64
docker buildx bake -f docker-bake_libonly.hcl armhf
