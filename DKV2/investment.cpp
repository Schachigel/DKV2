#include <QString>
#include <QSqlQuery>

#include "helper.h"
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
bool deleteInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString t)
{
    return deleteInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t);
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

QMap<qlonglong, QString> activeInvestments(const QDate& cDate)
{   LOG_CALL;
    QMap<qlonglong, QString> investments;
    QString where;
    if(cDate == EndOfTheFuckingWorld)
        where ="Offen";
    else {
        where =qsl("Offen AND Anfang <= date('%1') AND Ende > date('%1')").arg(cDate.toString(Qt::ISODate));
    }
    QSqlQuery q;
    q.prepare(qsl("SELECT rowid, Typ FROM Geldanlagen WHERE %1").arg(where));

    if( ! q.exec()) {
        qCritical() << "sql exec failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QMap<qlonglong, QString>();
    }
    while( q.next()) {
        QSqlRecord rec =q.record();
        investments[rec.value(qsl("rowid")).toLongLong()] =rec.value(fnInvestmentTyp).toString();
    }
    return investments;
}

int interestOfInvestmentByRowId(qlonglong rid)
{   LOG_CALL;
    const dbfield& dbf =investment::getTableDef()[fnInvestmentInterest];
    QString where =qsl("rowid=")+QString::number(rid);
    return executeSingleValueSql(dbf,  where).toInt();
}
QString investmentInfoForContract(qlonglong rowId, double amount, QDate cDate)
{   LOG_CALL;
    QLocale locale;
    QString s_amount =locale.toCurrencyString(amount);
    QString s_cDate  =cDate.toString("dd.MM.yyyy");

    QString sql {qsl("SELECT * FROM vInvestmentsOverview WHERE rowid=") +QString::number(rowId)};
    QSqlQuery q (sql); if( ! q.next()) Q_ASSERT(true);
    QSqlRecord r =q.record();
    qDebug() << q.lastQuery();
    double zSatz =r.value(qsl("ga.ZSatz")).toInt() /100. ;
    QString s_zSatz  = QString::number(zSatz, 'g', 2);
    QString from =r.value(qsl("ga.Anfang")).toDate().toString(qsl("dd.MM.yyyy"));
    QString bis  =r.value(qsl("ga.Ende")).toDate().toString(qsl("dd.MM.yyyy"));
    QString anzahl =QString::number(r.value(qsl("Anzahl")).toInt());
    QString summe =locale.toCurrencyString(euroFromCt(r.value(qsl("Summe")).toInt()));
    QString anzahlActive =QString::number(r.value(qsl("AnzahlAktive")).toInt());
    QString summeActive =locale.toCurrencyString(euroFromCt(r.value(qsl("SummeAktive")).toInt()));
    QString html1 =qsl("<tr><td>Zinssatz</td><td colspan=2>%1</td></tr>"
                   "<tr><td>Von - Bis<td><td>%2</td><td>%3</td></tr>"
                   "<tr><td>Aktive Vertr. </td><td>%4</td><td>%5</td></tr>"
                   "<tr><td>Alle Vertr.</td><td>%6</td><td>%7</td></tr>").arg(s_zSatz, from, bis, anzahl, summe, anzahlActive, summeActive);



    QString html2 ={qsl("<tr><td colspan=3>Nach der Buchung</td></tr>"
                    "<tr><td>Aktive Vertr. </td><td>%1</td><td>%2</td></tr>"
                    "<tr><td>Alle Vertr.</td><td>%3</td><td>%4</td></tr>")};
    QString ret =qsl("<table>%1 %2</table>").arg(html1, html2);
    qInfo() << ret;
    return ret;
}
