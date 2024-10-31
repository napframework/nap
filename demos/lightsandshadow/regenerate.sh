#!/bin/sh
project_dir=$( cd "$(dirname -- "$0")" ; pwd -P )
nap_root=$project_dir/../..
. $nap_root/tools/buildsystem/common/sh_shared.sh
configure_python $nap_root
$python $nap_root/tools/buildsystem/common/regenerate_app_by_dir.py $project_dir "$@"
