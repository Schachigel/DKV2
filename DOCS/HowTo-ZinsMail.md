# Zinsbriefe per eMail versenden
Seit der Pre-Release-Version 0.20.0.4 werden beim Erstellen der jährlichen Zinsmitteilungen noch zwei weitere Dokumente erstellt:

- eine Datei Zinsliste-XXXX.pdf (XXXX ist das Jahr der Abrechnung), in der sich eine Übersicht aller Kreditoren mit aktiven oder im Abrechnungsjahr beendeten Direktkrediten und eine Übersicht über alle anstehenden Überweisungen für die auszuzahlenden Jahreszinsen befindet.

- eine Datei zinsmailsXXXX.bat (XXXX ist wieder das Abrechnungsjahr). Dies ist eine Batch-Verarbeitungsdatei. Sie kann durch Aufruf der Datei in einer Systemkonsole (Linux) bzw. in der Eingabeaufforderung (Windows) ausgeführt werden und sorgt für die Versendung der Zinsbriefe per eMail soweit eMail-Adressen für die jeweiligen Kreditor:innen eingegeben sind.

Der Inhalt der beiden Dateien kann durch die Änderung der jeweiligen Vorlagen beeinflusst werden. Das Layout der Zinsliste wird durch die Datei Ausgabe/vorlagen/zinsliste.html bestimmt.

Der eMail-Text bzw. die genaue Funktion der eMail-Sendebefehle wird in der Datei Ausgabe/vorlagen/zinsmails.bat bestimmt.

## eMails Versenden mit DKV2
Aus der Vorlage Ausgabe/vorlagen/zinsmails.bat wird in DKV2 die Batchdatei Ausgabe/zinsmailsXXXX.bat erzeugt.

(Anm. "Batchdatei" bezeichnet eine Datei, die eine Reihe von Befehlen enthält, die dann durch einfaches Eingeben des Dateinamens in der Eingabeaufforderung ausgeführt werden.)

Zum Verständnis der Funktion hilft ein Blick in die mitgelieferte Vorlage

```
#!/bin/sh

{{#Kreditoren}}
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
{{/Kreditoren}}
```
Die erste Zeile (```#!/bin/sh```) ist nur für Linux interesseant und muss in Windows gelöscht werden. Dazu kann z.B. das Standard-Windows-Programm ```notepad.exe``` oder unter Linux ```gedit``` bzw. ```kate``` verwendet werden.

Die Anweisungen ```{{#Kreditoren}}``` sorgt dafür, dass alle weiteren Zeilen bis ```{{/Kreditoren}}``` für jeden Kreditor in der Abrechnung wiederholt werden.
Für Kreditoren mit eingegebener Email-Adresse wird der Text zwischen ```{{#Email}}``` und dem ersten ```{{\Email}}``` in die Batchdatei übernommen. Dabei werden die Werte ```{{}}``` durch den entsprechenden Text aus der DKV2-Datenbank übernommen. Zwischen ```{{^Email}}``` und dem zweiten ```{{/Email}}``` wird lediglich eine Fehlermeldung erzeugt, sofern für die Kreditor:in keine eMail-Adresse eingegeben ist.

Wichtig ist die Zeile 

```
./vorlagen/dkv2mail "{{Email}}" "Zinsabrechnung" "{{Attachment}}" "
```

Hier wird die Batchdatei dkv2mail.bat (unter Windows) bzw. dkv2mail (unter Linux) im Pfad Ausgabe/vorlagen aufgerufen. Diese Batchdatei versendet eine einzige eMail.

(Anm. Unter Windows muss nicht die Erweiterung '.bat' angegeben werden. Windows sucht automatisch nach Dateien mit dieser Dateinamen-Erweiterung.)

Der Befehl braucht folgende Angaben in dieser Reihenfolge:

- eMailadresse der Empfänger:in in Anführungszeichen
- Betrefftext in Anführungszeichen
- den Dateinamen der zu sendenden PDF-Datei in Anführungszeichen
- den Text der eMail (hier ist nur das Anfangs-Anführungszeichen zu sehen, das Ende-Anführungszeichen steht am Ende des mehrzeiligen eMail-Textes.)

