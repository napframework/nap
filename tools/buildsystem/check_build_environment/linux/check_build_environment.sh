#!/bin/sh
nap_root=$( cd "$(dirname -- "$0")" ; pwd -P )/..
. $nap_root/tools/buildsystem/common/sh_shared.sh
configure_python $nap_root
$python $nap_root/tools/buildsystem/common/check_build_environment_worker.py
