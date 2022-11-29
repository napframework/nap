#!/bin/sh
if [ -x "$(command -v python3)" ]; then
    PYTHON="python3"
else
    PYTHON="python"
fi
${PYTHON} $(dirname $0)/buildsystem/check_build_environment/linux/check_build_environment_worker.py
