@ECHO OFF
if "%VCINSTALLDIR%"=="" call "%VS140COMNTOOLS%vsvars32.bat"
if "%VCINSTALLDIR%"=="" call "%VS100COMNTOOLS%vsvars32.bat"
if "%VCINSTALLDIR%"=="" call "%VS110COMNTOOLS%vsvars32.bat"
if "%VCINSTALLDIR%"=="" call "%VS120COMNTOOLS%vsvars32.bat"
@nmake /nologo %1
