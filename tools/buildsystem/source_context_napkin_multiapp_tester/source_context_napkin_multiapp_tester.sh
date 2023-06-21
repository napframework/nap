#!/bin/sh
script_dir=$( cd "$(dirname -- "$0")" ; pwd -P )
nap_root=$script_dir/../../..
. $nap_root/tools/buildsystem/common/sh_shared.sh
configure_python $nap_root
$python $nap_root/tools/buildsystem/source_context_napkin_multiapp_tester/source_context_napkin_multiapp_tester.py "$@"
