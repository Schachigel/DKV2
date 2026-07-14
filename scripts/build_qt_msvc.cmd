@echo off
call "x:\tools\Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
set PATH=X:\tools\Qt\Tools\QtCreator\bin\jom;%PATH%
"x:\tools\Qt\Tools\CMake_64\bin\cmake.exe" --build --preset qt-msvc-debug --config Debug
