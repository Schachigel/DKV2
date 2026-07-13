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

1. **31.12.-Regel bei Ein-/Auszahlungen** (Doc 4.2): Dokument verlangt, dass
   Typ 1/Typ 2 grundsätzlich nicht am 31.12. zulässig sind. Der Code
   (`avoidYearEndBookings()` in `contract.cpp:330`) lehnt solche Buchungen
   nicht ab, sondern verschiebt sie still auf den 30.12.
2. **zActive: Rückwechsel verzinst → unverzinst** (Doc 3.2): Dokument sagt,
   der Übergang ist nicht vorgesehen. `contract::updateInterestActive(bool)`
   (`contract.cpp:285`) erlaubt technisch beide Richtungen; nur an einer
   Aufrufstelle (Undo einer Aktivierungsbuchung) wird tatsächlich
   deaktiviert, aber es gibt keine generische Sperre.
3. **maxInvestNbr / maxInvestSum** (Doc 7.5): Werden im Code nur als
   visuelle Warnindikatoren (Rot-Färbung) verwendet, nicht als harte,
   durchgesetzte Obergrenzen.
4. **Geldanlagen-Auswertungen und beendete Verträge** (Doc 7.6): Dokument
   beschreibt den aktuellen Stand als "nur laufende Verträge" und die
   Einbeziehung beendeter Verträge (< 1 Jahr) als geplante Erweiterung.
   Tatsächlich bezieht `vInvestmentsOverview` (`dkdbviews.cpp`) bereits jetzt
   `exVertraege` (beendete Verträge) ein, in Teilen sogar ohne jede
   Zeitbegrenzung.

~~5. Buchungsart 32 (`deferredMidYearInterest`) war im Dokument nicht
   erwähnt~~ — **behoben am 2026-07-13**: Implementierung geprüft (43/43,
   14/14, 14/14 Tests grün in `test_contract`/`test_booking`/
   `test_annualsettlement`, inkl. Referenzfall-Tests gegen `ZinsesZins_act_act`/
   `ZinsesZins_30_360`), funktional korrekt befunden und als neuer Abschnitt
   **4.4 „Zinsmodelle und Buchungslogik"** samt Zeile für Typ 32 in Abschnitt
   4.2 ins Dokument aufgenommen.
