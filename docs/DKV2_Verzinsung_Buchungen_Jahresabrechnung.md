# DKV2 – Verzinsung, Buchungen und Jahresabrechnung

*(Arbeitsstand)*

Dieses Dokument beschreibt die fachlichen Regeln und Datenstrukturen von **DKV2** im Bereich der verzinslichen Verträge, ihrer Buchungen und der jährlichen Abrechnung.

---

## 1. Zweck und Geltungsbereich

Dieses Dokument legt fest:

* welche Begriffe verwendet werden und wie sie zu verstehen sind,
* welche Bedeutung einzelnen Tabellen und Feldern zukommt,
* wie sich aus Buchungen, Vertragsparametern und Stichtagen relevante Größen ableiten lassen.

Der Schwerpunkt liegt auf **Konsistenz** und **Nachvollziehbarkeit** der Prozesse rund um Ein- und Auszahlungen, Verzinsung und Jahresabschluss. Technische Details, z. B. konkrete SQL-Abfragen oder Implementierungen, sind nachgeordnet.

---

## 2. Datenbankschema (relevanter Ausschnitt)

### 2.1 Kreditoren

```sql
CREATE TABLE "Kreditoren" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "Vorname" TEXT DEFAULT '',
  "Nachname" TEXT DEFAULT '',
  "Strasse" TEXT DEFAULT '',
  "Plz" TEXT DEFAULT '',
  "Stadt" TEXT DEFAULT '',
  "Land" TEXT DEFAULT '',
  "Telefon" TEXT DEFAULT '',
  "Email" TEXT DEFAULT '',
  "Anmerkung" TEXT DEFAULT '',
  "Kontakt" TEXT DEFAULT '',
  "Buchungskonto" TEXT DEFAULT '',
  "IBAN" TEXT DEFAULT '',
  "BIC" TEXT DEFAULT '',
  "Zeitstempel" DATETIME DEFAULT CURRENT_TIMESTAMP,
  UNIQUE("Vorname","Nachname","Strasse","Stadt")
);
```

### 2.2 Vertraege

```sql
CREATE TABLE "Vertraege" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "KreditorId" INTEGER NOT NULL,
  "Kennung" TEXT UNIQUE,
  "Anmerkung" TEXT DEFAULT '',
  "ZSatz" INTEGER NOT NULL DEFAULT 0,
  "Betrag" INTEGER NOT NULL DEFAULT 0,
  "thesaurierend" INTEGER NOT NULL DEFAULT 1,
  "Vertragsdatum" TEXTDATE NOT NULL,
  "Kfrist" INTEGER NOT NULL DEFAULT 6,
  "AnlagenId" INTEGER,
  "LaufzeitEnde" TEXTDATE NOT NULL DEFAULT '9999-12-31',
  "zActive" BOOLEAN DEFAULT TRUE,
  "KueDatum" TEXTDATE NOT NULL DEFAULT '9999-12-31',
  "Zeitstempel" DATETIME DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY("KreditorId") REFERENCES "Kreditoren"("id") ON DELETE CASCADE,
  FOREIGN KEY("AnlagenId") REFERENCES "Geldanlagen"("rowid") ON DELETE SET NULL
);
```

### 2.3 Buchungen

```sql
CREATE TABLE "Buchungen" (
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "VertragsId" INTEGER NOT NULL,
  "Datum" TEXTDATE NOT NULL DEFAULT '9999-12-31',
  "BuchungsArt" INTEGER NOT NULL,
  "Betrag" INTEGER NOT NULL,
  "Überschrieben" TEXTDATE NOT NULL DEFAULT '1900-01-01',
  "Zeitstempel" DATETIME DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY("VertragsId") REFERENCES "Vertraege"("id") ON DELETE RESTRICT
);
```

---

## 3. Zentrale Objekte

### 3.1 Kreditor

Ein *Kreditor* ist die natürliche oder juristische Person, zu der ein Vertrag gehört. Relevante Stammdaten sind u. a. Name, Adresse, E-Mail und IBAN.

### 3.2 Vertrag

Ein *Vertrag* repräsentiert eine verzinsliche Geldanlage eines Kreditors.

Wichtige Felder:

