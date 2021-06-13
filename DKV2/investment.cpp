#include <QString>

#include "helper.h"
#include "appconfig.h"
#include "helpersql.h"
#include "helperfin.h"
#include "tabledatainserter.h"
#include "investment.h"

investment::investment(qlonglong id /*=-1*/, int interest /*=0*/,
                       QDate start /*=EndOfTheFuckingWorld*/, QDate end /*=EndOfTheFuckingWorld*/,
                       const QString& type /*=qsl("")*/, bool state)
    : rowid(id), interest(interest), start (start), end (end), type(type), state(state)
{

}

QString investment::toString() const
{
    return d2percent_str(interest) + qsl(" (") + start.toString() + qsl(" - ") + end.toString() + ") " + type;
}

const QString fnInvestmentInterest{qsl("ZSatz")};
const QString fnInvestmentStart{qsl("Anfang")};
const QString fnInvestmentEnd{qsl("Ende")};
const QString fnInvestmentType{qsl("Typ")};
const QString fnInvestmentState{qsl("Offen")};

/*static*/ const dbtable& investment::getTableDef()
{
    static dbtable investmentTable(qsl("Geldanlagen"));
    if( 0 == investmentTable.Fields().size()){
        investmentTable.append(dbfield(qsl("rowid"), QVariant::Int).setPrimaryKey().setAutoInc());
        investmentTable.append(dbfield(fnInvestmentInterest, QVariant::Int).setNotNull());
        investmentTable.append(dbfield(fnInvestmentStart,    QVariant::Date).setNotNull());
        investmentTable.append(dbfield(fnInvestmentEnd,      QVariant::Date).setNotNull());
        investmentTable.append(dbfield(fnInvestmentType,      QVariant::String).setNotNull());
        investmentTable.append(dbfield(fnInvestmentState,    QVariant::Bool).setNotNull());
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
    if( d2percent(c.interestRate()) not_eq interest)
        msg +=qsl("Interest missmatch:") +QString::number(interest) +qsl("vs.") +QString::number(d2percent(c.investment()));
    if( c.conclusionDate() < start)
        msg +=qsl("start date missmatch;");
    if( c.conclusionDate() > end)
        msg += qsl("end date missmatch:");
    if( msg.isEmpty())
        return true;
    qInfo() << "investment  -contract mismatch: \n" << msg;
    qInfo() << toString();
    qInfo() << c.toString();
    return false;
}

qlonglong saveNewInvestment(int ZSatz, QDate start, QDate end, const QString &type)
{   LOG_CALL;
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, start);
    tdi.setValue(fnInvestmentEnd, end);
    tdi.setValue(fnInvestmentType, type);
    tdi.setValue(fnInvestmentState, 1);
    return tdi.InsertData();
}

qlonglong createInvestmentFromContractIfNeeded(const int ZSatz, QDate vDate)
{   LOG_CALL;
    QString sql{qsl("SELECT * FROM Geldanlagen WHERE ZSatz =%1 AND Anfang <= date('%2') AND Ende >= date('%3')")};
    if( 0 < rowCount(sql.arg(QString::number(ZSatz), vDate.toString(Qt::ISODate), vDate.toString(Qt::ISODate)))) {
        return -1;
    }
    QDate endDate =vDate.addYears(1).addDays(-1);
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, vDate);
    tdi.setValue(fnInvestmentEnd, endDate);
    QString type { qsl("100.tEuro pa /max 20")};
    tdi.setValue(fnInvestmentType, type);
    tdi.setValue(fnInvestmentState, true);
    return tdi.InsertData();
}

bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t)
{   LOG_CALL;
    QString sql{qsl("DELETE FROM Geldanlagen WHERE ZSatz=%1 AND Anfang='%2' AND Ende='%3' AND Typ='%4'")};
    sql =sql.arg(QString::number(ZSatz),v, b, t);
    return executeSql_wNoRecords(sql);
}
bool deleteInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t)
{
    return deleteInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t);
}

bool setInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t, bool state)
{   LOG_CALL;
    QString sql{qsl("UPDATE  Geldanlagen  SET Offen = %1 WHERE ZSatz=%2 AND Anfang='%3' AND Ende='%4' AND Typ='%5'")};

    sql =sql.arg(state?qsl("true"):qsl("false"),QString::number(ZSatz),v, b, t);
    return executeSql_wNoRecords(sql);
}
bool closeInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t)
{
    return setInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t, false);
}
bool openInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t)
{
    return setInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t, true);
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

    QString sql {(qsl("SELECT rowid, Typ FROM Geldanlagen WHERE %1 ORDER BY %2").arg(where, fnInvestmentInterest))};
    QVector<QSqlRecord> result;
    if( not executeSql(sql, QVariant(), result)) {
            return QVector<QPair<qlonglong, QString>>();
    }
    for(const auto& rec : qAsConst(result)) {
        investments.push_back({rec.value(qsl("rowid")).toLongLong(), rec.value(fnInvestmentType).toString()});
    }
    return investments;
}

