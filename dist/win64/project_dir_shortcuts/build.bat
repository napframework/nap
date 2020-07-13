@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\..\thirdparty\python\python %~dp0\..\..\tools\platform\build_project_by_dir.py %~dp0 %*
