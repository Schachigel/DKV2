@echo off

{{#Kreditoren}}
{{#Email}}
rem Alten mailtext.txt löschen
del mailtext.txt

rem Hier ist der Mailtext!
(
echo Liebe:r {{Vorname}} {{Nachname}},
echo.
echo Im Anhang befindet sich die Zinsabrechnung für das abgelaufene Jahr.
echo.
echo Wir bedanken uns für die Unterstützung.
echo.
echo Mit freundlichen Grüßen
echo.
echo Die Direktkredit-Verwaltung
) >mailtext.txt

echo {{Vorname}} {{Nachname}} -- {{Email}}
.\vorlagen\dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" mailtext.txt
rem Aktiviere eine von den drei Zeilen, indem du das 'rem' am Anfang löschst.
rem timeout /t 20
rem powershell -ExecutionPolicy Bypass -Command "Start-Sleep -Seconds 20"
rem ping localhost -t 20 >nul

{{/Email}}
{{^Email}}
echo "############ {{Vorname}} {{Nachname}} hat keine Email!"
{{/Email}}
{{/Kreditoren}}