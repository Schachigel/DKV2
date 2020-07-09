#ifndef REPORTHTML_H
#define REPORTHTML_H

#include <QString>

enum Uebersichten
{
    OVERVIEW = 0,
    BY_CONTRACT_END,
    INTEREST_DISTRIBUTION,
    CONTRACT_TERMS,
    ALL_CONTRACT_INFO
};

QString reportHtml(Uebersichten u);

#endif // REPORTHTML_H
