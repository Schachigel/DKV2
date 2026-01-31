#ifndef ANNUALSETTLEMENT_H
#define ANNUALSETTLEMENT_H
#include "contract.h"


QDate dateOfnextSettlement();
// for all(?) contracts
int executeCompleteAS(year y);
//int executeAnnualSettlement(int year, QString &csv);

//
// void writeAnnualSettlementCsv(int year);
//
#endif // ANNUALSETTLEMENT_H
