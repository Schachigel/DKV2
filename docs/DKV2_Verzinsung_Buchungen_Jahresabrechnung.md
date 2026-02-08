# DKV2 – Verzinsung, Buchungen und Jahresabrechnung

*(Arbeitsstand)*

Dieses Dokument beschreibt die fachlichen Regeln und Datenstrukturen von **DKV2** im Bereich der verzinslichen Verträge, ihrer Buchungen und der jährlichen Abrechnung.

---

## 1. Zweck und Geltungsbereich

Dieses Dokument legt fest:

- welche Begriffe verwendet werden und wie sie zu verstehen sind,
- welche Bedeutung einzelnen Tabellen und Feldern zukommt,
- wie sich aus Buchungen, Vertragsparametern und Stichtagen relevante Größen ableiten lassen.

Der Schwerpunkt liegt auf **Konsistenz** und **Nachvollziehbarkeit** der Prozesse rund um Ein- und Auszahlungen, Verzinsung und Jahresabschluss. Technische Details (konkrete SQL-Abfragen, Implementierungen) sind nachgeordnet.

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

- **Kennung**: eindeutige, **alphanumerische** Vertragskennung (nicht zwingend numerisch)
- **ZSatz**: Zinssatz als ganze Zahl in 1/100 %
- **thesaurierend**: steuert die Behandlung der Zinsen; die einzelnen Zinsmodelle und ihre Auswirkungen werden im Abschnitt **4. Buchungen** erläutert
- **zActive**: gibt an, ob der Vertrag aktuell verzinst wird; gespeichert als **1 (wahr)** bzw. **0 (falsch)**

Ein Vertrag kann zunächst unverzinst sein (`zActive = FALSE`).

- Die Aktivierung erfolgt über **Buchung Typ 16**.
- Typ 16 definiert den Start der Verzinsung ab seinem Buchungsdatum.
- Eine Buchung vom Typ 16 erfolgt **pro Vertrag genau einmal** und **nach dem ersten Geldeingang** (Buchung Typ 1).
- **Ein- und Auszahlungen vor der Aktivierung** der Verzinsung sind zulässig.
- **Alle Zinsberechnungen vor der Aktivierung** (d. h. bis einschließlich des Tages vor der Typ-16-Buchung) erfolgen **mit dem Zinssatz 0**.

---

## 4. Buchungen

### 4.1 Allgemeines

- Alle finanziellen Bewegungen werden als Buchungen gespeichert.
- Es werden **keine aggregierten Werte, Salden oder Zwischensummen** gespeichert; alle fachlichen Zustände ergeben sich ausschließlich aus den vorhandenen Buchungen.
- Betrag ist eine ganze Zahl in Cent.
- Jede Buchung ist genau einem Vertrag zugeordnet.

### 4.2 Buchungsarten

| Typ | Bedeutung |
|---:|---|
| 1 | Einzahlung (Betrag ganzzahlig in ct, positiv) |
| 2 | Auszahlung (Betrag ganzzahlig in ct, negativ) |
| 4 | Zinsanrechnung (unterjährig; Betrag ganzzahlig in ct) |
| 8 | Jahreszins / Jahresabschluss (Betrag ganzzahlig in ct) |
| 16 | Aktivierung der Zinszahlung für einen Vertrag (Betrag ganzzahlig in ct, Wert 0) |

Regeln:

- Erste Buchung eines Vertrags ist immer Typ 1.
- Typ 1 und 2 sind grundsätzlich **nicht am 31.12.** zulässig.
- Ausnahme: erste Einzahlung darf am 31.12. erfolgen.
- Typ 8 ist immer die letzte Buchung eines Jahres (31.12.).
- Typ 16 darf am 31.12. gebucht werden.

### 4.3 Ersteinzahlung und Verzinsungsbeginn

Für die erste Einzahlung eines Vertrags gelten besondere Regeln, da sie den Beginn des Kreditverhältnisses und der Verzinsung definieren:

