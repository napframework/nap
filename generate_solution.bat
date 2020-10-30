@echo OFF

set THIRDPARTY_DIR=%~dp0\..\thirdparty
if not exist %THIRDPARTY_DIR% (
    echo Error: The third party repository ^('thirdparty'^) needs to be cloned alongside the main repository.
    echo.
    echo Once thirdparty is in place run check_build_environment.bat first.
    exit /b
)

set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\msvc\python-embed-amd64\python %~dp0\build_tools\generate_solution\generate_solution.py %*