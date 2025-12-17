#include "helper.h"
#include "helperfin.h"
#include "helpersql.h"
#include "appconfig.h"
#include "tabledatainserter.h"
#include "investment.h"

const QString fnInvestmentInterest{qsl("ZSatz")};
const QString fnInvestmentStart{qsl("Anfang")};
const QString fnInvestmentEnd{qsl("Ende")};
const QString fnInvestmentType{qsl("Typ")};
const QString fnInvestmentState{qsl("Offen")};

investment::investment(qlonglong id /*=-1*/, int Interest /*=0*/,
                       const QDate Start /*=EndOfTheFuckingWorld*/, const QDate End /*=EndOfTheFuckingWorld*/,
                       const QString& Type /*=qsl("")*/, const bool State)
    : rowid(id), interest(Interest), start (Start), end (End), type(Type), state(State)
{
    if( isValidRowId (id)) {
        QSqlRecord rec =executeSingleRecordSql (getTableDef().Fields (), qsl("rowid=%1").arg(i2s(id)));
        if( rec.isEmpty ()) {
            return;
        }
        interest =rec.field(fnInvestmentInterest).value().toInt();
        type     =rec.field(fnInvestmentType).value().toString ();
        start    =rec.field(fnInvestmentStart).value().toDate ();
        end      =rec.field(fnInvestmentEnd).value().toDate();
        state    =rec.field(fnInvestmentState).value().toBool ();
    }
}

QString investment::toString() const
{
    if( end == EndOfTheFuckingWorld)
        return dbInterest2_str(interest) + qsl(" (") + start.toString() + qsl(" - ohne Enddatum) : ") + type;
    else
        return dbInterest2_str(interest) + qsl(" (") + start.toString() + qsl(" - ") + end.toString() + ") " + type;
}

/*static*/ const dbtable& investment::getTableDef()
{
    static dbtable investmentTable(qsl("Geldanlagen"));
    if( 0 == investmentTable.Fields().size()){
        investmentTable.append(dbfield(qsl("rowid"), QMetaType::LongLong).setAutoInc());
        investmentTable.append(dbfield(fnInvestmentInterest, QMetaType::Int).setNotNull());
        investmentTable.append(dbfield(fnInvestmentStart,    QMetaType::QDate).setNotNull());
        investmentTable.append(dbfield(fnInvestmentEnd,      QMetaType::QDate).setNotNull());
        investmentTable.append(dbfield(fnInvestmentType,     QMetaType::QString).setNotNull());
        investmentTable.append(dbfield(fnInvestmentState,    QMetaType::Bool).setNotNull());
        QVector<dbfield> unique;
        unique.append(investmentTable[fnInvestmentInterest]);
        unique.append(investmentTable[fnInvestmentStart]);
        unique.append(investmentTable[fnInvestmentEnd]);
        unique.append(investmentTable[fnInvestmentType]);
        investmentTable.setUnique(unique);
    }
    return investmentTable;
}

bool investment::matchesContract(const contract& c)
{   LOG_CALL;
    QString msg;
    if( c.dbInterest() not_eq interest)
        msg +=qsl("Interest missmatch: %1 vs. %2").arg( dbInterest2_str(interest), dbInterest2_str(c.dbInterest ()));
    if( c.conclusionDate() < start)
        msg +=qsl("\nstart date missmatch;");
    if( c.conclusionDate() > end)
        msg += qsl("\nend date missmatch:");
    if( msg.isEmpty())
        return true;
    qInfo().noquote () << "investment  -contract mismatch: \n" << msg;
    qInfo().noquote () << toString();
    qInfo().noquote () << c.toString(qsl("Mismatching contract:"));
    return false;
}

