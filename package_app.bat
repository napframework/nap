@echo OFF
set app_name = %1

if %1.==. GOTO :no_app_specified
if %2.==. GOTO :default_build_dir
goto :custom_build_dir

:custom_build_dir
    set app_name = %1
    set build_dir = %2
    set %app_title% = %app_name%
    goto :execute

:default_build_dir
    set app_name = %1
    set build_dir = build
    set %app_title% = %app_name%
    goto :execute

:execute
    rmdir %build_dir%/bin -r
    cmake -S . -B %build_dir% -DAPP_INSTALL_NAME=%app_name%
    cmake --build %build_dir% --target %app_name% --config Release
    cmake --install %build_dir% --prefix install
    goto :eof

:no_app_specified
    echo Error: no app specified. Usage: package_app [app name] [optional: build directory]
    goto :eof