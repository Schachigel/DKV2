#include "dkdbviews.h"
#include "helper_core.h"
#include "helpersql.h"
#include "csvwriter.h"

//////////////////////////////////////
// VIEWs to be created temporarily in the database
//////////////////////////////////////
const QString vnContractView {qsl("vVertraege_alle_4view")};
const QString sqlContractView{qsl(
    R"str(
SELECT
  V.id AS VertragsId
  ,K.id AS KreditorId
  ,V.AnlagenId AS AnlagenId
  ,K.Nachname || ', ' || K.Vorname          AS KreditorIn
  ,V.Kennung AS Vertragskennung
  ,V.Anmerkung AS Anmerkung
  ,V.Vertragsdatum     AS Vertragsdatum

  ,IFNULL(Eingangsbuchung, '(steht aus)') AS Geldeingang
  ,IIF(V.zActive, ' aktiv ', ' ausgesetzt ') AS VerzinsungsStatus
  ,IIF(V.thesaurierend > 3, 'Fehler'
         , IIF(V.Betrag <= IIF(V.thesaurierend == 0, IFNULL(summeAllerBuchungen, 0)
         , IIF(V.thesaurierend == 1, IFNULL(summeAllerBuchungen, 0)
         , IIF(V.thesaurierend == 2, IFNULL(summeEinUndAuszahlungen, 0)
         , IIF(V.thesaurierend == 3, IFNULL(summeEinUndAuszahlungen, 0), 0))))
    , V.Betrag /100., '[soll: ' || CAST( V.Betrag /100. AS TEXT) || ' €]' )) AS Nominalwert
  , IIF(IFNULL(AnlagenId, 0) == 0
      , CAST(V.Zsatz / 100. AS VARCHAR) || ' % (ohne Anlage)'
      , CAST(V.Zsatz / 100. AS VARCHAR) || ' % - ' || GA.Typ ) AS Zinssatz

-- Zinsmodus
  , V.thesaurierend AS Zinsmodus

-- VerzinslGuthaben
  ,IIF(V.thesaurierend == 0, IFNULL(summeAllerBuchungen /100., ' (inaktiv) ')
       , IIF(V.thesaurierend == 1, IFNULL(summeAllerBuchungen /100., ' (inaktiv) ')
       , IIF(V.thesaurierend == 2, IFNULL(summeEinUndAuszahlungen /100., ' (inaktiv) ')
       , IIF(V.thesaurierend == 3, IFNULL(summeAllerBuchungen /100., '(inaktiv) '), 'ERROR')))) AS VerzinslGuthaben

-- angesparter Zins
  ,IIF(V.thesaurierend == 0, IFNULL(summeAllerZinsZwBuchungen /100., '(noch 0) ')
       ,IIF(V.thesaurierend == 1, IFNULL(summeAllerZinsBuchungen /100., '(noch 0) ')
       ,IIF(V.thesaurierend == 2, IFNULL(summeAllerZinsBuchungen /100., '(noch 0) ')
       ,IIF(V.thesaurierend == 3, '(zinslos)   ', 'ERROR')))) AS angespZins

-- kdFrist/VEnd
  ,IIF( V.Kfrist == -1, strftime('%d.%m.%Y', V.LaufzeitEnde),
      IIF( date(V.LaufzeitEnde) > date('5000-12-31'), '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)',
          '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)' || char(10) || strftime('%d.%m.%Y', V.LaufzeitEnde))) AS KdgFristVertragsende
-- Datum der Kündigung
  ,IIF( V.KueDatum == '9999-12-31', '', V.KueDatum) AS KdgDatum

FROM Vertraege AS V
INNER JOIN Kreditoren AS K ON V.KreditorId = K.id
LEFT JOIN Geldanlagen AS GA ON V.AnlagenId = GA.rowid

-- Geldeingang und Betrag
LEFT JOIN
    (SELECT vid_min, minB_Datum AS Eingangsbuchung, Erstbuchungswert AS AktivierungsWert
         FROM   (SELECT B.VertragsId AS vid_min, min(B.Datum) AS minB_Datum, B.Betrag / 100. AS Erstbuchungswert, sum(B.Betrag) / 100. AS GesamtWert
            FROM Buchungen AS B GROUP BY B.VertragsId) ) ON V.id = vid_min

-- VerzinslGuthaben f auszahlende
LEFT JOIN
     (SELECT VertragsId, SUM(Betrag) AS summeAllerBuchungen FROM Buchungen AS B1 GROUP BY VertragsId) ON V.id = VertragsId

-- angespZins f auszahlende: nur zwischenzinsen
LEFT JOIN
     (SELECT VertragsId AS B2VertragsId, SUM(Betrag) AS summeAllerZinsZwBuchungen FROM Buchungen AS B2 WHERE B2.BuchungsArt = 4 /*ReInvInterest*/  GROUP BY B2.VertragsId) ON V.id = B2VertragsId

-- angespZins f thesaurierende: zwischenzinsen + Jahreszins
LEFT JOIN
     (SELECT VertragsId AS B4VertragsId, SUM(Betrag) AS summeAllerZinsBuchungen FROM Buchungen AS B4 WHERE B4.BuchungsArt = 8 /*annual*/ OR B4.BuchungsArt = 4 /*ReInvInterest*/  GROUP BY B4.VertragsId) ON V.id = B4VertragsId

-- VerzinslGuthaben FestZins: Summe aller ein und auszahlungen
LEFT JOIN
    (SELECT VertragsId AS B5VertragsId, SUM(Betrag) AS summeEinUndAuszahlungen FROM Buchungen AS B5 WHERE B5.BuchungsArt = 1 /*deposit*/ OR B5.BuchungsArt = 2 /*payout*/ GROUP BY B5.VertragsId) ON V.id = B5VertragsId
)str")};

const QString vnExContractView {qsl("vVertraege_geloescht")};
const QString sqlExContractView {qsl(
R"str(
SELECT
  V.id AS VertragsId
  , K.id AS KreditorId
  , K.Nachname || ', ' || K.Vorname AS KreditorIn
  , V.Kennung AS Vertragskennung
  , strftime('%d.%m.%Y',Aktivierungsdatum) AS Aktivierung
  , strftime('%d.%m.%Y', Vertragsende) AS Vertragsende
  , ifnull(AktivierungsWert, V.Betrag / 100.) AS Anfangswert
  , CAST(V.Zsatz / 100. AS VARCHAR) || ' %'    AS Zinssatz
  , IIF(V.thesaurierend = 0, 'Auszahlend',
       IIF( V.thesaurierend = 1, 'Thesaur.',
          IIF( V.thesaurierend = 2, 'Fester Zins',
             IIF( V.thesaurierend = 3, 'Zinslos', 'ERROR')))) AS Zinsmodus
  , IIF( V.thesaurierend  = 0, Zinsen_ausz, Zinsen_thesa) AS Zinsen
  , IIF( V.thesaurierend  > 0, -1* letzte_Auszahlung + sum_o_Zinsen, -1* letzte_Auszahlung + sum_mit_Zinsen) AS Einlagen
  , maxValue AS Endauszahlung
  ,IIF( V.KueDatum == '9999-12-31', '', V.KueDatum) AS KdgDatum

FROM exVertraege AS V  INNER JOIN Kreditoren AS K ON V.KreditorId = K.id

LEFT JOIN (
     SELECT
        B.VertragsId AS vid_min
        , min(B.id) AS idErsteBuchung
        , B.Datum AS Aktivierungsdatum
        , B.Betrag / 100. AS AktivierungsWert
        , sum(B.Betrag) / 100. AS GesamtWert
     FROM exBuchungen AS B GROUP BY B.VertragsId )
ON V.id = vid_min

LEFT JOIN (SELECT
         B.VertragsId AS vid_max
         , max(B.id) as idLetzteBuchung
         , B.Datum AS Vertragsende
         , B.Betrag /100. AS maxValue
       FROM exBuchungen AS B GROUP BY B.VertragsId )
ON V.id = vid_max

LEFT JOIN (SELECT
            B.VertragsId AS vidZinsThesa
            , SUM(B.betrag) /100. AS Zinsen_thesa
          FROM exBuchungen AS B
          WHERE B.BuchungsArt = 4 OR B.BuchungsArt = 8
          GROUP BY B.VertragsId )
ON V.id = vidZinsThesa

LEFT JOIN ( SELECT
              B.VertragsId AS vidZinsAus
              , SUM(B.betrag) /-100. AS Zinsen_ausz
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8 GROUP BY B.VertragsId )
ON V.id = vidZinsAus

LEFT JOIN ( SELECT
                B.VertragsId as vid_o_Zinsen
                , sum(B.Betrag) /100. AS sum_o_Zinsen
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2
            GROUP BY B.VertragsId  )
ON V.id = vid_o_Zinsen

LEFT JOIN ( SELECT
              B.VertragsId as vid_mit_Zinsen
              , sum(B.Betrag) /100. AS sum_mit_Zinsen
              , max(B.Datum) AS letzte_buchung
              , B.Betrag /100. AS letzte_Auszahlung
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8
            GROUP BY B.VertragsId  )
ON V.id = vid_mit_Zinsen
)str"
)};

