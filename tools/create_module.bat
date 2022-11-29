@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\thirdparty\python\python.exe
if not exist %python% (
    set python=%~dp0\..\..\thirdparty\python\msvc\x86_64\python
)
%python% %~dp0\buildsystem\common\create_module.py %*