Insbesondere der Betreff-Text (hier "Zinsabrechnung") und der eMail-Text können natürlich angepasst werden.

Hier ein Beispiel der fertig ausgegebenen Datei zinsmailXXXX.bat:
```
./vorlagen/dkv2mail "ruth.adams@aol.de" "Zinsabrechnung" "Jahresinfo 2021_19_Adams, Ruth.pdf" "
Liebe:r Ruth Adams,

Im Anhang befindet sich die Zinsabrechnung für das abgelaufene Jahr.

Wir bedanken uns für die Unterstützung.

Mit freundlichen Grüßen

Die Direktkredit-Verwaltung"


./vorlagen/dkv2mail "benno.hurgedue@telekom.de" "Zinsabrechnung" "Jahresinfo 2021_15_Hurgedue, Benno.pdf" "
Liebe:r Benno Hurgedue,

Im Anhang befindet sich die Zinsabrechnung für das abgelaufene Jahr.

Wir bedanken uns für die Unterstützung.

Mit freundlichen Grüßen

Die Direktkredit-Verwaltung"


echo "############ Rupert Naseweis hat keine Email!"
```
Diese Batchdatei würde zwei eMails versenden (/vorlagen/dkv2mail.bat wird zweimal aufgerufen) und würde eine Meldung ausgeben, dass der betreffende Kreditor keine eMail-Adresse in der Datenbank hat.

## Anpassen der Batchdateien für Linux
### Anpassung der Batchdatei dkv2mail
Die Datei dkv2mail (ohne Dateinamen-Erweiterung) wird ausschließlich unter Linux genutzt. (Für Windows ist die Datei dkv2mail.bat zuständig.) Sie hat folgenden Inhalt:

```
#!/bin/sh
# Dies ist die Linux-Version für das Senden von eMails
# Es wird das Programm mailx aus den mailutils genutzt.

echo "$4" | mailx -aFrom:mymail@mailbox.org -s "$2" -A "$3" "$1"
if [ $? -ne 0 ]
then
    echo "############### Fehler beim Senden: $1"
fi 
```
Die erste Zeile (```#!/bin/sh```) muss auf jeden Fall stehen bleiben, damit diese Datei als Batchdatei aufführbar ist.
Die weiteren Zeilen, die mit ```#``` beginnen, sind lediglich Kommentare. Wesentlich ist die Zeile, die mit ```echo "$4" | mailx``` beginnt. Hier muss auf jeden Fall der Text ```"mymail@mailbox.org"``` durch die eigene Sende-eMail-Adresse ersetzt werden.

Falls nicht das Programm ```mailx``` verwendet werden soll, muss natürlich die gesamte Befehlszeile an das genutzte Programm angepasst werden.

### eMail-Programm mailx installieren und einrichten

1. Das Programm ```mailx``` kann einfach installiert werden mit folgendem Befehl:
```
sudo apt-get install ssmtp mailutils
```

2. Jetzt kommt leider der hässliche Teil - es müssen einige Systemdateien geändert werden. Das ganze ist deutschsprachig hier beschrieben: https://decatec.de/home-server/linux-einfach-e-mails-senden-mit-ssmtp/ . Hier als Beispiel die Dateien, die editiert werden müssen:

