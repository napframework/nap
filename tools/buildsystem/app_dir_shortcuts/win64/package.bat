@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\..\..\tools\buildsystem\common\package_app_by_dir.py %~dp0 %*