- Die **erste Buchung eines Vertrags ist immer vom Typ 1 (Einzahlung)**.
- Die **Ersteinzahlung darf am 31.12.x erfolgen**. In diesem Fall erfolgt **keine Jahresabrechnung für das Jahr x**, da der **erste Tag eines Kredits zinsfrei** ist.
  - Zum Ausgleich wird der **Auszahlungstag verzinst**.
- Erfolgt die **Ersteinzahlung am 30.12.x**, wird der Vertrag grundsätzlich in die Jahresendabrechnung für das Jahr x einbezogen.
  - Bei der Zinsmethode **30/360** ergibt sich dennoch ein **Zins von 0**, da der **31. eines Monats nicht verzinst wird**.

Diese Regeln sind unabhängig vom später gewählten Zinsmodell (`thesaurierend`) und betreffen ausschließlich den **Start des Vertrags und der Verzinsung**.

Zusätzlich gelten für die **Aktivierung der Zinszahlung** folgende Regeln:

- Die **Aktivierung der Zinszahlung (Buchung Typ 16)** kann **erst nach der Ersteinzahlung** erfolgen.
- **Am Tag der Aktivierung** wird **unmittelbar vor der Aktivierungsbuchung (Typ 16)** eine **Zinsanrechnung (Buchung Typ 4) mit dem Betrag 0** erfasst.
- Erfolgt die **Aktivierung am 31.12.x**, so erfolgt **keine Jahresabrechnung für das Jahr x**.

---

## 5. Verzinsung

### 5.1 Zinsusance (Zinsmethode)

Festgelegt im Feld **`Meta.Zinsusance`** der Tabelle **`Meta`**. Die *Zinsusance* bezeichnet die verwendete Zinsmethode. Die Zinsusance definiert, **wie Zinstage gezählt** und **wie Teilzeiträume bewertet** werden.

#### 5.1.1 Gemeinsame Grundregeln

- Der **erste Tag eines Kreditzeitraums ist zinsfrei**.
- Der **letzte Tag (Auszahlungstag)** wird verzinst.
- Die Zinsberechnung erfolgt **zeitraumbezogen** zwischen zwei relevanten Buchungen (z. B. Ersteinzahlung, Typ 4 (unterjährige Zinsanrechnung), Typ 8 (Jahreszinsanrechnung), Vertragsende).
- Die Regeln aus **4.3 Ersteinzahlung und Verzinsungsbeginn** sind zu beachten.

#### 5.1.2 Zinsmethode `act/act` (tatsächliche Tage)

- Es werden die **tatsächlichen Kalendertage** gezählt.
- Schaltjahre werden korrekt berücksichtigt.
- Jeder Kalendertag innerhalb des Zinszeitraums zählt als Zinstag (unter Beachtung der Grundregeln).

#### 5.1.3 Zinsmethode `30/360` (kaufmännisch)

- Der Jahreszins wird auf **12 gleich gewichtete Monate** verteilt; **jeder volle Monat entspricht 1/12 des Jahreszinses**.
- Für volle Monate ergibt sich damit **kein Unterschied** zur Zinsmethode `act/act`.
- Abweichungen entstehen ausschließlich bei **angebrochenen Monaten** am Vertragsanfang oder -ende.

Für angebrochene Monate gilt:

- Es wird mit **kaufmännischen Zinstagen** gerechnet (Monat = 30 Zinstage, Jahr = 360 Zinstage).
- Der **31. eines Monats zählt dabei nicht als Zinstag**.

Konsequenzen:

- Kurze Monate (z. B. **Februar**) sind im Verhältnis **günstiger verzinst** als bei `act/act`.
- Zeiträume, die ausschließlich den **31. eines Monats** betreffen, ergeben einen **Zins von 0**.

#### 5.1.4 Bezug zur Jahresabrechnung

