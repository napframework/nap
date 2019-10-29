@echo off
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\msvc\python-embed-amd64\python %~dp0\build_tools\package\package.py %*
