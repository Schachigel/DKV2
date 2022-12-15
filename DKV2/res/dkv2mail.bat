rem Dies ist die Windows-Version für das Senden einer eMail-
rem Das Programm nutzt SwithMail.
rem Die Konfiguration der Sendeparameter erfolgt in der Bedienoberfläche
rem von SwithMail.

SwithMail.exe /s /from "mymail@mailbox.org" /to "%1" /sub "%2" /a "%3" /b "%4"

IF %ERRORLEVEL% NEQ 0 echo "############# Fehler beim Senden: %1"