const QString vnInvestmentsView {qsl("vInvestmentsOverview")};
const QString sqlInvestmentsView {qsl(
R"str(
WITH fortlaufend_temp AS (
SELECT ga.ZSatz
  , IIF( ga.Anfang == '1900-01-01', '-', ga.Anfang) AS Anfang
  , 'fortlaufend' AS Ende
  , ga.Typ
  , (SELECT count(*)
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid AND Vertraege.Vertragsdatum > DATE('now', '-1 year')
  ) AS Anzahl
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid AND Vertraege.Vertragsdatum > DATE('now', '-1 year')
  ) AS SummeVertraege
  , (SELECT count(*)
     FROM Vertraege AS v
     WHERE v.AnlagenId == ga.rowid
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId == v.id)>0 AND v.Vertragsdatum > DATE('now', '-1 year')
  ) AS AnzahlAktive
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege AS v
     WHERE v.AnlagenId == ga.rowid
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId == v.id)>0 AND v.Vertragsdatum > DATE('now', '-1 year')
  ) AS SummeAktive
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend == 0 AND Vertraege.AnlagenId == ga.rowid)
       AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2 OR Buchungen.BuchungsArt == 8)
     ) AS EinzahlungenAuszahlenderV
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend != 0 AND Vertraege.AnlagenId == ga.rowid)
       AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2)
     ) AS EinzahlungenRestlV
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.AnlagenId == ga.rowid)) AS SummeInclZins
  , CASE WHEN ga.Offen THEN 'Offen' ELSE 'Abgeschlossen' END AS Offen
  , ga.rowid
FROM Geldanlagen AS ga
WHERE ga.Ende = DATE('9999-12-31')
ORDER BY ga.Offen DESC, ga.ZSatz DESC
)
, abgeschl_temp AS (
SELECT ga.ZSatz
  , ga.Anfang AS Anfang
  , ga.Ende AS Ende
  , ga.Typ
  , (SELECT count(*)
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid
  ) AS Anzahl
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid
  ) AS SummeVertraege
  , (SELECT count(*)
     FROM Vertraege AS v
     WHERE v.AnlagenId == ga.rowid
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId == v.id)>0
  ) AS AnzahlAktive
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege AS v
     WHERE v.AnlagenId == ga.rowid
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId == v.id)>0
  ) AS SummeAktive
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend == 0 AND Vertraege.AnlagenId == ga.rowid)
       AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2 OR Buchungen.BuchungsArt == 8)
     ) AS EinzahlungenAuszahlenderV
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend != 0 AND Vertraege.AnlagenId == ga.rowid)
       AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2)
     ) AS EinzahlungenRestlV
  , (SELECT SUM(Betrag) /100.
     FROM Buchungen
     WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.AnlagenId == ga.rowid)) AS SummeInclZins

  , CASE WHEN ga.Offen THEN 'Offen' ELSE 'Abgeschlossen' END AS Offen
  , ga.rowid
