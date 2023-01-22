@echo off
set PYTHONPATH=
set PYTHONHOME=
set python=%~dp0\..\thirdparty\python\msvc\x86_64\python
%python% %~dp0\buildsystem\setup_module\add_module_from_zip.py %*
