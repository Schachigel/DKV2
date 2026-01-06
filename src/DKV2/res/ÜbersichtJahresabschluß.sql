WITH p AS (
-- Jahresabrechnung für ein Jahr :year (z.B. 2025)
  SELECT date(printf('%04d-12-31', :year)) AS end_date
)
SELECT
  k.Vorname,
  k.Nachname,
  k.Email,
  k.Strasse,
  k.Plz,
  k.Stadt,
  k.IBAN,

  v.Kennung,

  (
    CASE v.thesaurierend
      WHEN 0 THEN 'auszahlend'
      WHEN 1 THEN 'ansparend'
      WHEN 2 THEN 'fest'
      WHEN 3 THEN 'zinslos'
      ELSE 'unbekannt'
    END
    || CASE WHEN IFNULL(v.zActive, 0) <> 1 THEN ' / ausgesetzt' ELSE '' END
  ) AS Auszahlend,

  -- Beginn: letzte Buchung < 31.12.YYYY von Typ 4/8/16, sonst erste Einzahlung (Typ 1)
  strftime(
    '%d-%m-%Y',
    date(
      COALESCE(
        (SELECT MAX(b.Datum)
         FROM Buchungen b
         WHERE b.VertragsId = v.id
           AND date(b.Datum) < p.end_date
           AND b.BuchungsArt IN (4, 8, 16)),
        (SELECT MIN(b.Datum)
         FROM Buchungen b
         WHERE b.VertragsId = v.id
           AND b.BuchungsArt = 1)
      )
    )
  ) AS Beginn,

  strftime('%d-%m-%Y', p.end_date) AS Buchungsdatum,

  -- ZSatz ist in 1/100 %, Ausgabe in % mit 2 Nachkommastellen
  printf('%.2f', v.ZSatz / 100.0) AS Zinssatz,

  -- Kreditbetrag: Guthaben vor 31.12.YYYY (bei thesaurierend=2 nur Typ 1+2, sonst alle)
  printf('%.2f',
    (
      CASE WHEN v.thesaurierend = 2 THEN
        (SELECT COALESCE(SUM(b.Betrag), 0)
         FROM Buchungen b
         WHERE b.VertragsId = v.id
           AND date(b.Datum) < p.end_date
           AND b.BuchungsArt IN (1, 2))
      ELSE
        (SELECT COALESCE(SUM(b.Betrag), 0)
         FROM Buchungen b
         WHERE b.VertragsId = v.id
           AND date(b.Datum) < p.end_date)
      END
    ) / 100.0
  ) AS Kreditbetrag,

  -- Zins: Betrag der Buchung Typ 8 am 31.12.YYYY
  printf('%.2f', b8.Betrag / 100.0) AS Zins,

  -- Endbetrag: Guthaben inkl. aller Buchungen bis inkl. 31.12.YYYY
  printf('%.2f',
    (SELECT COALESCE(SUM(b.Betrag), 0)
     FROM Buchungen b
     WHERE b.VertragsId = v.id
       AND date(b.Datum) <= p.end_date) / 100.0
  ) AS Endbetrag

FROM p
JOIN Buchungen b8
  ON b8.BuchungsArt = 8
 AND date(b8.Datum) = p.end_date
JOIN Vertraege v
  ON v.id = b8.VertragsId
JOIN Kreditoren k
  ON k.id = v.KreditorId
ORDER BY k.Nachname, k.Vorname, v.Kennung;