```sudo gedit /etc/ssmtp/ssmtp.conf```
```
#
# Config file for sSMTP sendmail
#
# The person who gets all mail for userids < 1000
# Make this empty to disable rewriting.
root=

# The place where the mail goes. The actual machine name is required no 
# MX records are consulted. Commonly mailhosts are named mail.domain.com
# Beispiel: Mail-Provider mailbox.org
mailhub=smtp.mailbox.org:587
useSTARTTLS=YES
AuthUser=BEISPIEL@mailbox.org
AuthPass=BEISPIEL-PASSWORT
TLS_CA_File=/etc/pki/tls/certs/ca-bundle.crt

# Where will the mail seem to come from?
#rewriteDomain=

# The full hostname
hostname=BEISPIEL@mailbox.org

# Are users allowed to set their own From: address?
# YES - Allow the user to specify their own From: address
# NO - Use the system generated From: address
FromLineOverride=YES
```
```sudo gedit /etc/ssmtp/revaliases```
```
# sSMTP aliases
# 
# Format:	local_account:outgoing_address:mailhub
#
# Example: root:your_login@your.domain:mailhub.your.domain[:port]
# where [:port] is an optional port number that defaults to 25.
wilhelm:BEISPIEL@mailbox.org:smtp.mailbox.org:587
```
3. Du musst in der Datei ```Ausgabe/vorlagen/dkv2mail``` die Absenderadresse anpassen, falls da immer noch ```"mymail@mailbox.org"``` drin steht.

4. Passe die Datei ```Ausgabe/vorlagen/zinsmails.bat``` an deine Wünsche an:
- Erste Zeile unbedingt behalten (```#!/bin/sh```)
- Betreff-Text
- eMail-Text, hierbei auf die beiden Anführungszeichen am Anfang und am Ende achten!

5. Jetzt kannst du DKV2 starten und unter ```Verträge/Listen drucken/Zinsbriefe``` die Erstellung von Zinsbriefen anwerfen. Du solltest danach in deinem Ausgabe-Verzeichnis alle PDF-Dateien mit den Zinsbriefen haben und außerdem auch die Datei zinsmails2022.bat (als Beispiel).

