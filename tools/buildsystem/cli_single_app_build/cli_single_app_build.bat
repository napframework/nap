@echo OFF

set NAP_ROOT=%~dp0\..\..
set THIRDPARTY_DIR=%NAP_ROOT%\thirdparty
set PYTHONPATH=
set PYTHONHOME=
%THIRDPARTY_DIR%\python\msvc\x86_64\python %~dp0\cli_single_app_build.py %*