* **Kennung**: eindeutige, alphanumerische Vertragskennung (nicht zwingend numerisch)
* **ZSatz**: Zinssatz als ganze Zahl in Hundertstel-Prozent (z. B. 250 = 2,50 %)
* **thesaurierend**: steuert die Behandlung der Zinsen; die einzelnen Zinsmodelle und ihre Auswirkungen werden in Abschnitt **4.4 Zinsmodelle und Buchungslogik** erläutert
* **zActive**: gibt an, ob der Vertrag aktuell verzinst wird; gespeichert als **1 (wahr)** bzw. **0 (falsch)**. Mit diesem Feld kann ein Vertrag vorübergehend unverzinslich begonnen werden, um nach der Aktivierung der Zinszahlung normal verzinst zu werden. Der Übergang von verzinst zu unverzinst ist nicht vorgesehen.

---

## 4. Buchungen

### 4.1 Allgemeines

* Alle finanziellen Bewegungen werden als Buchungen gespeichert.
* Es werden **keine aggregierten Werte, Salden oder Zwischensummen** gespeichert; alle fachlichen Zustände ergeben sich ausschließlich aus den vorhandenen Buchungen.
* Beträge werden als ganze Zahlen in Cent gespeichert (1 EUR = 100 ct).
* Jede Buchung ist genau einem Vertrag zugeordnet.

### 4.2 Buchungsarten

| Typ | Bezeichnung          | Bedeutung                                                                      |
| --- | -------------------- | ------------------------------------------------------------------------------ |
| 1   | Einzahlung           | Betrag ganzzahlig in ct, positiv                                               |
| 2   | Auszahlung           | Betrag ganzzahlig in ct, negativ                                               |
| 4   | Zinsanrechnung       | unterjährig; Betrag ganzzahlig in ct                                           |
| 8   | Jahreszinsanrechnung | Jahreszins / Jahresabschluss; Betrag ganzzahlig in ct                          |
| 16  | Aktivierung          | Aktivierung der Zinszahlung für einen Vertrag; Betrag ganzzahlig in ct, Wert 0 |

Regeln:

* Die erste Buchung eines Vertrags ist immer **Typ 1 (Einzahlung)**.
* **Typ 1** und **Typ 2** sind grundsätzlich **nicht am 31.12.** zulässig.
* Ausnahme: Die erste Einzahlung darf am **31.12.** erfolgen.
* **Typ 8** ist immer die letzte Buchung eines Jahres.
* **Typ 16** darf am **31.12.** gebucht werden.

### 4.3 Ersteinzahlung und Verzinsungsbeginn

Für die erste Einzahlung eines Vertrags gelten besondere Regeln, da sie den Beginn des Kreditverhältnisses und der Verzinsung definiert.

* Die **erste Buchung eines Vertrags** ist immer **Typ 1 (Einzahlung)**.
* Die **Ersteinzahlung** darf am **31.12.x** erfolgen. In diesem Fall erfolgt **keine Jahresabrechnung für das Jahr x**, da der **erste Tag eines Kredits zinsfrei** ist.

  * Zum Ausgleich wird der **Auszahlungstag verzinst**.
* Erfolgt die **Ersteinzahlung am 30.12.x**, wird der Vertrag grundsätzlich in die Jahresabrechnung für das Jahr x einbezogen.

  * Bei der Zinsmethode **30/360** ergibt sich dennoch ein **Zins von 0**, da der **31. eines Monats nicht verzinst wird**.

Diese Regeln sind unabhängig vom später gewählten Zinsmodell (`thesaurierend`) und betreffen ausschließlich den **Start des Vertrags und der Verzinsung**.

Für die **Aktivierung der Zinszahlung** gelten zusätzlich folgende Regeln:

* Die Aktivierung der Zinszahlung (**Typ 16**) kann **erst nach der Ersteinzahlung** erfolgen.
* Am Tag der Aktivierung wird **unmittelbar vor der Aktivierungsbuchung** eine **Zinsanrechnung (Typ 4) mit dem Betrag 0** erfasst.
* Erfolgt die **Aktivierung am 31.12.x**, so erfolgt **keine Jahresabrechnung für das Jahr x**.

Für zunächst unverzinste Verträge (`zActive = FALSE`) gilt zusätzlich:

* In der Liste der Verträge wird der Zinsstatus **„ausgesetzt“** angezeigt.
* Die Aktivierung findet man im Kontextmenü als **„Zinszahlung aktivieren“**.
* Die Aktivierung wird als **Buchung Typ 16 (ohne Betrag)** gespeichert.
* **Typ 16** definiert den Start der Verzinsung ab seinem Buchungsdatum.
* Eine Buchung vom Typ 16 erfolgt **pro Vertrag genau einmal** und **nach dem ersten Geldeingang** (Typ 1).
* **Ein- und Auszahlungen vor der Aktivierung** der Verzinsung sind zulässig.
* **Alle Zinsberechnungen vor der Aktivierung** erfolgen **mit dem Zinssatz 0**, d. h. bis einschließlich des Tages vor der Typ-16-Buchung.

