#!/usr/bin/env bash

working_dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

python "$working_dir"/build.py