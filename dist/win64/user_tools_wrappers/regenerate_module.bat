@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\python %~dp0\platform\regenerate_module_by_name.py %*