investment::invStatisticData investment::getStatisticData(const QDate newContractDate)
{ LOG_CALL_W(newContractDate.toString ());
    invStatisticData ret;
    QString pStart = isContinouse ()
            ? newContractDate.addYears (-1).toString(Qt::ISODate) : start.toString(Qt::ISODate);
    QString pEnd   = isContinouse ()
            ? newContractDate.toString(Qt::ISODate) : end.toString(Qt::ISODate);
    QString id =i2s(rowid);

    // anzahlAlleVertraege, summeAlleVertraege
    // (! Betrachtung der Vertragswerte (ohne nachträgliche Ein- oder Auszahlungen)
    QString sqlContractDataAll =qsl(R"str(
SELECT COUNT(*), SUM(Betrag) FROM Vertraege
WHERE AnlagenId =%0 AND Vertraege.Vertragsdatum > '%1' AND Vertraege.Vertragsdatum <= '%2'
)str");
    QSqlRecord recContractDataAll =executeSingleRecordSql (
                sqlContractDataAll.arg(id, pStart, pEnd));
    ret.anzahlVertraege =recContractDataAll.field (0).value ().toInt ();
    ret.summeVertraege  =euroFromCt(recContractDataAll.field (1).value ().toInt());

    // einAuszahlungen beinhaltet
    // - Vertragswerte abgeschlossener Verträge ohne Einzahlung
    // - zzgl. Ein- bzw. Auszahlungen
    QString sqlContractsWithNoPayment =qsl(R"str(
SELECT SUM(Betrag) FROM Vertraege
WHERE Vertraege.id NOT IN (SELECT DISTINCT VertragsId FROM Buchungen)
  AND AnlagenId =%0
  AND Vertraege.Vertragsdatum >  '%1'
  AND Vertraege.Vertragsdatum <= '%2'
)str");
    double valuePassiveContracts =euroFromCt(executeSingleValueSql (sqlContractsWithNoPayment
                                                         .arg(id, pStart, pEnd)).toInt ());

    QString sqlPayments =qsl(R"str(
SELECT SUM(Buchungen.Betrag) FROM Buchungen
WHERE
  Buchungen.VertragsId IN (SELECT DISTINCT id FROM Vertraege WHERE AnlagenId =%0)
  AND Buchungen.BuchungsArt = 1 OR Buchungen.BuchungsArt = 2
  AND Buchungen.Datum >  '%1'
  AND Buchungen.Datum <= '%2'
)str");
    double valuePaymentsActiveContracts =euroFromCt(executeSingleValueSql (sqlPayments
                                                                .arg(id, pStart, pEnd)).toInt ());
    ret.EinAuszahlungen =valuePassiveContracts +valuePaymentsActiveContracts;

    QString sqlAllBookings =qsl(R"str(
SELECT SUM(Buchungen.Betrag) FROM Buchungen
WHERE
  Buchungen.VertragsId IN (SELECT DISTINCT id FROM Vertraege WHERE AnlagenId =%0)
  AND Buchungen.Datum >  '%1'
  AND Buchungen.Datum <= '%2'
)str");
    double allBookingsInclInterest =euroFromCt(executeSingleValueSql ( sqlAllBookings
                                           .arg(i2s(rowid), pStart, pEnd)).toInt ());
    ret.ZzglZins =valuePassiveContracts + allBookingsInclInterest;

    return ret;
}


///////////////////////////////////////////////////////
// related functions
///////////////////////////////////////////////////////
tableindex_t saveNewInvestment(int ZSatz, QDate start, QDate end, const QString &type)
{   LOG_CALL;
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, start);
    tdi.setValue(fnInvestmentEnd, end);
    tdi.setValue(fnInvestmentType, type);
    tdi.setValue(fnInvestmentState, 1);
    return tdi.InsertRecord();
}

tableindex_t createInvestmentFromContractIfNeeded(const int ZSatz, QDate vDate)
{   LOG_CALL;

    QString sql{qsl("SELECT * FROM Geldanlagen WHERE ZSatz =%1 AND Anfang <= date('%2') AND Ende >= date('%3')")};
    QDate endDate =vDate.addYears(1);
    if( vDate == BeginingOfTime) {
        sql =qsl("SELECT * FROM Geldanlagen WHERE ZSatz =%1 AND Ende == date('9999-12-31')");
        endDate =EndOfTheFuckingWorld;
    }
    if( 0 < rowCount(sql.arg(i2s(ZSatz), vDate.toString(Qt::ISODate), vDate.toString(Qt::ISODate)))) {
        return SQLITE_invalidRowId;
    }
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, vDate);
    tdi.setValue(fnInvestmentEnd, endDate);
    QString type { qsl("100.tEuro pa /max. 20 (%1)").arg(prozent2prozent_str(dbInterest2Interest(ZSatz)))};

    tdi.setValue(fnInvestmentType, type);
    tdi.setValue(fnInvestmentState, true);
    return tdi.InsertRecord();
}