FROM Geldanlagen AS ga
WHERE ga.Ende != DATE('9999-12-31')
ORDER BY ga.Offen DESC, ga.ZSatz DESC
)
, alle AS (
SELECT * FROM fortlaufend_temp
UNION
SELECT * FROM abgeschl_temp
)
SELECT
  ZSatz, Anfang, Ende, Typ, Anzahl
  , IFNULL(SummeVertraege, 0.) AS SummeVertraege
  , AnzahlAktive
  , IFNULL(SummeAktive, 0.) AS SummeAktive
  , IFNULL(EinzahlungenAuszahlenderV, 0.) + IFNULL(EinzahlungenRestlV, 0.) AS Einzahlungen
  , IFNULL(SummeInclZins, 0.) AS SummeInclZins
  , Offen, rowid AS AnlagenId
FROM alle
)str"
)};


//////////////////////////////////////
// VIEWs to be created in the database
//////////////////////////////////////
const QString vnBookingsOverview {qsl("vBuchungen")};
const QString sqlBookingsOverview {qsl( // clazy:exclude=non-pod-global-static
R"str(
SELECT
  IFNULL(B.Datum, V.Vertragsdatum),
  V.id,
  V.Kennung,
  CASE V.thesaurierend
    WHEN 0 THEN 'Ausz.'
    WHEN 1 THEN 'Thesa.'
    WHEN 2 THEN 'Fix'
    WHEN 3 THEN 'Zinslos'
    ELSE 'ERROR'
  END AS Zinsmodus,
  IFNULL(B.Betrag, V.Betrag),
  CASE B.BuchungsArt
    WHEN 1 THEN 'Einzahlung'
    WHEN 2 THEN 'Auszahlung'
    WHEN 4 THEN 'unterj.Zins'
    WHEN 8 THEN 'Jahreszins'
    ELSE 'Vertragsabschluss'
  END AS BuchungsArt
FROM Vertraege AS V
  LEFT JOIN Buchungen AS B ON V.id = B.VertragsId
ORDER BY B.Datum, V.id
)str"
)};

const QMap<QString, QString> views ={
    // convenientce view
    {vnBookingsOverview, sqlBookingsOverview},
    {vnContractView, sqlContractView},
    {vnExContractView, sqlExContractView},
    {vnInvestmentsView, sqlInvestmentsView}
};

bool createDkDbViews( const QMap<QString, QString>& vs, const QSqlDatabase& db)
{
//    foreach(QString view, views.keys()) {
    for ( auto [viewname, viewSql ] : vs.asKeyValueRange()) {
        if( not createPersistentDbView (viewname, viewSql, db))
            return false;
    }
    return true;
}

//////////////////////////////////////
// SQL Statemends (stored here to not clutter the source code with long constant strings)
//////////////////////////////////////

// find date for next annual settlement
const QString sqlNextAnnualSettlement {qsl(
R"str(
-- Ein einziges Datum: nächstes (frühestes) Jahresende, zu dem *irgendein* Vertrag
-- die nächste Jahresabrechnung machen kann.
WITH last_booking AS (
  SELECT
    b.VertragsId,
    b.Datum      AS lastDatum,
    b.BuchungsArt AS lastArt,
    ROW_NUMBER() OVER (
      PARTITION BY b.VertragsId
      ORDER BY b.Datum DESC, b.id DESC
    ) AS rn
  FROM Buchungen b
),
per_contract AS (
  SELECT
    v.id AS VertragsId,
    CASE
      -- letzte Buchung am 31.12. => nächste JZA ist 31.12. des Folgejahres
      WHEN substr(lb.lastDatum, 6, 5) = '12-31' THEN date(lb.lastDatum, '+1 year')

      -- sonst: 31.12. des Jahres der letzten Buchung
      ELSE date(strftime('%Y', lb.lastDatum) || '-12-31')
    END AS nextJzaDatum
  FROM Vertraege v
  JOIN last_booking lb
    ON lb.VertragsId = v.id AND lb.rn = 1
  -- nur Verträge mit mindestens einer Einzahlung (kein NULL/kein "leerer Vertrag")
  WHERE EXISTS (
    SELECT 1
    FROM Buchungen b1
    WHERE b1.VertragsId = v.id
      AND b1.BuchungsArt = 1
  )
)
SELECT MIN(nextJzaDatum) AS date FROM per_contract
)str"
)};

// input for AS calculation
const QString sqlContractDataForAnnualSettlement { qsl(
R"str(
-- Parameter: :YEAR  (z.B. 2025)
WITH
last_booking AS (
  SELECT
    b.VertragsId,
    MAX(b.Datum) AS lastDatum
  FROM Buchungen b
  GROUP BY b.VertragsId
),
has_deposit AS (
  SELECT DISTINCT b.VertragsId
  FROM Buchungen b
  WHERE b.BuchungsArt = 1
)
SELECT
  v.*
FROM Vertraege v
JOIN last_booking lb ON lb.VertragsId = v.id
JOIN has_deposit  hd ON hd.VertragsId = v.id
WHERE
  v.zActive = TRUE
  -- last AS or interest activation year end last year
  AND ( lb.lastDatum = printf('%04d-12-31', (:YEAR - 1))
  -- any booking in  YEAR
  OR ( lb.lastDatum >= printf('%04d-01-01', :YEAR)
  AND lb.lastDatum <  printf('%04d-12-31', :YEAR))
  );
)str")
};

