#ifndef ANNUALSETTLEMENT_H
#define ANNUALSETTLEMENT_H

#include "contract.h"

QDate dateOfnextSettlement();
// for all(?) contracts
int executeCompleteAS(year y);
QString formulate_AS_as_CSV(year y);

//
// void writeAnnualSettlementCsv(int year);
//
#endif // ANNUALSETTLEMENT_H
