@echo OFF
set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\msvc\python-embed-amd64\python %~dp0\build_tools\generate_solution\generate_solution.py %*