#ifndef INVESTMENT_H
#define INVESTMENT_H

#include "dbtable.h"

class investment
{
public:
    investment();
    static const dbtable& getTableDef();
};

#endif // INVESTMENT_H
