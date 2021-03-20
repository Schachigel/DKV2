#include <QString>
#include <QSqlQuery>

#include "helper.h"
#include "appconfig.h"
#include "helpersql.h"
#include "helperfin.h"
#include "tabledatainserter.h"
#include "investment.h"

investment::investment()
{

}

const QString fnInvestmentInterest{qsl("ZSatz")};
const QString fnInvestmentStart{qsl("Anfang")};
const QString fnInvestmentEnd{qsl("Ende")};
const QString fnInvestmentTyp{qsl("Typ")};
const QString fnInvestmentState{qsl("Offen")};

/*static*/ const dbtable& investment::getTableDef()
{
    static dbtable investmentTable(qsl("Geldanlagen"));
    if( 0 == investmentTable.Fields().size()){
        investmentTable.append(dbfield(fnInvestmentInterest, QVariant::Int).setNotNull());
        investmentTable.append(dbfield(fnInvestmentStart, QVariant::Date).setNotNull());
        investmentTable.append(dbfield(fnInvestmentEnd, QVariant::Date).setNotNull());
        investmentTable.append(dbfield(fnInvestmentTyp, QVariant::String).setNotNull());
        investmentTable.append(dbfield(fnInvestmentState, QVariant::Bool).setNotNull());
        QVector<dbfield> unique;
        unique.append(investmentTable[fnInvestmentInterest]);
        unique.append(investmentTable[fnInvestmentStart]);
        unique.append(investmentTable[fnInvestmentEnd]);
        unique.append(investmentTable[fnInvestmentTyp]);
        investmentTable.setUnique(unique);
    }
    return investmentTable;
}

bool saveNewInvestment(int ZSatz, QDate start, QDate end, QString type)
{   LOG_CALL;
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, start);
    tdi.setValue(fnInvestmentEnd, end);
    tdi.setValue(fnInvestmentTyp, type);
    tdi.setValue(fnInvestmentState, 1);
    return tdi.InsertData();
}

bool createInvestmentFromContractIfNeeded(const int ZSatz, const QDate& vDate)
{   LOG_CALL;
    QString sql{qsl("SELECT * FROM Geldanlagen WHERE ZSatz =%1 AND Anfang <= date('%2') AND Ende > date('%3')")};
    if( 0 < rowCount(sql.arg(QString::number(ZSatz), vDate.toString(Qt::ISODate), vDate.toString(Qt::ISODate)))) {
        return false;
    }
    QDate endDate =vDate.addYears(1);
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(fnInvestmentInterest, ZSatz);
    tdi.setValue(fnInvestmentStart, vDate);
    tdi.setValue(fnInvestmentEnd, endDate);
    QString type { QString::number(ZSatz/100.) +qsl(" % - 100.000 Euro pa")};
    tdi.setValue(fnInvestmentTyp, type);
    tdi.setValue(fnInvestmentState, true);
    return tdi.InsertData();
}
bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t)
{   LOG_CALL;
    QString sql{qsl("DELETE FROM Geldanlagen WHERE ZSatz=%1 AND Anfang='%2' AND Ende='%3' AND Typ='%4'")};
    sql =sql.arg(QString::number(ZSatz),v, b, t);
    return executeSql_wNoRecords(sql);
}
bool deleteInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString& t)
{
    return deleteInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t);
}


bool closeInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t)
{   LOG_CALL;
    QString sql{qsl("UPDATE  Geldanlagen  SET Offen = 0 WHERE ZSatz=%1 AND Anfang='%2' AND Ende='%3' AND Typ='%4'")};
    sql =sql.arg(QString::number(ZSatz),v, b, t);
    return executeSql_wNoRecords(sql);
}
bool closeInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString& t)
{
    return closeInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t);
}

int nbrActiveInvestments(const QDate& cDate/*=EndOfTheFuckingWorld*/)
{   LOG_CALL;
    QString field {qsl("count(*)")};
    QString tname {investment::getTableDef().Name()};
    QString where;
    if(cDate == EndOfTheFuckingWorld)
        where ="Offen";
    else {
        where =qsl("Offen AND Anfang <= date('%1') AND Ende > date('%1')").arg(cDate.toString(Qt::ISODate));
    }
    return executeSingleValueSql(field, tname, where).toInt();
}

QVector<QPair<qlonglong, QString>> activeInvestments(const QDate& cDate)
{   LOG_CALL;
    QVector<QPair<qlonglong, QString>> investments;
    QString where;
    if(cDate == EndOfTheFuckingWorld)
        where ="Offen";
    else {
        where =qsl("Offen AND Anfang <= date('%1') AND Ende > date('%1')").arg(cDate.toString(Qt::ISODate));
    }
    QSqlQuery q;
    if( not q.prepare(qsl("SELECT rowid, Typ FROM Geldanlagen WHERE %1 ORDER BY %2").arg(where, fnInvestmentInterest))) {
        qCritical() << "sql prep failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QVector<QPair<qlonglong, QString>>();
    }

    if( not q.exec()) {
        qCritical() << "sql exec failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QVector<QPair<qlonglong, QString>>();
    }
    while( q.next()) {
        QSqlRecord rec =q.record();
        investments.push_back({rec.value(qsl("rowid")).toLongLong(), rec.value(fnInvestmentTyp).toString()});
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
QString investmentInfoForContract(qlonglong rowId, double amount)
{   LOG_CALL;
    int maxNbr =dbConfig::readValue(MAX_INVESTMENT_NBR).toInt();
    double maxSum =dbConfig::readValue(MAX_INVESTMENT_SUM).toDouble();

    QString sql {qsl("SELECT * FROM vInvestmentsOverview WHERE rowid=") +QString::number(rowId)};
    QSqlQuery q (sql); if( not q.next()) Q_ASSERT(true);
    QSqlRecord r =q.record();
    qDebug() << q.lastQuery() << Qt::endl << r;
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
