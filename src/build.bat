setlocal
cd /d "%~dp0"
call toolchain.bat
msbuild OshiroAssist.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
set DIST_DIR="_dist\OshiroAssist"
mkdir "%DIST_DIR%"
mkdir "%DIST_DIR%\bin"
mkdir "%DIST_DIR%\resources"
copy _out\x64_Release\OshiroAssist.exe "%DIST_DIR%"
copy MouseReplayer\Externals\lib\tbb.dll "%DIST_DIR%\bin"
copy MouseReplayer\Externals\lib\opencv_core451.dll "%DIST_DIR%\bin"
copy MouseReplayer\Externals\lib\opencv_imgcodecs451.dll "%DIST_DIR%\bin"
copy MouseReplayer\Externals\lib\opencv_imgproc451.dll "%DIST_DIR%\bin"
copy keymap.txt "%DIST_DIR%"
copy resources "%DIST_DIR%\resources"
exit /B 0
