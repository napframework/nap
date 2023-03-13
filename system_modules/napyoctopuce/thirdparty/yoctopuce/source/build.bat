@ECHO OFF
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
echo.
echo Build yapi static and dynamic libray for C++
echo ============================================

cd Binaries\
call nmake /nologo %1
IF ERRORLEVEL 1 goto error
cd ..\
echo done

set failled=
FOR /D %%A IN (Examples\*) DO (call:BuildDir %%A %1)
IF NOT DEFINED failled goto end

echo.
echo COMPILATION HAS FAILLED ON DIRECTORIES :
echo %failled%

goto error
:BuildDir
echo build %~1 %~2
cd %~1
call nmake /nologo %~2
IF ERRORLEVEL 1 set failled=%failled% %~1
cd ..\..\
echo done
GOTO:EOF

goto end
:error
echo error
exit /b 1
:end