@echo OFF

set NAP_ROOT=%~dp0\..\..
set THIRDPARTY_DIR=%NAP_ROOT%\..\thirdparty
if not exist %THIRDPARTY_DIR% (
    echo Error: The third party repository ^('thirdparty'^) needs to be cloned alongside the main repository.
    echo.
    echo Once thirdparty is in place run check_build_environment.bat first.
    exit /b
)

set PYTHONPATH=
set PYTHONHOME=
%THIRDPARTY_DIR%\python\msvc\x86_64\python %~dp0\cli_single_project_build.py %*
