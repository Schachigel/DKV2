#ifndef TRANSAKTIONEN_H
#define TRANSAKTIONEN_H

#include "pch.h"
#include "contract.h"

void newCreditorAndContract();

void editCreditor(qlonglong creditorId);
void changeContractComment(contract* c);
void changeContractTermination(contract* c);

void bookInitialPaymentReceived(contract* v);
void activateInterest(contract* v);
void changeContractValue(contract* v);
void annualSettlement();
void annualSettlementLetters();
void finalizeContractLetter(contract *c);

void deleteInactiveContract(contract *c);
void terminateContract(contract* c);
void terminateContract_Final( contract& v);
void cancelContract( contract& v);

qlonglong createInvestment_matchingContract(int& interest/*inout*/, QDate& from /*inout*/, QDate& to /*out*/);
void createInvestment();

#endif // TRANSAKTIONEN_H
