@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\..\thirdparty\python\python %~dp0\..\..\tools\platform\regenerate_module_by_dir.py %~dp0 %*
