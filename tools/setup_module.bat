@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\buildsystem\setup_module\setup_module.py %*
