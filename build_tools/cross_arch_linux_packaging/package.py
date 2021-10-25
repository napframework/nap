#!/usr/bin/env python3
import argparse
import os
import pathlib
import shutil
from subprocess import run
import sys

def main(arch):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(script_dir)
    nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))
    thirdparty = os.path.abspath(os.path.join(nap_root, os.pardir, 'thirdparty'))

    # Create the docker build context. You want there to be as little as possible in there so as to reduce
    # the deployment time, image size, etc.
    docker_build_context = os.path.abspath(os.path.join(nap_root, os.pardir, 'linux_crossarch_package'))
    if not os.path.exists(docker_build_context):
        pathlib.Path(docker_build_context).mkdir(parents=True, exist_ok=True)

    # Deploy main repos. rsync is to enable only syncing updated content, at the cost
    # of probably locking this into a *nix machine for now.
    cmd = """
    rsync -av --delete \
    --exclude='/nap/.git' \
    --exclude='/nap/build' \
    --exclude='/nap/lib' \
    --exclude='/nap/bin' \
    --exclude='/nap/docs' \
    --exclude='*.swp' \
    --exclude='cached_project_json.cmake' \
    --exclude='cached_module_json.cmake' \
    --exclude='/nap/packaging_bin' \
    --exclude='/nap/packaging_lib' \
    --exclude='/nap/packaging_build_debug' \
    --exclude='/nap/packaging_build_release' \
    --exclude='/nap/packaging_staging' \
    --exclude='__pycache__' \
    {} {}
    """.format(nap_root, docker_build_context)
    run(cmd, shell=True)

    # Sync thirdparty
    # TODO Presuming our CMake modules allow for it we could prevent syncing the libs for
    # architectures/platforms that aren't in use.
    cmd = """
    rsync -av --delete \
    --exclude='/thirdparty/.git' \
    --exclude='*.swp' \
    --exclude='__pycache__' \
    {} {}
    """.format(thirdparty, docker_build_context)
    run(cmd, shell=True)

    shutil.copy('Dockerfile', docker_build_context)
    shutil.copy('docker-bake.hcl', docker_build_context)

    # TODO we could automatically create a Docker image if it doesn't exist

    os.chdir(docker_build_context)
    cmd = 'sudo docker buildx bake {} --progress plain --no-cache'.format(arch)
    run(cmd, shell=True)

    print("Look for output in {}".format(docker_build_context))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('architecture', choices=['arm64', 'armhf'])
    args = parser.parse_args()

    if args.architecture == 'armhf':
        print("The ARMhf process is currently broken, please verify timings against ARM64", file=sys.stderr)
        sys.exit(1)

    main(args.architecture)
