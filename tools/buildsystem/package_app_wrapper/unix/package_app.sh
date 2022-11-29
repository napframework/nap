#!/bin/sh
nap_root=$( cd "$(dirname -- "$0")" ; pwd -P )/..
. $nap_root/tools/buildsystem/common/sh_shared.sh
configure_python $nap_root
$nap_root/thirdparty/python/bin/python3 $nap_root/tools/buildsystem/common/package_app_by_name.py "$@"
