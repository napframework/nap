@echo off

set PYTHONPATH=
set PYTHONHOME=
%~dp0\thirdparty\python\msvc\x86_64\python %~dp0\tools\buildsystem\source_cli_build\build.py %*
