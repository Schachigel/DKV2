@ECHO OFF
PUSHD  "%~dp0"
CD 

IF EXIST "c:\Program Files\DK Verwaltung 2\DKV2.exe" (
    CALL :onERROR "TEST needs dkv2 to be uninstalled"
    EXIT /b %ERRORLEVEL%
)

REM installation
msiexec.exe /qn /norestart /l*v .\dkv2msi-install.log  /i dkv2.msi

REM check installation success
IF NOT EXIST "c:\Program Files\DK Verwaltung 2\DKV2.exe" (
    CALL :onERROR "ERRRO in file installation"
    EXIT /b %ERRORLEVEL%
)

IF NOT EXIST "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\DKV2\DKV2.lnk" (
    CALL :onERROR "ERROR in shortcut installation"
    EXIT /b %ERRORLEVEL%
)

REM uninstallation
msiexec.exe /qn /norestart /l*v .\dkv2msi-uninstall.log  /x dkv2.msi

REM check uninstallation success
IF EXIST "c:\Program Files\DK Verwaltung 2\DKV2.exe" (
    CALL :onERROR "ERROR in file un installation"
    EXIT /b %ERRORLEVEL%
)

IF EXIST "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\DKV2\DKV2.lnk" (
    CALL :onERROR  "ERROR in shortcut un installation"
    EXIT /b %ERRORLEVEL%
)
popd
ECHO TEST succeeded
goto:EOF

:onERROR
ECHO TEST failed.
ECHO %~1
POPD
SET ERRORLEVEL=1
EXIT /b 
