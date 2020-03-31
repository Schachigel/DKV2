@ECHO OFF

pushd  "%~dp0"

xcopy /y /d ..\..\..\DKV2.release\DKV2\DKV2.exe ..\..\..\DKV2.distrib\DKV2\

IF EXIST DKV2.msi.bak del DKV2.msi.bak
ren DKV2.msi DKV2.msi.bak

candle dkv2.wxs
if %errorlevel% neq 0 (
    echo FEHLER IN candle.exe!
    exit /b %errorlevel%
)


light dkv2.wixobj
if %errorlevel% neq 0 (
    echo FEHLER IN light.exe!
    exit /b %errorlevel%
)
popd
