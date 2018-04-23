#!/usr/bin/env bash
unset PYTHONHOME
unset PYTHONPATH
project_dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
nap_root=$project_dir/../..
$nap_root/thirdparty/python/bin/python3 $nap_root/tools/platform/package_project_by_dir.py $project_dir "$@"