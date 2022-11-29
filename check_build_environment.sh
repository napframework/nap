#!/bin/bash
if [ -x "$(command -v python3)" ]; then
    PYTHON="python3"
else
    PYTHON="python"
fi
${PYTHON} $(dirname $0)/tools/buildsystem/check_build_environment/source_user_onboarding.py
