@ECHO OFF

pushd  "%~dp0"

if NOT EXIST ..\..\..\DKV2.release\DKV2\release\DKV2.exe (
    @ECHO error - release file does not EXIST
    GOTO eof
)
ECHO "Copy file to distrib folder if necesarry"
xcopy /y /d  ..\..\..\DKV2.release\DKV2\release\DKV2.exe ..\..\..\DKV2.distrib\DKV2\

ECHO "Update bakup file"
IF EXIST DKV2.msi.bak del DKV2.msi.bak
ren DKV2.msi DKV2.msi.bak

ECHO "Running candle..."
candle dkv2.wxs
if %errorlevel% neq 0 (
    echo FEHLER IN candle.exe!
    exit /b %errorlevel%
)

ECHO "Running light..."
light dkv2.wixobj
if %errorlevel% neq 0 (
    echo FEHLER IN light.exe!
    exit /b %errorlevel%
)

ECHO "Copy result to distrib folder ..."
xcopy /d /y   DKV2.msi    ..\..\..\DKV2.distrib\

popd