bool deleteInvestment(const qlonglong rowid)
{   LOG_CALL;
    QString sql{qsl("DELETE FROM Geldanlagen WHERE rowid=%1").arg(i2s(rowid))};
    return executeSql_wNoRecords (sql);
}

bool setInvestment(const qlonglong rowid, bool state)
{   LOG_CALL;
    QString sql{qsl("UPDATE  Geldanlagen  SET Offen = %1 WHERE rowid == %2")};

    sql =sql.arg(state ? qsl("true") : qsl("false"), i2s(rowid));
    return executeSql_wNoRecords(sql);
}
bool closeInvestment(const qlonglong rowid)
{
    return setInvestment(rowid, false);
}
bool openInvestment(const qlonglong rowid)
{
    return setInvestment(rowid, true);
}

int nbrActiveInvestments(const QDate cDate/*=EndOfTheFuckingWorld*/)
{   LOG_CALL;
    QString field {qsl("count(*)")};
    QString tname {investment::getTableDef().Name()};
    QString where;
    if(cDate == EndOfTheFuckingWorld)
        where =qsl("Offen");
    else {
        where =qsl("Offen AND Anfang <= date('%1') AND Ende >= date('%1')").arg(cDate.toString(Qt::ISODate));
    }
    return executeSingleValueSql(field, tname, where).toInt();
}

QVector<QPair<qlonglong, QString>> activeInvestments(const QDate cDate)
{   LOG_CALL_W(cDate.toString(qsl("yyyy.MM.dd")));
// fill combo box to select investment / interest for new contract
    QVector<QPair<qlonglong, QString>> investments;
    QString where;
    if(cDate == EndOfTheFuckingWorld)
        where =qsl("Offen");
    else {
        where =qsl("Offen AND Anfang <= date('%1') AND Ende >= date('%1')").arg(cDate.toString(Qt::ISODate));
    }

    QString sql {(qsl("SELECT rowid, ZSatz, Typ FROM Geldanlagen WHERE %1 ORDER BY %2").arg(where, fnInvestmentInterest))};
    QVector<QSqlRecord> result;
    if( not executeSql(sql, result)) {
        qInfo() << "no investments";
        return QVector<QPair<qlonglong, QString>>();
    }
    for(const auto& rec : std::as_const(result)) {
        QString comboboxentry {qsl("%1 (%2)").arg(rec.value(fnInvestmentType).toString(), prozent2prozent_str (rec.value(qsl("ZSatz")).toInt ()/100.))};
        investments.push_back({rec.value(qsl("rowid")).toLongLong(), comboboxentry});
    }
    return investments;
}

int interestOfInvestmentByRowId(qlonglong rid)
{   LOG_CALL;
    const dbfield& dbf =investment::getTableDef()[fnInvestmentInterest];
    QString where =qsl("rowid=")+i2s(rid);
    return executeSingleValueSql(dbf,  where).toInt();
}

QString redOrBlack(int i, int max)
{
    if( i >= max) return qsl("<div style=\"color:red\">") +i2s(i) +qsl("</div>");
    else return i2s(i);
}
QString redOrBlack(double d, double max)
{
    if( d >= max) return qsl("<div style=\"color:red\">") +s_d2euro(d) +qsl("</div>");
    else return s_d2euro(d);
}

