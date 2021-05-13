:: this batch uses the tool WinDeployQt, which comes with Qt Creator
:: to gather all needed dlls and the executable into one folder
:: This is necessary to execute the binary
:: note: if you use the MSVC build system, you also need to install the
:: corresponding VC Runtime from Microsoft

::@ECHO OFF
:: Edit the paths and file names to the corresponding names on your system

:: update this path every time you update the qt version you are using
SET WINDEPLOY=C:\devtools\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe
:: use the release version of your binary
SET binary=C:\Users\Public\Documents\dev\build-dkv2-Desktop_Qt_5_15_2_MSVC2019_64bit-Release\DKV2\release\DKV2.exe
SET out=C:\Users\HoM\OneDrive\Documents\Esperanza\DKV2-Entwicklung\DKV2.distrib\DKV2

IF NOT EXIST %WINDEPLOY% (
    CALL :ERROR_OUT "Für die Ausführung wird 'WinDeployQt.exe' benötigt."
)
IF NOT EXIST %binary% (
    CALL :ERROR_OUT "Für die Ausführung wird die Angabe des zu installierenden Programms benötigt."
)
IF "%out%"=="" (
    CALL :ERROR_OUT "Ein Ausgabeverzeichnis muss angegeben werden"
)

IF EXIST %out% (
  rd "%out%" || rem
  IF ERRORLEVEL 145 (
    SET /P yesno="Darf das Ausgabeverzeichnis " %out% " geleert werden? (y/n) "
    IF "%yesno%"=="y" (
      rd /s /q %out%
    ) ELSE (
       ERROR_OUT "Das Skript verwendet nur leere Ausgabeverzeichnisse"
    )
  )
)
mkdir %out%
"%WINDEPLOY%" "%binary%" --dir "%out%"
COPY "%binary%" "%out%"
start explorer.exe root,"%out%"


REM: this is the end
GOTO :EOF

:ERROR_OUT
    ECHO ERROR! %1
EXIT 1