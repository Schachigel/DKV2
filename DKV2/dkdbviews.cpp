#include "helper.h"
#include "dkdbviews.h"
//////////////////////////////////////
// VIEWs to be created in the database
//////////////////////////////////////
const QString vnContractView {qsl("vVertraege_alle_4view")};
const QString sqlContractView {qsl(
R"str(
SELECT
  V.id AS VertragsId
  ,K.id AS KreditorId
  ,K.Nachname || ', ' || K.Vorname          AS KreditorIn
  ,V.Kennung AS Vertragskennung
  ,V.Anmerkung AS Anmerkung
  ,strftime('%d.%m.%Y',V.Vertragsdatum)     AS Vertragsdatum

  ,ifnull(strftime('%d.%m.%Y',Aktivierungsdatum), '(offen)') AS Aktivierungsdatum
  ,CASE WHEN AktivierungsWert IS NULL THEN V.Betrag /100.
      ELSE AktivierungsWert
      END AS Nominalwert

  ,CAST(V.Zsatz / 100. AS VARCHAR) || ' %'    AS Zinssatz

-- Zinsmodus
  ,V.thesaurierend AS Zinsmodus

-- VerzinslGuthaben
  ,CASE WHEN V.thesaurierend = 0 THEN ifnull(summeAllerBuchungen, ' (inaktiv) ')
       ELSE CASE WHEN V.thesaurierend = 1 THEN ifnull(summeAllerBuchungen, ' (inaktiv) ')
       ELSE CASE WHEN V.thesaurierend = 2 THEN ifnull(summeEinUndAuszahlungen, ' (inaktiv) ')
       ELSE CASE WHEN V.thesaurierend = 3 THEN ' ohne Verzinsung '
       ELSE 'ERROR' END END END END   AS VerzinslGuthaben

-- angesparter Zins
  ,CASE WHEN V.thesaurierend = 0 THEN ifnull(summeAllerZinsZwBuchungen, '(noch 0) ')
       ELSE CASE WHEN V.thesaurierend = 1 THEN ifnull(summeAllerZinsBuchungen, '(noch 0) ')
       ELSE CASE WHEN V.thesaurierend = 2 THEN ifnull(summeAllerZinsBuchungen, '(noch 0) ')
       ELSE CASE WHEN V.thesaurierend = 3 THEN '(zinslos)   '
       ELSE 'ERROR' END END END END    AS angespZins

-- letzte Buchungen
  ,ifnull(LetzteBuchung, ' - ') AS LetzteBuchung

-- kdFrist/VEnd
  ,CASE WHEN V.Kfrist = -1 THEN strftime('%d.%m.%Y', V.LaufzeitEnde)
     ELSE '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)' END AS KdgFristVertragsende

FROM Vertraege AS V  INNER JOIN Kreditoren AS K ON V.KreditorId = K.id

-- aktivierungsdatum und Betrag
LEFT JOIN
    (SELECT vid_min, minB_Datum AS Aktivierungsdatum, Erstbuchungswert AS AktivierungsWert
         FROM   (SELECT B.VertragsId AS vid_min, min(B.Datum) AS minB_Datum, B.Betrag / 100. AS Erstbuchungswert, sum(B.Betrag) / 100. AS GesamtWert
            FROM Buchungen AS B GROUP BY B.VertragsId) ) ON V.id = vid_min

-- letzte Buchungen
LEFT JOIN
     (SELECT VertragsId AS B3VertragsId, MAX(Datum) AS LetzteBuchung FROM Buchungen AS B3 GROUP BY VertragsId) on v.id = B3VertragsId

-- VerzinslGuthaben f auszahlende
LEFT JOIN
     (SELECT VertragsId, SUM(Betrag)/100. AS summeAllerBuchungen FROM Buchungen AS B1 GROUP BY VertragsId) ON V.id = VertragsId

-- angespZins f auszahlende: nur zwischenzinsen
LEFT JOIN
     (SELECT VertragsId AS B2VertragsId, SUM(Betrag)/100. AS summeAllerZinsZwBuchungen FROM Buchungen AS B2 WHERE B2.BuchungsArt = 4 /*ReInvInterest*/  GROUP BY B2.VertragsId) ON V.id = B2VertragsId

-- angespZins f thesaurierende: zwischenzinsen + Jahreszins
LEFT JOIN
     (SELECT VertragsId AS B4VertragsId, SUM(Betrag)/100. AS summeAllerZinsBuchungen FROM Buchungen AS B4 WHERE B4.BuchungsArt = 8 /*annual*/ OR B4.BuchungsArt = 4 /*ReInvInterest*/  GROUP BY B4.VertragsId) ON V.id = B4VertragsId

-- VerzinslGuthaben FestZins: Summe aller ein und auszahlungen
LEFT JOIN
    (SELECT VertragsId AS B5VertragsId, SUM(Betrag) /100. AS summeEinUndAuszahlungen FROM Buchungen AS B5 WHERE B5.BuchungsArt = 1 /*deposit*/ OR B5.BuchungsArt = 2 /*payout*/ GROUP BY B5.VertragsId) ON V.id = B5VertragsId

)str"
)};