QVector<QString> formatedStatisticData(investment::invStatisticData data, double Vertragswert)
{
    int maxNbr =dbConfig::readValue(MAX_INVESTMENT_NBR).toInt();
    double maxSum =dbConfig::readValue(MAX_INVESTMENT_SUM).toDouble();

    QVector<QString> result;
    result.push_back (redOrBlack(data.anzahlVertraege, maxNbr));
    result.push_back (redOrBlack(data.summeVertraege, maxSum));
    result.push_back (redOrBlack(data.EinAuszahlungen, maxSum));
    result.push_back (redOrBlack(data.ZzglZins, maxSum));

    result.push_back (redOrBlack(data.anzahlVertraege +1, maxNbr));
    result.push_back (redOrBlack(data.summeVertraege +Vertragswert, maxSum));

    result.push_back (redOrBlack(data.EinAuszahlungen +Vertragswert, maxSum));
    result.push_back (redOrBlack(data.ZzglZins +Vertragswert, maxSum));
    return result;
}
QString investmentInfoForNewContract(qlonglong ridInvestment, const double amount, const QDate newContractDate)
{   LOG_CALL;
    /* create the html to display statistical info about the investment during investment selection
     * while creating a new contract
     */
    investment invest(ridInvestment);
    QDate start = invest.isContinouse () ? newContractDate.addYears (-1) : invest.start;
    QDate end   = invest.isContinouse () ? newContractDate : invest.end;

    QString timeSpan;
    if( invest.isContinouse ())
        timeSpan =qsl("fortlaufend (Jahresfrist: %1 bis %2)").arg(start.toString(qsl("dd.MM.yyyy")), end.toString (qsl("dd.MM.yyyy")));
    else
        timeSpan =qsl("von %1 bis %2").arg(start.toString (qsl("dd.MM.yyyy")), end.toString (qsl("dd.MM.yyyy")));

    QString idLine1 {qsl("<tr><td colspan=2><b>Anlagedaten<b></tr>")};
    QString idLine2 {qsl("<tr><td>Zinssatz</td><td>%1</td></tr>").arg(dbInterest2_str (invest.interest))};
    QString idLine3 {qsl("<tr><td>Laufzeit</td><td>%1</td></tr>").arg(timeSpan)};
    QString tableInvestmentData {qsl("<table width=100%> \n %1 \n %2 \n %3 \n</table><p>").arg(idLine1, idLine2, idLine3)};

    QVector<QString> numbers =formatedStatisticData (invest.getStatisticData (newContractDate), amount);
    // investment details / statistics
    QString headers  {qsl("<tr> <td style='text-align: center'><b>Anzahl</b></td><td style='text-align: right'><b>Summe d.<br> Verträge</b></td><td style='text-align: right'><b>... incl. Ein- u.<br>Ausz.</b></td><td style='text-align: right'><b>...incl. Zinsen</b></td></tr>")};
    QString headerLine {qsl("<tr> <td colspan=4>Vor diesem Vertrag</td> </tr>")};
    QString s1Line     {qsl("<tr> <td style='text-align: center'>%0</td> <td style='text-align: right'>%1</td> <td style='text-align: right'>%2</td> <td style='text-align: right'>%3</td> </tr>").arg(numbers[0],numbers[1],numbers[2],numbers[3])};
    QString headerLine2{qsl("<tr> <td colspan=4>Nach diesem Vertrag</td> </tr>")};
    QString s2Line     {qsl("<tr> <td style='text-align: center'>%0</td> <td style='text-align: right'>%1</td> <td style='text-align: right'>%2</td> <td style='text-align: right'>%3</td> </tr>").arg(numbers[4],numbers[5],numbers[6],numbers[7])};

    QString tableStatistics {qsl("<table width=100%> \n %1 \n %2 \n %3 \n %4 \n %5 </table>").arg(
                    headers, headerLine, s1Line, headerLine2, s2Line)};
    return tableInvestmentData + tableStatistics;
}

QVector<investment> openInvestments(int rate, QDate conclusionDate)
{   LOG_CALL;

    QString sql{qsl("SELeCT * FROM Geldanlagen WHERE Offen AND ZSatz = %1 AND Anfang <= date('%2')  AND Ende >= date('%2')")};
    QVector<QSqlRecord> records;
    if( not executeSql(sql.arg(i2s(rate), conclusionDate.toString(Qt::ISODate)), records))
        return QVector<investment>();

    QVector<investment> result;
    for (const auto &record : std::as_const(records)) {
        result.push_back(investment(record.value(qsl("rowid")).toLongLong(),
                                    record.value(fnInvestmentInterest).toInt(),
                                    record.value(fnInvestmentStart).toDate(),
                                    record.value(fnInvestmentEnd).toDate(),
                                    record.value(fnInvestmentType).toString(),
                                    record.value(fnInvestmentState).toBool()));
    }
    return result;
}

int closeInvestmentsPriorTo(QDate d)
{   LOG_CALL_W(d.toString());
    QString sql{qsl("UPDATE Geldanlagen SET Offen = false WHERE Offen AND Ende < date('%1')")};
    QSqlQuery q;
    if( not q.exec(sql.arg(d.toString(Qt::ISODate))))
    RETURN_ERR( -1, qsl("failed to update investments "), q.lastError ().text (), qsl("\n"), q.lastQuery ());
    return q.numRowsAffected();
}
