#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "contract.h"

void activateContract(qlonglong vid);
void changeContractValue(qlonglong vid);

void deleteInactiveContract(qlonglong vid);
void terminateContract(qlonglong cid);
void terminateContract_Final( contract& v);
void cancelContract( contract& v);

void annualSettlement();

void newCreditorAndContract();

#endif // TRANSAKTIONEN_H
