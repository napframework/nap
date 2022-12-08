@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\buildsystem\common\upgrade_module_by_name.py %*