### 4.4 Zinsmodelle und Buchungslogik (`thesaurierend`)

#### 4.4.1 Jahreszinsanrechnung (31.12.)

Die folgenden Beschreibungen definieren die **Buchungsfolge bei der Jahreszinsanrechnung**. Alle Buchungen erfolgen am **31.12.** und gehen **gemeinsam** in die Bewertung des Vertrags ein. **Typ 8** ist dabei die **letzte Buchung des Jahres**.

* **Zinsmodus 0 – auszahlend**

  * **Typ 2**: Auszahlung der Zinsen
  * **Typ 8**: Jahreszinsanrechnung
  * **Wirkung**: Vertragsbetrag und *verzinsliches Darlehen* bleiben unverändert.

* **Zinsmodus 1 – anrechnend (thesaurierend)**

  * **Typ 8**: Jahreszinsanrechnung
  * **Wirkung**: Vertragsbetrag und *verzinsliches Darlehen* erhöhen sich um den Zins (Zinseszins-Effekt).

* **Zinsmodus 2 – fest**

  * **Typ 8**: Jahreszinsanrechnung
  * **Wirkung**: Der Vertragsbetrag erhöht sich um den Zins; das *verzinsliche Darlehen* bleibt unverändert.

* **Zinsmodus 3 – zinslos**

  * **Typ 8**: Jahreszinsanrechnung mit dem Betrag 0 Euro
  * **Wirkung**: Keine Änderung von Vertragsbetrag oder *verzinslichem Darlehen*.

#### 4.4.2 Ein- und Auszahlungen mit unterjähriger Zinsanrechnung

* **Zinsmodus 0 – auszahlend**

  * Der Nutzer wählt **Anrechnung** oder **Auszahlung** der bis zum Buchungsdatum aufgelaufenen Zinsen.
  * **Anrechnung**: **Typ 4 → Typ 1/2**
  * **Auszahlung**: **Typ 2 (Zinsauszahlung) → Typ 4 → Typ 1/2**

* **Zinsmodus 1, 2 und 3**

  * **Typ 4 → Typ 1/2**
  * Bei **Zinsmodus 3** erfolgt **Typ 4** mit **0 Euro**.

Zusammenfassung:

* Bei Ein- und Auszahlungen erfolgt die **Zinsanrechnung (Typ 4)** grundsätzlich **vor** der eigentlichen Buchung (**Typ 1** oder **Typ 2**).
* Im **Zinsmodus 0** kann zusätzlich eine **vorgelagerte Zinsauszahlung (Typ 2)** erfolgen.

---

## 5. Verzinsung

### 5.1 Zinsusance (Zinsmethode)

Die *Zinsusance* bezeichnet die verwendete Zinsmethode. Sie wird im Feld **Zinsusance** der Tabelle **Meta** festgelegt. Dieses Feld gilt für **alle Verträge einer Datenbank**. Mögliche Werte sind **„act/act“** und **„30/360“**.

#### 5.1.1 Gemeinsame Grundregeln

* Der **erste Tag eines Kreditzeitraums** ist zinsfrei.
* Der **letzte Tag (Auszahlungstag)** wird verzinst.
* Die Zinsberechnung erfolgt **zeitraumbezogen** zwischen zwei relevanten Buchungen, z. B. Ersteinzahlung, **Typ 4**, **Typ 8** oder Vertragsende.
* Die Regeln aus **Abschnitt 4.3 Ersteinzahlung und Verzinsungsbeginn** sind zu beachten.

#### 5.1.2 Zinsmethode `act/act` (tatsächliche Tage)

* Für **volle Jahre** ergibt sich der Zins als: *verzinsliches Darlehen × Zinssatz*.
* Unterschiede zu anderen Methoden entstehen **nur bei Zeiträumen, die kein volles Jahr umfassen**.

Für solche Zeiträume gilt:

* Es werden die **tatsächlichen Kalendertage** gezählt.
* Schaltjahre werden korrekt berücksichtigt.
* Der Zins wird **anteilig pro Kalendertag** berechnet.

  * Das entspricht **1/365 des Jahreszinses je Tag**, in Schaltjahren **1/366**.

#### 5.1.3 Zinsmethode `30/360` (kaufmännisch)

* Für **volle Jahre** ergibt sich ebenfalls der Zins als: *verzinsliches Darlehen × Zinssatz*.
* Unterschiede zu `act/act` entstehen **nur bei Zeiträumen, die kein volles Jahr umfassen**.

