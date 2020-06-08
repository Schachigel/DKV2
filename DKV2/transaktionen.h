#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "contract.h"

void activateContract(qlonglong vid);
void changeContractValue(qlonglong vid);

void beendeVertrag(qlonglong vid);
void terminateContract_Final( contract& v);
void cancelContract_wNoticePeriod( contract& v);
void terminatePassiveContract( contract& v);

#endif // TRANSAKTIONEN_H
