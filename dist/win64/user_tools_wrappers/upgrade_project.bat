@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\python %~dp0\platform\upgrade_project_by_name.py %*