int interestOfInvestmentByRowId(qlonglong rid)
{   LOG_CALL;
    const dbfield& dbf =investment::getTableDef()[fnInvestmentInterest];
    QString where =qsl("rowid=")+QString::number(rid);
    return executeSingleValueSql(dbf,  where).toInt();
}

QString redOrBlack(int i, int max)
{
    if( i >= max) return qsl("<div style=\"color:red\">") +QString::number(i) +qsl("</div>");
    else return QString::number(i);
}
QString redOrBlack(double d, double max)
{
    QLocale l;
    if( d >= max) return qsl("<div style=\"color:red\">") +l.toCurrencyString(d) +qsl("</div>");
    else return l.toCurrencyString(d);
}
QString investmentInfoForNewContract(qlonglong ridInvestment, double amount)
{   LOG_CALL;

    int maxNbr =dbConfig::readValue(MAX_INVESTMENT_NBR).toInt();
    double maxSum =dbConfig::readValue(MAX_INVESTMENT_SUM).toDouble();

    QString sql {qsl("SELECT * FROM vInvestmentsOverview WHERE rowid=") +QString::number(ridInvestment)};
    QSqlRecord r =executeSingleRecordSql(sql);
    QString s_zSatz  = QString::number(r.value(qsl("ZSatz")).toInt()/100., 'g', 2) +qsl(" %");
    QString from =r.value(qsl("Anfang")).toDate().toString(qsl("dd.MM.yyyy"));
    QString bis  =r.value(qsl("Ende")).toDate().toString(qsl("dd.MM.yyyy"));

    QString anzahl =redOrBlack(r.value(qsl("Anzahl")).toInt(), maxNbr);
    QString summe =redOrBlack(r.value(qsl("Summe")).toDouble(), maxSum);
    QString anzahlActive =redOrBlack(r.value(qsl("AnzahlAktive")).toInt(), maxNbr);
    QString summeActive =redOrBlack(r.value(qsl("SummeAktive")).toDouble(), maxSum);

    QString html1 =qsl("<tr><td colspan=3 align=left><b>Anlagedaten</b></td></tr>"
                   "<tr><td align=left>Zinssatz</td><td align=left colspan=2>%1</td></tr>"
                   "<tr><td align=left>Laufzeit </td><td align=left colspan=2> von %2 bis %3 </td></tr>"
                   "<tr><td colspan=3></td></tr>"
                   "<tr><td align=left><b>Bisherige Verträge</b></td><td align=right>Anzahl</td><td align=right>Summe</td></tr>"
                   "<tr><td align=left>Aktive Verträge </td><td  align=right>%4</td><td align=right>%5</td></tr>"
                   "<tr><td align=left>Alle Verträge</td><td align=right>%6</td><td align=right>%7</td></tr>").arg(s_zSatz, from, bis, anzahl, summe, anzahlActive, summeActive);


    QString anzahlNew =redOrBlack(r.value(qsl("Anzahl")).toInt() +1, maxNbr);
    QString summeNew =redOrBlack(r.value(qsl("Summe")).toDouble() +amount, maxSum);

    QString html2 ={qsl("<tr><td colspan=3><b>Nach der Buchung<b></td></tr>"
                    "<tr><td align=left>Alle Verträge   </td><td align=right> %1</td><td align=right> %2</td></tr>").arg(anzahlNew, summeNew)};
    QString ret =qsl("<table width=100% style=\"border-width:0px\">%1 %2</table>").arg(html1, html2);
    qDebug() << ret;
    return ret;
}

QVector<investment> openInvestments(int rate, QDate conclusionDate)
{   LOG_CALL;

    QString sql{qsl("SELeCT * FROM Geldanlagen WHERE Offen AND ZSatz = %1 AND Anfang <= date('%2')  AND Ende >= date('%2')")};
    QVector<QSqlRecord> records;
    if( not executeSql(sql.arg(QString::number(rate), conclusionDate.toString(Qt::ISODate)), QVector<QVariant>(), records))
        return QVector<investment>();

    QVector<investment> result;
    for (const auto &record : qAsConst(records)) {
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
    if( not q.exec(sql.arg(d.toString(Qt::ISODate)))) {
        qDebug() << "failed to update investments " << q.lastError() << "\n" << q.lastQuery();
        return -1;
    }
    return q.numRowsAffected();
}
