setlocal
cd /d "%~dp0"
call toolchain.
msbuild OshiroAssist.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo
set DIST_DIR="_dist\OshiroAssist"
mkdir "%DIST_DIR%"
mkdir "%DIST_DIR%\bin"
mkdir "%DIST_DIR%\recources"
copy _out\x64_Release\OshiroAssist.exe "%DIST_DIR%"
copy MouseReplayer\Externals\lib\*.dll "%DIST_DIR%\bin"
copy keymap.txt "%DIST_DIR%"
copy resources "%DIST_DIR%\recources"
exit /B 0
