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

./vorlagen/dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" mailtext.txt

{{/Email}}
{{^Email}}
echo "############ {{Vorname}} {{Nachname}} hat keine Email!"
{{/Email}}
{{/Kreditoren}}