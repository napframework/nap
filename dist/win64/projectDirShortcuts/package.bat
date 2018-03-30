@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\..\thirdparty\python\python %~dp0\..\..\tools\platform\packageProjectByDir.py %~dp0 %*