// SQL to collect all data for csv creation for a given year
const QString sqlComplete_AS_data { qsl(R"str(
WITH
JA_Buchungen AS (
    SELECT
        b.VertragsId,
        b.Betrag              AS Jahreszins_ct
    FROM Buchungen b
    WHERE b.BuchungsArt = 8
      AND b.Datum = printf('%04d-12-31', :year)
),

LetzteZinsbuchung AS (
    SELECT
        x.VertragsId,
        MAX(x.Datum) AS Beginn
    FROM (
        SELECT b.VertragsId, b.Datum
        FROM Buchungen b
        WHERE b.BuchungsArt = 8
          AND b.Datum <= date(printf('%04d-12-31', :year), '-1 year')

        UNION ALL

        SELECT b.VertragsId, b.Datum
        FROM Buchungen b
        WHERE b.BuchungsArt = 16
          AND b.Datum >= date(printf('%04d-12-31', :year), '-1 year')
          AND b.Datum <  printf('%04d-12-31', :year)

        UNION ALL

        SELECT b.VertragsId, b.Datum
        FROM Buchungen b
        WHERE (b.BuchungsArt = 4 OR b.BuchungsArt = 2 OR b.BuchungsArt = 1)
        -- alle buchungsarten - falls später Einzahlungen ohne Zinsbuchungen möglich werden
          AND b.Datum >= date(printf('%04d-12-31', :year), '-1 year')
          AND b.Datum <  printf('%04d-12-31', :year)
    ) x
    GROUP BY x.VertragsId
),

verzinslichesDarlehen AS (
    SELECT
        v.id AS VertragsId,
        CASE v.thesaurierend
            WHEN 2 THEN
                IFNULL((
                    SELECT SUM(b2.Betrag)
                    FROM Buchungen b2
                    WHERE b2.VertragsId = v.id
                      AND b2.BuchungsArt IN (1,2)
                      AND b2.Datum < printf('%04d-12-31', :year)
                ), 0)
            ELSE
                IFNULL((
                    SELECT SUM(b2.Betrag)
                    FROM Buchungen b2
                    WHERE b2.VertragsId = v.id
                      AND b2.Datum < printf('%04d-12-31', :year)
                ), 0)
        END AS verzinslichesDarlehen_ct
    FROM Vertraege v
),

Endbetrag AS (
    SELECT
        v.id AS VertragsId,
        CASE v.thesaurierend
            WHEN 2 THEN
                IFNULL((
                    SELECT SUM(b2.Betrag)
                    FROM Buchungen b2
                    WHERE b2.VertragsId = v.id
                      AND b2.BuchungsArt IN (1,2)
                      AND b2.Datum <= printf('%04d-12-31', :year)
                ), 0)
            ELSE
                IFNULL((
                    SELECT SUM(b2.Betrag)
                    FROM Buchungen b2
                    WHERE b2.VertragsId = v.id
                      AND b2.Datum <= printf('%04d-12-31', :year)
                ), 0)
        END AS Endbetrag_ct
    FROM Vertraege v
)

SELECT
    k.Vorname,
    k.Nachname,
    k.Strasse,
    k.Plz,
    k.Stadt,
    k.Email,
    k.Buchungskonto,
    k.IBAN,
    k.BIC,

    v.Kennung,
    CASE v.thesaurierend
        WHEN 0 THEN 'auszahlend'
        WHEN 1 THEN 'ansparend'
        WHEN 2 THEN 'fest'
        WHEN 3 THEN 'zinslos'
        ELSE 'Fehler im Verzinsungsmodus'
    END AS Auszahlungsart,

    lz.Beginn AS Beginn,
    printf('%04d-12-31', :year) AS Buchungsdatum,

    -- Kreditbetrag in Euro (formatiert)
    REPLACE( printf('%.2f', kb.verzinslichesDarlehen_ct / 100.0), '.', ',') || ' €' AS Kreditbetrag,

    -- Zinssatz in % (formatiert, z.B. 10,13%)
    REPLACE( printf('%.2f', v.ZSatz /10000.0), '.', ',') || '%' AS Zinssatz,

    -- Zins (Typ 8) in Euro (formatiert)
    REPLACE( printf('%.2f', ja.Jahreszins_ct / 100.0), '.', ',') || ' €' AS Zins,

    -- Endbetrag in Euro (formatiert)
    REPLACE( printf('%.2f', eb.Endbetrag_ct / 100.0), '.', ',') || ' €' AS Endbetrag


FROM JA_Buchungen ja
JOIN Vertraege v
  ON v.id = ja.VertragsId
JOIN Kreditoren k
  ON k.id = v.KreditorId
LEFT JOIN LetzteZinsbuchung lz
  ON lz.VertragsId = v.id
LEFT JOIN verzinslichesDarlehen kb
  ON kb.VertragsId = v.id
LEFT JOIN Endbetrag eb
  ON eb.VertragsId = v.id

ORDER BY k.Nachname, k.Vorname, v.id
)str")
};


