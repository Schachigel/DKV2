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
  ,V.Vertragsdatum     AS Vertragsdatum

  ,IFNULL(Aktivierungsdatum, '(offen)') AS Aktivierungsdatum
  ,IFNULL(AktivierungsWert, V.Betrag /100.) AS Nominalwert

  ,IIF(IFNULL(AnlagenId, 0) == 0
    , CAST(V.Zsatz / 100. AS VARCHAR) || ' % (ohne Anlage)'
    , CAST(V.Zsatz / 100. AS VARCHAR) || ' % - ' || GA.Typ) AS Zinssatz

-- Zinsmodus
  , V.thesaurierend AS Zinsmodus

-- VerzinslGuthaben
  ,IIF(V.thesaurierend == 0, IFNULL(summeAllerBuchungen, ' (inaktiv) ')
       , IIF(V.thesaurierend == 1, IFNULL(summeAllerBuchungen, ' (inaktiv) ')
       , IIF(V.thesaurierend == 2, IFNULL(summeEinUndAuszahlungen, ' (inaktiv) ')
       , IIF(V.thesaurierend == 3, ' ohne Verzinsung ', 'ERROR')))) AS VerzinslGuthaben

-- angesparter Zins
  ,IIF(V.thesaurierend == 0, IFNULL(summeAllerZinsZwBuchungen, '(noch 0) ')
       ,IIF(V.thesaurierend == 1, IFNULL(summeAllerZinsBuchungen, '(noch 0) ')
       ,IIF(V.thesaurierend == 2, IFNULL(summeAllerZinsBuchungen, '(noch 0) ')
       ,IIF(V.thesaurierend == 3, '(zinslos)   ', 'ERROR')))) AS angespZins

-- letzte Buchungen
  ,ifnull(LetzteBuchung, ' - ') AS LetzteBuchung

-- kdFrist/VEnd
  ,IIF( V.Kfrist == -1, strftime('%d.%m.%Y', V.LaufzeitEnde),
      IIF( date(V.LaufzeitEnde) > date('5000-12-31'), '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)',
          '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)' || char(10) || strftime('%d.%m.%Y', V.LaufzeitEnde))) AS KdgFristVertragsende

FROM Vertraege AS V
INNER JOIN Kreditoren AS K ON V.KreditorId = K.id
LEFT JOIN Geldanlagen AS GA ON V.AnlagenId = GA.rowid

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
SELECT ga.ZSatz
  , ga.Anfang
  , ga.Ende
  , ga.Typ
  , (SELECT count(*)
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid
  ) AS Anzahl
  , (SELECT SUM(Betrag) /100.
     FROM Vertraege
     WHERE Vertraege.AnlagenId == ga.rowid
  ) AS Summe
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

QMap<QString, QString> views ={
    // model of table view: contracts
    {vnContractView, sqlContractView},
    // model of table view: deleted contracts
    {vnExContractView, sqlExContractView},
    // model of table view: investments
    {vnInvestmentsView, sqlInvestmentsView},
    // convenientce view
    {vnBookingsOverview, sqlBookingsOverview},
};
const QMap<QString, QString>& getViews() {
    return views;
}
QString getView(const QString &vn) {
    return views.value(vn);
}
bool remove_all_views(const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{   LOG_CALL;
    QVector<QSqlRecord> views;
    if( executeSql(qsl("SELECT name FROM sqlite_master WHERE type = ?"), QVariant(qsl("view")), views, db)) {
        for( const auto& rec : qAsConst(views)) {
            if( executeSql_wNoRecords(qsl("DROP view %1").arg(rec.value(0).toString()), db))
                continue;
            else
                return false;
        }
        return true;
    }
    return false;
}

//////////////////////////////////////
// SQL Statemends (stored here to not clutter the source code with long constant strings)
//////////////////////////////////////

// interest calculation
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

// Listenausdruck createCsvActiveContracts
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

// Übersichten
// Übersicht: contractRuntimeDistribution, used in sqlContractsAllView
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
  SELEcT SUM(Betrag) /100.            AS GesamtVolumen
    , SUM(Betrag) /100. /COUNT(*)     AS MittlererVertragswert
    , SUM(VerzinslGuthaben *Zinssatz) /100. /100.
                                      AS Jahreszins
    , SUM(VerzinslGuthaben *Zinssatz) /100. /100. / SUM(Betrag) *100.
                                      AS ZinsRate
    FROM active_bookings
)
, inactive_sums AS (
  SELEcT SUM(Betrag) /100.             AS GesamtVolumen
  , SUM(Betrag) /100. /COUNT(*)        AS MittlererVertragswert
  , SUM(Betrag*ZSatz) /100. /100. /100.  AS JahresZins
  , SUM(Betrag*ZSatz) /100. /SUM(Betrag) AS ZinsRate

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
