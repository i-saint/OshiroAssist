setlocal
cd /d "%~dp0"
call toolchain.bat
msbuild OshiroAssist.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo

set EXTERNALS_DIR="%~dp0Marionette\src\Externals"
set ZIP="%EXTERNALS_DIR%\7za.exe"
set DIST_DIR="_dist\OshiroAssist"
set BIN_DIR="%DIST_DIR%\bin"
set RES_DIR="%DIST_DIR%\resources"
mkdir %DIST_DIR%
mkdir %RES_DIR%
copy _out\x64_Release\OshiroAssist.exe %DIST_DIR%
copy keymap.txt %DIST_DIR%
xcopy /Y resources\* %RES_DIR%
cd "_dist"
%ZIP% a OshiroAssist.zip OshiroAssist
exit /B 0
