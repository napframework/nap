@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\..\..\tools\buildsystem\source_cli_build\build.py napkin %~dp2 %*
