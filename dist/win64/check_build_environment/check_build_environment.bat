@echo OFF

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

rem set OS=32BIT
if %OS%==32BIT (
    echo Checking x86-64 architecture: FAIL
    echo.
    echo NAP only supports x86-64 systems. Not continuing checks.
    echo Press key to close...
    pause
    exit /B
) else (
    echo Checking x86-64 architecture: PASS
)

set PYTHONPATH=
set PYTHONHOME=
%~dp0\..\thirdparty\python\python %~dp0\platform\check_build_environment_continued.py