//////////////////////////////////////
// Listenausdruck  in createCsvActiveContracts
//////////////////////////////////////
const QString sqlContractsActiveDetailsView{ qsl(
R"str(
SELECT
  Vertraege.id          AS Vertragsnummer,
  Kreditoren.id         AS Kundennummer,
  Kreditoren.Vorname    AS Vorname,
  Kreditoren.Nachname   AS Nachname,
  Kreditoren.Strasse    AS Strasse,
  Kreditoren.Plz        AS PLZ,
  Kreditoren.Stadt      AS Stadt,
  Kreditoren.Email      AS "E-Mail",
  Kreditoren.IBAN       AS IBAN,
  Kreditoren.BIC        AS BIC,
  Vertraege.Kennung     AS Vertragskennung,
  REPLACE( printf('%.2f', Vertraege.Betrag /100.0), '.', ',') AS Vertragswert,
  REPLACE( printf('%.2f', Vertraege.ZSatz /10000.0, 2), '.', ',') AS Zinssatz,
  CASE Vertraege.thesaurierend
    WHEN 0 THEN 'Auszahlend'
    WHEN 1 THEN 'Ansparend'
    WHEN 2 THEN 'Fest'
    WHEN 3 THEN 'Zinslos'
    ELSE 'ERROR'
  END AS Zinsmodus,
  REPLACE( printf('%.2f', SUM(Buchungen.Betrag) / 100.0), '.', ',') AS Vertragswert,
  strftime('%d.%m.%Y', MIN(Buchungen.Datum))  AS Aktivierungsdatum,
    -- MIN(Buchungen.Datum)  AS Aktivierungsdatum,
  Vertraege.Kfrist      AS Kuendigungsfrist,
  COALESCE(strftime('%d.%m.%Y', Vertraege.LaufzeitEnde), '-') AS Vertragsende
FROM Vertraege
    INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id
    INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId
GROUP BY Vertraege.id
)str"
)};
// uebersicht contract use and other uses
const QString sqlContractsActiveView { qsl(
R"str(
SELECT id
  ,Kreditorin
  ,Vertragskennung
  ,Zinssatz
  ,Wert
  ,Aktivierungsdatum
  ,Kuendigungsfrist
  ,Vertragsende
  ,thesa
  ,KreditorId
FROM (%1)
)str").arg(sqlContractsActiveDetailsView)};

QString sqltableToCsvString(QString sql, QVector<QPair<QString, QVariant>> params)
{
    QVector<QSqlRecord> records;
    if( params.size()) {
        if( not executeSql(sql, params, records)) {
            qCritical() << "extracting data for table w params as csv failed";
            return QString();
        }
    } else {
        if( not executeSql(sql, records)){
            qCritical() << "extracting data for table as csv failed";
            return QString();
        }
    }

    if( records.isEmpty() || records[0].isEmpty()) {
        qInfo() << "no data for AS csv found";
        return QString();
    }
    CsvWriter csv;
    // add column header
    for( int cIndex =0; cIndex < records[0].count(); cIndex++) {
        csv.addColumn(records[0].fieldName(cIndex));
    }
    // add data
    for (const auto& record : std::as_const(records)) {

        qDebug() /*todo remvoe*/ << record;
        if( record.isEmpty()) {
            qCritical() << "empty data in AS result";
            continue;
        }
        for( int fIndex =0; fIndex < record.count(); fIndex++) {
            csv.appendValueToNextRecord(record.value(fIndex).toString());
        }
    }

    return csv.toString();

}

// Übersichten
// Übersicht: contractRuntimeDistribution, used in sqlContractsAllView

// Übersicht: abgeschl. Vertr. nach Zinssatz
const QString sqlContractsByYearByInterest {qsl(R"str(
SELECT SUBSTR(Vertraege.Vertragsdatum, 0, 5) as Year
  ,Vertraege.ZSatz /100. AS Zinssatz
  ,count(*) AS Anzahl
  ,sum(Vertraege.Betrag) /100. AS Summe
FROM Vertraege
GROUP BY Year, Zinssatz
ORDER BY Year DESC
)str")};
// Übersicht ausgezahlte Zinsen
const QString sqlInterestByYearOverview {qsl(R"str(
WITH alleBuchungen AS ( SELECT * FROM Buchungen UNION SELECT * FROM exBuchungen),
alleVertraege AS (SELECT * FROM Vertraege UNION SELECT * FROM exVertraege),
xTemp AS (
SELECT
  Datum as date,
  STRFTIME('%Y', Datum) as Year,
  SUM(alleBuchungen.Betrag) /100. as Summe,
  'Unterjährige Zinsen' as BA,
  ' ausgezahlte Zinsen ' as Thesa
FROM alleBuchungen INNER JOIN alleVertraege ON alleBuchungen.VertragsId = alleVertraege.id
WHERE alleBuchungen.BuchungsArt = 4 AND (SELECT COUNT(*) FROM alleBuchungen WHERE Datum = date) = 3
GROUP BY Year

UNION

SELECT
  Datum as date,
  STRFTIME('%Y', Datum) as Year,
  SUM(alleBuchungen.Betrag) /100. as Summe,
  'Unterjährige Zinsen' as BA,
  ' angerechnete Zinsen ' as Thesa
FROM alleBuchungen INNER JOIN alleVertraege ON alleBuchungen.VertragsId = alleVertraege.id
WHERE alleBuchungen.BuchungsArt = 4 AND (SELECT COUNT(*) FROM alleBuchungen WHERE Datum = date) = 2
GROUP BY Year

UNION

SELECT
  Datum as date,
  STRFTIME('%Y', Datum) as Year,
  SUM(alleBuchungen.Betrag) /100. as Summe,
  'Unterjährige Zinsen' as BA,
  ' gesamte Zinsen' as Thesa
FROM alleBuchungen INNER JOIN alleVertraege ON alleBuchungen.VertragsId = alleVertraege.id
WHERE alleBuchungen.BuchungsArt = 4
GROUP BY Year

)

SELECT
  STRFTIME('%Y', Datum) as Year,
  SUM(alleBuchungen.Betrag) /100. as Summe,
  'Zins aus Jahresendabrechnungen' as BA,
  CASE WHEN alleVertraege.thesaurierend = 0 THEN ' ausbezahlte Zinsen '
   WHEN alleVertraege.thesaurierend = 1 THEN ' angerechnete Zinsen '
   WHEN alleVertraege.thesaurierend = 2 THEN ' einbehaltene Zinsen '
   WHEN alleVertraege.thesaurierend = 3 THEN ' Auszahlung an zinslose Verträge (verm. ein Fehler) '
  END  as Thesa
FROM alleBuchungen INNER JOIN alleVertraege ON alleBuchungen.VertragsId = alleVertraege.id
WHERE alleBuchungen.BuchungsArt = 8
GROUP BY Year, thesaurierend

UNION

SELECT
  STRFTIME('%Y', Datum) as Year,
  SUM(alleBuchungen.Betrag) /100. as Summe,
  'Zins aus Jahresendabrechnungen' as BA,
  ' gesamte Zinsen ' AS Thesa
FROM alleBuchungen INNER JOIN alleVertraege ON alleBuchungen.VertragsId = alleVertraege.id
WHERE alleBuchungen.BuchungsArt = 8
GROUP BY Year, alleBuchungen.BuchungsArt

UNION

SELECT Year, Summe, BA, Thesa
FROM xTemp

ORDER BY YEAR DESC, BA DESC, Thesa ASC
)str"
)};
// Übersichten "Kurz info"
const QString sqlOverviewActiveContracts{qsl(R"str(
WITH Bookings AS (
  SELECT Vertraege.id        AS vid
  , KreditorId               AS kid
  , zSatz                    AS Zinssatz
  , Buchungen.Betrag         AS Betrag
  , IIF( Vertraege.thesaurierend = 2, IIF( Buchungen.BuchungsArt = 8, 0, Buchungen.Betrag), Buchungen.Betrag)
                             AS VerzinslGuthaben
  FROM Buchungen JOIN Vertraege ON Vertraege.id = Buchungen.VertragsId
)
, counts AS (
  SELEcT COUNT(DISTINCT vid) AS AnzahlVertraege
     , COUNT(DISTINCT kid) AS AnzahlKreditoren
  FROM
    Bookings
)
, mittlererZins AS (
  SELEcT avg(ZSatz) /100. AS MittelZins
  FROM Vertraege WHERE id IN (SELEcT DISTINCT VertragsId FROM Buchungen)
)
, sums AS (
  SELEcT SUM(Betrag) /100. AS GesamtVolumen
    , SUM(VerzinslGuthaben *Zinssatz)/100. /100. /100. AS Jahreszins
  FROM Bookings
)

SELECT AnzahlVertraege
  , AnzahlKreditoren
  , GesamtVolumen
  , GesamtVolumen / AnzahlVertraege AS MittlererVertragswert
  , Jahreszins
  , Jahreszins / GesamtVolumen * 100. AS ZinsRate
  , MittelZins
FROM counts INNER JOIN sums INNER JOIN mittlererZins
)str")
                    };
const QString sqlOverviewInActiveContracts{qsl(R"str(
SELEcT COUNT( DISTINCT KreditorId)     AS AnzahlKreditoren
  , COUNT( *)                          AS AnzahlVertraege
  , SUM(Betrag) /100.                  AS GesamtVolumen
  , SUM(Betrag) /100. /COUNT(*)        AS MittlererVertragswert
  , SUM(Betrag*ZSatz) /100. /100. /100.  AS JahresZins
  , SUM(Betrag*ZSatz) /100. /SUM(Betrag) AS ZinsRate
  , AVG(ZSatz) /100.                     AS MittelZins
FROM Vertraege
WHERE Id NOT IN (SELEcT DISTINCT VertragsId FROM Buchungen)
)str")};
const QString sqlOverviewAllContracts {qsl(R"str(
WITH all_counts AS (
  SELEcT COUNT(DISTINCT id)          AS AnzahlVertraege
    , COUNT(DISTINCT KreditorId)     AS AnzahlKreditoren
    , AVG(ZSatz) /100.               AS MittelZins
  FROM Vertraege
)
, active_bookings AS (
  SELECT ZSatz /100.                 AS Zinssatz
    , Buchungen.Betrag               AS Betrag
    , IIF( Vertraege.thesaurierend = 2, IIF( Buchungen.BuchungsArt = 8, 0, Buchungen.Betrag), Buchungen.Betrag)
                                     AS VerzinslGuthaben
    FROM Buchungen JOIN Vertraege ON Vertraege.id = Buchungen.VertragsId
)
, active_sums AS (
  SELEcT IFNULL( SUM(Betrag) /100., 0)            AS GesamtVolumen
    , IFNULL( SUM(Betrag) /100. /COUNT(*), 0)     AS MittlererVertragswert
    , IFNULL( SUM(VerzinslGuthaben *Zinssatz) /100. /100., 0)
                                      AS Jahreszins
    , IFNULL( SUM(VerzinslGuthaben *Zinssatz) /100. /100. / SUM(Betrag) *100., 0)
                                      AS ZinsRate
    FROM active_bookings
)
, inactive_sums AS (
  SELEcT IFNULL(SUM(Betrag) /100., 0)             AS GesamtVolumen
  , IFNULL(SUM(Betrag) /100. /COUNT(*), 0)        AS MittlererVertragswert
  , IFNULL(SUM(Betrag*ZSatz) /100. /100. /100., 0)  AS JahresZins
  , IFNULL(SUM(Betrag*ZSatz) /100. /SUM(Betrag), 0) AS ZinsRate

  FROM Vertraege
  WHERE id NOT IN (SELEcT DISTINCT VertragsId FROM Buchungen)
)

SELEcT all_counts.AnzahlVertraege
  , all_counts.AnzahlKreditoren
  , active_sums.GesamtVolumen + inactive_sums.GesamtVolumen AS GesamtVolumen
  , (active_sums.GesamtVolumen + inactive_sums.GesamtVolumen) /all_counts.AnzahlVertraege
                                                            AS MittlererVertragswert
  , (active_sums.Jahreszins + inactive_sums.Jahreszins)     AS Jahreszins
  , 100.* ((active_sums.Jahreszins + inactive_sums.Jahreszins))/(active_sums.GesamtVolumen + inactive_sums.GesamtVolumen)
                                                            AS ZinsRate
  , all_counts.MittelZins                                   AS MittelZins
FROM all_counts JOIN active_sums JOIN inactive_sums
)str")};

// used for time series statistics
const QString sqlStat_activeContracts_byIMode_toDate {qsl(
R"str(
WITH BookingsFromOpenContracts AS (
  SELECT Vertraege.id        AS vid
  , KreditorId     AS kid
  , zSatz          AS Zinssatz
  , thesaurierend  AS imode
  , Datum          AS BDatum
  , Buchungen.Betrag         AS Betrag
  , IIF( Vertraege.thesaurierend = 2, IIF( Buchungen.BuchungsArt = 8, 0, Buchungen.Betrag), Buchungen.Betrag) AS BetragOhneJahreszins
  FROM Buchungen JOIN Vertraege ON Vertraege.id = Buchungen.VertragsId
  WHERE BDatum <= date(':date')
)
, active_Ex_Contracts AS (
SELECT exBuchungen.VertragsId AS vid
FROM exBuchungen
GROUP BY exBuchungen.VertragsId
HAVING MIN(Datum) <= date(':date') AND MAX(Datum) > date(':date')
)
, Bookings_Ex_Contracts AS (
  SELECT exVertraege.id        AS vid
  , KreditorId     AS kid
  , zSatz          AS Zinssatz
  , thesaurierend  AS imode
  , Datum          AS BDatum
  , exBuchungen.Betrag         AS Betrag
  , IIF( exVertraege.thesaurierend = 2, IIF( exBuchungen.BuchungsArt = 8, 0, exBuchungen.Betrag), exBuchungen.Betrag) AS BetragOhneJahreszins
  FROM exBuchungen JOIN exVertraege ON exVertraege.id = exBuchungen.VertragsId
  WHERE BDatum <= date(':date') AND exBuchungen.VertragsId IN (SELEcT * FROM active_Ex_Contracts)
)
, allBookingsActiveContracts_Ex_Contracts AS (
 SELEcT * FROM Bookings_Ex_Contracts
   UNION
 SELECT * FROM BookingsFromOpenContracts
)

, nbrsBy_iMode AS (
SELEcT iMode
  , COUNT(DISTINCT vid) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
FROM
  allBookingsActiveContracts_Ex_Contracts
GROUP BY imode
)
, nbrsAll_iMode AS (
SELEcT 'all' AS iMode
  , COUNT(DISTINCT vid) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
FROM
  allBookingsActiveContracts_Ex_Contracts
)
/* Anzahl Kreditoren und Verträge
*/
, nbrs AS (
SELEcT * FROM nbrsBy_iMode
UNION
SELEcT * FROM nbrsAll_iMode
)
, WertUndZins_byVid AS (
SELEcT iMode
  , SUM(Betrag) /100. AS VertragsWert
  , IIF(iMode = 2, SUM(BetragOhneJahreszins)/100., SUM(Betrag)/100.) AS VerzinslGuthaben
  , IIF( iMode = 2, SUM(BetragOhneJahreszins) * Zinssatz /100./100./100., SUM(Betrag) * Zinssatz /100./100./100.) AS Jahreszins
FROM allBookingsActiveContracts_Ex_Contracts
GROUP BY vid
)
, WertUndZins_by_iMode AS (
SELEcT iMode
  , ROUND(SUM(VertragsWert), 2) AS totalVolume
  , ROUND(SUM(JahresZins), 2) AS totalInterest
  , ROUND(SUM(JahresZins)/SUM(VerzinslGuthaben)*100., 2) AS avgInterest
FROM WertUndZins_byVid
GROUP BY iMode
)
, WertUndZins_all_iMode AS (
SELEcT 'all' AS iMode
  , ROUND(SUM(VertragsWert), 2) AS totalVolume
  , ROUND(SUM(JahresZins), 2) AS totalInterest
  , ROUND(SUM(JahresZins)/ SUM(VerzinslGuthaben)*100.,2) AS avgInterest
FROM WertUndZins_byVid
)
, WertUndZins AS (
SELEcT * FROM WertUndZins_by_iMode
UNION
SELEcT * FROM WertUndZins_all_iMode
)

SELECT nbrs.iMode
  , AnzahlVertraege
  , AnzahlKreditoren
  , totalVolume
  , totalInterest
  , avgInterest
FROM nbrs
INNER JOIN WertUndZins On nbrs.iMode = WertUndZins.iMode
)str")
};
const QString sqlStat_inactiveContracts_byIMode_toDate {qsl(
R"str(
WITH alleDaten AS (

SELECT Vertraege.id AS vid
  , Vertraege.KreditorId AS kid
  , Vertraege.thesaurierend AS imode
  , Vertraege.Vertragsdatum AS VDatum
  , MIN(Buchungen.Datum) AS ADatum
  , Vertraege.Betrag AS Wert
  , Vertraege.ZSatz AS Zinssatz
FROM Vertraege
JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id
GROUP BY Vertraege.id

UNION

SELECT exVertraege.id AS vid
  , exVertraege.KreditorId AS kid
  , exVertraege.thesaurierend AS imode
  , exVertraege.Vertragsdatum AS VDatum
  , MIN(exBuchungen.Datum) AS ADatum
  , exVertraege.Betrag AS Wert
  , exVertraege.ZSatz AS Zinssatz
FROM exVertraege
JOIN exBuchungen ON exBuchungen.VertragsId = exVertraege.id
GROUP BY exVertraege.id

UNION

SELEcT Vertraege.id AS vid
  , Vertraege.KreditorId AS kid
  , Vertraege.thesaurierend AS imode
  , Vertraege.Vertragsdatum AS VDatum
  , date('9999-12-31') AS ADatum
  , Vertraege.Betrag AS Wert
  , Vertraege.ZSatz AS Zinssatz
FROM Vertraege
WHERE Vertraege.id NOT IN
  (SELEcT DISTINCT VertragsId FROM Buchungen)
)
, filteredByDate AS (
SELEcT *
FROM alleDaten
WHERE VDatum <= date(':date') AND ADatum > date(':date')
)

SELEcT imode
  , COUNT(vid)                  AS AnzahlKreditoren
  , COUNT(DISTINCT kid) AS AnzahlVertraege
  , SUM(Wert) /100.            AS totalVolume
  , ROUND(SUM(Wert * Zinssatz) /100. /100. /100., 2) AS totalInterest
  , ROUND((SUM(Wert * Zinssatz) /100.) / SUM(Wert), 2)  AS avgInterest
FROM filteredByDate
GROUP BY imode

UNION

SELEcT 'all' AS imode
  , COUNT(vid)          AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
  , SUM(Wert) /100.     AS totalVolume
  , ROUND(SUM(Wert * Zinssatz) /100. /100. /100., 2) AS totalInterest
  , ROUND((SUM(Wert * Zinssatz) /100.) / SUM(Wert), 2)  AS avgInterest
FROM filteredByDate
)str"
)};
const QString sqlStat_finishedContracts_toDate {qsl(
/* imode, AnzahlVertraege, AnzahlKreditoren, totalVolume, totalInterest, AvgInterest */
R"str(
SELECT 'all'
  , COUNT(*) AS AnzahVertraege
  , COUNT( DISTINCT(KreditorId)) AS AnzahlKreditoren
  , SUM(Betrag) /100. AS totalVolume
  , SUM(Betrag * ZSatz /100. /100. /100.) AS totalInterest
  , AVG(Betrag * ZSatz /100. /100. /100.) AS AvgInterest
FROM exVertraege
WHERE id in (  SELECT VertragsId
  FROM exBuchungen
  GROUP BY VertragsId
  HAVING MAX(exBuchungen.Datum) <= date(':date'))
)str"
)};
const QString sqlStat_allContracts_byIMode_toDate {qsl(
R"str(
WITH tmp_AktiveVertraege_IDs_zumDatum_date AS (
  SELEcT DISTINCT VertragsId
  FROM Buchungen
  GROUP BY VertragsId
  HAVING MIN(Datum) <= date(':date')
)
, tmpAktiveVertraege AS (
  SELEcT * FROM Vertraege WHERE Vertraege.id IN tmp_AktiveVertraege_IDs_zumDatum_date
)
, tmpBuchungenAktiverVertraege AS (
  SELEcT tmpAktiveVertraege.id      AS vid
    , tmpAktiveVertraege.KreditorId AS kid
    , tmpAktiveVertraege.ZSatz      AS Zinssatz
    , tmpAktiveVertraege.thesaurierend AS iMode
    , Buchungen.Betrag     AS Betrag
    , IIF( tmpAktiveVertraege.thesaurierend = 2,
          IIF( Buchungen.BuchungsArt = 8, 0, Buchungen.Betrag), Buchungen.Betrag) AS verzWert_for_fix_interest_contracts
  FROM Buchungen JOIN tmpAktiveVertraege ON Buchungen.VertragsId = tmpAktiveVertraege.id
  WHERE Buchungen.Datum <= date(':date')
)
, tmpWerteAktiverVertraege AS (
  SELEcT vid
    , kid
    , iMode
    , SUM(Betrag) /100. AS VertragsWert
    , SUM(verzWert_for_fix_interest_contracts) /100. AS VerzinslGuthaben
    , SUM(verzWert_for_fix_interest_contracts) *Zinssatz /100./100./100. AS jaehrlicherZins
  FROM tmpBuchungenAktiverVertraege
  GROUP BY vid
)
, tmpWertePassiverVertraege AS (
  SELECT id AS vid
    , KreditorId AS kid
    , thesaurierend AS iMode
    , Betrag /100. AS VertragsWert
    , Betrag /100. AS VerzinslGuthaben
    , Betrag * ZSatz /100./100./100. AS jaehrlicherZins
  FROM Vertraege WHERE Vertragsdatum <= date(':date') AND vid NOT IN tmp_AktiveVertraege_IDs_zumDatum_date
)
, tmpId_Aktive_exVertrage_IDs_zumDatum_date AS (
  SELEcT DISTINCT VertragsId
  FROM exBuchungen
  GROUP BY VertragsId
  HAVING MIN(Datum) <= date(':date') AND MAX(Datum) > date(':date')
)
, tmpBuchungenAktiveExVertraege AS (
  SELECT exVertraege.id AS vid
    , KreditorId AS kid
    , thesaurierend AS iMode
    , ZSatz as Zinssatz
    , exVertraege.Betrag  AS Betrag
    , IIF( thesaurierend = 2, IIF (exBuchungen.BuchungsArt = 8, 0, exBuchungen.Betrag), 0) AS verzWert_for_fix_interest_contracts
  FrOM exBuchungen JOIN exVertraege ON exBuchungen.VertragsId = exVertraege.id
  WHERE exBuchungen.Datum < date(':date') AND exVertraege.id IN tmpId_Aktive_exVertrage_IDs_zumDatum_date
)
, tmpWerteAktiverExVertraege AS (
  SELECT vid
    , kid
    , iMode
    , SUM(Betrag) /100. AS VertragsWert
    , SUM(verzWert_for_fix_interest_contracts)/100. AS VerzinslGuthaben
    , IIF(iMode =2, SUM(verzWert_for_fix_interest_contracts) *Zinssatz /100./100./100., Betrag * Zinssatz /100./100./100.) AS jaehrlicherZins
  FROM tmpBuchungenAktiveExVertraege
  GROUP BY vid
)
, tmpId_Passive_exVertrage_IDs_zumDatum_date AS (
  SELEcT exBuchungen.VertragsId
  FROM exBuchungen INNER JOIN exVertraege ON exBuchungen.VertragsId = exVertraege.id
  GROUP BY exVertraege.id
  HAVING exVertraege.Vertragsdatum <= date(':date') AND MIN(exBuchungen.datum) > date(':date')
)

, tmpWertePassiverExVertraege AS (
  SELEcT id AS vid
    , KreditorId AS kid
    , thesaurierend AS iMode
    , Betrag /100. AS VertragsWert
    , Betrag /100. AS VerzinslGuthaben
    , Betrag * ZSatz /100. /100. /100. AS jaehrlicherZins
  FROM exVertraege
  WHERE Vertragsdatum <= date(':date') AND id IN tmpId_Passive_exVertrage_IDs_zumDatum_date
)
, tmpWerteAllerVertraege AS (
SELEcT * FRom tmpWerteAktiverVertraege
  UNION
SELECT * FROM tmpWertePassiverVertraege
  UNION
SELECT * FROM tmpWerteAktiverExVertraege
  UNION
SELEcT * FROM tmpWertePassiverExVertraege
)

SELEcT iMode
  , COUNT(vid) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
  , SUM(VertragsWert) AS totalVolume
  , SUM(jaehrlicherZins) AS totalInterest
  , ROUND(jaehrlicherZins/VerzinslGuthaben *100., 2) AS avgInterest
FROM tmpWerteAllerVertraege
GROUP BY iMode

UNION

SELECT 'all' AS iMode
  , COUNT(vid) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
  , SUM(VertragsWert) AS totalVolume
  , SUM(jaehrlicherZins) AS totalInterest
  , ROUND(SUM(jaehrlicherZins)/SUM(VerzinslGuthaben) *100., 2) AS avgInterest
FROM tmpWerteAllerVertraege
)str")};

// {qsl(R"str()str")};
