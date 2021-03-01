#ifndef INVESTMENT_H
#define INVESTMENT_H

#include "dbtable.h"

class investment
{
public:
    investment();
    static const dbtable& getTableDef();
};

bool createInvestmentIfApplicable(const int ZSatz, const QDate& vDate);
bool deleteInvestment(const int ZSatz, const QDate& v, const QDate& b, const QString t);
bool saveNewInvestment(int ZSatz, QDate start, QDate end, QString type);

//bool deleteInvestment(const int ZSatz, const QString& v, const QString& b, const QString& t);


#endif // INVESTMENT_H
