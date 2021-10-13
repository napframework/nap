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

set PYTHONPATH=""
set PYTHONHOME=""

if "%~1" == "--source" goto SOURCE
"%~dp0\..\thirdparty\python\python" "%~dp0\platform\check_build_environment_continued.py"
exit /B

: SOURCE
set NAP_ROOT=%~dp0\..\..\..
set THIRDPARTY_DIR=%NAP_ROOT%\..\thirdparty
if exist %THIRDPARTY_DIR% (
    echo Checking for third party repository: PASS
) else (
    echo Checking for third party repository: FAIL
    echo.
    echo The third party repository ^('thirdparty'^) needs to be cloned alongside the main repository.
    echo.
    echo Not continuing checks. Re-run this script after cloning.
    echo.
    echo Press key to close...
    pause
    exit /B        
)
"%THIRDPARTY_DIR%\python\msvc\x86_64\python" "%~dp0\check_build_environment_continued.py" "--source"
exit /B