- Für die Jahresabrechnung (Buchung Typ 8 – Jahreszinsanrechnung – am 31.12.) wird der Zins **für den Zeitraum seit der letzten relevanten Zinsbuchung** berechnet.
- Der zugrunde liegende Zeitraum und die Tageszählung ergeben sich ausschließlich aus der gewählten Zinsusance.

### 5.2 Verzinsliches Darlehen

Die folgenden Definitionen gelten **unter Beachtung der Regeln zum Verzinsungsbeginn gemäß Abschnitt 4.3 (Ersteinzahlung und Verzinsungsbeginn)**.

**Verzinsliches Darlehen**

Das *verzinsliche Darlehen* ist die **Bemessungsgrundlage für alle Zinsberechnungen** eines Vertrags. Es wird sowohl bei **unterjährigen Zinsanrechnungen (Buchung Typ 4)** als auch bei der **Jahreszinsanrechnung (Buchung Typ 8)** verwendet.

Welche Buchungen in das verzinsliche Darlehen eingehen, hängt vom im Vertrag festgelegten Zinsmodell (`thesaurierend`) ab.

Bei den Zinsmodellen **auszahlend (0)**, **ansparend (1)** und **zinslos (3)** entspricht das verzinsliche Darlehen jeweils der **Summe aller Buchungen** des Vertrags. Beim zinslosen Modell ist der Zinssatz dabei stets **0**.

Beim Zinsmodell **fest (2)** ergibt sich das verzinsliche Darlehen **ausschließlich aus Ein- und Auszahlungen** (Buchung Typ 1 und 2); Zinsanrechnungen gehen nicht in die Bemessungsgrundlage ein.

---

## 6. Jahresabrechnung

### 6.1 Allgemeines

- Für jeden Vertrag wird jährlich ein Zins berechnet.
- Ergebnis wird als **Buchung Typ 8 am 31.12.** gespeichert.


### 6.2 Bedeutung der Buchung Typ 8

- Enthält den Jahreszins
- Eindeutiger Anker für Abrechnungsjahr
- Definiert Endbestand zum Jahresende

---

## 7. Anforderungen an die Jahresabrechnungstabelle

### 7.1 Grundstruktur

- Eine Zeile pro Vertrag
- Nur Verträge mit Buchung Typ 8

### 7.2 Felder

- Kreditor: Name, Adresse, IBAN
- Vertrag: Kennung, Auszahlungsart
- Beginn der Zinsperiode
- Zinssatz
- Kreditbetrag
- Verzinsliches Darlehen
- Zins (Typ 8)
- Endbetrag

---

## 8. Beenden von Verträgen

### 8.1 Konsistenzbedingungen

- Vertragsdatum liegt vor erster Einzahlung
- Buchungen sind streng chronologisch

### 8.2 Laufzeit und Kündigung

- Kündigungsfrist: `Kfrist >= 0`, `LaufzeitEnde = 9999-12-31`
- Feste Laufzeit: `Kfrist = -1`, `LaufzeitEnde` gesetzt

### 8.3 Kündigung

- `Kfrist = -1`
- `KueDatum` gesetzt
- `LaufzeitEnde = KueDatum + Kündigungsfrist`

### 8.4 Technisches Beenden

- Ausstehende Jahresabrechnungen durchführen
- Abschlusszins als Typ 4 (unterjährige Zinsanrechnung)
- Gesamtauszahlung als Typ 2

### 8.5 Archivierung

- Vertrag → `exVertraege`
- Buchungen → `exBuchungen`
- Originaldaten löschen

---

## 9. Konsequenzen für SQL

- Vertragslebenszyklus vollständig datengetrieben
- Buchungen sind alleiniger Zustandsträger
- Beendete Verträge nur über Archivtabellen
- Zinsen werden **nicht in SQL berechnet**, sondern als **fachliches Ergebnis** ermittelt und ausschließlich in Form von **Buchungen** (Typ 4 und Typ 8) persistiert.