Für solche Zeiträume gilt:

* Jeder volle Monat entspricht **1/12 des Jahreszinses**.
* Für angebrochene Monate wird der Zins anteilig berechnet.

Für angebrochene Monate gilt im Detail:

* Es wird mit **kaufmännischen Zinstagen** gerechnet (Monat = 30 Zinstage, Jahr = 360 Zinstage).
* Der anteilige Zins entspricht **1/(12×30)** des Jahreszinses je Zinstag.
* Der **31. eines Monats** zählt nicht als Zinstag.

Konsequenzen:

* Monate mit weniger als 30 Tagen, z. B. **Februar**, sind im Verhältnis **günstiger verzinst** als bei `act/act`.
* Monate mit 31 Tagen liefern im Vergleich zu `act/act` einen **geringeren anteiligen Zins**, da der 31. nicht zählt.

#### 5.1.4 Bezug zur Jahresabrechnung

* Für die Jahresabrechnung (**Typ 8**) wird der Zins **für den Zeitraum seit der letzten relevanten Zinsbuchung** berechnet.
* Der zugrunde liegende Zeitraum ergibt sich aus den **relevanten Buchungen**.
* Die Zinsusance bestimmt ausschließlich, **wie dieser Zeitraum bewertet wird**, d. h. Tageszählung und Gewichtung.

### 5.2 Verzinsliches Darlehen

Die folgenden Definitionen gelten **unter Beachtung der Regeln zum Verzinsungsbeginn gemäß Abschnitt 4.3**.

Das *verzinsliche Darlehen* ist die **Bemessungsgrundlage für alle Zinsberechnungen** eines Vertrags. Es wird sowohl bei **unterjährigen Zinsanrechnungen (Typ 4)** als auch bei der **Jahreszinsanrechnung (Typ 8)** verwendet.

Welche Buchungen in das verzinsliche Darlehen eingehen, hängt vom im Vertrag festgelegten Zinsmodell (`thesaurierend`) ab.

* Bei den Zinsmodellen **auszahlend (0)**, **ansparend (1)** und **zinslos (3)** entspricht das verzinsliche Darlehen der **Summe aller Buchungen** des Vertrags.

  * Beim zinslosen Modell ist der Zinssatz stets **0**.
* Beim Zinsmodell **fest (2)** ergibt sich das verzinsliche Darlehen **ausschließlich aus Ein- und Auszahlungen** (**Typ 1** und **Typ 2**); Zinsanrechnungen gehen nicht in die Bemessungsgrundlage ein.

---

## 6. Beenden von Verträgen

Verträge können **befristet** oder **unbefristet** sein.

* Bei **befristeten Verträgen** ist das Vertragsende bereits bei der Anlage im Feld *LaufzeitEnde* festgelegt.
* Bei **unbefristeten Verträgen** wird das Vertragsende erst durch eine Kündigung bestimmt.

Für beide Fälle gilt:

* Ein Vertrag kann beendet werden, sobald im Feld *LaufzeitEnde* ein von **9999-12-31** abweichendes Datum hinterlegt ist.
* Dieses Datum kann entweder bereits bei der Anlage (befristet) oder durch eine Kündigung (unbefristet) gesetzt werden.
* DKV2 erzwingt nicht, dass das Beenden erst nach Erreichen von *LaufzeitEnde* erfolgt.

### 6.1 Relevante Felder

* **KueDatum**: Datum der Kündigung. Wird initial mit **9999-12-31** belegt und beim Start des Kündigungsprozesses gesetzt.
* **LaufzeitEnde**: Datum, zu dem der Vertrag beendet werden soll.

  * Bei **befristeten Verträgen** ist dieses Datum bereits bei Vertragsanlage gesetzt.
  * Bei **unbefristeten Verträgen** ist es initial **9999-12-31**.
* **Kfrist**: Kündigungsfrist in Monaten (ganzzahlig). Beschreibt den Zeitraum zwischen Kündigung und Vertragsende.

### 6.2 Kündigung

* Eine Kündigung ist nur bei **unbefristeten Verträgen** erforderlich.
* Die Kündigung wird in DKV2 über das Kontextmenü ausgelöst.
* Dabei wird ein Vertragsende vorgeschlagen, das **Kfrist Monate in der Zukunft** liegt.
* Dieses Datum kann angepasst werden und wird in **LaufzeitEnde** gespeichert.

### 6.3 Technisches Beenden

Beim Beenden eines Vertrags erfolgt die folgende Verarbeitung in festgelegter Reihenfolge:

