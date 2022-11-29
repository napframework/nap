#!/usr/bin/env bash
nap_root=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )/../..

thirdparty="$nap_root/../thirdparty"
if [ ! -d $thirdparty ]; then
    echo "Error: The third party repository ('thirdparty') needs to be cloned alongside the main repository."
    echo
    echo "Once thirdparty is in place run check_build_environment.sh first."
    exit 1
fi

if [ "$(uname)" == "Darwin" ]; then
    python="$thirdparty/python/macos/x86_64/bin/python3"
else
    case "$(arch)" in
    "x86_64")
        python="$thirdparty/python/linux/x86_64/bin/python3"
        ;;
    "aarch64")
        python="$thirdparty/python/linux/arm64/bin/python3"
        ;;
    *)
        python="$thirdparty/python/linux/armhf/bin/python3"
        ;;
    esac
fi

unset PYTHONHOME
unset PYTHONPATH
$python $nap_root/build_tools/source_context_napkin_multiproject_tester/source_context_napkin_multiproject_tester.py "$@"
