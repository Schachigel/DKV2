#ifndef INVESTMENT_H
#define INVESTMENT_H
#include <QDate>
#include "dbtable.h"

class investment
{
public:
    investment(qlonglong id =-1, int interest =0, QDate start =EndOfTheFuckingWorld, QDate end =EndOfTheFuckingWorld, const QString& type =qsl(""));
    static const dbtable& getTableDef();
    qlonglong rowid;
    int interest;
    QDate start;
    QDate end;
    QString type;
    QString toString() const;
};

bool createInvestmentFromContractIfNeeded(const int ZSatz, QDate vDate);
bool deleteInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
bool saveNewInvestment(int ZSatz, QDate start, QDate end, const QString &type);
bool closeInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);
bool openInvestment(const int ZSatz, const QDate v, const QDate b, const QString& t);

int nbrActiveInvestments(const QDate contractDate=EndOfTheFuckingWorld);
QVector<QPair<qlonglong, QString>> activeInvestments(const QDate contractDate=EndOfTheFuckingWorld);
int interestOfInvestmentByRowId(qlonglong rid);
QString investmentInfoForNewContract(qlonglong rowId, double amount);
//bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t);
QVector<investment> investments(int rate, QDate concluseionDate);


#endif // INVESTMENT_H