const QString vnExContractView {qsl("vVertraege_geloescht")};
const QString sqlExContractView {qsl(
R"str(
SELECT
  V.id AS VertragsId,
  K.id AS KreditorId,
  K.Nachname || ', ' || K.Vorname AS KreditorIn,
  V.Kennung AS Vertragskennung,
  strftime('%d.%m.%Y',Aktivierungsdatum) AS Aktivierung,
  strftime('%d.%m.%Y', Vertragsende) AS Vertragsende,
  ifnull(AktivierungsWert, V.Betrag / 100.) AS Anfangswert,
  CAST(V.Zsatz / 100. AS VARCHAR) || ' %'    AS Zinssatz,
  CASE WHEN V.thesaurierend = 0
  THEN 'Auszahlend'
  ELSE CASE WHEN V.thesaurierend = 1
       THEN 'Thesaur.'
        ELSE CASE WHEN V.thesaurierend = 2
             THEN 'Fester Zins'
             ELSE 'ERROR'
             END
        END
  END AS Zinsmodus,

  CASE WHEN V.thesaurierend  = 0 THEN Zinsen_ausz
  ELSE Zinsen_thesa
  END AS Zinsen,
  CASE WHEN V.thesaurierend  > 0
  THEN -1* letzte_Auszahlung + sum_o_Zinsen
  ELSE -1* letzte_Auszahlung + sum_mit_Zinsen
  END AS Einlagen,

  maxValue AS Endauszahlung

FROM exVertraege AS V  INNER JOIN Kreditoren AS K ON V.KreditorId = K.id

LEFT JOIN (
     SELECT
        B.VertragsId AS vid_min,
    min(B.id) AS idErsteBuchung,
    B.Datum AS Aktivierungsdatum,
    B.Betrag / 100. AS AktivierungsWert,
    sum(B.Betrag) / 100. AS GesamtWert
     FROM exBuchungen AS B GROUP BY B.VertragsId )
ON V.id = vid_min

LEFT JOIN (SELECT
         B.VertragsId AS vid_max,
         max(B.id) as idLetzteBuchung,
         B.Datum AS Vertragsende,
         B.Betrag /100. AS maxValue
       FROM exBuchungen AS B GROUP BY B.VertragsId )
ON V.id = vid_max

LEFT JOIN (SELECT
            B.VertragsId AS vidZinsThesa,
            SUM(B.betrag) /100. AS Zinsen_thesa
          FROM exBuchungen AS B
          WHERE B.BuchungsArt = 4 OR B.BuchungsArt = 8
          GROUP BY B.VertragsId )
ON V.id = vidZinsThesa

LEFT JOIN ( SELECT
              B.VertragsId AS vidZinsAus,
              SUM(B.betrag) /-100. AS Zinsen_ausz
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8 GROUP BY B.VertragsId )
ON V.id = vidZinsAus

LEFT JOIN ( SELECT
                B.VertragsId as vid_o_Zinsen,
                sum(B.Betrag) /100. AS sum_o_Zinsen
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2
            GROUP BY B.VertragsId  )
ON V.id = vid_o_Zinsen

LEFT JOIN ( SELECT
              B.VertragsId as vid_mit_Zinsen,
              sum(B.Betrag) /100. AS sum_mit_Zinsen,
              max(B.Datum) AS letzte_buchung,
              B.Betrag /100. AS letzte_Auszahlung
            FROM exBuchungen AS B
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8
            GROUP BY B.VertragsId  )
ON V.id = vid_mit_Zinsen
)str"
)};

