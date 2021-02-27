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

#endif // INVESTMENT_H
