#!/bin/sh
module_dir=$( cd "$(dirname -- "$0")" ; pwd -P )
nap_root=$module_dir/../..
. $nap_root/tools/buildsystem/common/sh_shared.sh
configure_python $nap_root
$python $nap_root/tools/buildsystem/prepare_module_to_share/prepare_module_to_share_by_dir.py $module_dir "$@"
