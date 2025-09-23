#!/bin/sh

{{#Kreditoren}}
{{#Email}}
rm mailtext.txt

echo "
Liebe:r {{Vorname}} {{Nachname}},

Im Anhang befindet sich die Zinsabrechnung für das abgelaufene Jahr.

Wir bedanken uns für die Unterstützung.

Mit freundlichen Grüßen

Die Direktkredit-Verwaltung" >mailtext.txt

echo "{{Vorname}} {{Nachname}} -> {{Email}}"
./vorlagen/dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" mailtext.txt
sleep 20
{{/Email}}
{{^Email}}
echo "############ {{Vorname}} {{Nachname}} hat keine Email!"
{{/Email}}
{{/Kreditoren}}