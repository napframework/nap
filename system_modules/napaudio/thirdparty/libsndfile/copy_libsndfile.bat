@echo off
setlocal

:: === Base paths ===
set VCPKG_ROOT=C:\vcpkg
set VCPKG=%VCPKG_ROOT%\installed\x64-windows
set DEST_BASE=C:\Users\roy\nap\system_modules\napaudio\thirdparty
set ARCH=msvc\x86_64

echo Copying libraries...

:: === libFLAC ===
set DEST_FLAC=%DEST_BASE%\libflac\%ARCH%
if not exist "%DEST_FLAC%\lib" mkdir "%DEST_FLAC%\lib"
if not exist "%DEST_FLAC%\include" mkdir "%DEST_FLAC%\include"


copy "%VCPKG%\bin\FLAC++.dll" "%DEST_FLAC%\lib\"
copy "%VCPKG%\bin\FLAC++.pdb" "%DEST_FLAC%\lib\"
copy "%VCPKG%\bin\FLAC.dll" "%DEST_FLAC%\lib\"
copy "%VCPKG%\bin\FLAC.pdb" "%DEST_FLAC%\lib\"
copy "%VCPKG%\lib\FLAC++.lib" "%DEST_FLAC%\lib\"
copy "%VCPKG%\lib\FLAC.lib" "%DEST_FLAC%\lib\"
robocopy "%VCPKG%\include\FLAC" "%DEST_FLAC%\include\FLAC" /E
copy "%VCPKG_ROOT%\packages\libflac_x64-windows\share\libflac\copyright" "%DEST_FLAC%\copyright"

:: === libmp3lame ===
set DEST_LAME=%DEST_BASE%\libmp3lame\%ARCH%
if not exist "%DEST_LAME%\lib" mkdir "%DEST_LAME%\lib"
if not exist "%DEST_LAME%\include" mkdir "%DEST_LAME%\include"


copy "%VCPKG%\bin\libmp3lame.dll" "%DEST_LAME%\lib\"
copy "%VCPKG%\bin\libmp3lame.pdb" "%DEST_LAME%\lib\"
copy "%VCPKG%\lib\libmp3lame.lib" "%DEST_LAME%\lib\"
robocopy "%VCPKG%\include\lame" "%DEST_LAME%\include\lame" /E
copy "%VCPKG_ROOT%\packages\mp3lame_x64-windows\share\mp3lame\copyright" "%DEST_LAME%\copyright"


:: === libmpg123 ===
set DEST_MPG=%DEST_BASE%\libmpg123\%ARCH%
if not exist "%DEST_MPG%\lib" mkdir "%DEST_MPG%\lib"
if not exist "%DEST_MPG%\include" mkdir "%DEST_MPG%\include"

copy "%VCPKG%\bin\mpg123.dll" "%DEST_MPG%\lib\"
copy "%VCPKG%\bin\mpg123.pdb" "%DEST_MPG%\lib\"
copy "%VCPKG%\lib\mpg123.lib" "%DEST_MPG%\lib\"
copy "%VCPKG%\include\fmt123.h" "%DEST_MPG%\include\"
copy "%VCPKG%\include\mpg123.h" "%DEST_MPG%\include\"
copy "%VCPKG_ROOT%\packages\mpg123_x64-windows\share\mpg123\copyright" "%DEST_MPG%\copyright"


:: === libogg ===
set VCPKG=%VCPKG%
set DEST_OGG=%DEST_BASE%\libogg\%ARCH%
if not exist "%DEST_OGG%\lib" mkdir "%DEST_OGG%\lib"
if not exist "%DEST_OGG%\include" mkdir "%DEST_OGG%\include"


copy "%VCPKG%\bin\ogg.dll" "%DEST_OGG%\lib\"
copy "%VCPKG%\bin\ogg.pdb" "%DEST_OGG%\lib\"
copy "%VCPKG%\lib\ogg.lib" "%DEST_OGG%\lib\"
robocopy "%VCPKG%\include\ogg" "%DEST_OGG%\include\ogg" /E
copy "%VCPKG_ROOT%\packages\libogg_x64-windows\share\libogg\copyright" "%DEST_OGG%\copyright"


:: === libopus ===
set VCPKG=%VCPKG%
set DEST_OPUS=%DEST_BASE%\libopus\%ARCH%
if not exist "%DEST_OPUS%\lib" mkdir "%DEST_OPUS%\lib"
if not exist "%DEST_OPUS%\include" mkdir "%DEST_OPUS%\include"

copy "%VCPKG%\bin\opus.dll" "%DEST_OPUS%\lib\"
copy "%VCPKG%\bin\opus.pdb" "%DEST_OPUS%\lib\"
copy "%VCPKG%\lib\opus.lib" "%DEST_OPUS%\lib\"
robocopy "%VCPKG%\include\opus" "%DEST_OPUS%\include\opus" /E
copy "%VCPKG_ROOT%\packages\opus_x64-windows\share\opus\copyright" "%DEST_OPUS%\copyright"



:: === libsndfile ===
set DEST_SNDFILE=%DEST_BASE%\libsndfile\%ARCH%
if not exist "%DEST_SNDFILE%\lib" mkdir "%DEST_SNDFILE%\lib"
if not exist "%DEST_SNDFILE%\include" mkdir "%DEST_SNDFILE%\include"

copy "%VCPKG%\bin\sndfile.dll" "%DEST_SNDFILE%\lib\"
copy "%VCPKG%\bin\sndfile.pdb" "%DEST_SNDFILE%\lib\"
copy "%VCPKG%\lib\sndfile.lib" "%DEST_SNDFILE%\lib\"
copy "%VCPKG%\include\sndfile.h" "%DEST_SNDFILE%\include\"
copy "%VCPKG%\include\sndfile.hh" "%DEST_SNDFILE%\include\"
copy "%VCPKG_ROOT%\packages\libsndfile_x64-windows\share\libsndfile\copyright" "%DEST_SNDFILE%\copyright"



:: === libvorbis ===
set DEST_VORBIS=%DEST_BASE%\libvorbis\%ARCH%
if not exist "%DEST_VORBIS%\lib" mkdir "%DEST_VORBIS%\lib"
if not exist "%DEST_VORBIS%\include" mkdir "%DEST_VORBIS%\include"
copy "%VCPKG_ROOT%\packages\libvorbis_x64-windows\share\libvorbis\copyright" "%DEST_VORBIS%\copyright"


copy "%VCPKG%\bin\vorbis.dll" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\bin\vorbis.pdb" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\lib\vorbis.lib" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\bin\vorbisenc.dll" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\bin\vorbisenc.pdb" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\lib\vorbisenc.lib" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\bin\vorbisfile.dll" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\bin\vorbisfile.pdb" "%DEST_VORBIS%\lib\"
copy "%VCPKG%\lib\vorbisfile.lib" "%DEST_VORBIS%\lib\"
robocopy "%VCPKG%\include\vorbis" "%DEST_VORBIS%\include\vorbis" /E





echo Done!
pause
endlocal
