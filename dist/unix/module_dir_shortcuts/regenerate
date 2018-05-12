#!/usr/bin/env bash
unset PYTHONHOME
unset PYTHONPATH
module_dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
nap_root=$module_dir/../..
$nap_root/thirdparty/python/bin/python3 $nap_root/tools/platform/regenerate_module_by_dir.py $module_dir "$@"