1. Durchführung ggf. ausstehender **Jahreszinsanrechnungen**
2. Berechnung und Buchung der seit der letzten Zinsanrechnung aufgelaufenen Zinsen als **unterjährige Zinsanrechnung (Typ 4)**
3. Buchung der **Gesamtauszahlung (Typ 2)**
4. Verschieben aller Vertrags- und Buchungsdaten in die Tabellen **exVertraege** und **exBuchungen**

Ein einmal beendeter Vertrag ist damit vollständig aus den aktiven Tabellen entfernt.

---

## 7. Geldanlagen

*Geldanlagen* sind ein **optionales organisatorisches Modell** zur Gruppierung von Verträgen. Sie haben **keinen Einfluss auf Zinsberechnung oder Buchungslogik**.

### 7.1 Grundprinzip

* Ein Vertrag kann **optional genau einer Geldanlage** zugeordnet sein (über das Feld **Vertraege.AnlagenId**).
* Die Zuordnung dient der **Organisation, Auswertung und Überwachung von Grenzen**.

### 7.2 Fachliche Möglichkeiten in DKV2

DKV2 unterstützt folgende Arbeitsweisen:

* Beim Anlegen eines Vertrags kann statt eines Zinssatzes eine **Geldanlage ausgewählt** werden.
* Geldanlagen können **automatisch aus bestehenden Verträgen erzeugt** werden, mit oder ohne Zeitintervall.
* Bestehende Geldanlagen können **automatisch Verträgen zugeordnet** werden, wenn Zinssatz und ggf. Zeitraum passen.
* Geldanlagen, deren **Ende überschritten ist**, können automatisch als **geschlossen** markiert werden.

### 7.3 Tabelle Geldanlagen

| Feld             | Bedeutung                                                                                                                                                                                                         |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| *rowid*          | eindeutiger Schlüssel                                                                                                                                                                                             |
| *ZSatz*          | Zinssatz der Geldanlage; dient der Zuordnung und UI-Validierung. Verträge werden konsistent mit diesem Zinssatz angelegt; der tatsächlich wirksame Zinssatz steht im Vertrag und wird für Berechnungen verwendet. |
| *Anfang*, *Ende* | optionaler Zeitraum der Geldanlage                                                                                                                                                                                |
| *Typ*            | freier Bezeichner                                                                                                                                                                                                 |
| *Offen*          | gibt an, ob neue Verträge zugeordnet werden können oder ob die Geldanlage als geschlossen betrachtet wird, sodass sie nicht für zukünftige Verträge angeboten wird                                                |

### 7.4 Verhalten bei Löschung

* Beim Löschen einer Geldanlage werden die **AnlagenId** aller betroffenen Verträge automatisch auf **NULL** gesetzt (**ON DELETE SET NULL**).

### 7.5 Globale Parameter (Meta)

Die folgenden Felder der Tabelle *Meta* gelten für alle Geldanlagen:

| Feld           | Bedeutung                     |
| -------------- | ----------------------------- |
| *maxInvestNbr* | maximale Anzahl von Verträgen |
| *maxInvestSum* | maximale Gesamtsumme          |

### 7.6 Auswertungen

DKV2 stellt verschiedene Auswertungen bereit, u. a.:

* Anzahl von Geldanlagen
* Summen der zugeordneten Verträge
* zeitlicher Verlauf dieser Werte

Aktueller Stand:

* In die Auswertungen gehen **alle laufenden Verträge** ein.

Geplante Erweiterung:

* Zusätzlich sollen **beendete Verträge** berücksichtigt werden, sofern die Beendigung **weniger als 1 Jahr zurückliegt**.

Diese Auswertungen dienen der **Überwachung und Analyse** des Gesamtbestands.

### 7.7 Zeitfenster und „fortlaufende“ Geldanlagen

Geldanlagen ohne explizites Zeitfenster (*Anfang*, *Ende*) werden in DKV2 als **„fortlaufend“** bezeichnet.

Für fortlaufende Geldanlagen gilt:

* Es wird ein **dynamisches Zeitfenster** verwendet.
* Maßgeblich ist ein Zeitraum von **1 Jahr rückwirkend ab dem aktuellen Buchungsdatum**.
* Dieses Zeitfenster dient insbesondere der **Erfüllung gesetzlicher Anforderungen**.

Konsequenzen:

* Die Zuordnung und Auswertung erfolgen relativ zum aktuellen Zeitpunkt.
* Mit der geplanten Erweiterung (siehe 7.6) werden auch **kürzlich beendete Verträge** in dieses Zeitfenster einbezogen.
