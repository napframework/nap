#!/usr/bin/env bash
unset PYTHONHOME
unset PYTHONPATH
nap_root=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
if [ "$(uname)" == "Darwin" ]; then
    python="$nap_root/../thirdparty/python/osx/install/bin/python3"
else
    python="$nap_root/../thirdparty/python/linux/install/bin/python3"
fi
$python $nap_root/package.py "$@"