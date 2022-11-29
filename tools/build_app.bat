@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\thirdparty\python\python.exe
if not exist %python% (
    set python=%~dp0\..\..\thirdparty\python\msvc\x86_64\python
)
%python% %~dp0\buildsystem\cli_single_app_build\cli_single_app_build.py %*
