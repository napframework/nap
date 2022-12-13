@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\..\..\tools\buildsystem\prepare_module_to_share\prepare_module_to_share.py %~dp0 %*
