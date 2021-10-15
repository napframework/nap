# Linux Cross-Architecture Example Process

This is an example method to package NAP to any Linux architecture independent of the host platform architecture. 

The main purpose of this at the time of writing is to determine whether the performance of this Docker & QEMU-based approach is something that's worth integrating into the main packaging process. There are rough edges that would be smoothed if a process like this were heading into production.

This is currently dependent on a Unix host, but could be modified to run on any OS.

## How to use

1. Create the Docker image for the target architecture, see `/build_tools/docker_images`
2. Deploy Qt for the architecture you're compiling in this directory, titled eg. `qt-armhf` for ARMhf. Directly within that directory should be `/lib/` etc.
3. Run `./package.py <arch>`, eg. `./package.py armhf` for ARMhf.

The output, if successful will be in a folder titled `linux_crossarch_package` alongside `nap` and `thirdparty`.