6. Öffne die Batchdatei zinsmails2022.bat (z.B. mit ```gedit``` und lösche alle Zeilen von der zweiten Zeile an, die mit ```dkv2mail``` beginnt. In der ersten Zeile, die mit ```dkv2mail``` beginnt, ersetzt du die eMail-Adresse des Empfängers durch deine eigene eMail-Adresse. Jetzt Abspeichern.

7. Öffne die Windows-Eingabeaufforderung und wechsele in das Ausgabeverzeichnis von DKV2, z.B. so:

```
cd /home/wilhelm/DKV2/Ausgabe
```
8. Gib in der Eingabeaufforderung den Befehl zum Senden der eMails:
```
./zinsmails2022.bat
```
Jetzt solltest du auf deine eigene eMail-Adresse eine Mail mit dem gewünschten Text und dem Zinsbrief des ersten Kreditor als Anhang erhalten.

Wenn du an dieser Stelle hängen bleibst und nicht weißt, wie du weiter kommst, kannst du mich per eMail kontaktieren:
mailto:Wilhelm.Pflueger@mailbox.org

9. Wenn die eMail wie gewünscht bei dir angekommen ist, kannst du die Erzeugung der Zinsbriefe noch einmal starten (siehe Punkt 7.). Nun kannst du den Befehl zum Senden aller Mails geben, indem du wie in Punkt 8. die erzeugte Batchdatei zinsmails2022 jetzt unverändert startest.

## Anpassen der Batchdateien für Windows

### Anpassung der Batchdatei dkv2mail.bat
Die Datei dkv2mail.bat wird ausschließlich unter Windows genutzt. (Für Linux ist die Datei dkv2mail ohne Dateinamen-Erweiterung zuständig.) Sie hat folgenden Inhalt:

```
rem Dies ist die Windows-Version für das Senden einer eMail-
rem Das Programm nutzt SwithMail.
rem Die Konfiguration der Sendeparameter erfolgt in der Bedienoberfläche
rem von SwithMail.

SwithMail.exe /s /from "mymail@mailbox.org" /to "%1" /sub "%2" /a "%3" /b "%4"

IF %ERRORLEVEL% NEQ 0 echo "############# Fehler beim Senden: %1"

```

Die ersten Zeilen, die mit ```rem``` beginnen, sind lediglich Kommentare. Wesentlich ist die Zeile, die mit ```SwithMail.exe``` beginnt. Hier muss auf jeden Fall der Text ```"mymail@mailbox.org"``` durch die eigene Sende-eMail-Adresse ersetzt werden.

Falls nicht das Programm SwithMail.exe verwendet werden soll, muss natürlich die gesamte Befehlszeile an das genutzte Programm angepasst werden.

### eMail-Programm SwithMail installieren und einrichten

1. Das Programm ```SwithMail``` kann kostenfrei heruntergeladen und genutzt werden. Hier ist die Download-Seite:
https://www.tbare.com/software/swithmail/ .

2. Nach Druck auf den Download-Button bekommst du eine Datei ```SwithMailv2240.zip``. Diese zip-Datei muss durch Doppelklick ausgepackt werden.

3. In der zip-Datei befindet sich das eigentliche Programm ```SwithMail.exe``` und eine (recht schmale) Beschreibung in der Datei ```Readme.txt```. Kopiere die Programmdatei ```SwithMail.exe``` am besten in dein DKV2-Ausgabeverzeichnis.

4. Nun musst du durch Start von ```SwithMail.exe``` in der grafischen Benutzeroberfläche die notwendigen Angaben über dein eMail-Konto machen: Benutzername, Password, Server usw. Diese Angaben kannst du in den Einstellungen deines eMail-Programmes nachsehen (z.B. in Thunderbird). Eine deutschsprachige Anleitung gibt es z.B. hier: https://www.tutonaut.de/anleitung-mails-ohne-interaktion-per-terminalbatch-verschicken/ . Du kannst dir nun selbst (oder eine:r Freund:in) eine eMail senden und damit ausprobieren, ob das Versenden mit SwithMail funktioniert.

5. Du musst in der Datei ```Ausgabe/vorlagen/dkv2mail.bat``` die Absenderadresse anpassen, falls da immer noch ```"mymail@mailbox.org"``` drin steht.

6. Passe die Datei ```Ausgabe/vorlagen/zinsmails.bat``` an deine Wünsche an:
- Erste Zeile löschen (```#!/bin/sh```)
- Betreff-Text
- eMail-Text, hierbei auf die beiden Anführungszeichen am Anfang und am Ende achten!

7. Jetzt kannst du DKV2 starten und unter ```Verträge/Listen drucken/Zinsbriefe``` die Erstellung von Zinsbriefen anwerfen. Du solltest danach in deinem Ausgabe-Verzeichnis alle PDF-Dateien mit den Zinsbriefen haben und außerdem auch die Datei zinsmails2022.bat (als Beispiel).

8. Öffne die Batchdatei zinsmails2022.bat (z.B. mit ```notepad.exe``` und lösche alle Zeilen von der zweiten Zeile an, die mit ```dkv2mail``` beginnt. In der ersten Zeile, die mit ```dkv2mail``` beginnt, ersetzt du die eMail-Adresse des Empfängers durch deine eigene eMail-Adresse. Jetzt Abspeichern.

9. Öffne die Windows-Eingabeaufforderung und wechsele in das Ausgabeverzeichnis von DKV2, z.B. so:

```
cd c:\User\Wilhelm\Direktkredite\Ausgabe
```
10. Gib in der Eingabeaufforderung den Befehl zum Senden der eMails:
```
zinsmails2022
```
Jetzt solltest du auf deine eigene eMail-Adresse eine Mail mit dem gewünschten Text und dem Zinsbrief des ersten Kreditor als Anhang erhalten.

Wenn du an dieser Stelle hängen bleibst und nicht weißt, wie du weiter kommst, kannst du mich per eMail kontaktieren:
mailto:Wilhelm.Pflueger@mailbox.org

11. Wenn die eMail wie gewünscht bei dir angekommen ist, kannst du die Erzeugung der Zinsbriefe noch einmal starten (siehe Punkt 7.). Nun kannst du den Befehl zum Senden aller Mails geben, indem du wie in Punkt 10. die erzeugte Batchdatei zinsmails2022 jetzt unverändert startest.

Good Luck
