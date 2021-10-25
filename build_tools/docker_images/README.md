# Docker image creation and usage

The Docker images which can be created using the content provided are used to build third-party libraries and create framework releases across architectures for Linux. The images rely on QEMU for emulation and as a result receive a considerable performance overhead. For third-party libraries this is arguably outweighed by the convenience provided.

#### Document conventions

Commands starting with a `#` should be run as superuser / root

### Prerequisites

These processes were developed on a Debian-based Linux distro and were executed as root. In theory the processes described within could be run on other OSes and distros, but the instructions below focus on Linux. Non-root Docker work is possible but hasn't been explored at the time of writing.

First you need Docker and QEMU

```
# apt install docker.io qemu-user-static
```

Then follow the instructions on the [Docker Buildx](https://docs.docker.com/buildx/working-with-buildx/) page to get Buildx installed, running eg. `# docker buildx version` to verify its installation.

eg. at time of writing, with Buildx v0.6.3

```
sudo sh -c " \
wget https://github.com/docker/buildx/releases/download/v0.6.3/buildx-v0.6.3.linux-amd64 && \
mkdir -p ~/.docker/cli-plugins ; \
mv buildx-v0.6.3.linux-amd64 ~/.docker/cli-plugins/docker-buildx && \
chmod +x ~/.docker/cli-plugins/docker-buildx && \
docker buildx install && \
docker buildx version \
"
```

The final line of the output should show the installed Docker Buildx version, eg.

```
github.com/docker/buildx v0.6.3 266c0eac611d64fcc0c72d80206aa364e826758d
```

### Creating an image

Before using a Docker-driven process an image needs to be created for the target architecture you're compiling for. This is the case even if you're using a Docker Buildx process targeting your native architecture.

The wrapper script `create_image.py` exists to guide the image creation process, including bringing in CMake from `thirdparty`. Provide the architecture identifier to that script to build an image.

eg. for ARMhf
```
# ./create_image.py armhf
```

Check the Docker image list to verify it's been created

```
# docker images ls
```

### Compiling a third-party library

At this point you're ready to use the created image to build a library from thirdparty.

eg. to build Assimp for ARMhf change into that directory then run the following

```
# rm -rf linux/armhf
# docker buildx bake armhf
```

The build output will (for ARMhf) be deployed into `linux/armhf`. You may want to then change ownership to the normal user.

Useful options when running Buildx's `bake` include:

* Invalidate the cache: `--no-cache`
* Get plain log output: `--progress plain`

### Packaging the framework

Follow the instructions within `build_tools/cross_arch_linux_packaging` for a work-in-progress process to build for another Linux architecture using emulation.

### Modifying the Dockerfile

Edit the `DockerFile` in the root of the third-party library to make changes to the instructions executed for the build. In most cases where there are no NAP-specific (downstream) changes to the library a new version of the library can be deployed by bumping the version in the `Dockerfile` and running the Buildx bake command (above).

### Cleaning up

The following can be run to recover disk space
```
# docker system prune -f
```

More aggressively, to remove all images, containers, layers, etc:
```
# docker system prune -fa
```

To delete a single image
```
# docker image rm <image_name>
```

eg. 
```
# docker image rm nap-armhf-raspbian_buster
```
