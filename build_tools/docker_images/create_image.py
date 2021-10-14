#!/usr/bin/env python3
import argparse
import os
from subprocess import run
import shutil

def main(arch):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    nap_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir))
    cmake_third_party = os.path.abspath(os.path.join(nap_root, os.pardir, 'thirdparty', 'cmake'))
    os.chdir(script_dir)

    if os.path.exists('cmake'):
        shutil.rmtree('cmake')
    cmake_arch_path = os.path.join(cmake_third_party, 'linux', arch)
    print("Copying: {}".format(cmake_arch_path))
    shutil.copytree(cmake_arch_path, 'cmake')

    cmd = 'docker buildx bake {} --progress plain --no-cache'.format(arch)
    print("Running: {}".format(cmd))
    run(cmd, shell=True)
    shutil.rmtree('cmake')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('architecture', choices=['x86_64', 'arm64', 'armhf'])
    args = parser.parse_args()
    main(args.architecture)
