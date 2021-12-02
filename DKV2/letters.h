#ifndef LETTERS_H
#define LETTERS_H

#include "lettersnippets.h"


enum class letterType {
    all =snippet::allLetters,
    annPayoutL        =int(interestModel::payout)   +1,
    annReinvestL      =int(interestModel::reinvest) +1,
    annInterestInfoL  =int(interestModel::fixed)    +1,
    annInfoL          =int(interestModel::zero)     +1,
    maxValue
};
int fromLetterType( letterType lt);

letterType letterTypeFromInt(int lt);

class letters
{
public:
    letters();

};

/// init for new db
int writeDefaultSnippets(QSqlDatabase db =QSqlDatabase::database ());


#endif // LETTERS_H
