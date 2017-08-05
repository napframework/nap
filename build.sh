#!/usr/bin/env bash

working_dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
"$working_dir" git config credential.helper cache
git pull https://github.com/naivisoftware/nap.git
python "$working_dir"/build.py
