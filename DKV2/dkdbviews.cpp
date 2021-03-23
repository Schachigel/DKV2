#include "helper.h"
#include "dkdbviews.h"

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
  ,CASE WHEN V.thesaurierend = 0 THEN 'Auszahlend'
       ELSE CASE WHEN V.thesaurierend = 1 THEN 'Thesaur.'
       ELSE CASE WHEN V.thesaurierend = 2 THEN 'Fester Zins'
       ELSE 'ERROR' END END END       AS Zinsmodus

-- VerzinslGuthaben
  ,CASE WHEN V.thesaurierend = 0 THEN ifnull(summeAllerBuchungen, '-')
       ELSE CASE WHEN V.thesaurierend = 1 THEN ifnull(summeAllerBuchungen, '-')
       ELSE CASE WHEN V.thesaurierend = 2 THEN ifnull(summeEinUndAuszahlungen, '-')
       ELSE 'ERROR' END END END    AS VerzinslGuthaben

-- angesparter Zins
  ,CASE WHEN V.thesaurierend = 0 THEN ifnull(summeAllerZinsZwBuchungen, '-')
       ELSE CASE WHEN V.thesaurierend = 1 THEN ifnull(summeAllerZinsBuchungen, '-')
       ELSE CASE WHEN V.thesaurierend = 2 THEN ifnull(summeAllerZinsBuchungen, '-')
       ELSE 'ERROR' END END END    AS angespZins

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

const QString vnContractsActiveDetailsView {qsl("vVertraege_aktiv_detail")};
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

const QString vnContractsActiveView {qsl("vVertraege_aktiv")};
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

const QString vnContractsInactiveView {qsl("vVertraege_passiv")};
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

const QString vnContractsAllView {qsl("vVertraege_alle")};
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

const QString vnNextAnnualSettlement_firstAS {qsl("vNextAnnualS_first")};
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

const QString vnNextAnnualSettlement_nextAS {qsl("vNextAnnualS_next")};
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

const QString vnNextAnnualSettlement {qsl("vNextAnnualSettlement")};
const QString sqlNextAnnualSettlement {qsl(
R"str(
SELECT MIN(nextInterestDate) AS date
FROM
  (SELECT nextInterestDate FROM (%1)
     UNION
  SELECT nextInterestDate FROM (%2))
)str").arg(sqlNextAnnualSettlement_firstAS, sqlNextAnnualSettlement_nextAS)};

const QString vnContractsByYearByInterest {qsl("vContractsByYearByInterest")};
const QString sqlContractsByYearByInterest {qsl(R"str(
SELECT SUBSTR(Vertraege.Vertragsdatum, 0, 5) as Year
  ,Vertraege.ZSatz /100. AS Zinssatz
  ,count(*) AS Anzahl
  ,sum(Vertraege.Betrag) /100. AS Summe
FROM Vertraege
GROUP BY Year, Zinssatz
)str")};

const QString vnNbrAllCreditors {qsl("vAnzahl_allerKreditoren")};
const QString sqlNbrAllCreditors {qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM Vertraege)
)str")};

const QString vnNbrAllCreditors_thesa {qsl("vAnzahl_allerKreditoren_thesa")};
const QString sqlNbrAllCreditors_thesa{qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM Vertraege
   WHERE thesaurierend)
)str")};

const QString vnNbrAllCreditors_payout {qsl("vAnzahl_allerKreditoren_ausz")};
const QString sqlNbrAllCreditors_payout{qsl(R"str(
SELECT COUNT(*) AS Anzahl
FROM
   (SELECT DISTINCT KreditorId
    FROM Vertraege
    WHERE NOT thesaurierend)
)str")};

const QString vnNbrActiveCreditors {qsl("vAnzahl_aktiverKreditoren")};
const QString sqlNbrActiveCreditors {qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1))
)str").arg(sqlContractsActiveView)};

const QString vnNbrActiveCreditors_thesa {qsl("vAnzahl_aktiverKreditoren_thesa")};
const QString sqlNbrActiveCreditors_thesa{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE thesa)
)str").arg(sqlContractsActiveView)};

const QString vnNbrActiveCreditors_payout {qsl("vAnzahl_aktiverKreditoren_ausz")};
const QString sqlNbrActiveCreditors_payout{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE NOT thesa)
)str").arg(sqlContractsActiveView)};

const QString vnInactiveCreditors {qsl("vAnzahl_passiverKreditoren")};
const QString sqlInactiveCreditors{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1))
)str").arg(sqlContractsInactiveView)};

const QString vnInactiveCreditors_thesa {qsl("vAnzahl_passiverKreditoren_thesa")};
const QString sqlInactiveCreditors_thesa{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE thesa)
)str").arg(sqlContractsInactiveView)};

const QString vnInactiveCreditors_payout {qsl("vAnzahl_passiverKreditoren_ausz")};
const QString sqlInactiveCreditors_payout{qsl(R"str(
SELECT count(*) AS Anzahl
FROM
  (SELECT DISTINCT KreditorId
   FROM (%1) WHERE NOT thesa)
)str").arg(sqlContractsInactiveView)};

const QString vnInterestByYearOverview {qsl("vStat_InterestByYear")};
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

const QString vnBookingsOverview {qsl("vBuchungen")};
const QString sqlBookingsOverview {qsl(
R"str(
SELECT
  B.Datum,
  V.id,
  V.Kennung,
  CASE V.thesaurierend
    WHEN 0 THEN 'Ausz.'
    WHEN 1 THEN 'Thesa.'
    WHEN 2 THEN 'Fix'
    ELSE 'ERROR'
  END AS Zinsmodus,
  B.Betrag,
  CASE B.BuchungsArt
    WHEN 1 THEN 'Einzahlung'
    WHEN 2 THEN 'Auszahlung'
    WHEN 4 THEN 'unterj.Zins'
    WHEN 8 THEN 'Jahreszins'
    ELSE 'ERROR'
  END AS BuchungsArt
FROM Vertraege AS V
  LEFT JOIN Buchungen AS B ON V.id = B.VertragsId
ORDER BY V.id, B.Datum
)str"
)};