const QString vnInvestmentsView {qsl("vInvestmentsOverview")};
const QString sqlInvestmentsView {qsl(
R"str(
SELECT ga.ZSatz
  , ga.Anfang
  , ga.Ende
  , ga.Typ
  , (SELECT count(*)
     FROM Vertraege
     WHERE Vertragsdatum >= ga.Anfang
       AND Vertragsdatum < ga.Ende
       AND ZSatz = ga.ZSatz
  ) AS Anzahl
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege
     WHERE Vertragsdatum >= ga.Anfang
       AND Vertragsdatum < ga.Ende
       AND ZSatz = ga.ZSatz
  ) AS Summe
  , (SELECT count(*)
     FROM Vertraege AS v
     WHERE Vertragsdatum >= ga.Anfang
       AND Vertragsdatum < ga.Ende
       AND ZSatz = ga.ZSatz
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId=v.id)>0
  ) AS AnzahlAktive
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege AS v
     WHERE Vertragsdatum >= ga.Anfang
       AND Vertragsdatum < ga.Ende
       AND ZSatz = ga.ZSatz
       AND (SELECT count(*)
            FROM Buchungen AS B
            WHERE B.VertragsId=v.id)>0
  ) AS SummeAktive
  , CASE WHEN ga.Offen THEN 'Offen' ELSE 'Abgeschlossen' END AS Offen
  , ga.rowid
FROM Geldanlagen AS ga
ORDER BY ga.Offen DESC, ga.ZSatz DESC
)str"
)};

