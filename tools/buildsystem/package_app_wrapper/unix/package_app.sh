#!/usr/bin/env bash
unset PYTHONHOME
unset PYTHONPATH
nap_root=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )/..
$nap_root/thirdparty/python/bin/python3 $nap_root/tools/platform/package_project_by_name.py "$@"
