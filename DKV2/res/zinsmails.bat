#!/bin/sh

{{#creditors}}
{{#Email}}

./vorlagen/dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" "
Liebe:r {{Vorname}} {{Nachname}},

Im Anhang befindet sich die Zinsabrechnung für das abgelaufene Jahr.

Wir bedanken uns für die Unterstützung.

Mit freundlichen Grüßen

Die Direktkredit-Verwaltung"

{{/Email}}
{{^Email}}
echo "############ {{Vorname}} {{Nachname}} hat keine Email!"
{{/Email}}
{{/creditors}}