const QString vnBookingsOverview {qsl("vBuchungen")};
const QString sqlBookingsOverview {qsl(
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

const QString vnStat_activeContracts_byIMode{qsl("vStat_aktiveVertraege_nachZinsModus")};
const QString sqlStat_activeContracts_byIMode{qsl(
R"str(
-- no terminated contracts here because the payout cancels their value
WITH
sum_by_iModes AS ( -- Summe Kredite, Summe Zinsen nach Zeit und Zinsmodus
  SELECT thesaurierend
    , sum(Betrag) /100. AS Kreditvolumen
    , round(sum(Betrag * Zinssatz) /100./100./100., 2) AS Jahreszins

  FROM (
    SELECT B.Datum as Buchungsdatum
      , B.Betrag as Betrag
      , V.ZSatz as Zinssatz
      , thesaurierend

    FROM Buchungen AS B
    INNER JOIN Vertraege AS V
      On B.VertragsId = V.id
    )
  GROUP BY thesaurierend
)
, count_by_iModes AS ( -- Anzahl *aktiver* Verträge / Creditoren nach Zeit und Zinsmodus
  SELECT thesaurierend
    , count(*) AS nbrContracts
    , count(DISTINCT KreditorId) AS nbrCreditors
  FROM Vertraege AS V
  INNER JOIN
  (
    SELECT DISTINCT B.VertragsId AS Vid, min(B.Datum) AS Aktivierungsdatum
    FROM Buchungen AS B
    GROUP BY Vid
  ) ON V.id =Vid
  GROUP BY thesaurierend
)
, sum_all_iModes AS ( -- Summe Kredite und Zinsen für alle Kredite
  SELEcT 'all' AS thesaurierend
    , sum(Betrag) /100. AS KreditVolumen
    , round(sum(Betrag * Zinssatz) /100. /100. /100., 2) AS Jahreszins
  FROM (
    SELECT B.Datum AS BuchungsDatum
    , B.Betrag AS Betrag
    , V.ZSatz as Zinssatz
    FROM Buchungen AS B
    INNER JOIN Vertraege AS V
      On B.VertragsId = V.id
  )
)
, count_all_iModes AS (
  SELECT 'all' AS thesaurierend
    , count(*) AS nbrContracts
    , count(DISTINCT KreditorId) AS nbrCreditors
  FROM Vertraege AS V
  INNER JOIN
  (
    SELECT DISTINCT B.VertragsId AS Vid, min(B.Datum) AS Aktivierungsdatum
    FROM Buchungen AS B
    GROUP BY Vid
  ) ON V.id =Vid
)
------ THE ACTION STARTS HERE
SELEcT sum_by_iModes.thesaurierend
  , count_by_iModes.nbrCreditors AS nbrCreditorsActiveContracts
  , count_by_iModes.nbrContracts AS nbrActiveContracts
  , sum_by_iModes.KreditVolumen AS activeCreditVolume
  , sum_by_iModes.Jahreszins AS activeAnnualInterest
  , round(100. * sum_by_iModes.Jahreszins / sum_by_iModes.KreditVolumen,2) AS activeContractsAvgInterest
FROM sum_by_iModes
INNER JOIN count_by_iModes ON sum_by_iModes.thesaurierend = count_by_iModes.thesaurierend

UNION

SELEcT sum_all_iModes.thesaurierend
  , count_all_iModes.nbrCreditors AS nbrCreditorsActiveContracts
  , count_all_iModes.nbrContracts AS nbrActiveContracts
  , sum_all_iModes.KreditVolumen AS activeCreditVolume
  , sum_all_iModes.Jahreszins AS activeAnnualInterest
  , round(100. * sum_all_iModes.Jahreszins / sum_all_iModes.KreditVolumen,2) AS activeContractsAvgInterest
FROM sum_all_iModes
INNER JOIN count_all_iModes ON sum_all_iModes.thesaurierend = count_all_iModes.thesaurierend
)str"
)};

const QString vnStat_inactiveContracts_byIMode {qsl("vStat_inaktiveVertraege_nachZinsmodus")};
const QString sqlStat_inactiveContracts_byIMode {qsl(
R"str(
-- Werte aller inaktiven Vertraege
SELECT thesaurierend
  , nbrCreditorsInactiveContracts
  , nbrInactiveContracts
  , inactiveCreditVolume
  , inactiveAnnualInterest
  , round(100. * inactiveAnnualInterest / inactiveCreditVolume, 2) AS inactiveContractsAvgInterest
FROM (
  SELECT thesaurierend
    , COUNT(*) AS nbrInactiveContracts
    , COUNT( DISTINCT KreditorId) AS nbrCreditorsInactiveContracts
    , SUM(Betrag) /100. AS inactiveCreditVolume
    , round(SUM(Betrag * ZSatz) /100. /100. /100.,2) AS inactiveAnnualInterest

  FROM Vertraege
  WHERE id NOT IN (SELECT DISTINCT VertragsId FROM Buchungen)
  GROUP BY thesaurierend
)

UNION

SELECT 'all' AS thesaurierend
  , nbrCreditorsInactiveContracts
  , nbrInactiveContracts
  , inactiveCreditVolume
  , inactiveAnnualInterest
  , round(100. * inactiveAnnualInterest / inactiveCreditVolume, 2) AS inactiveContractsAvgInterest
FROM (
  SELECT thesaurierend
    , COUNT(*) AS nbrInactiveContracts
    , COUNT( DISTINCT KreditorId) AS nbrCreditorsInactiveContracts
    , SUM(Betrag) /100. AS inactiveCreditVolume
    , round(SUM(Betrag * ZSatz) /100. /100. /100.,2) AS inactiveAnnualInterest
  FROM Vertraege
  WHERE id NOT IN (SELECT DISTINCT VertragsId FROM Buchungen)
)
)str")};

QMap<QString, QString> views ={
    // model of table view: contracts
    {vnContractView, sqlContractView},
    // model of table view: deleted contracts
    {vnExContractView, sqlExContractView},
    // model of table view: investments
    {vnInvestmentsView, sqlInvestmentsView},
    // convenientce view
    {vnBookingsOverview, sqlBookingsOverview},
    // statistics of contracts by Interest Mode
    {vnStat_activeContracts_byIMode, sqlStat_activeContracts_byIMode},
    {vnStat_inactiveContracts_byIMode, sqlStat_inactiveContracts_byIMode}
};

const QMap<QString, QString>& getViews() {
    return views;
}

QString getView(const QString vn) {
    return views.value(vn);
}

bool remove_all_views(const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{   LOG_CALL;
    QVector<QSqlRecord> views;
    if( not executeSql(qsl("SELECT name FROM sqlite_master WHERE type = ?"), QVariant(qsl("view")), views, db)) {
        return false;
    }
    for( auto rec : views) {
        if( executeSql_wNoRecords(qsl("DROP view %1").arg(rec.value(0).toString()), db))
            continue;
        return false;
    }
    return true;
}



//////////////////////////////////////
// SQL Statemends (stored here to not clutter the source code with long constant strings)
//////////////////////////////////////
const QString sqlContractsActiveDetailsView{ qsl(
R"str(
SELECT
  Vertraege.id          AS id,
  Vertraege.Kennung     AS Vertragskennung,
  Vertraege.ZSatz /100. AS Zinssatz,
  SUM(Buchungen.Betrag) /100. AS Wert,
  MIN(Buchungen.Datum)  AS Aktivierungsdatum,
  Vertraege.Kfrist      AS Kuendigungsfrist,
  Vertraege.LaufzeitEnde  AS Vertragsende,
  Vertraege.thesaurierend AS thesa,
  Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin,
  Kreditoren.id         AS KreditorId,
  Kreditoren.Nachname   AS Nachname,
  Kreditoren.Vorname    AS Vorname,
  Kreditoren.Strasse    AS Strasse,
  Kreditoren.Plz        AS Plz,
  Kreditoren.Stadt      AS Stadt,
  Kreditoren.Email      AS Email,
  Kreditoren.IBAN       AS Iban,
  Kreditoren.BIC        AS Bic
FROM Vertraege
    INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id
    INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId
GROUP BY Vertraege.id
)str"
)};

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

const QString sqlContractsInactiveView {qsl(
R"str(
SELECT Vertraege.id AS id,
  Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin,
  Vertraege.Kennung AS Vertragskennung,
  Vertraege.ZSatz /100. AS Zinssatz,
  Vertraege.Betrag /100. AS Wert,
  Vertraege.Vertragsdatum AS Abschlussdatum,
  Vertraege.Kfrist AS Kuendigungsfrist,
  Vertraege.LaufzeitEnde AS Vertragsende,
  Vertraege.thesaurierend AS thesa,
  Kreditoren.id AS KreditorId
FROM Vertraege
  INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId
WHERE (SELECT count(*) FROM Buchungen WHERE Buchungen.VertragsId=Vertraege.id) = 0
)str"
)};

const QString sqlContractsAllView {qsl(
R"str(
    SELECT id
      ,Kreditorin
      ,Vertragskennung
      ,Zinssatz
      ,Wert
      ,Aktivierungsdatum AS Datum
      ,Kuendigungsfrist
      ,Vertragsende
      ,thesa
      ,KreditorId
    FROM (%1)
UNION
    SELECT id
      ,Kreditorin
      ,Vertragskennung
      ,Zinssatz
      ,Wert
      ,Abschlussdatum  AS Datum
      ,Kuendigungsfrist
      ,Vertragsende
      ,thesa
      ,KreditorId
  FROM (%2)
)str"
).arg(sqlContractsActiveView, sqlContractsInactiveView)};

