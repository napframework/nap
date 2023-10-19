@echo OFF

for /F "tokens=2* skip=2" %%a in ('reg query "HKLM\System\CurrentControlSet\Control\Session Manager\Environment" /v "PROCESSOR_ARCHITECTURE"') do set ARCH=%%b

if %ARCH%==AMD64 (
    echo Checking x86-64 architecture: PASS
) else (
    echo Checking x86-64 architecture: FAIL
    echo.
    echo NAP only supports x86-64 systems. Not continuing checks.
    echo Press key to close...
    pause
    exit /B
)

set PYTHONPATH=""
set PYTHONHOME=""

if "%~1" == "--source" (
    set NAP_ROOT=%~dp0\..\..\..\..
) else (
    set NAP_ROOT=%~dp0\..
)
set THIRDPARTY_DIR=%NAP_ROOT%\thirdparty
"%THIRDPARTY_DIR%\python\msvc\x86_64\python" "%NAP_ROOT%\tools\buildsystem\check_build_environment\win64\check_build_environment_continued.py" "%~1"
exit /B
