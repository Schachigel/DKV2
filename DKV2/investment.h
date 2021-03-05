#ifndef INVESTMENT_H
#define INVESTMENT_H
#include <QDate>
#include "dbtable.h"

class investment
{
public:
    investment();
    static const dbtable& getTableDef();
};

bool createInvestmentFromContractIfNeeded(const int ZSatz, const QDate& vDate);
bool deleteInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString t);
bool saveNewInvestment(int ZSatz, QDate start, QDate end, QString type);
int nbrActiveInvestments(const QDate& contractDate=EndOfTheFuckingWorld);
QVector<QPair<qlonglong, QString>> activeInvestments(const QDate& contractDate=EndOfTheFuckingWorld);
int interestOfInvestmentByRowId(qlonglong rid);
QString investmentInfoForContract(qlonglong rowId, double amount);
//bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t);


#endif // INVESTMENT_H