const QString sqlNextAnnualSettlement_firstAS {qsl(
R"str(
SELECT
  STRFTIME('%Y-%m-%d', MIN(Datum), '1 year', 'start of year', '-1 day')  as nextInterestDate
FROM Buchungen INNER JOIN Vertraege ON Vertraege.id = buchungen.VertragsId
/* buchungen von Verträgen für die es keine Zinsbuchungen gibt */
WHERE
  (SELECT count(*)
   FROM Buchungen
   WHERE (Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8) AND Buchungen.VertragsId=Vertraege.id)=0
GROUP BY Vertraege.id
)str"
)};

const QString sqlNextAnnualSettlement_nextAS {qsl(
R"str(
SELECT
  STRFTIME('%Y-%m-%d', MAX(Datum), '1 day', '1 year', 'start of year', '-1 day') as nextInterestDate
FROM Buchungen INNER JOIN Vertraege ON Vertraege.id=buchungen.VertragsId
WHERE Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8
GROUP BY Buchungen.VertragsId
ORDER BY Datum ASC LIMIT 1
)str"
)};

const QString sqlNextAnnualSettlement {qsl(
R"str(
SELECT MIN(nextInterestDate) AS date
FROM
  (SELECT nextInterestDate FROM (%1)
     UNION
  SELECT nextInterestDate FROM (%2))
)str").arg(sqlNextAnnualSettlement_firstAS, sqlNextAnnualSettlement_nextAS)};

const QString sqlContractsByYearByInterest {qsl(R"str(
SELECT SUBSTR(Vertraege.Vertragsdatum, 0, 5) as Year
  ,Vertraege.ZSatz /100. AS Zinssatz
  ,count(*) AS Anzahl
  ,sum(Vertraege.Betrag) /100. AS Summe
FROM Vertraege
GROUP BY Year, Zinssatz
)str")};

const QString sqlNbrAllCreditors {qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM Vertraege)
)str")};

const QString sqlNbrAllCreditors_thesa{qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM Vertraege
   WHERE thesaurierend = 1)
)str")};