const QString vnStat_allerVertraege {qsl("vStat_allerVertraege")};
const QString vnStat_allerVertraege_thesa {qsl("vnStat_allerVertraege_thesa")};
const QString vnStat_allerVertraege_ausz {qsl("vStat_allerVertraege_ausz")};
const QString vnStat_aktiverVertraege {qsl("vStat_aktiverVertraege")};
const QString vnStat_aktiverVertraege_thesa {qsl("vStat_aktiverVertraege_thesa")};
const QString vnStat_aktiverVertraege_ausz {qsl("vStat_aktiverVertraege_ausz")};
const QString vnStat_passiverVertraege {qsl("vStat_passiverVertraege")};
const QString vnStat_passiverVertraege_thesa {qsl("vStat_passiverVertraege_thesa")};
const QString vnStat_passiverVertraege_ausz {qsl("vStat_passiverVertraege_ausz")};


// {qsl(R"str()str")};

QMap<QString, QString> sqls = {
    // reporthtml.cpp: Übersichten: Ausgabe aller Vertragsdaten
    // menü: Verträge -> Liste drucken
    // base of sqlContractsActiveView
    {vnContractsActiveDetailsView, sqlContractsActiveDetailsView},

    // vVertraege_aktiv is the base of  vStat_aktiverVertraege, ..._thesa, ..._ausz
    {vnContractsActiveView, sqlContractsActiveView},
    // vVertraege_passiv is the base of vStat_passiverVertraege, ..._thesa, ..._ausz
    {vnContractsInactiveView, sqlContractsInactiveView},
    // vVertraege_alle is the base of  vStat_allerVertraege, ..._thesa, ..._ausz
    {vnContractsAllView, sqlContractsAllView},

    // statistics: cound creditors etc.
    {vnContractsByYearByInterest, sqlContractsByYearByInterest},
    {vnNbrAllCreditors, sqlNbrAllCreditors},
    {vnNbrAllCreditors_thesa, sqlNbrAllCreditors_thesa},
    {vnNbrAllCreditors_payout, sqlNbrAllCreditors_payout},
    {vnNbrActiveCreditors, sqlNbrActiveCreditors},
    {vnNbrActiveCreditors_thesa, sqlNbrActiveCreditors_thesa},
    {vnNbrActiveCreditors_payout, sqlNbrActiveCreditors_payout},
    {vnInactiveCreditors, sqlInactiveCreditors},
    {vnInactiveCreditors_thesa, sqlInactiveCreditors_thesa},
    {vnInactiveCreditors_payout,  sqlInactiveCreditors_payout},
    {vnInterestByYearOverview, sqlInterestByYearOverview},
    // calculation of the next annual statement
//    {vnNextAnnualSettlement_firstAS,sqlNextAnnualSettlement_firstAS},
//    {vnNextAnnualSettlement_nextAS, sqlNextAnnualSettlement_nextAS},
    {vnNextAnnualSettlement, sqlNextAnnualSettlement}
};

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
QMap<QString, QString>& getViews() {
    return views;
}

QMap<QString, QString>& getSqls() {
    static bool init =false;
    if( not init) {
        QString sql_precalc {
            qsl("SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel FROM ("
               "SELECT "
                  "count(*) as Anzahl, "
                  "SUM(Wert) as Wert, "
                  "SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,"
                  "ROUND(AVG(Zinssatz),4) as mittlereRate "
               "FROM (%1) %2)")};
        sqls.insert(vnStat_allerVertraege,         sql_precalc.arg(sqls[vnContractsAllView], qsl("")));
        sqls.insert(vnStat_allerVertraege_thesa,   sql_precalc.arg(sqls[vnContractsAllView], qsl("WHERE thesa")));
        sqls.insert(vnStat_allerVertraege_ausz,    sql_precalc.arg(sqls[vnContractsAllView], qsl("WHERE NOT thesa")));
        sqls.insert(vnStat_aktiverVertraege,       sql_precalc.arg(sqls[vnContractsActiveView], qsl("")));
        sqls.insert(vnStat_aktiverVertraege_thesa, sql_precalc.arg(sqls[vnContractsActiveView], qsl("WHERE thesa")));
        sqls.insert(vnStat_aktiverVertraege_ausz,  sql_precalc.arg(sqls[vnContractsActiveView], qsl("WHERE NOT thesa")));
        sqls.insert(vnStat_passiverVertraege,      sql_precalc.arg(sqls[vnContractsInactiveView], qsl("")));
        sqls.insert(vnStat_passiverVertraege_thesa,sql_precalc.arg(sqls[vnContractsInactiveView], qsl("WHERE thesa")));
        sqls.insert(vnStat_passiverVertraege_ausz, sql_precalc.arg(sqls[vnContractsInactiveView], qsl("WHERE NOT thesa")));
        init =true;
    }
    return sqls;
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

