#include <QString>
#include <QSqlQuery>

#include "helper.h"
#include "helpersql.h"
#include "tabledatainserter.h"
#include "investment.h"

investment::investment()
{

}

/*static*/ const dbtable& investment::getTableDef()
{
    static dbtable investmentTable(qsl("Geldanlagen"));
    if( 0 == investmentTable.Fields().size()){
        investmentTable.append(dbfield(qsl("ZSatz"), QVariant::Int).setNotNull());
        investmentTable.append(dbfield(qsl("Anfang"), QVariant::Date).setNotNull());
        investmentTable.append(dbfield(qsl("Ende"), QVariant::Date).setNotNull());
        investmentTable.append(dbfield(qsl("Typ"), QVariant::String).setNotNull());
        investmentTable.append(dbfield(qsl("Offen"), QVariant::Bool).setNotNull());
        QVector<dbfield> unique;
        unique.append(investmentTable[qsl("ZSatz")]);
        unique.append(investmentTable[qsl("Anfang")]);
        unique.append(investmentTable[qsl("Ende")]);
        unique.append(investmentTable[qsl("Typ")]);
        investmentTable.setUnique(unique);
    }
    return investmentTable;
}

bool saveNewInvestment(int ZSatz, QDate start, QDate end, QString type) {
    QVector<QVariant> values;
    values.push_back(QVariant(ZSatz));
    values.push_back(QVariant(start));
    values.push_back(QVariant(end));
    values.push_back(QVariant(type));
    values.push_back(QVariant(true));

    return executeSql_wNoRecords(qsl("INSERT INTO Geldanlagen VALUE (?, ?, ?, ?, ?) "), values);
}

bool createInvestmentIfApplicable(const int ZSatz, const QDate& vDate)
{   LOG_CALL;
    QString sql{qsl("SELECT * FROM Geldanlagen WHERE ZSatz =%1 AND Anfang <= '%2' AND Ende > '%3'")};
    if( 0 < rowCount(sql.arg(QString::number(ZSatz), vDate.toString(Qt::ISODate), vDate.toString(Qt::ISODate)))) {
        return false;
    }
    QDate endDate =vDate.addYears(1);
    TableDataInserter tdi(investment::getTableDef());
    tdi.setValue(qsl("ZSatz"), ZSatz);
    tdi.setValue(qsl("Anfang"), vDate);
    tdi.setValue(qsl("Ende"), endDate);
    QString type { QString::number(ZSatz/100.) +qsl(" % - 100.000 Euro pa")};
    tdi.setValue(qsl("Typ"), type);
    tdi.setValue(qsl("Offen"), true);
    return tdi.InsertData();
}
bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t)
{
    QString sql{qsl("DELETE FROM Geldanlagen WHERE ZSatz=%1 AND Anfang='%2' AND Ende='%3' AND Typ='%4'")};
    sql =sql.arg(QString::number(ZSatz),v, b, t);
    return executeSql_wNoRecords(sql);
}
bool deleteInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString t)
{
    return deleteInvestment(ZSatz, v.toString(Qt::ISODate), b.toString(Qt::ISODate), t);
}