const QString sqlNbrAllCreditors_payout{qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
   (SELECT DISTINCT KreditorId
    FROM Vertraege
    WHERE NOT thesaurierend)
)str")};

const QString sqlNbrActiveCreditors {qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1))
)str").arg(sqlContractsActiveView)};

const QString sqlNbrActiveCreditors_thesa{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE thesa)
)str").arg(sqlContractsActiveView)};

const QString sqlNbrActiveCreditors_payout{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE NOT thesa)
)str").arg(sqlContractsActiveView)};

const QString sqlInactiveCreditors{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1))
)str").arg(sqlContractsInactiveView)};

const QString sqlInactiveCreditors_thesa{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE thesa)
)str").arg(sqlContractsInactiveView)};

const QString sqlInactiveCreditors_payout{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE NOT thesa)
)str").arg(sqlContractsInactiveView)};

const QString sqlInterestByYearOverview {qsl(
R"str(
SELECT
  STRFTIME('%Y', Datum) as Year,
  SUM(Buchungen.Betrag) /100. as Summe,
  'Jahreszins' as BA,
  CASE WHEN Vertraege.thesaurierend = 0 THEN 'ausbezahlt'
   WHEN Vertraege.thesaurierend = 1 THEN 'angerechnet'
   WHEN Vertraege.thesaurierend = 2 THEN 'einbehalten'
  END  as Thesa
FROM Buchungen INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id
WHERE Buchungen.BuchungsArt = 8
GROUP BY Year, Buchungen.BuchungsArt, thesaurierend

UNION

SELECT
  STRFTIME('%Y', Datum) as Year,
  SUM(Buchungen.Betrag) /100. as Summe,
  'Jahreszins' as BA,
  '(gesamt)' AS Thesa
FROM Buchungen INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id
WHERE Buchungen.BuchungsArt = 8
GROUP BY Year, Buchungen.BuchungsArt

UNION

SELECT
  STRFTIME('%Y', Datum) as Year,
  SUM(Buchungen.Betrag) /100. as Summe,
  'unterjähriger Zins' as BA,
  '(gesamt)' as Thesa
FROM Buchungen INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id
WHERE Buchungen.BuchungsArt = 4
GROUP BY Year

ORDER BY YEAR DESC, Thesa DESC
)str"
)};

