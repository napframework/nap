#!/bin/bash
if [ -x "$(command -v python3)" ]; then
    PYTHON="python3"
else
    PYTHON="python"
fi
${PYTHON} $(dirname $0)/check_build_environment.py
