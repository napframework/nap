#!/usr/bin/env bash

working_dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
git config credential.helper cache
git pull https://github.com/naivisoftware/nap.git
python build.py
