# DKV2 – Projektkontext für Claude

## Fachliche Spezifikation (verbindlich)

[docs/DKV2_Verzinsung_Buchungen_Jahresabrechnung.md](docs/DKV2_Verzinsung_Buchungen_Jahresabrechnung.md)
ist die verbindliche fachliche Definition von DKV2 für die Bereiche Verträge,
Buchungen, Verzinsung, Jahresabrechnung, Vertragsbeendigung und Geldanlagen.
Sie deckt nicht alle Funktionen ab (z. B. Details zu Geldanlagen nur grob),
aber den größten Teil der Kernlogik.

**Regel: Der Code soll nicht von diesem Dokument abweichen.**

Bei Änderungen an Buchungslogik, Zinsberechnung (`helperfin.cpp`), Vertrags-
lebenszyklus (`contract.cpp`), Vertragsbeendigung (`wizterminatecontract.cpp`,
`transaktionen.cpp`) oder Geldanlagen (`investment.cpp`, `dkdbviews.cpp`):

* Vor der Änderung den relevanten Abschnitt des Dokuments lesen.
* Wenn Code und Dokument im Widerspruch stehen (egal ob vorher schon vorhanden
  oder durch die eigene Änderung neu entstanden), **nicht stillschweigend nach
  eigenem Ermessen entscheiden**, sondern den Nutzer aktiv darauf hinweisen und
  klären, ob der Code oder das Dokument korrigiert werden soll.
* Wenn das Dokument eine Funktion nicht abdeckt (z. B. Geldanlagen-Details),
  ist das kein Widerspruch — dort gilt der Code als Quelle der Wahrheit, bis
  das Dokument erweitert wird.

## Bekannte offene Abweichungen (Stand 2026-07-13)

Bei einer ersten Prüfung wurden folgende Abweichungen zwischen Code und
Dokument gefunden. Diese sind mit dem Nutzer gemeinsam zu klären (siehe auch
Projekt-Memory `code_doc_deviations`):

