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

.\vorlagen\dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" mailtext.txt

{{/Email}}
{{^Email}}
echo "############ {{Vorname}} {{Nachname}} hat keine Email!"
{{/Email}}
{{/Kreditoren}}