const QString sqlStat_activeContracts_byIMode_toDate {qsl(
R"str(
WITH BookingsFromOpenContracts AS (
  SELECT Vertraege.id        AS vid
  , Vertraege.KreditorId     AS kid
  , Vertraege.zSatz          AS Zinssatz
  , Vertraege.thesaurierend  AS imode
  , Buchungen.Datum          AS BDatum
  , Buchungen.Betrag         AS Betrag
  , CASE WHEN Vertraege.thesaurierend = 2
         THEN CASE WHEN Buchungen.BuchungsArt = 8
                   THEN 0
              ELSE Buchungen.Betrag
         END
    ELSE 0
    END AS verzWert_for_fix_interest_contracts
  FROM Buchungen JOIN Vertraege ON Vertraege.id = Buchungen.VertragsId
  WHERE BDatum <= date(':date')
)
, BookingsExContracts AS (
  SELECT exVertraege.id        AS vid
  , exVertraege.KreditorId     AS kid
  , exVertraege.zSatz          AS Zinssatz
  , exVertraege.thesaurierend  AS imode
  , exBuchungen.Datum          AS BDatum
  , exBuchungen.Betrag         AS Betrag
  , CASE WHEN exVertraege.thesaurierend = 2
         THEN CASE WHEN exBuchungen.BuchungsArt = 8
                   THEN 0
                   ELSE exBuchungen.Betrag
              END
         ELSE 0
    END AS verzWert_for_fix_interest_contracts
  FROM exBuchungen JOIN exVertraege ON exVertraege.id = exBuchungen.VertragsId
  WHERE BDatum <= date(':date')
)
, bUndExB AS (
  SELECT vid
       , kid
       , Zinssatz
       , imode
       , SUM(BookingsFromOpenContracts.Betrag)          AS gesamtBetrag
       , SUM(BookingsFromOpenContracts.Betrag*BookingsFromOpenContracts.Zinssatz)/100./100./100. AS ZinsAusGesamtBetrag
       , SUM(BookingsFromOpenContracts.verzWert_for_fix_interest_contracts)        AS BetragOhneJahresZins
       , SUM(BookingsFromOpenContracts.verzWert_for_fix_interest_contracts*BookingsFromOpenContracts.Zinssatz)/100./100./100. AS ZinsOhneJahreszins
  FROM BookingsFromOpenContracts
  GROUP BY vid

  UNION

  SELECT vid
       , kid
       , Zinssatz
       , imode
       , SUM(BookingsExContracts.Betrag)          AS gesamtBetrag
       , SUM(BookingsExContracts.Betrag*BookingsExContracts.Zinssatz)/100./100./100. AS ZinsAusGesamtBetrag
       , SUM(BookingsExContracts.verzWert_for_fix_interest_contracts)        AS BetragOhneJahresZins
       , SUM(BookingsExContracts.verzWert_for_fix_interest_contracts*BookingsExContracts.Zinssatz)/100./100./100. AS ZinsOhneJahreszins
  FROM BookingsExContracts
  GROUP BY vid
)
, valueTableByIMode AS (
SELEcT imode
  , COUNT(*) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
  , CASE WHEN imode = 2
         THEN SUM(BetragOhneJahresZins) /100.
         ELSE SUM(gesamtbetrag) /100.
    END AS VerzinslGuthabenFIX
  , CASE WHEN imode = 2
         THEN SUM(ZinsOhneJahreszins)
         ELSE SUM(ZinsAusGesamtBetrag)
    END AS JahresZins

FROM bUndExB
WHERE gesamtBetrag > 0
GROUP BY imode
)
, valueTableAll AS (
SELEcT imode
  , COUNT(*) AS AnzahlVertraege
  , COUNT(DISTINCT kid) AS AnzahlKreditoren
  , CASE WHEN imode = 2
         THEN SUM(BetragOhneJahresZins) /100.
         ELSE SUM(gesamtbetrag) /100.
    END AS VerzinslGuthabenFIX
  , CASE WHEN imode = 2
         THEN SUM(ZinsOhneJahreszins)
         ELSE SUM(ZinsAusGesamtBetrag)
    END AS JahresZins

FROM bUndExB
WHERE gesamtBetrag > 0
)

SELEcT imode
  , AnzahlVertraege
  , AnzahlKreditoren
  , VerzinslGuthabenFIX  AS totalVolume
  , ROUND(JahresZins, 2) AS  totalInterest
  , ROUND(JahresZins/ VerzinslGuthabenFIX *100., 2) AS avgInterest
FROM valueTableByIMode

UNION

SELEcT 'all' as imode
  , AnzahlVertraege
  , AnzahlKreditoren
  , VerzinslGuthabenFIX  AS totalVolume
  , ROUND(JahresZins, 2) AS  totalInterest
  , ROUND(JahresZins/ VerzinslGuthabenFIX *100., 2) AS avgInterest
FROM valueTableAll
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

const QString sqlStat_allContracts_byIMode_toDate {};

// {qsl(R"str()str")};

QString sql_precalc {
qsl("SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel FROM ("
   "SELECT "
      "count(*) as Anzahl, "
      "SUM(Wert) as Wert, "
      "SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,"
      "ROUND(AVG(Zinssatz),4) as mittlereRate "
   "FROM (%1) %2)")};

const QString sqlStat_allerVertraege         {sql_precalc.arg(sqlContractsAllView, qsl(""))};
const QString sqlStat_allerVertraege_thesa   {sql_precalc.arg(sqlContractsAllView, qsl("WHERE thesa"))};
const QString sqlStat_allerVertraege_ausz    {sql_precalc.arg(sqlContractsAllView, qsl("WHERE NOT thesa"))};
const QString sqlStat_aktiverVertraege       {sql_precalc.arg(sqlContractsActiveView, qsl(""))};
const QString sqlStat_aktiverVertraege_thesa {sql_precalc.arg(sqlContractsActiveView, qsl("WHERE thesa"))};
const QString sqlStat_aktiverVertraege_ausz  {sql_precalc.arg(sqlContractsActiveView, qsl("WHERE NOT thesa"))};
const QString sqlStat_passiverVertraege      {sql_precalc.arg(sqlContractsInactiveView, qsl(""))};
const QString sqlStat_passiverVertraege_thesa{sql_precalc.arg(sqlContractsInactiveView, qsl("WHERE thesa"))};
const QString sqlStat_passiverVertraege_ausz {sql_precalc.arg(sqlContractsInactiveView, qsl("WHERE NOT thesa"))};