~~1. **31.12.-Regel bei Ein-/Auszahlungen** (Doc 4.2)~~ — **geklärt am
   2026-07-14**: kein echter Widerspruch. Das Dokument nennt die Ausnahme für
   die Ersteinzahlung bereits selbst (4.2, Zeile 129: "Ausnahme: Die erste
   Einzahlung darf am 31.12. erfolgen"). Die UI implementiert genau das über
   zwei getrennte Wizards: `wizInitialPayment`
   (`wizactivatecontract.cpp`, nur für die Ersteinzahlung, kein 31.12.-Verbot;
   `contract::bookInitialPayment()` in `contract.cpp:343-370` umgeht
   `avoidYearEndBookings()` bewusst) vs. `wizChangeContract`
   (`wizchangecontractvalue.cpp:237-254`, blockt 31.12. aktiv per
   Fehlermeldung für alle Folgebuchungen, gated über
   `initialPaymentReceived()`). Der stille Shift in `avoidYearEndBookings()`
   (`contract.cpp:330`, weiterhin aufgerufen von `deposit()`/`payout()`) ist
   nur ein Backend-Backstop, der bei UI-geführten Buchungen praktisch nie
   greift.
~~2. **zActive: Rückwechsel verzinst → unverzinst** (Doc 3.2)~~ — **behoben
   am 2026-07-14**: der generische, richtungslose Setter
   `contract::updateInterestActive(bool)` wurde entfernt und durch zwei
   eindeutig benannte, einseitige Methoden ersetzt:
   `contract::activateInterestPayment()` (setzt immer `zActive = true`,
   nutzt intern die vormals private `updateSetInterestActive()`) und
   `contract::markInterestPaymentDelayed()` (setzt immer `zActive = false`,
   nur für den Undo-Fall in `undoBookingDateGroup()`,
   `transaktionen.cpp:391`, sowie Testfixtures für Verträge, die mit
   verzögerter Zinszahlung starten). Damit ist die Einbahnstraßen-Regel aus
   der Doku im Code strukturell abgebildet statt nur durch Konvention.
   Hintergrund/Motivation vom Nutzer ergänzt: entgegenkommende Kreditgeber
   können auf Zinsen verzichten, bis das Projekt regelmäßige Einnahmen hat
   (Doc 3.2 entsprechend ergänzt). Alle ~30 Testaufrufstellen in
   `test_contract.cpp`/`test_annualsettlement.cpp` angepasst, betroffene
   Testsuiten grün (`test_contract` 43/43, `test_annualsettlement` 14/14,
   `test_booking` 14/14, `test_views` 10/10).
~~3. **maxInvestNbr / maxInvestSum** (Doc 7.5)~~ — **geklärt am
   2026-07-14**: kein zu behebender Bug, aber Doku-Formulierung präzisiert.
   `maxInvestNbr`/`maxInvestSum` bilden eine **gesetzliche Vorgabe** ab (vgl.
   7.7, "Erfüllung gesetzlicher Anforderungen"). Nutzerentscheidung: DKV2
   soll diese Grenze bewusst **nicht technisch durchsetzen** — nur durch
   visuelle Warnung (Rot-Färbung, `redOrBlack()` in `investment.cpp`)
   unterstützen. Die Verantwortung für die Einhaltung bleibt bei den
   Projekten, DKV2 verbietet nichts. Doc 7.5 entsprechend präzisiert
   ("gesetzliche Obergrenze" statt der irreführenden ersten Formulierung
   "empfohlene Obergrenze" — es ist kein bloßer Ratschlag, sondern eine
   gesetzliche Pflicht der Projekte, nur eben nicht softwareseitig
   erzwungen).
~~4. **Geldanlagen-Auswertungen und beendete Verträge** (Doc 7.6)~~ —
   **behoben am 2026-07-14**: Ersteinzahlungsdatum als Anker entschieden
   (siehe Memory `code_doc_deviations_2026_07`) und implementiert in
   `perpetualInvestment_bookings()` (`dkdbhelper.cpp`): neue CTE
   `ersteinzahlungen` ermittelt je Vertrag das Datum der ersten Typ-1-Buchung
   und zählt ihn für genau 1 Jahr danach mit — unabhängig von einer
   zwischenzeitlichen Vertragsbeendigung. Zusätzlich auf Nutzerwunsch die
   beiden bisherigen UI-Tabellen zusammengelegt: "Liste fortlaufender
   Geldanlagen" (`perpetualInvestmentByContracts()`, rein
   vertragsdatumsbasiert, nominelle Vertragswerte) wurde ersatzlos entfernt
   (Combo-Eintrag, Enum-Wert `PERPETUAL_INVESTMENTS_CHECK_BY_CONTRACTS`,
   Render-Funktion, SQL-Funktion), da reine Vertragsdaten ohne Buchungsbezug
   als nicht relevant eingeschätzt wurden. "Prüfung der Grenzwerte für
   fortlaufende Geldanlagen anhand aller Buchungen"
   (`perpetualInvestment_bookings()`) ist jetzt die einzige verbleibende
   Tabelle und wurde um die Spalte "Anzahl Verträge (lfd. 12M)" erweitert,
   damit sowohl `maxInvestNbr` als auch `maxInvestSum` (Doc 7.5) aus einer
   Tabelle ablesbar sind. Das ungenutzte `vInvestmentsOverview`-DB-View
   (`dkdbviews.cpp`, nur von einem Test referenziert, nirgends im UI
   gerendert) wurde dabei bewusst nicht angefasst — außerhalb des
   besprochenen Scopes. Doc 7.6/7.7 entsprechend überarbeitet. Tests
   `test_perpetualInvestmentBookings_*` in `test_views.cpp` angepasst und
   grün (10/10).

~~5. Buchungsart 32 (`deferredMidYearInterest`) war im Dokument nicht
   erwähnt~~ — **behoben am 2026-07-13**: Implementierung geprüft (43/43,
   14/14, 14/14 Tests grün in `test_contract`/`test_booking`/
   `test_annualsettlement`, inkl. Referenzfall-Tests gegen `ZinsesZins_act_act`/
   `ZinsesZins_30_360`), funktional korrekt befunden und als neuer Abschnitt
   **4.4 „Zinsmodelle und Buchungslogik"** samt Zeile für Typ 32 in Abschnitt
   4.2 ins Dokument aufgenommen.
