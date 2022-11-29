@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\python %~dp0\platform\cli_single_project_build.py %*
